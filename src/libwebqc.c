#include <stddef.h>
#include <string.h>
#include <unistd.h>

#ifdef __APPLE__
  #include <malloc/malloc.h>
  #include <stdlib.h>
#else
  #include <malloc.h>
#endif

#include "libwebqc.h"
#include "webqc-handler.h"
#include "webqc-web-access.h"
#include "webqc-json.h"



struct wqc_return_value init_webqc_return_value()
{
    struct wqc_return_value rv;
    rv.error_code = WEBQC_SUCCESS;
    rv.error_message[0] = '\0';
    rv.file = NULL;
    rv.func = NULL;
    rv.line = 0;
    return rv;
}

static void init_basis_functions(WQC *handler)
{
    bzero(&handler->eri_info, sizeof(handler->eri_info));
    handler->basis_functions = NULL;
    handler->basis_function_primitives = NULL;
    handler->number_of_primitives = 0;
}

static void cleanup_basis_functions(WQC *handler)
{
    if (handler->basis_functions) {
        free(handler->basis_functions);
    }
    if (handler->basis_function_primitives) {
        free(handler->basis_function_primitives);
    }
    handler->basis_functions = NULL;
    handler->basis_function_primitives = NULL;
}


WQC *wqc_init()
{
    WQC *handler = malloc(sizeof(struct webqc_handler_t));
    handler->return_value = init_webqc_return_value();
    handler->access_token = strdup(WQC_FREE_ACCESS_TOKEN);
    handler->webqc_server_name = strdup(DEFAULT_WEBQC_SERVER_NAME);
    handler->webqc_server_port = DEFAULT_WEBQC_SERVER_PORT;
    handler->insecure_ssl = false;
    handler->job_id[0] = '\0';
    handler->wqc_endpoint = NULL;
    handler->job_type = WQC_NULL_JOB;
    handler->is_duplicate = false;
    handler->job_status = WQC_JOB_STATUS_UNKNOWN;
    handler->eri_status = NULL;
    handler->ERI_items_count = 0;
    init_basis_functions(handler);

    wqc_init_web_calls(handler);

    return handler;
}

void wqc_reset(WQC *handler)
{
    if (handler) {
        reset_reply_buffer(&handler->web_call_info.web_reply);
        handler->web_call_info.http_reply_code = 0;
        handler->job_status = WQC_JOB_STATUS_UNKNOWN;
        cleanup_web_call(handler);
    }
}


void wqc_cleanup(WQC *handler)
{
    if (handler) {

        wqc_reset(handler);

        if (handler->access_token) {
            free(handler->access_token);
            handler->access_token = NULL;
        }
        if ( handler->eri_status ) {
            for ( int i = 0 ; i < handler->ERI_items_count; ++i) {
                free(handler->eri_status[i].output_blob_name);
            }
            free(handler->eri_status);
        }
        free(handler->webqc_server_name);
        cleanup_basis_functions(handler);
        free(handler);
    }
}

bool wqc_job_is_duplicate(WQC *handler)
{
    return handler->is_duplicate;
}

static bool start_wqc_job(WQC *handler)
{
    bool rv = false;

    handler->is_duplicate = false;

    rv = prepare_web_call(handler, handler->wqc_endpoint);

    if ( rv ) {
        struct name_value_pair start_job_parameters[] = {
                {"job_id",     WQC_STRING_TYPE, {.str_value=handler->job_id}},
                {"parameter_set_id",   WQC_STRING_TYPE, {.str_value=handler->parameter_set_id}}
        };
        rv = set_POST_fields(handler, start_job_parameters, ARRAY_SIZE(start_job_parameters));
    }

    if ( rv ) {
        rv = make_web_call(handler);
    }

    if (rv) {
        rv = update_job_details(handler);
        wqc_reset(handler);
    }
    cleanup_web_call(handler);

    return rv;
}


static bool create_new_job(WQC *handler)
{
    bool rv = false;

    rv = prepare_web_call(handler, NEW_JOB_SERVICE_ENDPOINT);

    if ( rv ) {
        set_no_parameters(handler);
        rv = make_web_call(handler);

        if ( rv ) {
            get_job_id_from_reply(handler);
            wqc_reset(handler);
        }
    }

    cleanup_web_call(handler);

    return rv;
}

