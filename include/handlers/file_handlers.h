#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <microhttpd.h>

int handle_create_folder(struct MHD_Connection *connection, 
                        const char *upload_data, 
                        size_t *upload_data_size);

int handle_file_upload(struct MHD_Connection *connection, 
                    const char *folder_id,
                    const char *upload_data, 
                    size_t *upload_data_size, 
                    void **con_cls);

int handle_file_download(struct MHD_Connection *connection, 
                        const char *file_id);

#endif