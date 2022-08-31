#include <string.h>
#include <cjson/cJSON.h>

#ifdef __APPLE__
#include <malloc/malloc.h>
#include <stdlib.h>
#else

#include <malloc.h>

#endif

#include <assert.h>

#include "webqc-handler.h"
#include "webqc-json.h"

static const char *JSON_field_types[] = { "integer", "string", "boolean" , "array", "number"};

static const char *get_JSON_field_type_name(enum json_field_types t)
{
    return JSON_field_types[t];
}

static bool
extract_one_json_field(WQC *handler, const cJSON *json_object, const struct json_field_info *field)
{
    bool rv = false;

    switch (field->field_type) {
        case WQC_JSON_INT:
            rv = get_int_from_JSON(json_object, field->field_name, (int*)(field->target));
            break;
        case WQC_JSON_STRING:
            rv = get_string_from_JSON(json_object, field->field_name, (char *)(field->target), field->max_size);
            break;
        case WQC_JSON_BOOL:
            rv = get_bool_from_JSON(json_object, field->field_name, (bool*)(field->target));
            break;
        case WQC_JSON_ARRAY:
            rv = get_array_from_JSON(json_object, field->field_name, (cJSON **)(field->target));
            break;
        case WQC_JSON_NUMBER:
            rv = get_number_from_JSON(json_object, field->field_name, (double *)(field->target));
            break;
    }

    if ( ! rv ) {
        const char *extra_messages[] = {"Cannot find field ",field->field_name, " type " ,
                                        get_JSON_field_type_name(field->field_type), " in JSON reply", NULL};
        wqc_set_error_with_messages(handler, WEBQC_WEB_CALL_ERROR, extra_messages);
    }

    return rv;
}


bool
extract_json_fields(WQC *handler, const cJSON *json_object, const struct json_field_info *fields)
{
    bool rv = true;

    const struct json_field_info *field = fields;
    for ( field = fields ; field->field_name ; ++field ) {
        if ( ! extract_one_json_field(handler, json_object, field) ) {
            rv = false;
        }
    }

    return rv;
}

static void add_function_to_basis_set(WQC *handler, struct basis_function_instance *function_instance)
{
    struct ERI_information *eri_info = & handler->eri_info;
    if ( eri_info->basis_functions == NULL) {
        eri_info->basis_functions = malloc ( eri_info->number_of_functions * sizeof (struct basis_function_instance));
    }
    if ( eri_info->next_function == 0 ) {
        function_instance->first_primitives = 0;
    } else {
        function_instance->first_primitives = eri_info->basis_functions[eri_info->next_function-1].first_primitives+
            eri_info->basis_functions[eri_info->next_function-1].number_of_primitives;
    }
    memcpy(&(eri_info->basis_functions[eri_info->next_function++]), function_instance, sizeof (*function_instance));
}

static void calculate_shell_to_function_mapping(WQC *handler)
{
    struct ERI_information *eri_info = & handler->eri_info;
    if ( eri_info->shell_to_function ) {
        free(eri_info->shell_to_function);
    }
    eri_info->shell_to_function = calloc( eri_info->number_of_shells +1, sizeof(int) );
    for ( int i = 0 ; i < eri_info->number_of_functions ; i++ ) {
        assert(eri_info->basis_functions[i].shell_index< eri_info->number_of_shells);
        if (eri_info->basis_functions[i].shell_index == eri_info->number_of_shells-1 ) {
            break;
        }
        eri_info->shell_to_function[eri_info->basis_functions[i].shell_index+1]++;
    }
    for ( int i = 1 ; i < eri_info->number_of_shells ; i++ ) {
        eri_info->shell_to_function[i]+=eri_info->shell_to_function[i-1];
    }
    eri_info->shell_to_function[eri_info->number_of_shells] = eri_info->number_of_functions;
}

static void add_radial_info_to_basis_set(WQC *handler, struct radial_function_info *radial_info)
{
    struct ERI_information *eri_info = & handler->eri_info;
    if ( eri_info->basis_function_primitives == NULL ) {
        eri_info->basis_function_primitives = malloc(eri_info->number_of_primitives * sizeof(struct radial_function_info));
    }
    memcpy(&(eri_info->basis_function_primitives[eri_info->next_primitive++]), radial_info, sizeof (*radial_info));
}

