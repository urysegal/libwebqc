#include "libwebqc.h"
#include "include/webqc-handler.h"
#include <stddef.h>
#include <malloc.h>

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

void wqc_cleanup(WQC *handler)
{
    if ( handler->access_token )
    {
        free(handler->access_token);
        handler->access_token = NULL;
    }
}
