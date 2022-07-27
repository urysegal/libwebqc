#pragma once
#include <stdbool.h>
#include <cjson/cJSON.h>
#include "libwebqc.h"

#ifdef __cplusplus
extern "C" {
#endif

//! Get the job ID from the most recent call to the WebQC API
//! \param handler handler the call was made on
//! \return true if the job id was found. The job ID is saved in the handler.
bool get_job_id_from_reply(
        WQC *handler
);

//! Get a parameter set ID from the most recent call to the WebQC API
//! \param handler handler the call was made on
//! \return true if the job id was found. The set ID is saved in the handler.
bool get_parameter_set_id_from_reply(
        WQC *handler
);

//! Parse a WEB API JSON text reply into a cJSON structure.
//! \param handler handler where we recieved the reply
//! \param reply_json if JSON is parsed successfully, a cJSON handler. Caller must release memory with cJSON_Delete.
//! \return true on success, false on failure (and error set on the handler).
bool parse_JSON_reply(
    WQC *handler,
    cJSON **reply_json
);


//! Update the handler's detail after a job submission
//! \param handler handelr on which an HTTP reply was just recieved
//! \return true on success, false on failure (and error set on the handler).
bool update_job_details(
    WQC *handler
);


#ifdef __cplusplus
} // "extern C"
#endif
