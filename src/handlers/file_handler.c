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

static int send_json_response(struct MDH_Connection *connection, 
                            int status_code, 
                            cJSON *json_body
) {
    char *response_str = cJSON_PrintUnformatted(json_body);
    if (!response_str) {
        cJSON_Delete(json_body);
        return MHD_NO;
    }

    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(response_str),
        (void *)response_str,
        MHD_RESPMEM_MUST_FREE
    );

    MHD_add_response_header(response, "Content-Type", "application/json");

    int ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    cJSON_Delete(json_body);
    return ret;
}

static int send_error_response(struct MHD_Connection *connection,
                            int status_code,
                            const char* message
) {

}

static char* generate_uuid() {
    uuid_t uuid;
    char *uuid_str = malloc(37);
    uuid_generate_random(uuid);
    uuid_unparse_lower(uuid, uuid_str);
    return uuid_str;
}

int handle_create_folder(struct MHD_Connection *connection,
                        const char *upload_data,
                        size_t *upload_data_size
) {

}