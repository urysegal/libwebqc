#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "include/webqc-errors.h"

typedef struct webqc_handler_t WQC; ///< A handler to a WQC operation. When starting an asynchronous job, a WQC is returned to the caller.

//! Initialize a new job handler. You must call wqc_cleanup when the job is done and you do not need any more information
//! about it.
//! \return a new WQC handler
WQC *wqc_init();

/// Cleanup a handler when you are done with it.
/// \param handler the handler to cleanup and dispoose
void wqc_cleanup(WQC *handler);



//! Return the error occured on the most recent API call using the givern handler
//! \param handler the handler to get error from
//! \param error_structure output paramer - the last error is copied to this pointer
//! \return true on success. If one of the parameters in null, a value of false will be returned.
bool get_last_error(
        WQC *handler,
        webqc_return_value_t *error_structure
);

//! Submit a job to calculate all the two-electron integrals for a given geometry in XYZ format. The basis set is
//! downloaded from the Basis Set Exchange. This is the simplest case, with no screening or symmetry consideration.
//! \param handler a job handler that will make the call and keep its status
//! \param basis_set_name basis set names. It must be prsent on the Basis Set Exchange at https://www.basissetexchange.org/
//! \param geometry XYZ-file formatted screen. FOr example, a content of an XYZ file. See for example https://openbabel.org/wiki/XYZ_(format)
//! \param access_token access token to the WebQC service
//! \param errors error information in case the operation failed
//! \return true on success, false on failure

bool submit_two_electron_integrals_job
(
        WQC *handler,
        const char *basis_set_name,
        const char *geometry,
        const char *access_token,
        webqc_return_value_t *errors
) ;




