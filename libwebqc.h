#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "include/webqc-errors.h"
#include "include/webqc-options.h"

#define DEFAULT_WEBQC_SERVER_NAME "cloudcompchem.ca"
#define DEFAULT_WEBQC_SERVER_PORT (443)


typedef struct webqc_handler_t WQC; ///< A handler to a WQC operation. When starting an asynchronous job, a WQC is returned to the caller.

/// List of all the possible calls to WecQC service
typedef enum wqc_job_types_tag {
    TWO_ELECTRONS_INTEGRAL = 1 /// Calculate all two-electrons repulsion integrals
} wqc_job_type;


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
        struct wqc_return_value *error_structure
);

/// @brief Parameters for a jobs that calculates the two-electrons repulsion integrals in the given
/// basis set on the given geormtry. The Geometry is in XYZ format.
struct two_electron_integrals_job_parameters {
    const char *basis_set_name; /// basis set name. It must be prsent on the Basis Set Exchange at https://www.basissetexchange.org/
    const char *geometry; /// XYZ-file formatted screen. FOr example, a content of an XYZ file. See for example https://openbabel.org/wiki/XYZ_(format)
} ;


/// Submit an asynchronous job to the WQC server. It is not alowed to submit two jobs on the same handler without
/// calling wqc_reset on the handler first.
/// \param handler Handler to submit the job on, created by wqc_init()
/// \param job_type Which type of job to perform
/// \param job_parameters Parameters for the job
/// \return true on successful submission of the job, false otherwise
bool wqc_submit_job
        (
                WQC *handler,
                wqc_job_type job_type,
                void *job_parameters
        );


/// Get a reply for the given job. If result is not yet ready, wait for it.
/// \param handler handler the job was submitted on
/// \return true on successful retrival and if the job completed successfully, false otherwise
bool wqc_get_reply
(
    WQC *handler
);


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

//! @breif Get the value of an option.
//! For string or struct values, a pointer is returned, which is const and you must not
//! change. The pointer is valid until the next call to the library with the same handler.
//! \param handler a job handler to get the option from
//! \param option which option to get
//! \param ... pointer to space that can contain the value. FOr string or struct values, a pointer to a const-pointer.
//! \return true on success, false on failure
bool wqc_get_option(
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

//! Set the error code on the handler. Error message is also set automatically, plus a NULL-terminated array
//! of additional messages
//! \param handler  Handler to set the error on
//! \param code the error code to set
//! \param extra_messages NULL-terminated array of additional messages
void wqc_set_error_with_messages(
        WQC *handler,
        error_code_t code,
        const char *extra_messages[]
);


#ifdef __cplusplus
} // "extern C"
#endif
