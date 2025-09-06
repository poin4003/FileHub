#include "handlers/file_handlers.h"
#include "database/db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uuid/uuid.h>
#include <cjson/cJSON.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FILE_STORAGE_PATH "./resource"

struct UploadStatus {
    FILE *file_handle;
    char *disk_filename;
    char *original_filename;
    struct MHD_PostProcessor *post_processor;
};

// static int send_json_response(struct MHD_Connection *connection, 
//                             int status_code, 
//                             cJSON *json_body
// ) {
//     int ret = MHD_NO;
//     char *response_str = NULL;
//     struct MHD_Response *response = NULL;

//     response_str = cJSON_PrintUnFormatted(json_body);
//     if (!response_str) goto cleanup;

//     response = MHD_create_response_from_buffer(
//         strlen(response_str),
//         (void *)response_str,
//         MHD_RESPMEM_MUST_FREE
//     );
//     if (!response) goto cleanup;

//     MHD_add_response_header(response, "Content-Type", "application/json");
//     ret = MHD_queue_response(connection, status_code, response);

// cleanup:
//     if (response) MHD_destroy_response(response);
//     if (json_body) cJSON_Delete(json_body);
//     return ret;
// }

// static char* generate_uuid() {
//     uuid_t uuid;
//     char *uuid_str = malloc(37);
//     uuid_generate_random(uuid);
//     uuid_unparse_lower(uuid, uuid_str);
//     return uuid_str;
// }

// int handle_create_folder(struct MHD_Connection *connection,
//                         const char *upload_data,
//                         size_t *upload_data_size
// ) {

// }