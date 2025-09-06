#ifndef RESPONSE_H
#define RESPONSE_H

#include <microhttpd.h>
#include <cjson/cJSON.h>

int send_json_response(struct MDH_Connection *connection,
                    int http_status,
                    cJSON *json_body);

int ok(struct MDH_Connection *connection,
    int status_code,
    const char *message);

#endif