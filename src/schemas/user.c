#include "schemas/user.h"
#include "utils/json.h"
#include <string.h>

json_t *user_to_json(const user_t *user)
{
    if (!user)
        return NULL;

    json_t *root = json_object();
    if (!root)
        return NULL;

#define X(type, name) TO_JSON_FIELD(type, name)
    USER_FIELDS
#undef X

    return root;
}

user_t *user_from_json(const json_t *root)
{
    if (!root || !json_is_object(root))
        return NULL;

    user_t *user = malloc(sizeof(user_t));
    if (!user)
        return NULL;

#define X(type, name) FROM_JSON_FIELD(user, type, name)
    USER_FIELDS
#undef X

    return user;
}

void free_user(user_t *user) 
{
    if (!user)
        return;

#define X(type, field) FREE_FIELD(type, user->field)
    USER_FIELDS
#undef X

    free(user);
}