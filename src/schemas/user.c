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