#include "core/router.h"
#include "core/response.h"
#include "utils/hash.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ===============================
 * Constants & Config
 * =============================== */
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

    struct trie_node_s *children;
    struct trie_node_s *next_sibling;

    handler_chain_t *chains[METHOD_COUNT];
} trie_node_t;

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
static void free_chain(handler_chain_t *chain);
static trie_node_t *create_node(const char *segment, size_t len);

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
static trie_node_t *create_node(const char *segment, size_t len)
{
    trie_node_t *node = calloc(1, sizeof(trie_node_t));
    if (!node)
        return NULL;

    node->segment = strndup(segment, len);
    if (!node->segment)
    {
        free(node);
        return NULL;
    }

    node->is_param = (len > 0 && segment[0] == ':');
    return node;
}

/** Recursively free trie */
void free_trie(trie_node_t *node)
{
    if (!node)
        return;

    free(node->segment);
    free_trie(node->children);
    free_trie(node->next_sibling);

    for (int i = 0; i < METHOD_COUNT; ++i)
        free_chain(node->chains[i]);

    free(node);
}

/* ===============================
 * Router Lifecycle
 * =============================== */

/** Init router */
void router_init(void)
{
    init_method_map();
    router_root = create_node("", 0);
}

/** Free router */
void router_free(void)
{
    free_trie(router_root);
    router_root = NULL;
}

/** Free chain */
static void free_chain(handler_chain_t *chain)
{
    if (chain)
        free(chain);
}

/** Build chain from group middleware + route hanlders */
handler_chain_t *build_chain(handler_func group_mws[],
                             handler_func route_handlers[],
                             int mws_count,
                             int route_handler_count)
{
    handler_chain_t *chain = calloc(1, sizeof(handler_chain_t));
    if (!chain)
        return NULL;

    for (int i = 0; i < mws_count && chain->len < MAX_MIDDLEWARE_CHAIN; ++i)
        chain->handlers[chain->len++] = group_mws[i];
    for (int i = 0; i < route_handler_count && chain->len < MAX_MIDDLEWARE_CHAIN; ++i)
        chain->handlers[chain->len++] = route_handlers[i];

    return chain;
}

/** Add route with handlers */
static void router_add_route(http_method_t method,
                             const char *path,
                             handler_func handlers[],
                             int handler_count)
{
    if (!router_root || !path || handler_count <= 0 || handler_count > MAX_MIDDLEWARE_CHAIN)
        return;

    trie_node_t *curr = router_root;
    char *path_copy = strdup(path);
    if (!path_copy)
        return;

    char *segment = strtok(path_copy, "/");
    while (segment != NULL)
    {
        trie_node_t *child = curr->children;
        trie_node_t *prev_sibling = NULL;
        while (child)
        {
            if (strcmp(child->segment, segment) == 0)
            {
                curr = child;
                goto next_segment;
            }
            prev_sibling = child;
            child = child->next_sibling;
        }

        trie_node_t *new_node = create_node(segment, strlen(segment));
        if (!new_node)
            goto cleanup;

        if (prev_sibling)
            prev_sibling->next_sibling = new_node;
        else
            curr->children = new_node;
        curr = new_node;

    next_segment:
        segment = strtok(NULL, "/");
    }

    handler_chain_t *new_chain = malloc(sizeof(handler_chain_t));
    if (!new_chain)
        goto cleanup;

    new_chain->len = handler_count;
    memcpy(new_chain->handlers, handlers, handler_count * sizeof(handler_func));

    if (curr->chains[method])
        free_chain(curr->chains[method]);

    curr->chains[method] = new_chain;
    new_chain = NULL;

cleanup:
    if (path_copy)
        free(path_copy);
    if (new_chain)
        free(new_chain);
}

router_group_t router_group(
    router_group_t *parent,
    const char *relative_path,
    handler_func mws[],
    int mw_count)
{
    router_group_t group = {0};
    if (parent)
    {
        snprintf(group.prefix, sizeof(group.prefix), "%s%s", parent->prefix, relative_path);
        if (parent->mw_count > 0)
        {
            memcpy(group.middlewares, parent->middlewares, parent->mw_count * sizeof(handler_func));
            group.mw_count = parent->mw_count;
        }
    }
    else
        snprintf(group.prefix, sizeof(group.prefix), "%s", relative_path);

    if (mw_count > 0)
    {
        memcpy(&group.middlewares[group.mw_count], mws, mw_count * sizeof(handler_func));
        group.mw_count += mw_count;
    }

    return group;
}

void router_group_add_route(
    router_group_t *group,
    http_method_t method,
    const char *path,
    handler_func handlers[],
    int handler_count)
{
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s", group->prefix, path);

    handler_func combined_chain[MAX_MIDDLEWARE_CHAIN];
    int total_len = 0;

    if (group->mw_count > 0)
    {
        memcpy(combined_chain, group->middlewares, group->mw_count * sizeof(handler_func));
        total_len += group->mw_count;
    }

    if (handler_count > 0)
    {
        memcpy(&combined_chain[total_len], handlers, handler_count * sizeof(handler_func));
        total_len += handler_count;
    }

    router_add_route(method, full_path, combined_chain, total_len);
}

/* ===============================
 * Node finding
 * =============================== */

/** Find matching node and extract params */
static trie_node_t *find_node_and_params(request_t *req)
{
    trie_node_t *curr = router_root;
    char *url_copy = strdup(req->url);
    if (!url_copy)
        return NULL;

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

        if (param_node) {
            if (req->num_params < MAX_ROUTE_PARAMS) {
                route_param_t *p = &req->params[req->num_params++];
                p->key = strdup(param_node->segment + 1);
                p->value = strdup(segment);

                if (!p->key || !p->value) {
                    free(p->key);
                    free(p->value);
                    p->key = NULL;
                    p->value = NULL;
                    req->num_params--;
                    curr = NULL;
                    goto cleanup;
                }
            }
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
    if (url_copy)
        free(url_copy);

    return curr;
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
    trie_node_t *node = NULL;

    req.c = c;
    req.url = url;
    req.method_enum = parse_method(method_str);
    req.upload_data = upload_data;
    req.upload_data_size = upload_data_size;

    node = find_node_and_params(&req);
    if (!node)
        goto cleanup;

    handler_chain_t *chain = node ? node->chains[req.method_enum] : NULL;
    if (!chain)
    {
        result = bad_request(c, 404, "Not found");
        goto cleanup;
    }

    req.chain_len = chain->len;
    req.chain = malloc(req.chain_len * sizeof(handler_func));
    if (!req.chain)
    {
        result = internal_server_error(c, 500, "Internal server error");
        goto cleanup;
    }

    memcpy(req.chain, chain->handlers, req.chain_len * sizeof(handler_func));
    result = next_handler(&req);

cleanup:
    free(req.chain);
    for (int i = 0; i < req.num_params; i++)
    {
        free(req.params[i].key);
        free(req.params[i].value);
    }

    return result;
}