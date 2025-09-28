#include "core/router.h"
#include <string.h>
#include <stdint.h>

#define METHOD_MAP_SIZE 11

typedef struct
{
    const char *key;
    http_method_t val;
    uint32_t hash;
} method_entry_t;

static method_entry_t method_map[METHOD_MAP_SIZE];
static int method_map_inited = 0;

static uint32_t djb2_hash(const char *str)
{
    uint32_t hash = 5381;
    unsigned char c;
    while ((c = (unsigned char)*str++))
    {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

static void method_map_insert(const char *key, http_method_t val)
{
    uint32_t h = djb2_hash(key);
    uint32_t idx = h % METHOD_MAP_SIZE;
    for (uint32_t i = 0; i < METHOD_MAP_SIZE; i++)
    {
        uint32_t pos = (idx + i) % METHOD_MAP_SIZE;
        if (method_map[pos].key == NULL)
        {
            method_map[pos].key = key;
            method_map[pos].val = val;
            method_map[pos].hash = h;
            return;
        }
    }
}

void init_method_map(void)
{
    if (method_map_inited)
        return;
    for (int i = 0; i < METHOD_MAP_SIZE; i++)
    {
        method_map[i].key = NULL;
        method_map[i].hash = 0;
    }

    method_map_insert("GET", HTTP_GET);
    method_map_insert("POST", HTTP_POST);
    method_map_insert("PUT", HTTP_PUT);
    method_map_insert("DELETE", HTTP_DELETE);
    method_map_insert("OPTIONS", HTTP_OPTIONS);

    method_map_inited = 1;
}

http_method_t parse_method(const char *method)
{
    if (!method)
        return (http_method_t)-1;
    if (!method_map_inited)
        init_method_map();

    uint32_t h = djb2_hash(method);
    uint32_t idx = h % METHOD_MAP_SIZE;

    for (uint32_t i = 0; i < METHOD_MAP_SIZE; i++)
    {
        uint32_t pos = (idx + i) % METHOD_MAP_SIZE;
        if (method_map[pos].key == NULL)
            return (http_method_t)-1;
        if (method_map[pos].hash == h && strcmp(method_map[pos].key, method) == 0)
            return method_map[pos].val;
    }
    return (http_method_t)-1;
}