#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "include/webqc-errors.h"
#include "include/webqc-options.h"


#define TWO_ELECTRONS_INTEGRAL_SERVICE_ENDPOINT "eri"
#define NEW_JOB_SERVICE_ENDPOINT "job"
#define PARAMETERS_SERVICE_ENDPOINT "params"

typedef double wqc_real;

#define DEFAULT_WEBQC_SERVER_NAME "webqc.urysegal.com"
#define DEFAULT_WEBQC_SERVER_PORT (5000)


#define WQC_PRECISION_UNKNOWN ((wqc_real)0.0)  /// A number is of unknown precision
#define WQC_PRECISION_EXACT ((wqc_real)1.0)  /// A number is "exact" -- with as good a precision a wqc_real can hold

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/// When sending data to the cloud service, use these constants to specify what type is each parameter
enum wqc_data_type
{
    WQC_STRING_TYPE = 1, /// WebQC cloud call parameter is a string
    WQC_REAL_TYPE = 2 /// WebQC cloud call parameter is a real number
};

typedef struct webqc_handler_t WQC; ///< A handler to a WQC operation. When starting an asynchronous job, a WQC is returned to the caller.

/// List of all the possible calls to WecQC service
enum wqc_job_type {
    WQC_JOB_NEW_JOB = 1, /// Create a new generic job
    WQC_JOB_SET_PARAMETERS = 2, /// Set parameters for a job
    WQC_JOB_TWO_ELECTRONS_INTEGRALS = 3 /// Calculate all two-electrons repulsion integrals
};

//! Initialize the WQC library. Call once before calling any thing WQC functions.
void wqc_global_init();

//! Cleanip WQC library. Call one after finishing all calls to WQC functions.
void wqc_global_cleanup();



//! Initialize a new job handler. You must call wqc_cleanup when the job is done and you do not need any more information
//! about it.
//! \return a new WQC handler
WQC *wqc_init();

/// Cleanup a handler when you are done with it.
/// \param handler the handler to cleanup and dispoose
void wqc_cleanup(WQC *handler);

/// Reset a handler so we can make another call with same access parameters
/// \param handler the handler to reset
void wqc_reset(WQC *handler);


//! Return the error occured on the most recent API call using the givern handler
//! \param handler the handler to get error from
//! \param error_structure output paramer - the last error is copied to this pointer
//! \return true on success. If one of the parameters in null, a value of false will be returned.
bool wqc_get_last_error(
        WQC *handler,
        struct wqc_return_value *error_structure
);

/// @brief Parameters for a jobs that calculates the two-electrons repulsion integrals in the given
/// basis set on the given geormtry. The Geometry is in XYZ format. Units can be "angstrom" , "SI" for
/// picometers or "au" (atomic units).
struct two_electron_integrals_job_parameters {
    const char *basis_set_name; /// basis set name. It must be prsent on the Basis Set Exchange at https://www.basissetexchange.org/
    const char *geometry; /// XYZ-file formatted screen. FOr example, a content of an XYZ file. See for example https://openbabel.org/wiki/XYZ_(format)
    wqc_real geometry_precision;  /// A number between 0 to 1 specifying how accurate the geometry is
    const char *geometry_units; /// What units are the geomtry X/Y/Z postions in.
} ;


/// Submit an asynchronous job to the WQC server. It is not allowed to submit two jobs on the same handler without
/// calling wqc_reset on the handler first. This call will create a new job, set the parameters on it, and launch the
/// job.
/// \param handler Handler to submit the job on, created by wqc_init()
/// \param job_type Which type of job to perform
/// \param job_parameters Parameters for the job
/// \return true on successful submission of the job, false otherwise
bool wqc_submit_job
(
    WQC *handler,
    enum wqc_job_type job_type,
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

//! @brief Initialize variables of type  webqc_return_value_t.
//! \return empty webqc_return_value_t.
struct wqc_return_value init_webqc_return_value();

//! @brief return the error message associated with the error code
//! \param error_code error code
//! \return read-only string that contains the corresponding error message
const char *
wqc_get_error_by_code(
        error_code_t error_code
);


#ifdef __cplusplus
} // "extern C"
#endif
