#include "core/app.h"
#include <microhttpd.h>
#include <stdio.h>

#define PORT 8080

int main(int argc, char *argv[]) {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                            &request_handler, NULL, MHD_OPTION_END);

    if (daemon == NULL) {
        fprintf(stderr, "Failed to start server on port %d\n", PORT);
        return 1;
    }

    printf("Server running on http://localhost:%d\n", PORT);

    getchar();

    MHD_stop_daemon(daemon);
    return 0;
}