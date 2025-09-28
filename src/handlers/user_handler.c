#include "handlers/user_handlers.h"
#include "core/response.h"
#include "core/router.h"
#include "schemas/user.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

static route_t routes[] = {
    {HTTP_POST, "/user", create_user_handler},
    {HTTP_GET, "/user/", get_user_hanlder},
    {HTTP_DELETE, "/user/", delete_user_handler}};

void register_routes(void)
{
    (void)routes;
}

static int parse_user_id_from_url(const char *url)
{
    if (!url)
        return -1;

    const char *p = url;
    if (strncmp(p, "/user/", 6) != 0) 
        return -1;

    p += 6;
    char *end = NULL;
    long id = strtol(p, &end, 10);
    if (p == end)
        return -1;
    return (int)id;
}

int create_user_handler(struct MHD_Connection *c,
                        const char *url,
                        const char *method,
                        const char *upload_data,
                        size_t *upload_data_size)
{
    int ret = MHD_HTTP_INTERNAL_SERVER_ERROR;
    char *body = NULL;
    user_t *u = NULL;
    json_t *j = NULL;
    char *s = NULL;

    if (!upload_data || *upload_data_size == 0) {
        ret = bad_request(c, 400, "Empty body");
        goto cleanup;
    }


cleanup:
    if (s) free(s);
    if (j) json_decref(j);
}

int get_user_hanlder(struct MHD_Connection *c,
                     const char *url,
                     const char *method,
                     const char *upload_data,
                     size_t *upload_data_size)
{
}

int delete_user_handler(struct MHD_Connection *c,
                        const char *url,
                        const char *method,
                        const char *upload_data,
                        size_t *upload_data_size)
{
}