#include "core/response.h"
#include "core/status_codes.h"
#include <string.h>

static json_t *create_response_body(int status_code,
                                    const char *message,
                                    const char *key,
                                    json_t *data)
{
    json_t *body = json_object();
    if (!body)
        return NULL;

    json_object_set_new(body, "status_code", json_integer(status_code));
    if (message)
        json_object_set_new(body, key, json_string(message));
    if (data)
        json_object_set(body, "data", json_incref(data));

    return body;
}

int send_json_response(struct MHD_Connection *connection,
                       int http_status,
                       json_t *json_body)
{
    if (!json_body)
        return MHD_NO;

    int ret = MHD_NO;
    char *response_str = json_dumps(json_body, JSON_COMPACT);
    struct MHD_Response *response = NULL;

    if (!response_str)
        goto cleanup;

    response = MHD_create_response_from_buffer(strlen(response_str),
                                               (void *)response_str,
                                               MHD_RESPMEM_MUST_FREE);

    if (!response)
        goto cleanup;

    MHD_add_response_header(response, "Content-Type", "application/json");
    ret = MHD_queue_response(connection, http_status, response);

cleanup:
    if (response)
        MHD_destroy_response(response);
    if (json_body)
        json_decref(json_body);

    return ret;
}

int ok(struct MHD_Connection *connection,
       int status_code,
       const char *message,
       const void *data,
       to_json_func to_json)
{
    json_t *json_data = NULL;
    if (data && to_json)
        json_data = to_json(data);

    json_t *body = create_response_body(status_code, message, "message", json_data);
    return send_json_response(connection, MHD_HTTP_OK, body);
}

int bad_request(struct MHD_Connection *connection,
                int status_code,
                const char *message)
{
    json_t *body = create_response_body(status_code, message, "error", NULL);
    return send_json_response(connection, MHD_HTTP_BAD_REQUEST, body);
}

int internal_server_error(struct MHD_Connection *connection,
                          int status_code,
                          const char *message)
{
    json_t *body = create_response_body(status_code, message, "error", NULL);
    return send_json_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, body);
}