#include "utils/hash.h"

uint32_t djb2_hash(const char *str)
{
    uint32_t hash = 5381;
    unsigned char c;
    while ((c = (unsigned char)*str++))
    {
        hash = ((hash << 5) + hash + c);
    }
    return hash;
}