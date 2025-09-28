#ifndef USER_H
#define USER_H

#include <jansson.h>
#include "utils/macro.h"

#define USER_FIELDS  \
    X(int, id)       \
    X(STR, name)     \
    X(STR, email)    \
    X(double, score) \
    X(int, is_active)

typedef struct
{
#define X(type, name) type name;
    USER_FIELDS
#undef X
} user_t;

json_t *user_to_json(const user_t *user);
user_t *user_from_json(const json_t *root);

#endif