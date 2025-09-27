#ifndef JSON_H
#define JSON_H

#include <jansson.h>
#include <stdbool.h>
#include <stdint.h>
#include "macro.h"

static inline json_t *json_from_int(int val) { return json_integer(val); }
static inline json_t *json_from_double(double val) { return json_real(val); }
static inline json_t *json_from_const_char_ptr(const_char_ptr val) { return val ? json_string(val) : json_null(); }
static inline json_t *json_from_bool(bool val) { return json_integer(val ? 1 : 0); }
static inline json_t *json_from_json(json_t *val) { return val ? val : json_null(); }
static inline json_t *json_from_pointer(void *val) { return json_integer((intptr_t)val); }

#define TO_JSON_FIELD(type, field) \
    json_object_set_new(root, #field, json_from_##type(user->field));

#endif