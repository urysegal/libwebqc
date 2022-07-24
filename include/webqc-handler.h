#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "webqc-errors.h"
#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_URL_SIZE (1024) /// Maximum URL size
#define UUID_LENGTH (37) /// UUID - 32 bytes for hex + 4 dash + terminating null
#define WQC_JOB_ID_LENGTH (UUID_LENGTH) /// Job IDs are UUIDs
#define WQC_PARAM_SET_ID_LENGTH (UUID_LENGTH) /// Parameter set IDs are UUIDs

//! Structure to hold a reply from a cURL call
struct web_reply_buffer {
    char *reply; /// Reply data
    size_t size; /// Maximum length that can be stored in the "reply" buffer above
};

/**
 * The cURL-library related part of the data saved per handler.
 */
struct handler_curl_info
{
    CURL *curl_handler; /// current CURL call handler
    char full_URL[MAX_URL_SIZE]; /// The full URL of the REST call to make
    struct web_reply_buffer web_reply; /// Buffer to collect replies from a web service
    char web_error_bufffer[CURL_ERROR_SIZE]; /// Buffer for errors from the web
    struct curl_slist *http_headers; /// HTTP headers to use in a web call
    int http_reply_code; /// HTTP Replu code from last call
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
    char job_id[WQC_JOB_ID_LENGTH]; /// Job ID the handler is currently doing
    char parameter_set_id[WQC_PARAM_SET_ID_LENGTH]; /// Job ID the handler is currently doing
    const char *wqc_endpoint; /// Which WebQC endpoint to call
};

//! Add reply data received from the web service to a reply buffer
//! \param data data recieved
//! \param total_size total size of data recieved (in bytes)
//! \param buf reply buffer to add to
//! \return number of bytes added
size_t wqc_collect_downloaded_data
(
    void *data,
    size_t total_size,
    struct web_reply_buffer *buf
);

#ifdef __cplusplus
} // "extern C"
#endif
