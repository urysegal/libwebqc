#include "include/webqc-errors.h"
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
        }
};


const char *
wqc_get_error_by_code(error_code_t error_code)
{
    int error_index = 0;
    const char *res = "Unknown Error Code";

    for ( error_index = 0 ; error_code < sizeof webqc_error_list ; ++error_index ) {
        if (webqc_error_list[error_index].error_code == error_code ) {
            res =  webqc_error_list[error_index].error_message;
        }
    }
    return res;
}