static bool get_primitives_info(WQC *handler, struct basis_function_instance *function_instance, const cJSON *primitives_info)
{
    bool rv = false;
    cJSON *primitive = NULL;

    cJSON_ArrayForEach(primitive, primitives_info) {
        struct radial_function_info radial_info;
        struct json_field_info fields[] = {
            {"coefficient", WQC_JSON_NUMBER, &radial_info.coefficient},
            {"exponent", WQC_JSON_NUMBER, &radial_info.exponent},
            {NULL}
        };

        rv = extract_json_fields(handler, primitive, fields);

        if ( rv ) {
            add_radial_info_to_basis_set(handler, &radial_info);
        } else {
            break;
        }
    }

    return rv;
}

static bool get_origin_info(WQC *handler, struct basis_function_instance *function_instance, const cJSON *origin_info)
{
    bool rv = true;
    const cJSON *val = NULL;
    int i = 0;

    cJSON_ArrayForEach(val, origin_info) {
        if (cJSON_IsNumber(val) ) {
            function_instance->origin[i++] = cJSON_GetNumberValue(val);
        } else {
            wqc_set_error_with_message(handler, WEBQC_WEB_CALL_ERROR, "Non-Number in origin array field on a basis function");
            rv = false;
        }
        if ( i == 3) {
            break;
        }
    }
    return rv;
}

static bool
get_function_info(WQC *handler, struct basis_function_instance *function_instance, const cJSON *function_info)
{
    bool rv = false;
    bool spherical = false;
    cJSON *primitives_info = NULL;
    cJSON *origin_info = NULL;

    struct json_field_info fields[] = {
        {"angular_moment_symbol", WQC_JSON_STRING, &function_instance->angular_moment_symbol, sizeof(function_instance->angular_moment_symbol)},
        {"element_name", WQC_JSON_STRING, &function_instance->element_name, sizeof(function_instance->element_name)},
        {"element_symbol", WQC_JSON_STRING, &function_instance->element_symbol, sizeof(function_instance->element_symbol)},
        {"function_label", WQC_JSON_STRING, &function_instance->function_label , sizeof(function_instance->function_label)},
        {"angular_moment_l", WQC_JSON_INT, &function_instance->angular_moment_l},
        {"atom_index", WQC_JSON_INT, &function_instance->atom_index},
        {"shell_index", WQC_JSON_INT, &function_instance->shell_index},
        {"atomic_number", WQC_JSON_INT, &function_instance->atomic_number},
        {"number_of_primitives", WQC_JSON_INT, &function_instance->number_of_primitives},
        {"spherical", WQC_JSON_BOOL, &spherical},
        {"primitives", WQC_JSON_ARRAY, &primitives_info},
        {"origin", WQC_JSON_ARRAY, &origin_info},
        {NULL}
    };

    rv = extract_json_fields(handler, function_info, fields);

    if ( rv ) {
        if ( spherical ) {
            function_instance->coordinate_type = WQC_SPHERICAL;
        } else {
            function_instance->coordinate_type = WQC_CARTESIAN;
        }
        rv = get_origin_info(handler, function_instance, origin_info);
    }

    if ( rv ) {
        rv = get_primitives_info(handler, function_instance, primitives_info);
    }

    return rv;

}


static bool
get_functions(WQC *handler, const cJSON *functions_info)
{
    bool rv = false;
    const cJSON *function = NULL;

    cJSON_ArrayForEach(function, functions_info) {
        struct basis_function_instance function_instance;
        bzero(&function_instance, sizeof function_instance);
        rv = get_function_info(handler, &function_instance, function);
        if ( rv ) {
            add_function_to_basis_set(handler, &function_instance);
        } else {
            break;
        }
    }
    if (rv) {
        calculate_shell_to_function_mapping(handler);
    }
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
        {"number_of_primitives", WQC_JSON_INT, &handler->eri_info.number_of_primitives},
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
            rv = get_array_from_JSON(system_info, "functions", &functions_info);
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
