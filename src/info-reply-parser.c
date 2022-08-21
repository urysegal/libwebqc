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

static bool
get_functions(WQC *handler, const cJSON *functions_info)
{
    bool rv = false;
    foo();
    return rv;
}


static bool
get_system_sizes(WQC *handler, const cJSON *system_info)
{
    bool rv = false;
    struct json_field_info fields[] = {
        {"number_of_atoms", WQC_JSON_INT, &handler->eri_info.number_of_atoms},
        {"number_of_electrons", WQC_JSON_INT, &handler->eri_info.number_of_electrons},
        {"number_of_functions", WQC_JSON_INT, &handler->eri_info.number_of_functions},
        {"number_of_integrals", WQC_JSON_INT, &handler->eri_info.number_of_integrals},
        {"number_of_shells", WQC_JSON_INT, &handler->eri_info.number_of_shells},
        {NULL}
    };

    rv = extract_json_fields(handler, system_info, fields);

    return rv;
}

static bool
parse_integrals_info(WQC *handler, const cJSON *system_info, const cJSON *functions_info)
{
    bool rv = false;

    rv = get_system_sizes(handler, system_info);
    if ( rv ) {
        rv = get_functions(handler, functions_info);
    }

    return rv;
}

bool
update_eri_details(WQC *handler)
{
    bool rv = false;
    cJSON *reply_json = NULL;
    cJSON *system_info = NULL;
    cJSON  *functions_info = NULL;

    rv = parse_JSON_reply(handler, &reply_json);

    if ( rv ) {
        rv = get_object_from_reply(reply_json, "system", &system_info);
        if (rv && system_info) {
            rv = get_array_from_reply(system_info, "functions", &functions_info);
            if ( rv && functions_info ) {
                rv = parse_integrals_info(handler, system_info, functions_info);
            } else {
                wqc_set_error_with_message(handler, WEBQC_WEB_CALL_ERROR, "Cannot find array 'functions' in reply");
            }
        } else {
            wqc_set_error_with_message(handler, WEBQC_WEB_CALL_ERROR, "Cannot find object 'system' in reply");
        }
    }

    if ( reply_json ) {
        cJSON_Delete(reply_json);
    }

    return rv;
}
