#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "include/webqc-errors.h"
#include "include/webqc-options.h"

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
bool wqc_get_last_error(
        WQC *handler,
        webqc_return_value_t *error_structure
);

//! Submit a job to calculate all the two-electron integrals for a given geometry in XYZ format. The basis set is
//! downloaded from the Basis Set Exchange. This is the simplest case, with no screening or symmetry consideration.
//! \param handler a job handler that will make the call and keep its status
//! \param basis_set_name basis set names. It must be prsent on the Basis Set Exchange at https://www.basissetexchange.org/
//! \param geometry XYZ-file formatted screen. FOr example, a content of an XYZ file. See for example https://openbabel.org/wiki/XYZ_(format)
//! \return true on success, false on failure

bool wqc_submit_two_electron_integrals_job
(
        WQC *handler,
        const char *basis_set_name,
        const char *geometry
) ;

//! Set up an option for a WQC call
//! \param handler a job handler to set options on
//! \param option which options to set
//! \param ... value for the option
//! \return true on success, false on failure
bool wqc_set_option(
        WQC *handler,
        wqc_option_t option,
        ...
);


//! Set the error code on the handler. Error message is also set automatically.
//! \param handler Handler to set the error on
//! \param code the error code to set
void wqc_set_error(
        WQC *handler,
        error_code_t code
);
