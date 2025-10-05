#include "handlers/user_handlers.h"
#include "core/response.h"
#include "core/router.h"
#include "schemas/user.h"
#include "database/db.h"
#include "core/status_codes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

int get_user_handler(request_t *req)
{
    printf("Executing handler: get_user_hanlder\n");
    return ok(req->c, 200, "fetch user successfully", NULL, NULL);
}

int create_user_handler(request_t *req)
{
    printf("Executing handler: create_user_hanlder\n");
    return ok(req->c, 200, "create user successfully", NULL, NULL);
}

void register_user_routes(router_group_t *group)
{
    printf("Registering user routes...\n");

    handler_func get_users_chain[] = { get_user_handler };
    router_group_add_route(group, HTTP_GET, "", get_users_chain, 1);

    handler_func create_user_chain[] = { create_user_handler };
    router_group_add_route(group, HTTP_POST, "", create_user_chain, 1);
}