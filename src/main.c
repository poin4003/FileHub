#include "core/response.h"
#include "core/router.h"
#include "middlewares/cors.h"
#include "core/status_codes.h"

#include "handlers/user_handlers.h"

#include <microhttpd.h>
#include <string.h>
#include <stdio.h>

#define PORT 8080

enum MHD_Result access_handler_cb(void *cls,
                                  struct MHD_Connection *c,
                                  const char *url,
                                  const char *method,
                                  const char *version,
                                  const char *upload_data,
                                  size_t *upload_data_size, 
                                  void **con_cls)
{
    return dispatch_request(c, url, method, upload_data, upload_data_size);
}

int main(void)
{
    router_init();

    handler_func global_middlewares[] = { cors_middleware };
    
    router_group_t api_group = router_group(NULL, "/api", global_middlewares, 1);
    router_group_t user_group = router_group(&api_group, "/users", NULL, 0);

    register_user_routes(&user_group);

    struct MHD_Daemon *daemon;
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
                              PORT,
                              NULL,
                              NULL,
                              &access_handler_cb,
                              NULL,
                              MHD_OPTION_END);
    if (NULL == daemon)
        return 1;

    printf("Server running on http://localhost:%d\n", PORT);
    getchar();
    MHD_stop_daemon(daemon);
    return 0;
}