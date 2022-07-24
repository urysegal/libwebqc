#include <stddef.h>
#include <string.h>

#ifdef __APPLE__
  #include <malloc/malloc.h>
  #include <stdlib.h>
#else
  #include <malloc.h>
#endif

#include "../libwebqc.h"
#include "../include/webqc-handler.h"
#include "../include/webqc-curl.h"
#include "../include/webqc-json.h"



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

    handler->curl_info.curl_handler = NULL;
    handler->curl_info.full_URL[0] = '\0';
    handler->curl_info.web_reply.size = 0;
    handler->curl_info.web_reply.reply = NULL;
    handler->curl_info.web_error_bufffer[0] = '\0';
    handler->curl_info.http_headers = NULL;
    handler->curl_info.http_reply_code = 0;

    return handler;
}

void wqc_reset(WQC *handler)
{
    if (handler) {
        if (handler->curl_info.web_reply.reply) {
            free(handler->curl_info.web_reply.reply);
            handler->curl_info.web_reply.reply = NULL;
            handler->curl_info.web_reply.size=0;
        }
        handler->return_value = init_webqc_return_value();
        handler->curl_info.http_reply_code = 0;
        handler->wqc_endpoint = NULL;
        cleanup_curl(handler);
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


static bool perform_REST_API_call(WQC *handler, const char *endpoint)
{
    bool rv = false;

    rv = prepare_curl(handler, endpoint);

    if ( rv ) {
        rv = make_curl_call(handler);
    }

    cleanup_curl(handler);

    return rv;
}


static bool create_new_job(WQC *handler)
{
    bool rv = false;

    rv = prepare_curl(handler, NEW_JOB_SERVICE_ENDPOINT);

    if ( rv ) {
        curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_POST, 1L);
        curl_easy_setopt(handler->curl_info.curl_handler, CURLOPT_POSTFIELDS, "{}");
        rv = make_curl_call(handler);

        if ( rv ) {
            get_job_id_from_reply(handler);
            wqc_reset(handler);
        }
    }

    cleanup_curl(handler);

    return rv;
}

static bool wqc_create_parameter_set(WQC *handler, enum wqc_job_type job_type, void *job_parameters)
{
    bool rv = false;
    const char *endpoint = NULL;

    rv = prepare_curl(handler, PARAMETERS_SERVICE_ENDPOINT);

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

        rv = make_curl_call(handler);

        if ( rv ) {
            get_parameter_set_id_from_reply(handler);
            wqc_reset(handler);
            handler->wqc_endpoint = endpoint;
        }
    }

    cleanup_curl(handler);

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
        rv = perform_REST_API_call(handler, handler->wqc_endpoint);
    }

    return rv ;
}

bool wqc_get_reply(WQC *handler)
{
    wqc_set_error(handler, WEBQC_NOT_IMPLEMENTED);
    return false;
}

void wqc_global_init()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void wqc_global_cleanup()
{
    curl_global_cleanup();
}