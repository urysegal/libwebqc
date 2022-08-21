#include <string.h>
#include <cjson/cJSON.h>

#ifdef __APPLE__
#include <malloc/malloc.h>
#include <stdlib.h>
#else

#include <malloc.h>

#endif

#include "webqc-handler.h"
#include "webqc-json.h"

bool get_string_from_JSON(const cJSON *json, const char *field_name, char *dest, unsigned int max_size)
{
    bool rv = false;
    cJSON *job_id = cJSON_GetObjectItemCaseSensitive(json, field_name);
    if (cJSON_IsString(job_id) && (job_id->valuestring != NULL)) {
        strncpy(dest, job_id->valuestring, max_size);
        rv = true;
    }
    return rv;
}

bool get_object_from_reply(const cJSON *json, const char *field_name, cJSON **obj)
{
    bool rv = false;
    *obj = cJSON_GetObjectItemCaseSensitive(json, field_name);
    if (*obj && cJSON_IsObject(*obj) ) {
        rv = true;
    }
    return rv;
}

bool get_array_from_JSON(const cJSON *json, const char *field_name, cJSON **obj)
{
    bool rv = false;
    *obj = cJSON_GetObjectItemCaseSensitive(json, field_name);
    if (*obj && cJSON_IsArray(*obj) ) {
        rv = true;
    }
    return rv;
}

bool get_int_from_JSON(const cJSON *json, const char *field_name, int *dest)
{
    bool rv = false;
    cJSON *job_id = cJSON_GetObjectItemCaseSensitive(json, field_name);
    if (cJSON_IsNumber(job_id) ) {
        *dest = job_id->valueint;
        rv = true;
    }
    return rv;
}

bool get_bool_from_JSON(const cJSON *json, const char *field_name, bool *dest)
{
    bool rv = false;
    cJSON *f = cJSON_GetObjectItemCaseSensitive(json, field_name);
    if (cJSON_IsBool(f) ) {
        *dest = cJSON_IsTrue(f);
        rv = true;
    }
    return rv;
}



bool parse_JSON_reply(WQC *handler, cJSON **reply_json)
{
    bool rv = true;

    *reply_json = cJSON_Parse(handler->web_call_info.web_reply.reply);

    if (*reply_json == NULL) {

        rv = false;
        char position_str[12] = "no position";
        const char *extra_messages[] =
                {
                        "Error parsing JSON reply: Error before",
                        "(no location)",
                        " at offset " ,
                        position_str,
                        NULL
                };
        const char *error_ptr = cJSON_GetErrorPtr();

        if (error_ptr != NULL)
        {
            extra_messages[1] = error_ptr;
            snprintf(position_str, sizeof position_str, "%lu", error_ptr - handler->web_call_info.web_reply.reply);
        }
        wqc_set_error_with_messages(handler, WEBQC_WEB_CALL_ERROR, extra_messages);
    }
    return rv;
}

static bool get_string_field_from_reply(WQC *handler, const char *label, char *target, int target_len)
{
    bool rv = false;

    cJSON *reply_json = NULL;

    rv = parse_JSON_reply(handler, &reply_json);

    if ( rv ) {
        rv = get_string_from_JSON(reply_json, label, target, target_len);
        cJSON_Delete(reply_json);
    }

    return rv;
}

bool get_job_id_from_reply(WQC *handler)
{
    return get_string_field_from_reply(handler, "job_id", handler->job_id, WQC_JOB_ID_LENGTH);
}

bool get_parameter_set_id_from_reply(WQC *handler)
{
    return get_string_field_from_reply(handler, "set_id", handler->parameter_set_id, WQC_PARAM_SET_ID_LENGTH);
}

bool update_eri_job_details(WQC *handler)
{
    bool rv = false;
    cJSON *reply_json = NULL;
    cJSON *job_status = NULL;

    rv = parse_JSON_reply(handler, &reply_json);

    if ( rv ) {
        rv = get_object_from_reply(reply_json, "job_status", &job_status);
    }

    char eri_job_id[WQC_JOB_ID_LENGTH];

    if ( rv )
    {
        if ( ! (rv= get_string_from_JSON(job_status, "job_id", eri_job_id, WQC_JOB_ID_LENGTH) )) {
            wqc_set_error_with_message(handler, WEBQC_WEB_CALL_ERROR, "Cannot find ERI job ID in reply");
        }
    }

    if ( rv )
    {
        if (strncmp(handler->job_id, eri_job_id, WQC_JOB_ID_LENGTH)) {
            handler->is_duplicate = true;
        }
        memcpy(handler->job_id, eri_job_id, WQC_JOB_ID_LENGTH);
    }

    if ( reply_json ) {
        cJSON_Delete(reply_json);
    }
    return rv;

}


