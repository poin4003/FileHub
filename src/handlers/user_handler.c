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

static route_t routes[] = {
    {HTTP_POST, "/user", create_user_handler},
    {HTTP_GET, "/user/:id", get_user_hanlder},
    {HTTP_DELETE, "/user/:id", delete_user_handler}};

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

    body = strndup(upload_data, *upload_data_size);
    if (!body) {
        ret = internal_server_error(c, 500, "alloc failed");
        goto cleanup;
    }
    *upload_data_size = 0;

    u = user_from_json_str(body);
    if (!u) {
        ret = bad_request(c, 400, "Invalid Json");
        goto cleanup;
    }

    char key[64];
    snprintf(key, sizeof(key), "user:%d", u->id);

    j = user_to_json(u);
    if (!j) {
        ret = internal_server_error(c, 500, "serialize failed");
        goto cleanup;
    }

    s = json_dumps(j, JSON_COMPACT);
    if (!s) {
        ret = internal_server_error(c, 500, "serialize failed");
        goto cleanup;
    }

    ret = ok(c, STATUS_OK, "user", u, (to_json_func)user_to_json);

cleanup:
    if (s) free(s);
    if (j) json_decref(j);
    if (u) free_user(u);
    if (body) free(body);
    return ret;
}

int get_user_hanlder(struct MHD_Connection *c,
                     const char *url,
                     const char *method,
                     const char *upload_data,
                     size_t *upload_data_size)
{
    int ret = MHD_HTTP_INTERNAL_SERVER_ERROR;
    char *val = NULL;
    user_t *u = NULL;

    int id = parse_user_id_from_url(url);
    if (id < 0) {
        ret = bad_request(c, 400, "Invalid user id");
        goto cleanup;
    }

    char key[64];
    snprintf(key, sizeof(key), "user:%d", id);

    val = db_get(key);
    if (!val) {
        ret = bad_request(c, 404, "User not found");
        goto cleanup;
    }

    u = user_from_json(val);
    if (!u) {
        ret = internal_server_error(c, 500, "parse user failed");
        goto cleanup;
    }

    ret = ok(c, STATUS_OK, "User found", u, (to_json_func)user_to_json);

cleanup:
    if (u) free_user(u);
    if (val) free(val);
    return ret;
}

int delete_user_handler(struct MHD_Connection *c,
                        const char *url,
                        const char *method,
                        const char *upload_data,
                        size_t *upload_data_size)
{
    int ret = MHD_HTTP_INTERNAL_SERVER_ERROR;

    int id = parse_user_id_from_url(url);
    if (id < 0) {
        ret = bad_request(c, 400, "Invalid user id");
        goto cleanup;
    }

    ret = ok(c, STATUS_OK, "User deleted", NULL, NULL);

cleanup:
    return ret;
}