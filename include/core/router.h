#ifndef ROUTER_H
#define ROUTER_H

#include <microhttpd.h>
#include <stddef.h>

#define MAX_ROUTE_PARAMS 8
#define MAX_MIDDLEWARE_CHAIN 16

typedef enum
{
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_PATCH,
    HTTP_DELETE,
    HTTP_OPTIONS,
    HTTP_METHOD_UNKNOWN,
    METHOD_COUNT
} http_method_t;

typedef struct
{
    char *key;
    char *value;
} route_param_t;

struct request_s;
typedef int (*handler_func)(struct request_s *req);

typedef struct request_s
{
    struct MHD_Connection *c;
    const char *url;
    http_method_t method_enum;
    const char *upload_data;
    size_t *upload_data_size;

    handler_func *chain;
    int chain_len;
    int current_handler_idx;

    route_param_t params[MAX_ROUTE_PARAMS];
    int num_params;

    void *user_data;
} request_t;

typedef struct router_group_s
{
    char prefix[128];
    handler_func middlewares[MAX_MIDDLEWARE_CHAIN];
    int mw_count;
} router_group_t;

void router_init(void);
void router_free(void);

void router_add_route(
    http_method_t method,
    const char *path,
    handler_func handlers[],
    int handler_count);

router_group_t router_group(
    router_group_t *parent,
    const char *relative_path,
    handler_func mws[],
    int mw_count);

void router_group_add_route(
    router_group_t *group,
    http_method_t method,
    const char *path,
    handler_func handlers[],
    int handler_count);

int dispatch_request(struct MHD_Connection *c,
                     const char *url,
                     const char *method,
                     const char *upload_data,
                     size_t *upload_data_size);

int next_handler(request_t *req);

#endif