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
#include "webqc-handler.h"
#include "webqc-web-access.h"


void reset_reply_buffer(struct web_reply_buffer *buf)
{
    if ( buf->reply ) {
        free(buf->reply);
        buf->reply = NULL;
    }
    buf->size=0;
}


static size_t collect_curl_downloaded_data(void *data, size_t size, size_t nmemb, void *userp)
{
    size_t total_size = size * nmemb;
    struct web_reply_buffer *buf = &(((struct webqc_handler_t *) userp)->web_call_info.web_reply);

    return wqc_collect_downloaded_data(data, total_size, buf);
}

size_t wqc_set_downloaded_data(void *data, size_t total_size, struct web_reply_buffer *buf)
{
    reset_reply_buffer(buf);
    return wqc_collect_downloaded_data(data, total_size, buf);
}

size_t wqc_collect_downloaded_data(void *data, size_t total_size, struct web_reply_buffer *buf)
{
    char *ptr = realloc(buf->reply, buf->size + total_size + 1);

    if (ptr == NULL) {
        return 0; // LCOV_EXCL_LINE
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
        handler->web_call_info.http_headers = curl_slist_append(handler->web_call_info.http_headers, auth_header);
        free(auth_header);

        if (handler->insecure_ssl) {
            curl_easy_setopt(handler->web_call_info.curl_handler, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(handler->web_call_info.curl_handler, CURLOPT_SSL_VERIFYHOST, 0L);
        }
        rv = true;
    } else {
        wqc_set_error(handler, WEBQC_OUT_OF_MEMORY); // LCOV_EXCL_LINE
    }

    return rv;
}

static void
prepare_curl_reply_buffers(WQC *handler)
{
    curl_easy_setopt(handler->web_call_info.curl_handler, CURLOPT_WRITEFUNCTION, collect_curl_downloaded_data);
    curl_easy_setopt(handler->web_call_info.curl_handler, CURLOPT_WRITEDATA, handler);
    curl_easy_setopt(handler->web_call_info.curl_handler, CURLOPT_ERRORBUFFER, handler->web_call_info.web_error_bufffer);
}



static bool
prepare_curl_URL(WQC *handler, const char *web_endpoint)
{
    bool rv = false;
    const char *scheme = "https";

    assert(web_endpoint);

    if (snprintf(handler->web_call_info.full_URL, MAX_URL_SIZE, "%s://%s:%u/%s", scheme, handler->webqc_server_name, handler->webqc_server_port,
                 web_endpoint) < MAX_URL_SIZE) {
        curl_easy_setopt(handler->web_call_info.curl_handler, CURLOPT_URL, handler->web_call_info.full_URL);
        rv = true;
    } else {
        wqc_set_error(handler, WEBQC_OUT_OF_MEMORY); // LCOV_EXCL_LINE
    }

    return rv;
}


bool
prepare_web_call(WQC *handler, const char *web_endpoint)
{
    bool rv = false;
    assert(handler->web_call_info.curl_handler == NULL);

    handler->web_call_info.curl_handler = curl_easy_init();
    if (handler->web_call_info.curl_handler) {

        curl_easy_setopt(handler->web_call_info.curl_handler, CURLOPT_USERAGENT, "curl/7.68.0");
        curl_easy_setopt(handler->web_call_info.curl_handler, CURLOPT_FOLLOWLOCATION, 1L);

        prepare_curl_reply_buffers(handler);

        if ((rv = prepare_curl_URL(handler, web_endpoint))) {

            if ((rv = prepare_curl_security(handler))) {
                handler->web_call_info.http_headers = curl_slist_append(handler->web_call_info.http_headers,
                                                                        "Content-Type: application/json");
                curl_easy_setopt(handler->web_call_info.curl_handler, CURLOPT_HTTPHEADER, handler->web_call_info.http_headers);
                rv = true;
            }
        }
    }

    return rv;
}

bool make_web_call(WQC *handler)
{
    assert (handler) ;
    CURLcode res;
    bool rv = false;

    res = curl_easy_perform(handler->web_call_info.curl_handler);

    if (res) {
        const char *additional_messages[] = {
                handler->web_call_info.full_URL,
                handler->web_call_info.web_error_bufffer,
                curl_easy_strerror(res),
                NULL
        };
        wqc_set_error_with_messages(handler, WEBQC_WEB_CALL_ERROR, additional_messages); //need some more error information
    } else {
        curl_easy_getinfo(handler->web_call_info.curl_handler, CURLINFO_RESPONSE_CODE, &handler->web_call_info.http_reply_code);
        if (handler->web_call_info.http_reply_code < 200 || handler->web_call_info.http_reply_code >= 300 ) {

            char http_error_code[4] = {0,0,0,0};
            snprintf(http_error_code, sizeof(http_error_code), "%u", handler->web_call_info.http_reply_code);

            const char *additional_messages[] = {
                    handler->web_call_info.full_URL,
                    "HTTP Error Code: ",
                    http_error_code,
                    NULL
            };
            wqc_set_error_with_messages(handler, WEBQC_WEB_CALL_ERROR, additional_messages);
        } else {
            rv = true;
        }
    }

    return rv;
}

bool cleanup_web_call(WQC *handler)
{
    assert (handler) ;
    if (handler->web_call_info.http_headers) {
        curl_slist_free_all(handler->web_call_info.http_headers);
        handler->web_call_info.http_headers = NULL;
    }
    if (handler->web_call_info.curl_handler) {
        curl_easy_cleanup(handler->web_call_info.curl_handler);
        handler->web_call_info.curl_handler = NULL;
    }
    return true;
}


static bool
add_json_field(cJSON *object, const struct name_value_pair *pair)
{
    cJSON *value = NULL;

    if ( pair->type == WQC_REAL_TYPE ) {
        value = cJSON_CreateNumber(pair->value.real_value);
    } else if ( pair->type == WQC_STRING_TYPE ) {
        value = cJSON_CreateString(pair->value.str_value);
    }

    if (value) {
        cJSON_AddItemToObject(object, pair->name, value);
    }

    return value != NULL;
}

//! Add name-value pairs as string to a JSON object
//! \param object object to add values to
//! \param pairs name=value pairs to add
//! \param pairs_count how many pairs passed.
//! \return true on success, false on failure
static bool
add_json_fields(cJSON *object, const struct name_value_pair *pairs, int pairs_count)
{
    bool rv = true;
    int i;
    for (i = 0; rv && (i < pairs_count); ++i) {
        assert(pairs[i].name);
        rv = add_json_field(object, &pairs[i]);
    }
    return rv;
}

bool set_POST_fields(WQC *handler, struct name_value_pair values[], size_t num_values)
{
    bool rv = false;

    cJSON *ERI_request = cJSON_CreateObject();

    if (ERI_request) {

        if (add_json_fields(ERI_request, values, num_values ) ){

            char *json_as_string = cJSON_Print(ERI_request);

            curl_easy_setopt(handler->web_call_info.curl_handler, CURLOPT_POST, 1L);
            curl_easy_setopt(handler->web_call_info.curl_handler, CURLOPT_COPYPOSTFIELDS, json_as_string);
            free(json_as_string);
            cJSON_Delete(ERI_request);
            rv = true;
        } else {
            wqc_set_error(handler, WEBQC_OUT_OF_MEMORY); // LCOV_EXCL_LINE
        }
    } else {
        wqc_set_error(handler, WEBQC_OUT_OF_MEMORY); // LCOV_EXCL_LINE
    }
    return rv;
}

bool set_eri_job_parameters(WQC *handler, const struct two_electron_integrals_job_parameters *job_parameters)
{
    bool rv = false;

    handler->wqc_endpoint = TWO_ELECTRONS_INTEGRAL_SERVICE_ENDPOINT;

    struct name_value_pair two_e_parameters_pairs[] = {
            {"basis_set_name",   WQC_STRING_TYPE, { .str_value=job_parameters->basis_set_name} },
            {"xyz_file_content", WQC_STRING_TYPE, { .str_value=job_parameters->geometry} },
            {"geometry_precision", WQC_REAL_TYPE, { .real_value=job_parameters->geometry_precision} },
            {"geometry_units", WQC_STRING_TYPE, { .str_value=job_parameters->geometry_units} },
            {"shell_sets_per_file", WQC_REAL_TYPE, { .real_value=job_parameters->shell_set_per_file} },
    };
    rv = set_POST_fields(handler, two_e_parameters_pairs, ARRAY_SIZE(two_e_parameters_pairs));

    return rv;
}


void wqc_init_web_calls(WQC *handler)
{
    handler->web_call_info.curl_handler = NULL;
    handler->web_call_info.full_URL[0] = '\0';
    handler->web_call_info.web_reply.size = 0;
    handler->web_call_info.web_reply.reply = NULL;
    handler->web_call_info.web_error_bufffer[0] = '\0';
    handler->web_call_info.http_headers = NULL;
    handler->web_call_info.http_reply_code = 0;
}


bool
set_no_parameters(WQC *handler)
{
    curl_easy_setopt(handler->web_call_info.curl_handler, CURLOPT_POST, 1L);
    curl_easy_setopt(handler->web_call_info.curl_handler, CURLOPT_POSTFIELDS, "{}");
    return true;
}

bool prepare_get_parameter( WQC *handler, const char *param_name, const char *param_value)
{
    bool rv = false;

    char URL_with_options[MAX_URL_SIZE];

    if (snprintf(URL_with_options, MAX_URL_SIZE, "%s?%s=%s", handler->web_call_info.full_URL, param_name,
                 param_value) < MAX_URL_SIZE) {
        strncpy(handler->web_call_info.full_URL, URL_with_options, MAX_URL_SIZE);
        curl_easy_setopt(handler->web_call_info.curl_handler, CURLOPT_URL, handler->web_call_info.full_URL);
        rv = true;
    } else {
        wqc_set_error(handler, WEBQC_OUT_OF_MEMORY); // LCOV_EXCL_LINE
    }

    return rv;
}

bool wqc_download_file(WQC *handler, const char *URL, FILE *fp)
{
    bool rv = false;
    CURL *curl = curl_easy_init();
    CURLcode res = CURLE_OK;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, URL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, handler->web_call_info.web_error_bufffer);


        res = curl_easy_perform(curl);

        if (res) {
            const char *additional_messages[] = {
                URL,
                handler->web_call_info.web_error_bufffer,
                curl_easy_strerror(res),
                NULL
            };
            wqc_set_error_with_messages(handler, WEBQC_WEB_CALL_ERROR,
                                        additional_messages); //need some more error information
        } else {
            rv = true;
        }

        curl_easy_cleanup(curl);
    }
    return rv;
}



void web_access_init()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void web_access_cleanup()
{
    curl_global_cleanup();
}