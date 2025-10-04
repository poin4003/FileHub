#include "core/router.h"
#include "core/response.h"
#include "utils/hash.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ===============================
 * Constants & Config
 * =============================== */
#define MAX_MIDDLEWARE_CHAIN 10
#define METHOD_MAP_SIZE 8
#define MAX_PARAMS 16

/* ===============================
 * Struct Definations
 * =============================== */
typedef struct handler_chain_s
{
    handler_func handlers[MAX_MIDDLEWARE_CHAIN];
    int len;
} handler_chain_t;

/* Trie node for routing */
typedef struct trie_node_s
{
    char *segment;
    int is_param;

    struct trie_node_t *children;
    struct trie_node_t *next_sibling;

    handler_chain_t *chains[METHOD_COUNT];
} trie_node_t;

/* Result of finding a node + captured params */
typedef struct find_result_s
{
    trie_node_t *node;
    route_param_t params[MAX_PARAMS];
    int num_params;
} find_result_t;

/* Method map entry */
typedef struct
{
    const char *key;
    http_method_t val;
    uint32_t hash;
} method_entry_t;

/* ===============================
 * Globals
 * =============================== */
static trie_node_t *router_root = NULL;
static method_entry_t method_map[METHOD_MAP_SIZE];
static int method_map_inited = 0;

/** Insert a method into hash map */
static void method_map_insert(const char *key, http_method_t val)
{
    uint32_t h = djb2_hash(key);
    uint32_t idx = h % METHOD_MAP_SIZE;
    for (uint32_t i = 0; i < METHOD_MAP_SIZE; i++)
    {
        uint32_t pos = (idx + i) % METHOD_MAP_SIZE;
        if (method_map[pos].key == NULL)
        {
            method_map[pos].key = key;
            method_map[pos].val = val;
            method_map[pos].hash = h;
            return;
        }
    }
}

/** Init method lookup table */
void init_method_map(void)
{
    if (method_map_inited)
        return;

    for (int i = 0; i < METHOD_MAP_SIZE; i++)
    {
        method_map[i].key = NULL;
        method_map[i].hash = 0;
    }

    method_map_insert("GET", HTTP_GET);
    method_map_insert("POST", HTTP_POST);
    method_map_insert("PUT", HTTP_PUT);
    method_map_insert("PATCH", HTTP_PATCH);
    method_map_insert("DELETE", HTTP_DELETE);
    method_map_insert("OPTIONS", HTTP_OPTIONS);
    method_map_inited = 1;
}

/** Parse HTTP method string to enum */
http_method_t parse_method(const char *method)
{
    if (!method)
        return HTTP_METHOD_UNKNOWN;

    if (!method_map_inited)
        init_method_map();

    uint32_t h = djb2_hash(method);
    uint32_t idx = h % METHOD_MAP_SIZE;
    for (uint32_t i = 0; i < METHOD_MAP_SIZE; i++)
    {
        uint32_t pos = (idx + i) % METHOD_MAP_SIZE;
        if (method_map[pos].key == NULL)
            return HTTP_METHOD_UNKNOWN;

        if (method_map[pos].hash == h && strcmp(method_map[pos].key, method) == 0)
            return method_map[pos].val;
    }

    return HTTP_METHOD_UNKNOWN;
}

/** Create a trie node */
static trie_node_t *create_note(const char *segment, size_t len)
{
    trie_node_t *node = calloc(1, sizeof(trie_node_t));
    if (!node)
        return NULL;

    node->segment = strndup(segment, len);
    if (len > 0 && segment[0] == ':')
    {
        node->is_param = 1;
    }

    return node;
}

/** Recursively free trie */
void free_trie(trie_node_t *node)
{
    if (!node)
        return;

    free(node->segment);
    for (int i = 0; i < METHOD_COUNT; i++)
        free(node->chains[i]);

    free_trie(node->children);
    free_trie(node->next_sibling);
    free(node);
}

/* ===============================
 * Router Lifecycle
 * =============================== */

/** Init router */
void router_init(void)
{
    init_method_map();
    router_root = create_note("/", 1);
}

/** Free router */
void router_free(void)
{
    free_trie(router_root);
    router_root = NULL;
}

/** Build chain from group middleware + route hanlders */
handler_chain_t *build_chain(handler_func group_mws[],
                             handler_func route_handlers[],
                             int group_len,
                             int route_len)
{
    handler_chain_t *chain = calloc(1, sizeof(handler_chain_t));
    if (!chain)
        return NULL;

    chain->len = 0;
    for (int i = 0; i < group_len; i++)
        chain->handlers[chain->len++] = group_mws[i];
    for (int i = 0; i < route_len; i++)
        chain->handlers[chain->len++] = route_handlers[i];

    return chain;
}

