#pragma once
#include <stdbool.h>
#include "../libwebqc.h"

//! Prepare a CURL object to make a call to the WebQC server
//! \param handler handler to make a call with
//! \param web_endpoint specific service on the WebQC server
//! \return true on success, false on failure
bool prepare_curl(
        WQC *handler,
        const char *web_endpoint
);


//! Perform the call to the WebQC server.
//! \param handler handler to make the call on
//! \return true on success, false on failure. Success means the call was successful in getting a 2XX HTTP reply
bool make_curl_call(
    WQC *handler
);

//! Cleanup CURL related information. Release all memory
//! \param handler handler to clean up
//! \return true on success, false on failure
bool cleanup_curl(
    WQC *handler
);


//! Generate the parameters for calculating the Electron Repulsion Integrals (ERI).
//! \param handler handler that call will be make on
//! \param job_parameters Details of the ERI calculation desired
//! \return true on success, false on failure
bool set_eri_job_parameters(
    WQC *handler,
    const struct two_electron_integrals_job_parameters *job_parameters
);

