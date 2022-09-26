#include <string.h>
#include <errno.h>

#ifdef __APPLE__
#include <malloc/malloc.h>
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#include "webqc-handler.h"
#include "webqc-web-access.h"
#include "webqc-json.h"
#include "webqc-errors.h"
#include "libwebqc.h"

static void
print_system_sizes(const struct ERI_information *eri, FILE *fp)
{
    fprintf(fp,"%u atoms, %u electrons, %u shells, %u functions, %u ERI integrals (%u primitives)\n",
    eri-> number_of_atoms,
    eri-> number_of_electrons,
    eri-> number_of_shells,
    eri-> number_of_functions,
    eri-> number_of_integrals,
    eri-> number_of_primitives);
}

static void
print_primitives(const struct ERI_information *eri,  FILE *fp)
{
    fprintf(fp, "primId\tcoefficient\texponent\n");

    for (int i = 0U; i < eri->number_of_primitives; ++i) {
        const struct radial_function_info *func = &eri->basis_function_primitives[i];
        fprintf(fp, "%u\t%f\t%f\n", i, func->coefficient, func->exponent);
    }
}

static void
print_atoms(const struct ERI_information *eri,  FILE *fp)
{
    fprintf(fp, "AtomID\tSymbol\tX\t\tY\t\tZ\t\tName\n");
    unsigned int atom_id = -1;

    for (int i = 0U; i < eri->number_of_functions; ++i) {
        const struct basis_function_instance *func = &eri->basis_functions[i];

        if (atom_id == func->atom_index) {
            continue;
        }
        atom_id = func->atom_index;
        fprintf(fp, "%u\t%s\t%f\t%f\t%f\t%s\n",
                atom_id,
                func->element_symbol,
                func->origin[0], func->origin[1], func->origin[2],
                func->element_name);

    }
}

static void
print_basis_set(const struct ERI_information *eri,  FILE *fp)
{
    fprintf(fp,"AtomID\tElem\tShell\tam\tCrtsian\tFunc\tPrims\tFirst\n");
    for ( int i = 0U ; i < eri->number_of_functions ; ++i ) {
        const struct basis_function_instance *func = &eri->basis_functions[i];
        fprintf(fp, "%u\t%s\t%u\t%u\t%s\t%s\t%u\t%u\n",
                func->atom_index,
                func->element_symbol,
                func->shell_index,
                func->angular_moment_l,
                func->coordinate_type == WQC_CARTESIAN ? "Yes" : "No",
                func->function_label,
                func->number_of_primitives,
                func->first_primitives);
    }
}


void wqc_print_integrals_details( WQC *handler, FILE *fp)
{
    struct ERI_information *eri = &handler->eri_info;

    print_system_sizes(eri, fp);
    fprintf(fp, "\n");
    print_atoms(eri, fp);
    fprintf(fp, "\n");
    print_basis_set(eri, fp);
    fprintf(fp, "\n");
    print_primitives(eri, fp);
}


bool wqc_indices_equal(const eri_shell_index_t *eri_index_a, const eri_shell_index_t *eri_index_b)
{
    bool rv = true ;
    for ( unsigned int i =0 ; rv && i < 4 ; ++i ) {
        if ( (*eri_index_a)[i] != (*eri_index_b)[i] ) {
            rv = false;
        }
    }
    return rv;
}


static int
shell_index_to_shell_order(WQC *handler, const eri_shell_index_t *eri_index)
{
    unsigned int n = handler->eri_info.number_of_shells;
    unsigned int offset = 1;
    unsigned int pos = 0;

    for ( unsigned int i = 0 ; i < 4 ; i++ ) {
        pos += offset * (*eri_index)[3-i];
        offset*=n;
    }

    return pos;
}

static bool
shell_available_in_handler(WQC *handler, const eri_shell_index_t *eri_index)
{
    unsigned int first_shell_position = shell_index_to_shell_order(handler, &handler->eri_info.eri_values.begin_eri_index);
    unsigned int end_shell_position = shell_index_to_shell_order(handler, &handler->eri_info.eri_values.end_eri_index);
    unsigned int idx_position = shell_index_to_shell_order(handler, eri_index);
    return idx_position >= first_shell_position && idx_position < end_shell_position;
}


bool wqc_next_shell_index(
    WQC *handler,
    eri_shell_index_t *eri_index
)
{
    unsigned int number_of_shells = handler->eri_info.number_of_shells;
    (*eri_index)[3]++;
    if ( (*eri_index)[3] == number_of_shells ) {
        (*eri_index)[3] = 0;
        (*eri_index)[2]++;
        if ( (*eri_index)[2] == number_of_shells ) {
            (*eri_index)[2] = 0;
            (*eri_index)[1]++;
            if ((*eri_index)[1] == number_of_shells) {
                (*eri_index)[1] = 0;
                (*eri_index)[0]++;
            }
        }
    }

    return shell_available_in_handler(handler, eri_index);
}


bool wqc_get_number_of_functions_in_shells(WQC *handler, const int *shells, int *number_of_functions, int n)
{
    bool rv = true;

    if ( handler->eri_info.eri_values.eri_values) {
        int i;
        for ( i = 0 ; i < n ; i ++ ) {
            number_of_functions[i] = handler->eri_info.shell_to_function[shells[i]+1]  - handler->eri_info.shell_to_function[shells[i]] ;
        }
        rv = true;
    }  else {
        rv = false;
        wqc_set_error_with_message(handler, WEBQC_NOT_FETCHED , "Reading ERI value that was not retrieved from the WQC server");
    }

    return rv;
}


