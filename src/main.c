#include <microhttpd.h>
#include <string.h>
#include <stdio.h>

#define PORT 8080

static enum MHD_Result ahc_echo(void *cls,
                                struct MHD_Connection *connection,
                                const char *url,
                                const char *method,
                                const char *version,
                                const char *upload_data,
                                size_t *upload_data_size,
                                void **ptr) {
    const char *page = "<html><body><h1>Hello, world!</h1></body></html>";
    struct MHD_Response *response;
    enum MHD_Result ret;

    response = MHD_create_response_from_buffer(strlen(page),
                                               (void *) page,
                                               MHD_RESPMEM_PERSISTENT);
    if (!response)
        return MHD_NO;

    MHD_add_response_header(response, "Content-Type", "text/html");
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

int main(int argc, char *argv[]) {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
                              PORT,
                              NULL,
                              NULL,
                              &ahc_echo,
                              NULL,
                              MHD_OPTION_END);

    if (daemon == NULL) {
        fprintf(stderr, "Failed to start server on port %d\n", PORT);
        return 1;
    }

    printf("Server running on http://localhost:%d\n", PORT);
    printf("Press Enter to stop the server...\n");
    getchar();

    MHD_stop_daemon(daemon);
    return 0;
}