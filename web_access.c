#include <curl/curl.h>
#include <string.h>
#include <assert.h>
#include <cjson/cJSON.h>

#ifdef __APPLE__
#include <malloc/malloc.h>
#include <stdlib.h>
#else

#include <malloc.h>

#endif

#include "libwebqc.h"
#include "include/webqc-handler.h"


static size_t collect_downloaded_data_in_string(void *data, size_t size, size_t nmemb, void *userp)
{
    size_t total_size = size * nmemb;
    struct web_reply_buffer *buf = (struct web_reply_buffer *) userp;

    char *ptr = realloc(buf->reply, buf->size + total_size + 1);

    if (ptr == NULL) {
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
    char *auth_header = (char *) malloc(strlen(AUTH_HEADER) + strlen(handler->access_token) + 1);

    if (auth_header) {
        strncpy(auth_header, AUTH_HEADER, strlen(AUTH_HEADER) + 1);
        strncat(auth_header, handler->access_token, strlen(handler->access_token));
        handler->curl_info.http_headers = curl_slist_append(handler->curl_info.http_headers, auth_header);
        free(auth_header);

        if (handler->insecure_ssl) {
            curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_SSL_VERIFYHOST, 0L);
        }
        rv = true;
    } else {
        wqc_set_error(handler, WEBQC_OUT_OF_MEMORY); //// LCOV_EXCL_LINE
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
    char *URL = (char *) malloc(MAX_URL_SIZE);
    const char *scheme = "https";

    if (URL) {
        if (snprintf(URL, MAX_URL_SIZE, "%s://%s:%u/%s", scheme, handler->webqc_server_name, handler->webqc_server_port,
                     web_endpoint) < MAX_URL_SIZE) {
            curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_URL, URL);
            rv = true;
        }
        free(URL);
    } else {
        wqc_set_error(handler, WEBQC_OUT_OF_MEMORY); //// LCOV_EXCL_LINE
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

        if ((rv = prepare_curl_URL(handler, web_endpoint))) {

            if ((rv = prepare_curl_security(handler))) {
                handler->curl_info.curl_handler = curl_slist_append(handler->curl_info.curl_handler,
                                                                    "Content-Type: application/json");
                curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_HTTPHEADER, handler->curl_info.http_headers);
                rv = true;
            }
        }
    }

    return rv;
}

bool cleanup_curl(WQC *handler)
{
    assert (handler) ;
    if (handler->curl_info.http_headers) {
        curl_slist_free_all(handler->curl_info.http_headers);
        handler->curl_info.http_headers = NULL;
    }
    if (handler->curl_info.curl_handler) {
        curl_easy_cleanup(handler->curl_info.curl_handler);
        handler->curl_info.curl_handler = NULL;
    }
    return true;
}

/// A name-value pair for adding multiple JSON fields at once
struct name_value_pair {
    const char *name;
    const char *value;
};

//! Add name-value pairs as string to a JSON object
//! \param object object to add values to
//! \param pairs name=value pairs to add
//! \param pairs_count how many pairs passed.
//! \return true on success, false on failure
static bool
add_json_string_fields(cJSON *object, struct name_value_pair *pairs, int pairs_count)
{
    bool rv = true;
    int i;
    for (i = 0; rv && (i < pairs_count); ++i) {
        assert(pairs[i].name);
        assert(pairs[i].value);
        cJSON *value = cJSON_CreateString(pairs[i].value);
        if (value == NULL) {
            rv = false;
        }
        cJSON_AddItemToObject(object, pairs[i].name, value);
    }
    return rv;
}

bool make_eri_request(WQC *handler, const struct two_electron_integrals_job_parameters *job_parameters)
{
    bool rv = false;

    struct name_value_pair two_e_parameters_pairs[] = {
            {"basis_set_name",   job_parameters->basis_set_name},
            {"xyz_file_content", job_parameters->geometry} // ADD IT TO PYTHON!!
    };

    cJSON *ERI_request = cJSON_CreateObject();

    if (ERI_request) {
        if (add_json_string_fields(ERI_request, two_e_parameters_pairs,
                                   sizeof(two_e_parameters_pairs) / sizeof(struct name_value_pair))) {

            char *json_as_string = cJSON_Print(ERI_request);

            curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_POST, 1L);
            curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_POSTFIELDS, json_as_string);
            free(json_as_string);
            cJSON_Delete(ERI_request);
            rv = true;
        } else {
            wqc_set_error(handler, WEBQC_OUT_OF_MEMORY); //// LCOV_EXCL_LINE
        }
    } else {
        wqc_set_error(handler, WEBQC_OUT_OF_MEMORY); //// LCOV_EXCL_LINE
    }
    return rv;
}

