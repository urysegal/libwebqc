#include <string.h>
#include <cjson/cJSON.h>

#ifdef __APPLE__
#include <malloc/malloc.h>
#include <stdlib.h>
#else

#include <malloc.h>

#endif

#include "../libwebqc.h"
#include "../include/webqc-handler.h"

bool get_string_from_JSON(cJSON *json, const char *field_name, char *dest, unsigned int max_size)
{
    bool rv = false;
    cJSON *job_id = cJSON_GetObjectItemCaseSensitive(json, field_name);
    if (cJSON_IsString(job_id) && (job_id->valuestring != NULL)) {
        strncpy(dest, job_id->valuestring, max_size);
        rv = true;
    }
    return rv;
}

bool get_object_from_reply(cJSON *json, const char *field_name, cJSON **obj)
{
    bool rv = false;
    *obj = cJSON_GetObjectItemCaseSensitive(json, field_name);
    if (*obj && cJSON_IsObject(*obj) ) {
        rv = true;
    }
    return rv;
}



bool parse_JSON_reply(WQC *handler, cJSON **reply_json)
{
    bool rv = true;

    *reply_json = cJSON_Parse(handler->curl_info.web_reply.reply);

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
            snprintf(position_str, sizeof position_str, "%lu", error_ptr - handler->curl_info.web_reply.reply);
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
