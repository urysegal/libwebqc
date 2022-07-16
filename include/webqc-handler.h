#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "webqc-errors.h"
#include <curl/curl.h>

struct web_reply_buffer {
    char *reply;
    size_t size;
};

struct handler_curl_info
{
    CURL *curl_handler; /// current CURL call handler
    struct web_reply_buffer web_reply; /// Buffer to collect replies from a web service
    char web_error_bufffer[CURL_ERROR_SIZE]; /// Buffer for errors from the web
    struct curl_slist *http_headers; /// HTTP headers to use in a web call
};


/**
 * @brief Internal structure that maintains the status of an asynchrounous WQC operation.
 */
struct webqc_handler_t {
    uint64_t handler_id; /// A unique ID for this operation
    struct wqc_return_value return_value; /// Return value from last call to WQC API
    char *access_token; /// Token that authorizes access to the WEBQC web service
    struct handler_curl_info curl_info; /// Info for calling web services using libCURL
    char *webqc_server_name; /// WebQC server name
    unsigned short webqc_server_port; /// Port of the WebQC server
    bool insecure_ssl; /// Do not verify SSL certificates
};