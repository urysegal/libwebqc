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

#define TWO_ELECTRONS_INTEGRAL_SERVICE_ENDPOINT "eri"


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
    handler->access_token = NULL;
    handler->webqc_server_name = strdup(DEFAULT_WEBQC_SERVER_NAME);
    handler->webqc_server_port = DEFAULT_WEBQC_SERVER_PORT;
    handler->insecure_ssl = false;

    handler->curl_info.curl_handler = NULL;
    handler->curl_info.full_URL[0] = '\0';
    handler->curl_info.web_reply.size = 0;
    handler->curl_info.web_reply.reply = NULL;
    handler->curl_info.web_error_bufffer[0] = '\0';
    handler->curl_info.http_headers = NULL;
    handler->curl_info.http_reply_code = 0;

    return handler;
}

void wqc_cleanup(WQC *handler)
{
    if (handler) {
        if (handler->access_token) {
            free(handler->access_token);
            handler->access_token = NULL;
        }
        free(handler->webqc_server_name);
        cleanup_curl(handler);
        free(handler);
    }
}


static bool submit_2e_job(WQC *handler, const struct two_electron_integrals_job_parameters *job_parameters)
{
    bool rv = false;

    rv = prepare_curl(handler, TWO_ELECTRONS_INTEGRAL_SERVICE_ENDPOINT);

    if ( rv ) {
        rv = make_eri_request(handler, job_parameters);
    }

    if ( rv ) {
        rv = make_curl_call(handler);
    }

    cleanup_curl(handler);

    return rv;
}


bool wqc_submit_job(WQC *handler, wqc_job_type job_type, void *job_parameters)
{
    if (job_type == TWO_ELECTRONS_INTEGRAL) {
        return submit_2e_job(handler,
                             (const struct two_electron_integrals_job_parameters *)job_parameters);
    }

    wqc_set_error(handler, WEBQC_NOT_IMPLEMENTED);
    return false;
}

bool wqc_get_reply(WQC *handler)
{
    wqc_set_error(handler, WEBQC_NOT_IMPLEMENTED);
    return false;
}
