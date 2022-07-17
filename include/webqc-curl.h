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


//! Cleanup CURL related information. Release all memory
//! \param handler handler to clean up
//! \return true on success, false on failure
bool cleanup_curl(
    WQC *handler
);


//! Configure the handler with parameter to calculate the Electron Repulsion Integrals (ERI)
//! \param handler handler that call will be make on
//! \param job_parameters Details of the ERI calculation desired
//! \return true on success, false on failure
bool make_eri_request(
    WQC *handler,
    const struct two_electron_integrals_job_parameters *job_parameters
);
