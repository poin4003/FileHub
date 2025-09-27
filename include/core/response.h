#ifndef RESPONSE_H
#define RESPONSE_H

#include <microhttpd.h>
#include <jansson.h>

typedef json_t *(*to_json_func)(const void *data);

int ok(struct MHD_Connection *connection,
       int status_code,
       const char *message,
       const void *data,
       to_json_func to_json);

int bad_request(struct MHD_Connection *connection,
                int status_code,
                const char *message);

int internal_server_error(struct MHD_Connection *connection,
                          int status_code,
                          const char *message);

int send_json_response(struct MHD_Connection *connection,
                       int http_status,
                       json_t *json_body);
#endif