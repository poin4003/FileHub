#include "core/response.h"
#include "core/status_codes.h"
#include "schemas/user.h"
#include <microhttpd.h>
#include <string.h>
#include <stdio.h>

#define PORT 8080

static enum MHD_Result ahc_handler(void *cls, struct MHD_Connection *connection,
                                   const char *url, const char *method,
                                   const char *version, const char *upload_data,
                                   size_t *upload_data_size, void **con_cls)
{
    printf(">>> %s %s\n", method, url);
    
    const char *response_str = "{\"message\":\"Hello World\"}";
    struct MHD_Response *response;
    int ret;

    if (0 == strcmp(method, "OPTIONS"))
    {
        response = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
        MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type, Authorization");
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }

    if (0 == strcmp(method, "GET") && strcmp(url, "/user") == 0)
    {
        user_t user = {
            .id = 1,
            .name = "Alice",
            .email = "alice@example.com",
            .score = 95.5,
            .is_active = 1
        };

        return ok(connection, STATUS_OK, "Successfully retrived user profile", &user, (to_json_func)user_to_json);
    }

    return bad_request(connection, 400, "Not Found");
}

int main()
{
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT,
                              NULL, NULL,
                              &ahc_handler, NULL,
                              MHD_OPTION_END);
    if (NULL == daemon)
        return 1;

    printf("Server running on http://localhost:%d\n", PORT);
    getchar();
    MHD_stop_daemon(daemon);
    return 0;
}