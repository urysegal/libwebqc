#include <stddef.h>
#include <malloc.h>
#include <string.h>

#include "libwebqc.h"
#include "include/webqc-handler.h"

webqc_return_value_t init_webqc_return_value()
{
    webqc_return_value_t rv;
    rv.error_code = WEBQC_SUCCESS;
    rv.error_message[0]= '\0';
    rv.file = NULL;
    rv.func = NULL;
    rv.line=0;
    return  rv;
}

WQC *wqc_init()
{
    WQC *handler = malloc(sizeof (struct webqc_handler_t));
    handler->return_value = init_webqc_return_value();
    return handler;
}

void wqc_cleanup(WQC *handler)
{
    if ( handler ) {
        if (handler->access_token) {
            free(handler->access_token);
            handler->access_token = NULL;
        }
        free (handler);
    }
}

void wqc_set_error(WQC *handler, error_code_t code)
{
    handler->return_value.error_code = code;
    const char *error_message = wqc_get_error_by_code(code);
    strncpy(handler->return_value.error_message, error_message, MAX_WEBQC_ERROR_MESSAGE_LEN);
}


bool wqc_get_last_error(WQC *handler, webqc_return_value_t *error_structure)
{
    bool res = false;
    if ( handler && error_structure )
    {
        *error_structure = handler->return_value;
        res = true;
    }
    return res;
}
