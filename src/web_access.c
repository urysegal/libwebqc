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

#include "../libwebqc.h"
#include "../include/webqc-handler.h"


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
        wqc_set_error(handler, WEBQC_OUT_OF_MEMORY); // LCOV_EXCL_LINE
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



static bool
prepare_curl_URL(WQC *handler, const char *web_endpoint)
{
    bool rv = false;
    const char *scheme = "https";

    if (snprintf(handler->curl_info.full_URL, MAX_URL_SIZE, "%s://%s:%u/%s", scheme, handler->webqc_server_name, handler->webqc_server_port,
                 web_endpoint) < MAX_URL_SIZE) {
        curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_URL, handler->curl_info.full_URL);
        rv = true;
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
                handler->curl_info.http_headers = curl_slist_append(handler->curl_info.http_headers,
                                                                    "Content-Type: application/json");
                curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_HTTPHEADER, handler->curl_info.http_headers);
                rv = true;
            }
        }
    }

    return rv;
}

bool make_curl_call(WQC *handler)
{
    assert (handler) ;
    CURLcode res;
    bool rv = false;

    res = curl_easy_perform(handler->curl_info.curl_handler);

    if (res) {
        const char *additional_messages[] = {
                handler->curl_info.full_URL,
                handler->curl_info.web_error_bufffer,
                curl_easy_strerror(res),
                NULL
        };
        wqc_set_error_with_messages(handler, WEBQC_WEB_CALL_ERROR, additional_messages); //need some more error information
    }
    curl_easy_getinfo( handler->curl_info.curl_handler, CURLINFO_RESPONSE_CODE, &handler->curl_info.http_reply_code);

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
    const char *name; /// field name
    enum wqc_data_type type;
    union
    {
        const char *str_value;
        wqc_real real_value;
        int64_t int_value;
    } value;
};

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

bool get_string_from_reply(cJSON *json, const char *field_name, char *dest, unsigned int max_size)
{
    bool rv = false;
    cJSON *job_id = cJSON_GetObjectItemCaseSensitive(json, field_name);
    if (cJSON_IsString(job_id) && (job_id->valuestring != NULL)) {
        strncpy(dest, job_id->valuestring, max_size);
        rv = true;
    }
    return rv;
}

bool get_job_id_from_reply(WQC *handler)
{
    bool rv = false;

    cJSON *reply_json = cJSON_Parse(handler->curl_info.web_reply.reply);
    if (reply_json == NULL) {
        const char *extra_messages[] =
                {
                    "Error parsing JSON reply: Error before",
                    "(no location)",
                    NULL
                };
        const char *error_ptr = cJSON_GetErrorPtr();

        if (error_ptr != NULL)
        {
            extra_messages[1] = error_ptr;
        }
        wqc_set_error_with_messages(handler, WEBQC_WEB_CALL_ERROR, extra_messages);
    } else {
        rv = get_string_from_reply(reply_json, "job_id", handler->job_id, WQC_JOB_ID_LENGTH);
        free (reply_json);
    }
    return rv;
}


bool set_eri_job_parameters(WQC *handler, const struct two_electron_integrals_job_parameters *job_parameters)
{
    bool rv = false;

    struct name_value_pair two_e_parameters_pairs[] = {
            {"basis_set_name",   WQC_STRING_TYPE, { .str_value=job_parameters->basis_set_name} },
            {"xyz_file_content", WQC_STRING_TYPE, { .str_value=job_parameters->geometry} }, // ADD IT TO PYTHON!!
            {"geometry_precision", WQC_REAL_TYPE, { .real_value=job_parameters->geometry_precision} }, // ADD IT TO PYTHON!!
            {"geometry_units", WQC_STRING_TYPE, { .str_value=job_parameters->geometry_units} } // ADD IT TO PYTHON!!
    };

    cJSON *ERI_request = cJSON_CreateObject();

    if (ERI_request) {
        if (add_json_fields(ERI_request, two_e_parameters_pairs, ARRAY_SIZE(two_e_parameters_pairs) ) ){

            char *json_as_string = cJSON_Print(ERI_request);

            curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_POST, 1L);
            curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_COPYPOSTFIELDS, json_as_string);
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