bool
wqc_get_eri_values(WQC *handler, const double **eri_values, double *eri_precision)
{
    bool rv = false;

    if ( handler->eri_info.eri_values.eri_values) {
        *eri_precision = handler->eri_info.eri_values.eri_precision;
        *eri_values = handler->eri_info.eri_values.eri_values;
        rv = true;
    }  else {
        wqc_set_error_with_message(handler, WEBQC_NOT_FETCHED , "Reading ERI value that was not retrieved from the WQC server");
    }

    return rv;
}

bool
wqc_get_shell_set_range(WQC *handler, eri_shell_index_t *begin, eri_shell_index_t *end)
{
    bool res = false;
    if ( handler->eri_info.eri_values.eri_values ) {
        memcpy(begin, handler->eri_info.eri_values.begin_eri_index, sizeof(eri_shell_index_t));
        memcpy(end, handler->eri_info.eri_values.end_eri_index, sizeof(eri_shell_index_t));
        res = true;
    }
    return res;
}

static bool
make_ERI_request_URI_parameters(WQC *handler, const eri_shell_index_t *shell_range)
{
    bool rv = false;
    char URL_with_options[MAX_URL_SIZE];

    if (snprintf(URL_with_options, MAX_URL_SIZE, "%s?%s=%s&%s=%u_%u_%u_%u", handler->web_call_info.full_URL,
                 "set_id", handler->parameter_set_id,
                 "begin", (*shell_range)[0], (*shell_range)[1], (*shell_range)[2], (*shell_range)[3]
    ) < MAX_URL_SIZE) {
        strncpy(handler
        ->web_call_info.full_URL, URL_with_options, MAX_URL_SIZE);
        curl_easy_setopt(handler->web_call_info.curl_handler, CURLOPT_URL, handler->web_call_info.full_URL);
        rv = true;
        }

    return rv;
}

bool
wqc_fetch_ERI_values(WQC *handler, const eri_shell_index_t *shell_index)
{
    bool rv = false;

    rv = prepare_web_call(handler, "eri_values");

    if ( rv ) {

        rv = make_ERI_request_URI_parameters(handler, shell_index );

        if (! rv ) {
            wqc_set_error(handler, WEBQC_OUT_OF_MEMORY); // LCOV_EXCL_LINE
        }
    }
    if ( rv ) {
        rv = make_web_call(handler);
    }

    if (rv) {
        rv = update_eri_values(handler);
        wqc_reset(handler);
    }

    return rv;
}


static bool allocate_memory_for_ERIs(WQC *handler)
{
    bool rv = true;

    free(handler->eri_info.eri_values.eri_values);

    handler->eri_info.eri_values.eri_values = malloc (handler->eri_info.eri_values.eri_data_size);

    if ( ! handler->eri_info.eri_values.eri_values ) {
        wqc_set_error_with_message(handler, WEBQC_OUT_OF_MEMORY, "Not enough memory to read ERI values"); //LCOV_EXCL_LINE
        rv = false; //LCOV_EXCL_LINE
    }

    return rv;
}

bool read_ERI_values_from_file(WQC *handler, FILE *fp)
{
    int no_of_values = handler->eri_info.eri_values.eri_data_size/(sizeof (double));

    bool rv = allocate_memory_for_ERIs(handler);

    if ( rv ) {
        if (fread(handler->eri_info.eri_values.eri_values, sizeof(double), no_of_values, fp) != no_of_values) {
            wqc_set_error_with_message(handler, WEBQC_IO_ERROR, "Cannot read ERI values from downloaded file");
            rv = false;
        }
    }
    return rv;
}


static bool download_ERI_values(WQC *handler, const char *URL)
{
    bool rv = false;
    FILE *fp = tmpfile();
    if ( ! fp ) {
        const char * messages[] = { "Cannot open temporary ERI values file" , strerror(errno), NULL} ; // LCOV_EXCL_LINE
        wqc_set_error_with_messages(handler, WEBQC_IO_ERROR, messages); // LCOV_EXCL_LINE
    } else {
        rv = wqc_download_file(handler, URL, fp);
        if ( rv ) {
            fflush(fp);
            if ( fseek(fp, 0L, SEEK_SET) < 0 ) {
                const char * messages[] = { "Cannot rewind temporary ERI values file" , strerror(errno), NULL}; // LCOV_EXCL_LINE
                wqc_set_error_with_messages(handler, WEBQC_IO_ERROR, messages);// LCOV_EXCL_LINE
            } else {
                rv = read_ERI_values_from_file(handler, fp);
            }
        }
        fclose(fp);
    }
    return rv;
}


bool update_eri_values(WQC *handler)
{
    bool rv = false;
    cJSON *reply_json = NULL;
    cJSON *begin_info = NULL;
    cJSON *end_info = NULL;
    char ERI_URL[MAX_URL_SIZE];

    rv = parse_JSON_reply(handler, &reply_json);

    if ( rv ) {

        struct json_field_info fields[] = {
            {"raw_data_url", WQC_JSON_STRING, ERI_URL, sizeof(ERI_URL)},
            {"begin",        WQC_JSON_ARRAY,  &begin_info},
            {"end",          WQC_JSON_ARRAY,  &end_info},
            {"precision",    WQC_JSON_NUMBER, &handler->eri_info.eri_values.eri_precision},
            {"size",    WQC_JSON_INT, &handler->eri_info.eri_values.eri_data_size},
            {NULL}
        };

        rv = extract_json_fields(handler, reply_json, fields);
    }

    if (rv) {
        rv = parse_int_array(handler, begin_info, handler->eri_info.eri_values.begin_eri_index, 4);
    }

    if ( rv ) {
        rv = parse_int_array(handler, end_info, handler->eri_info.eri_values.end_eri_index , 4);
    }

    if ( rv ) {
        rv = download_ERI_values(handler, ERI_URL);
    }

    if ( reply_json ) {
        cJSON_Delete(reply_json);
    }

    return rv;
}
