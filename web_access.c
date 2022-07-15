#include <curl/curl.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

#include "libwebqc.h"
#include "include/webqc-handler.h"


static size_t collect_downloaded_data_in_string(void *data, size_t size, size_t nmemb, void *userp)
{
    size_t total_size = size * nmemb;
    struct web_reply_buffer *buf = (struct web_reply_buffer *) userp;

    char *ptr = realloc(buf->reply, buf->size + total_size + 1);

    if(ptr == NULL) {
        return 0;
    }

    buf->reply = ptr;
    memcpy(&(buf->reply[buf->size]), data, total_size);
    buf->size += total_size;
    buf->reply[buf->size] = 0;

    return total_size;
}

#define AUTH_HEADER "Authorization: Bearer "
static bool
prepare_curl_security(WQC *handler)
{
    bool rv = false;
    char *auth_header =(char *) malloc(strlen(AUTH_HEADER) + strlen(handler->access_token) + 1);

    if ( auth_header ) {
        strncpy(auth_header, AUTH_HEADER, strlen(AUTH_HEADER)+1);
        strncat(auth_header, handler->access_token, strlen(handler->access_token));
        handler->curl_info.http_headers = curl_slist_append(handler->curl_info.http_headers, auth_header);
        free(auth_header);

        curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_SSL_VERIFYHOST, 0L);
        rv = true;
    } else {
        wqc_set_error(handler, WEBQC_OUT_OF_MEMORY);
    }

    return rv;
}

static void
prepare_curl_reply_buffers(WQC *handler)
{
    curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_WRITEFUNCTION, collect_downloaded_data_in_string);
    curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_WRITEDATA, &handler->curl_info.web_reply);
    curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_ERRORBUFFER, handler->curl_info.web_error_bufffer);
}


#define MAX_URL_SIZE 1024
static bool
prepare_curl_URL(WQC *handler, const char *web_endpoint)
{
    bool rv = false;
    char *URL =(char *) malloc(MAX_URL_SIZE);
    const char *scheme = "https";

    if ( URL ) {
        if ( snprintf(URL, MAX_URL_SIZE, "%s://%s:%u/%s", scheme, handler->webqc_server_name, handler->webqc_server_port, web_endpoint) <  MAX_URL_SIZE ) {
            curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_URL, URL);
            rv = true;
        }
        free(URL);
    } else {
        wqc_set_error(handler, WEBQC_OUT_OF_MEMORY);
    }

    return rv;
}


bool
prepare_curl(WQC *handler, const char *web_endpoint)
{
    bool rv = false;
    assert(handler->curl_info.curl_handler == NULL);

    handler->curl_info.curl_handler = curl_easy_init();
    if (handler->curl_info.curl_handler) {

        curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_USERAGENT, "curl/7.68.0");

        prepare_curl_reply_buffers(handler);

        if ( (rv = prepare_curl_URL(handler, web_endpoint) ) ) {

            if ((rv = prepare_curl_security(handler))) {
                curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_HTTPHEADER, handler->curl_info.http_headers);
                rv = true;
            }
        }
    }

    return rv;
}