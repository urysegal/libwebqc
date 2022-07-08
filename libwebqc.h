#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "include/webqc-errors.h"

typedef int64_t webqc_job_t; ///< When starting an asynchronous job, a webqc_job_t is returned to the caller

//! Submit a job to calculate all the two-electron integrals for a given geometry in XYZ format. The basis set is
//! downloaded from the Basis Set Exchange. This is the simplest case, with no screening or symmetry consideration.
//! \param basis_set_name basis set names. It must be prsent on the Basis Set Exchange at https://www.basissetexchange.org/
//! \param geometry XYZ-file formatted screen. FOr example, a content of an XYZ file. See for example https://openbabel.org/wiki/XYZ_(format)
//! \param access_token access token to the WebQC service
//! \param job_handler returns a job handler used to check the state of the job and retrieve the resulting integrals when the job is done
//! \param errors error information in case the operation failed
//! \return true on success, false on failure

bool submit_two_electron_integrals_job
(
        const char *basis_set_name,
        const char *geometry,
        const char *access_token,
        webqc_job_t *job_handler,
        webqc_return_value_t *errors
) ;