static bool wqc_create_parameter_set(WQC *handler, enum wqc_job_type job_type, void *job_parameters)
{
    bool rv = false;
    const char *endpoint = NULL;


    rv = prepare_web_call(handler, PARAMETERS_SERVICE_ENDPOINT);

    if ( rv ) {
        if (job_type == WQC_JOB_TWO_ELECTRONS_INTEGRALS) {
            rv = set_eri_job_parameters(handler, (const struct two_electron_integrals_job_parameters *) job_parameters);
            endpoint = TWO_ELECTRONS_INTEGRAL_SERVICE_ENDPOINT;
        } else {
            wqc_set_error(handler, WEBQC_NOT_IMPLEMENTED);
            rv = false;
        }
    }

    if ( rv ) {

        rv = make_web_call(handler);

        if ( rv ) {
            get_parameter_set_id_from_reply(handler);
            wqc_reset(handler);
            handler->wqc_endpoint = endpoint;
            handler->job_type = job_type;
        }
    }

    cleanup_web_call(handler);

    return rv;
}


bool wqc_submit_job(WQC *handler, enum wqc_job_type job_type, void *job_parameters)
{
    bool rv = false;

    rv = create_new_job(handler);

    if ( rv ) {
        rv = wqc_create_parameter_set(handler, job_type, job_parameters);
    }

    if ( rv ) {
        rv = start_wqc_job(handler);
    }

    return rv ;
}

bool
wqc_get_integrals_details(WQC *handler)
{
    bool rv = false;

    rv = prepare_web_call(handler, "int_info");

    if ( rv ) {
        rv = prepare_get_parameter(handler, "set_id", handler->parameter_set_id);
    }
    if ( rv ) {
        rv = make_web_call(handler);
    }

    if (rv) {
        rv = update_eri_details(handler);
        wqc_reset(handler);
    }

    return rv;
}

static bool
get_eri_job_status(WQC *handler)
{
    bool rv = false;

    rv = prepare_web_call(handler, handler->wqc_endpoint);

    if ( rv ) {
        rv = prepare_get_parameter(handler, "job_id", handler->job_id);
    }
    if ( rv ) {
        rv = make_web_call(handler);
    }

    if (rv) {
        rv = update_eri_job_status(handler);
        wqc_reset(handler);
    }

    return rv;
}


bool wqc_get_status(WQC *handler)
{
    bool rv = false;
    if (handler->job_type == WQC_JOB_TWO_ELECTRONS_INTEGRALS) {
        rv = get_eri_job_status(handler);
    } else {
        wqc_set_error(handler, WEBQC_NOT_IMPLEMENTED);
        rv = false;
    }
    return rv;
}

static bool integrals_job_done(WQC *handler)
{
    if ( handler->ERI_items_count == 0 ) {
        return false; // We don't know what the status is
    }
    bool rv = true;
    for ( int i = 0 ; rv && i < handler->ERI_items_count ; i++ ) {
        enum job_status_t subjob_status = handler->eri_status[i].status;
        rv =  ( subjob_status == WQC_JOB_STATUS_DONE || subjob_status == WQC_JOB_STATUS_ERROR ) ;
    }
    return rv;
}


bool wqc_job_done( WQC *handler)
{
    if ( handler->job_type == WQC_JOB_TWO_ELECTRONS_INTEGRALS ) {
        return integrals_job_done(handler);
    }
    return handler->job_status == WQC_JOB_STATUS_DONE ||
        handler->job_status == WQC_JOB_STATUS_ERROR ;
}

static const useconds_t initial_sleep = 8000;
static const useconds_t max_sleep = 1000*1000000;


bool wqc_wait_for_job( WQC *handler, int64_t milliseconds_to_wait)
{
    bool rv = false;
    useconds_t microseconds_to_wait = milliseconds_to_wait * 1000;

    useconds_t microseconds_to_sleep = max_sleep;
    while ( rv == false && microseconds_to_wait > 0 ) {

        microseconds_to_sleep *= 2;
        if ( microseconds_to_sleep > max_sleep ) {
            microseconds_to_sleep = initial_sleep;
        }
        usleep(microseconds_to_sleep);
        microseconds_to_wait -= microseconds_to_sleep;

        rv = wqc_get_status(handler);
        if ( rv ) {
            rv = wqc_job_done(handler);
        }
    }
    return rv;
}


const char *
wqc_get_parameter_set_id(WQC *handler)
{
    return handler->parameter_set_id;
}


void wqc_global_init()
{
    web_access_init(CURL_GLOBAL_DEFAULT);
}

void wqc_global_cleanup()
{
    web_access_cleanup();
}