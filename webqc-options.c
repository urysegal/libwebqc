#include <stdarg.h>
#include <string.h>

#ifdef __APPLE__
  #include <malloc/malloc.h>
  #include <stdlib.h>
#else
  #include <malloc.h>
#endif

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


#define STRING_OPTION_SET_FUNCTION_NAME(struct_member_name) handle_##struct_member_name##_option_set
#define STRING_OPTION_GET_FUNCTION_NAME(struct_member_name) handle_##struct_member_name##_option_get
#define BOOL_OPTION_SET_FUNCTION_NAME(struct_member_name) handle_##struct_member_name##_bool_option_set
#define BOOL_OPTION_GET_FUNCTION_NAME(struct_member_name) handle_##struct_member_name##_bool_option_get


#define STRING_OPTION_TABLE_ENTRY(option_name, struct_member_name ) { option_name,  STRING_OPTION_SET_FUNCTION_NAME(struct_member_name), STRING_OPTION_GET_FUNCTION_NAME(struct_member_name) }
#define BOOL_OPTION_TABLE_ENTRY(option_name, struct_member_name ) { option_name,  BOOL_OPTION_SET_FUNCTION_NAME(struct_member_name), BOOL_OPTION_GET_FUNCTION_NAME(struct_member_name) }

#define MAKE_STRING_OPTION_SET(struct_member_name)\
bool STRING_OPTION_SET_FUNCTION_NAME(struct_member_name) (WQC *handler, wqc_option_t option, va_list *ap)\
{\
    bool result = false;\
    const char *value = get_string_option_value(ap);\
    if ( value ) {\
        if ( handler->struct_member_name ) {\
            free(handler->struct_member_name);\
        }\
        handler->struct_member_name = strdup(value);\
        result = true;\
    } else {\
        wqc_set_error(handler, WEBQC_BAD_OPTION_VALUE);\
    }\
    return result;\
}

#define MAKE_STRING_OPTION_GET(struct_member_name)\
bool STRING_OPTION_GET_FUNCTION_NAME(struct_member_name) (WQC *handler, wqc_option_t option, va_list *ap)\
{\
    const char **valptr = get_string_ptr_option_value(ap);\
    *valptr = handler->struct_member_name;\
    return true;\
}

#define MAKE_BOOL_OPTION_SET(struct_member_name)\
bool BOOL_OPTION_SET_FUNCTION_NAME(struct_member_name) (WQC *handler, wqc_option_t option, va_list *ap)\
{\
    int value = va_arg(*ap, int);\
    handler->struct_member_name = (value!=0);\
    return true;\
}

#define MAKE_BOOL_OPTION_GET(struct_member_name)\
bool BOOL_OPTION_GET_FUNCTION_NAME(struct_member_name) (WQC *handler, wqc_option_t option, va_list *ap)\
{\
    int *valptr = va_arg(*ap, int *);\
    *valptr = handler->struct_member_name;\
    return true;\
}


MAKE_STRING_OPTION_SET(webqc_server_name)
MAKE_STRING_OPTION_GET(webqc_server_name)

MAKE_STRING_OPTION_SET(access_token)
MAKE_STRING_OPTION_GET(access_token)

MAKE_BOOL_OPTION_SET(insecure_ssl)
MAKE_BOOL_OPTION_GET(insecure_ssl)


static struct webqc_options_info {
    wqc_option_t options_value;
    option_handler_func option_handler_set;
    option_handler_func option_handler_get;
} webqc_options_hanlders[] =
        {
                STRING_OPTION_TABLE_ENTRY(WQC_OPTION_ACCESS_TOKEN, access_token),
                STRING_OPTION_TABLE_ENTRY(WQC_OPTION_SERVER_NAME, webqc_server_name),
                BOOL_OPTION_TABLE_ENTRY(WQC_OPTION_INSECURE_SSL, insecure_ssl),
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
