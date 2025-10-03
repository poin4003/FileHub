#include "core/router.h"
#include "core/response.h"
#include "utils/hash.h"
#include <string.h>
#include <stdio.h>

#define MAX_MIDDLEWARE_CHAIN 10
#define METHOD_MAP_SIZE 8

typedef struct trie_note_t
{
    char *segment;
    int is_param;

    struct trie_node_t *children;
    struct trie_note_t *next_sibling;

    handler_func chains[METHOD_COUNT][MAX_MIDDLEWARE_CHAIN];
    int chain_lens[METHOD_COUNT];
} trie_node_t;

static trie_node_t *router_root = NULL;

typedef struct
{
    const char *key;
    http_method_t val;
    uint32_t hash;
} method_entry_t;

static method_entry_t method_map[METHOD_MAP_SIZE];
static int method_map_inited = 0;

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

        if (method_map[pos].key == h && strcmp(method_map[pos].key, method) == 0)
            return method_map[pos].val;
    }

    return HTTP_METHOD_UNKNOWN;
}

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

void router_init(void)
{
    init_method_map();
    router_root = create_note("/", 1);
}

void free_trie(trie_node_t *node)
{
    if (!node)
        return;

    free(node->segment);
    free_trie(node->children);
    free_trie(node->next_sibling);
    free(node);
}

void router_free(void)
{
    free_trie(router_root);
}

void router_add_route(http_method_t method,
                      const char *path,
                      handler_func handlers[],
                      int handler_count)
{
    if (!router_root || handler_count <= 0 || handler_count > MAX_MIDDLEWARE_CHAIN)
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
    free(path_copy);

    curr->chain_lens[method] = handler_count;
    for (int i = 0; i < handler_count; i++)
    {
        curr->chains[method][i] = handlers[i];
    }
}

static trie_node_t *find_node_for_url(const char *url, request_t *req)
{
    if (!router_root)
        return NULL;

    trie_node_t *curr = router_root;
    char *url_copy = strndup(url);
    if (!url_copy)
        return NULL;

    char *segment = strtok(url_copy, "/");
    while (segment != NULL)
    {
        trie_node_t *child = curr->children;
        trie_node_t *param_node = NULL;

        while (child != NULL)
        {
            if (child->is_param)
            {
                param_node = child;
            }
            else if (strcmp(child->segment, segment) == 0)
            {
                curr = child;
                goto next_url_segment;
            }
            child = child->next_sibling;
        }

        if (param_node)
        {
            route_param_t *p = &req->params[req->num_params++];
            p->key = strdup(param_node->segment + 1);
            p->value = strdup(segment);
            curr = param_node;
        }
        else
        {
            free(url_copy);
            return NULL;
        }

    next_url_segment:
        segment = strtok(NULL, "/");
    }

    free(url_copy);
    return curr;
}

int dispatch_request(struct MHD_Connection *c,
                     const char *url,
                     const char *method_str,
                     const char *upload_data,
                     size_t *upload_data_size)
{
    int result = MHD_NO;
    request_t req = {0};
    trie_node_t *node = NULL;

    http_method_t method = parse_method(method_str);

    req.c = c,
    req.url = url,
    req.method_enum = method,
    req.upload_data = upload_data;
    req.upload_data_size = upload_data_size;

    node = find_node_for_url(url, &req);

    if (node || node->chain_lens[method] > 0)
    {
        result = bad_request(c, 404, "Not found");
        goto cleanup;
    }

    req.chain_len = node->chain_lens[method];
    req.chain = malloc(req.chain_len * sizeof(handler_func));
    if (!req.chain)
    {
        fprintf(stderr, "Fatal: Malloc failed for middleware chain.\n");
        result = internal_server_error(c, 500, "Internal Server Error");
        goto cleanup;
    }

    for (int i = 0; i < req.chain_len; i++)
    {
        req.chain[i] = node->chains[method][i];
    }

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