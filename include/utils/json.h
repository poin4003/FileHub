#ifndef JSON_H
#define JSON_H

#include <jansson.h>
#include <stdbool.h>
#include <stdint.h>
#include "macro.h"

// --- TO JSON ---
static inline json_t *json_from_int(int val)
{
    return json_integer(val);
}
static inline json_t *json_from_double(double val)
{
    return json_real(val);
}
static inline json_t *json_from_const_char_ptr(const_char_ptr val)
{
    return val ? json_string(val) : json_null();
}
static inline json_t *json_from_bool(bool val)
{
    return json_integer(val ? 1 : 0);
}
static inline json_t *json_from_json(json_t *val)
{
    return val ? val : json_null();
}
static inline json_t *json_from_pointer(void *val)
{
    return json_integer((intptr_t)val);
}

// --- FROM JSON ---
static inline int json_to_int(json_t *val)
{
    return json_is_integer(val) ? (int)json_integer_value(val) : 0;
}
static inline double json_to_double(json_t *val)
{
    return json_is_real(val) ? json_real_value(val) : 0.0;
}
static inline const_char_ptr json_to_const_char_ptr(json_t *val)
{
    return json_is_string(val) ? json_string_value(val) : NULL;
}
static inline bool json_to_bool(json_t *val)
{
    return json_is_integer(val) ? json_integer_value(val) != 0 : false;
}
static inline void *json_to_pointer(json_t *val)
{
    return json_is_integer(val) ? (void *)(intptr_t)json_integer_value(val) : NULL;
}

#define TO_JSON_FIELD(type, field) \
    json_object_set_new(root, #field, json_from_##type(user->field));

#define FROM_JSON_FIELD(obj, type, field) \
    (obj)->field = json_to_##type(json_object_get(root, #field));

#endif