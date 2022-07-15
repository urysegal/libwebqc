#pragma once
#include <stdbool.h>

typedef struct webqc_handler_t WQC; ///< A handler to a WQC operation. When starting an asynchronous job, a WQC is returned to the caller.

//! Prepare a CURL object to make a call to the WebQC server
//! \param handler handler to make a call with
//! \param web_endpoint specific service on the WebQC server
//! \return true on success, false on failure
bool prepare_curl(
        WQC *handler,
        const char *web_endpoint
);
