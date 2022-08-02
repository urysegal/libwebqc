#include <stddef.h>
#include <string.h>

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

    wqc_init_web_calls(handler);

    return handler;
}

void wqc_reset(WQC *handler)
{
    if (handler) {
        if (handler->web_call_info.web_reply.reply) {
            free(handler->web_call_info.web_reply.reply);
            handler->web_call_info.web_reply.reply = NULL;
            handler->web_call_info.web_reply.size=0;
        }
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

        free(handler->webqc_server_name);
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


bool wqc_job_done( WQC *handler)
{
    return handler->job_status == WQC_JOB_STATUS_DONE ||
        handler->job_status == WQC_JOB_STATUS_ERROR ;
}


void wqc_global_init()
{
    web_access_init(CURL_GLOBAL_DEFAULT);
}

void wqc_global_cleanup()
{
    web_access_cleanup();
}