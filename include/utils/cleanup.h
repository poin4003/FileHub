#ifndef CLEANUP_H
#define CLEANUP_H

#define FREE_FIELD_const_char_ptr(field) if (field) free((char*)field);
#define FREE_FIELD_int(field)
#define FREE_FIELD_double(field)
#define FREE_FIELD_bool(field)
#define FREE_FIELD_pointer(field) free(field);

#define FREE_FIELD(type, field) FREE_FIELD_##type(field)

#endif