/** Add route with handlers */
void router_add_route(http_method_t method,
                      const char *path,
                      handler_func handlers[],
                      int handler_count)
{
    if (!router_root || handler_count <= 0 || handler_count > MAX_MIDDLEWARE_CHAIN)
        return;

    trie_node_t *curr = router_root;
    char *path_copy = NULL;
    handler_chain_t *new_chain = NULL;

    path_copy = strdup(path);
    if (!path_copy)
        goto cleanup;

    char *segment = strtok(path_copy, "/");
    while (segment != NULL)
    {
        trie_node_t *child = curr->children;
        trie_node_t *prev_sibling = NULL;

        while (child != NULL)
        {
            if (strcmp(child->segment, segment) == 0)
            {
                curr = child;
                goto next_segment;
            }
            prev_sibling = child;
            child = child->next_sibling;
        }

        trie_node_t *new_node = create_note(segment, strlen(segment));
        if (!new_node)
            goto cleanup;

        if (prev_sibling == NULL)
        {
            curr->children = new_node;
        }
        else
        {
            prev_sibling->next_sibling = new_node;
        }
        curr = new_node;

    next_segment:
        segment = strtok(NULL, "/");
    }

    new_chain = build_chain(NULL, handlers, 0, handler_count);
    if (!new_chain)
        goto cleanup;

    if (curr->chains[method])
    {
        free(curr->chains[method]);
        curr->chains[method] = NULL;
    }

    curr->chains[method] = new_chain;
    new_chain = NULL;

cleanup:
    if (path_copy)
        free(path_copy);
    if (new_chain)
        free(new_chain);
}

/* ===============================
 * Node finding
 * =============================== */

/** Find matching node and extract params */
find_result_t find_node_and_params(const char *url)
{
    find_result_t res = {0};
    trie_node_t *curr = router_root;
    char *url_copy = NULL;

    if (!router_root)
        return res;

    url_copy = strndup(url, strlen(url));
    if (!url_copy)
        return res;

    char *segment = strtok(url_copy, "/");
    while (segment != NULL && curr)
    {
        trie_node_t *child = curr->children;
        trie_node_t *param_node = NULL;

        while (child != NULL)
        {
            if (child->is_param)
                param_node = child;
            else if (strcmp(child->segment, segment) == 0)
            {
                curr = child;
                goto next_url_segment;
            }
            child = child->next_sibling;
        }

        if (param_node && res.num_params < MAX_PARAMS)
        {
            char *k = strdup(param_node->segment + 1);
            char *v = strdup(segment);
            if (!k || !v)
            {
                if (k) free(k);
                if (v) free(v);
                curr = NULL;
                goto cleanup;
            }

            res.params[res.num_params].key = k;
            res.params[res.num_params].value = v;
            res.num_params++;
            curr = param_node;
        }
        else
        {
            curr = NULL;
            goto cleanup;
        }

    next_url_segment:
        segment = strtok(NULL, "/");
    }

cleanup:
    if (!curr)
    {
        for (int i = 0; i < res.num_params; i++)
        {
            free(res.params[i].key);
            free(res.params[i].value);
        }
        res.num_params = 0;
        res.node = NULL;
    } else {
        res.node = curr;
    }

    if (url_copy)
        free(url_copy);

    return res;
}

/** Free find_result params */
void free_find_result(find_result_t *fr)
{
    for (int i = 0; i < fr->num_params; i++)
    {
        free(fr->params[i].key);
        free(fr->params[i].value);
    }
    fr->num_params = 0;
    fr->node = NULL;
}

/* ===============================
 * Dispatching
 * =============================== */

/** Run chain for a request */
int next_handler(request_t *req)
{
    if (req->current_handler_idx < req->chain_len)
    {
        handler_func current_handler = req->chain[req->current_handler_idx];
        req->current_handler_idx++;
        return current_handler(req);
    }
    return MHD_HTTP_OK;
}

/** Dispatch HTTP request */
int dispatch_request(struct MHD_Connection *c,
                     const char *url,
                     const char *method_str,
                     const char *upload_data,
                     size_t *upload_data_size)
{
    int result = MHD_NO;
    request_t req = {0};

    find_result_t fr = find_node_and_params(url);

    if (!fr.node)
    {
        free_find_result(&fr);
        return bad_request(c, 404, "Not found");
    }

    http_method_t method = parse_method(method_str);
    handler_chain_t *chain = fr.node->chains[method];
    if (!chain)
    {
        free_find_result(&fr);
        return bad_request(c, 404, "Not found");
    }

    for (int i = 0; i < fr.num_params; i++)
    {
        req.params[i] = fr.params[i];
        fr.params[i].key = NULL;
        fr.params[i].value = NULL;
    }
    req.num_params = fr.num_params;
    free_find_result(&fr);

    req.c = c,
    req.url = url,
    req.method_enum = method,
    req.upload_data = upload_data;
    req.upload_data_size = upload_data_size;
    req.chain = chain->handlers;
    req.chain_len = chain->len;

    result = next_handler(&req);

    for (int i = 0; i < req.num_params; i++)
    {
        free(req.params[i].key);
        free(req.params[i].value);
    }

    return result;
}