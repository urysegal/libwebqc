#include <string.h>

#include "webqc-errors.h"
#include "webqc-handler.h"
#include "libwebqc.h"

static struct webqc_error_strings {
    error_code_t error_code;
    const char *error_message;
} webqc_error_list[] = {

        {
             WEBQC_ERROR_UNKNOWN_OPTION,
            "An unknown option was passed to the API"
        } ,
        {
             WEBQC_BAD_OPTION_VALUE,
             "An illegal option value was passed"
        } ,
        {
             WEBQC_NOT_IMPLEMENTED,
             "The operation is not implemented yet"
        },
        {
            WEBQC_OUT_OF_MEMORY,
            "Out of memory"
        },
        {
            WEBQC_WEB_CALL_ERROR,
            "Error calling the Web API"
        }
};


const char *
wqc_get_error_by_code(error_code_t error_code)
{
    int error_index = 0;
    const char *res = "Unknown Error Code";

    for ( error_index = 0 ; error_index < ARRAY_SIZE(webqc_error_list) ; ++error_index ) {
        if (webqc_error_list[error_index].error_code == error_code ) {
            res =  webqc_error_list[error_index].error_message;
        }
    }
    return res;
}

void wqc_set_error(WQC *handler, error_code_t code)
{
    handler->return_value.error_code = code;
    const char *error_message = wqc_get_error_by_code(code);
    strncpy(handler->return_value.error_message, error_message, MAX_WEBQC_ERROR_MESSAGE_LEN);
}


bool wqc_get_last_error(WQC *handler, struct wqc_return_value *error_structure)
{
    bool res = false;
    if (handler && error_structure) {
        *error_structure = handler->return_value;
        res = true;
    }
    return res;
}

void wqc_set_error_with_message(WQC *handler, error_code_t code, const char *extra_message)
{
    wqc_set_error(handler, code);

    int bytes_left = MAX_WEBQC_ERROR_MESSAGE_LEN - strlen(handler->return_value.error_message);

    strncat(handler->return_value.error_message, ": ", 3);
    strncat(handler->return_value.error_message, extra_message, bytes_left - 2);
}

void wqc_set_error_with_messages(WQC *handler, error_code_t code, const char *extra_messages[])
{
    int i = 0;

    wqc_set_error(handler, code);

    int bytes_left = MAX_WEBQC_ERROR_MESSAGE_LEN - strlen(handler->return_value.error_message);


    for (i = 0 ; extra_messages[i] && bytes_left > 2 ; ++i ) {
            strncat(handler->return_value.error_message, ": ", 3);
            strncat(handler->return_value.error_message, extra_messages[i], bytes_left - 2);
            bytes_left = MAX_WEBQC_ERROR_MESSAGE_LEN - strlen(handler->return_value.error_message);
    }
}