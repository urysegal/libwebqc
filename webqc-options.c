#include <stdarg.h>
#include <string.h>

#include "include/webqc-handler.h"
#include "libwebqc.h"

typedef bool (*option_handler_func)(WQC *handler, wqc_option_t option, va_list *);

const char *get_string_option_value(va_list *ap)
{
    return va_arg(*ap, char *);
}


bool
handle_access_token_option(WQC *handler, wqc_option_t option, va_list *ap)
{
    bool result = false;
    const char *access_token = get_string_option_value(ap);
    if ( access_token )
    {
        handler->access_token = strdup(access_token);
        result = true;
    }
    else
    {
        handler->return_value.error_code=foo;
    }
    return result;
}

static struct webqc_options_info {
    wqc_option_t options_value;
    option_handler_func option_handler;
} webqc_options_hanlders[] =
        {
                {
                        WQC_OPTION_ACCESS_TOKEN,
                        handle_access_token_option
                }
        } ;

bool wqc_set_option(
        WQC *handler,
        wqc_option_t option,
        ...
)
{
    va_list valist;
    int option_index = 0;
    bool result = false;
    bool option_found = false;

    va_start(valist, option);

    for ( option_index = 0 ; option_index < sizeof(webqc_options_hanlders) ; ++option_index)
    {
        if (webqc_options_hanlders[option_index].options_value == option) {
            result = webqc_options_hanlders[option_index].option_handler(handler, option, &valist);
            option_found = true;
            break;
        }
    }

    va_end(valist);

    if ( ! option_found )
    {
        handler->return_value.error_code = WEBQC_ERROR_UNKNOWN_OPTION;
    }

    return result ;
}
