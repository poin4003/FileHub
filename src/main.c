#include <microhttpd.h>
#include <string.h>
#include <stdio.h>

#define PORT 8080

static enum MHD_Result ahc_handler(void *cls, struct MHD_Connection *connection,
            const char *url, const char *method,
            const char *version, const char *upload_data,
            size_t *upload_data_size, void **con_cls) {
    
    const char *response_str = "{\"message\":\"Hello World\"}";
    struct MHD_Response *response;
    int ret;

    if (0 == strcmp(method, "OPTIONS")) {
        response = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
        MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type, Authorization");
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }

    response = MHD_create_response_from_buffer(strlen(response_str),
                                               (void *) response_str,
                                               MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, "Content-Type", "application/json");
    MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

int main() {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT,
                              NULL, NULL,
                              &ahc_handler, NULL,
                              MHD_OPTION_END);
    if (NULL == daemon) return 1;
    
    printf("Server running on http://localhost:%d\n", PORT);
    getchar();
    MHD_stop_daemon(daemon);
    return 0;
}