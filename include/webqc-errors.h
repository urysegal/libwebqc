#pragma once

#define BAD_JOB_HANDLER (0) ///< Bad WebQC job ID, e.g. returned when creating a new job failed.
#define MAX_WEBQC_ERROR_MESSAGE_LEN (128) ///< Maximum size of error message in webqc_return_value_t;

#define WEBQC_SUCCESS (0) ///< Call was successful
#define WEBQC_ERROR_UNKNOWN_OPTION (1) ///< An unknown option was passed to the API

typedef uint64_t error_code_t; ///< Numerical error code

/**
 * @brief webqc_return_value_t contains all information about an error. All calls to the client library return this
 * structure.
 */

typedef struct webqc_return_value_tag {
        error_code_t error_code; ///< Error code if error or WEBQC_SUCCESS on success
        char error_message[MAX_WEBQC_ERROR_MESSAGE_LEN]; ///< Human readable error description

        int line ; ///< Line number where error occurred
        const char *file; ///< File name where error occurred
        const char *func;///< Function name where error occurred
} webqc_return_value_t;

//! @brief Initialize variables of type  webqc_return_value_t.
//! \return empty webqc_return_value_t.
webqc_return_value_t init_webqc_return_value();