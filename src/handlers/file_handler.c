#include "handlers/file_handlers.h"
#include "database/db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uuid/uuid.h>
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