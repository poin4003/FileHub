#ifndef ROUTER_H
#define ROUTER_H

#include <microhttpd.h>
#include <stddef.h>

/* Maximum number of route parameters supported per route */
#define MAX_ROUTE_PARAMS 8

/* Maximum number of middleware or handler functions in a chain */
#define MAX_MIDDLEWARE_CHAIN 16

/* ===============================
 * HTTP Method Enumeration
 * ===============================
 * Represents all supported HTTP methods.
 * HTTP_METHOD_UNKNOWN is used for invalid/unrecognized methods.
 */
typedef enum
{
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_PATCH,
    HTTP_DELETE,
    HTTP_OPTIONS,
    HTTP_METHOD_UNKNOWN,
    METHOD_COUNT  // Total number of supported methods
} http_method_t;

/* ===============================
 * Route Parameter
 * ===============================
 * Represents a key-value pair extracted from the URL path
 * for dynamic route segments
 */
typedef struct
{
    char *key;    // Name of the parameter
    char *value;  // Value extracted from the URL
} route_param_t;

/* Forward declaration of request structure */
struct request_s;

/* Type definition for route/middleware handler functions
 * A handler takes a pointer to a request structure and returns an int status
 */
typedef int (*handler_func)(struct request_s *req);

/* ===============================
 * Request Structure
 * ===============================
 * Represents an incoming HTTP request and its context.
 */
typedef struct request_s
{
    struct MHD_Connection *c;      // microhttpd connection handle
    const char *url;               // Requested URL path
    http_method_t method_enum;     // HTTP method as enum
    const char *upload_data;       // Pointer to uploaded data (if any)
    size_t *upload_data_size;      // Size of uploaded data

    handler_func *chain;           // Array of middleware/handler functions to run
    int chain_len;                 // Length of the chain
    int current_handler_idx;       // Index of the current handler being executed

    route_param_t params[MAX_ROUTE_PARAMS]; // Extracted route parameters
    int num_params;                           // Number of parameters extracted

    void *user_data;               // Optional user data for handler use
} request_t;

/* ===============================
 * Router Group
 * ===============================
 * Represents a group of routes sharing a common path prefix
 * and optional middlewares.
 */
typedef struct router_group_s
{
    char prefix[256];                        // Path prefix for all routes in the group
    handler_func middlewares[MAX_MIDDLEWARE_CHAIN]; // Middlewares applied to all routes
    int mw_count;                            // Number of middlewares
} router_group_t;

/* ===============================
 * Router API
 * =============================== */

/* Initialize the router data structures */
void router_init(void);

/* Free all allocated memory and clean up router */
void router_free(void);

/* Create a nested router group with optional middlewares */
router_group_t router_group(
    router_group_t *parent,     // Parent group (NULL for top-level)
    const char *relative_path,  // Relative path for this group
    handler_func mws[],         // Array of middleware functions
    int mw_count                // Number of middlewares
);

/* Add a route to a router group with HTTP method and handlers */
void router_group_add_route(
    router_group_t *group,      // Group to which route belongs
    http_method_t method,       // HTTP method for this route
    const char *path,           // URL path for the route
    handler_func handlers[],    // Array of handler functions
    int handler_count           // Number of handlers
);

/* Dispatch an incoming HTTP request to the proper route and handlers */
int dispatch_request(struct MHD_Connection *c,
                     const char *url,
                     const char *method,
                     const char *upload_data,
                     size_t *upload_data_size);

/* Execute the next handler in the request chain */
int next_handler(request_t *req);

#endif