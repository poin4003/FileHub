#include "middlewares/cors.h"
#include "core/response.h"

int cors_middleware(request_t *req)
{
    if (req-> method_enum == HTTP_OPTIONS)
    {
        printf("[CORS] handling OPTIONS prelight request for URL: %s\n", req->url);

        struct MHD_Response *response = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);
        if (!response)
            return MHD_NO;

        MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
        MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, PUT, PATCH, DELETE, OPTIONS");
        MHD_add_response_footer(response, "Access-Control-Allow-Headers", "Content-Type, Authorization");

        int ret = MHD_queue_response(req->c, MHD_HTTP_NO_CONTENT, response);
        MHD_destroy_response(response);

        return ret;
    }

    return next_handler(req);
}