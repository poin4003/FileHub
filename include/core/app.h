#ifndef APP_H
#define APP_H

#include <microhttpd.h>

enum MHD_Result request_handler(void *cls, 
                         struct MHD_Connection *connection,
                         const char *url, 
                         const char *method,
                         const char *version, 
                         const char *upload_data,
                         size_t *upload_data_size, 
                         void **con_cls
);

#endif