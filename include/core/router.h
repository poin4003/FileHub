#ifndef ROUTER_H
#define ROUTER_H

#include <microhttpd.h>

#define METHOD_MAP_SIZE 11

typedef enum
{
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE,
    HTTP_OPTIONS
} http_method_t;

typedef int (*route_handler)(struct MHD_Connection *connection,
                             const char *url,
                             const char *method,
                             const char *upload_data,
                             size_t *upload_data_size);

typedef struct {
    http_method_t method;
    const char *path;
    route_handler handler;
} route_t;

http_method_t parse_method(const char *method);

void init_method_map(void);

#endif