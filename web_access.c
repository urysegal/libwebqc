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

static bool
prepare_curl(WQC *handler)
{
    bool rv = false;
    assert(handler->curl_info.curl_handler == NULL);

    handler->curl_info.curl_handler = curl_easy_init();
    if (handler->curl_info.curl_handler) {
        curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_URL, URL.c_str());
        curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_WRITEFUNCTION, collect_downloaded_data_in_string);
        curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_WRITEDATA, &handler->curl_info.web_reply);
        curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_ERRORBUFFER, handler->curl_info.web_error_bufffer);

        handler->curl_info.http_headers = curl_slist_append(handler->curl_info.http_headers, auth_header.c_str());
        curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_HTTPHEADER, handler->curl_info.http_headers);

        curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_USERAGENT, "curl/7.68.0");
    }

    return rv;
}