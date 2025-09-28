#ifndef USER_HANLDER
#define USER_HANDLER

#include "core/router.h"

void register_routes(void);

int create_user_handler(struct MHD_Connection *c,
                        const char *url,
                        const char *method,
                        const char *upload_data,
                        size_t *upload_data_size);
                        
int get_user_hanlder(struct MHD_Connection *c,
                     const char *url,
                     const char *method,
                     const char *upload_data,
                     size_t *upload_data_size);

int delete_user_handler(struct MHD_Connection *c,
                        const char *url,
                        const char *method,
                        const char *upload_data,
                        size_t *upload_data_size);

#endif