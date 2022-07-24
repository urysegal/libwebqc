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

/// A name-value pair for adding multiple JSON fields at once
struct name_value_pair {
    const char *name; /// field name
    enum wqc_data_type type; /// field type
    union /// fill one of the three options below
    {
        const char *str_value;
        wqc_real real_value;
        int64_t int_value;
    } value;
};

//! Set up HTTP POST fields on a handler
//! \param handler which handler to set the POST fields on
//! \param values fields to set, array of struct name_value_pair
//! \param num_values how many members in the array of fields to add
//! \return trur on success, false on failure (and sets error on the handler)
bool set_POST_fields(
    WQC *handler,
    struct name_value_pair values[],
    size_t num_values
);

