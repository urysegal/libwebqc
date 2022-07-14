#include <stdarg.h>
#include <string.h>

#include "include/webqc-handler.h"
#include "libwebqc.h"

typedef bool (*option_handler_func)(WQC *handler, wqc_option_t option, va_list *);

const char *get_string_option_value(va_list *ap)
{
    return va_arg(*ap, char *);
}

const char **get_string_ptr_option_value(va_list *ap)
{
    return va_arg(*ap, const char **);
}


bool
handle_access_token_option_set(WQC *handler, wqc_option_t option, va_list *ap)
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
        wqc_set_error(handler, WEBQC_BAD_OPTION_VALUE);
    }
    return result;
}

bool
handle_access_token_option_get(WQC *handler, wqc_option_t option, va_list *ap)
{
    const char **access_token = get_string_ptr_option_value(ap);
    *access_token = handler->access_token;
    return true;
}


static struct webqc_options_info {
    wqc_option_t options_value;
    option_handler_func option_handler_set;
    option_handler_func option_handler_get;
} webqc_options_hanlders[] =
        {
                {
                        WQC_OPTION_ACCESS_TOKEN,
                        handle_access_token_option_set,
                        handle_access_token_option_get
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

    for ( option_index = 0 ; option_index < sizeof(webqc_options_hanlders)/sizeof(struct webqc_options_info) ; ++option_index)
    {
        if (webqc_options_hanlders[option_index].options_value == option) {
            result = webqc_options_hanlders[option_index].option_handler_set(handler, option, &valist);
            option_found = true;
            break;
        }
    }

    va_end(valist);

    if ( ! option_found )
    {
        wqc_set_error(handler, WEBQC_ERROR_UNKNOWN_OPTION);
        handler->return_value.error_code = WEBQC_ERROR_UNKNOWN_OPTION;
    }

    return result ;
}

bool wqc_get_option(
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

    for ( option_index = 0 ; option_index < sizeof(webqc_options_hanlders)/sizeof(struct webqc_options_info) ; ++option_index)
    {
        if (webqc_options_hanlders[option_index].options_value == option) {
            result = webqc_options_hanlders[option_index].option_handler_get(handler, option, &valist);
            option_found = true;
            break;
        }
    }

    va_end(valist);

    if ( ! option_found )
    {
        wqc_set_error(handler, WEBQC_ERROR_UNKNOWN_OPTION);
        handler->return_value.error_code = WEBQC_ERROR_UNKNOWN_OPTION;
    }

    return result ;
}