bool update_job_details(WQC *handler)
{
    bool rv = true;
    char reply_job_id[WQC_JOB_ID_LENGTH];

    rv=get_string_field_from_reply(handler, "job_id", reply_job_id, WQC_JOB_ID_LENGTH);

    if ( rv && memcmp(handler->job_id, reply_job_id, WQC_JOB_ID_LENGTH)) {
        const char *extra_messages[] = {"Job ID in call does not match Job ID in reply ", handler->job_id, " ", reply_job_id, NULL};
        wqc_set_error_with_messages(handler, WEBQC_WEB_CALL_ERROR, extra_messages);
        rv = false;
    }

    if ( rv ) {
        switch (handler->job_type) {
            case WQC_JOB_TWO_ELECTRONS_INTEGRALS:
                rv = update_eri_job_details(handler);
                break;
            case WQC_NULL_JOB:
            case WQC_JOB_NEW_JOB:
            case WQC_JOB_SET_PARAMETERS:
                wqc_set_error_with_message(handler, WEBQC_WEB_CALL_ERROR, "Internal Error - handling job run reply, but request was not to run a job");
                rv = false;
                break;
        }
    }
    return rv;
}

static bool
parse_eri_integral_range(WQC *handler, cJSON *item, const char *field_name, int *range)
{
    cJSON *range_array = NULL;
    int i = 0;

    bool rv = get_array_from_JSON(item, field_name, &range_array);

    if (rv && range_array) {
        cJSON *array_iterator = NULL;

        cJSON_ArrayForEach(array_iterator, range_array) {

            if (cJSON_IsNumber(array_iterator)) {
                range[i++] = array_iterator->valueint;
            } else {
                rv = false;
            }

            if (!rv || i == 4) {
                break;
            }
        }
    } else {
        wqc_set_error_with_message(handler, WEBQC_WEB_CALL_ERROR, "Cannot find range array in ERI reply");
    }

    if ( i != 4 ) {
        rv = false;
        wqc_set_error_with_message(handler, WEBQC_WEB_CALL_ERROR, "Not enough indices in ERI reply range");
    }

    return rv;
}


enum job_status_t get_status_from_string(const char *status_str, int str_max_len)
{
    if (strncmp(status_str, "done", str_max_len) == 0 ) {
        return WQC_JOB_STATUS_DONE;
    }
    if (strncmp(status_str, "pending", str_max_len) == 0 ) {
        return WQC_JOB_STATUS_PENDING;
    }
    if (strncmp(status_str, "processing", str_max_len) == 0 ) {
        return WQC_JOB_STATUS_PROCESSING;
    }
    if (strncmp(status_str, "error", str_max_len) == 0 ) {
        return WQC_JOB_STATUS_ERROR;
    }

    return WQC_JOB_STATUS_UNKNOWN;
}

static bool
parse_eri_status_item(WQC *handler, cJSON *iterator, struct ERI_item_status *status)
{
    bool rv = false;
    char status_str[16];

    rv = get_string_from_JSON(iterator, "status", status_str, sizeof status_str);
    if ( rv ) {
        status->status = get_status_from_string(status_str, sizeof status_str);
        rv = get_int_from_JSON(iterator, "id", &status->id);
    }

    if ( rv && (status->status == WQC_JOB_STATUS_DONE) ) {
        char blob_path[MAX_URL_SIZE];
        rv = get_string_from_JSON(iterator, "result_blob", blob_path, sizeof blob_path);
        status->output_blob_name = strdup(blob_path);
    }

    if (!rv) {
        wqc_set_error_with_message(handler, WEBQC_WEB_CALL_ERROR, "Cannot parse ERI items reply");
    }

    if ( rv ) {
        rv = parse_eri_integral_range(handler, iterator, "begin", status->range_begin);
    }

    if ( rv ) {
        rv = parse_eri_integral_range(handler, iterator, "end", status->range_end);
    }


    return rv;
}

static void
add_eri_status_item(WQC *handler, struct ERI_item_status *status)
{
    handler->eri_status = realloc(handler->eri_status, (handler->ERI_items_count+1) * sizeof (struct ERI_item_status));
    memcpy(&handler->eri_status[handler->ERI_items_count], status, sizeof (struct ERI_item_status));
    handler->ERI_items_count++;
    status->output_blob_name=NULL;
}

static bool
parse_eri_status_array(WQC *handler, cJSON *eri_items)
{
    bool rv = false;
    cJSON *iterator = NULL;

    cJSON_ArrayForEach(iterator, eri_items) {
        if (cJSON_IsObject(iterator)) {
            struct ERI_item_status status;
            bzero(&status, sizeof status);
            rv = parse_eri_status_item(handler, iterator, &status);
            if ( rv ) {
                add_eri_status_item(handler, &status);
            }
        } else {
            wqc_set_error_with_message(handler, WEBQC_WEB_CALL_ERROR, "Array 'items' in ERI reply must contain objects");
            rv = false;
        }
        if ( ! rv ) {
            break;
        }
    }

    return rv;
}


bool
update_eri_job_status(WQC *handler)
{
    bool rv = false;
    cJSON *reply_json = NULL;
    cJSON *eri_items = NULL;

    rv = parse_JSON_reply(handler, &reply_json);

    if ( rv ) {
        rv = get_array_from_JSON(reply_json, "items", &eri_items);
        if (rv && eri_items) {
            rv = parse_eri_status_array(handler, eri_items);
        } else {
            wqc_set_error_with_message(handler, WEBQC_WEB_CALL_ERROR, "Cannot find array 'items' in reply");
        }
    }

    if ( reply_json ) {
        cJSON_Delete(reply_json);
    }

    return rv;
}

