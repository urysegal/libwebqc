#pragma once

#include <stdint.h>


#define MAX_WEBQC_ERROR_MESSAGE_LEN (128) ///< Maximum size of error message in webqc_return_value_t;

#define WEBQC_SUCCESS (0) ///< Call was successful
#define WEBQC_ERROR_UNKNOWN_OPTION (1) ///< An unknown option was passed to the API
#define WEBQC_BAD_OPTION_VALUE (2) ///< An illegal option value was passed
#define WEBQC_NOT_IMPLEMENTED (3) ///< The operation is not yet implemented
#define WEBQC_OUT_OF_MEMORY (4) ///< Run out of memory

typedef uint64_t error_code_t; ///< Numerical error code

/**
 * @brief webqc_return_value_t contains all information about an error. All calls to the client library return this
 * structure.
 */

struct wqc_return_value {
        error_code_t error_code; ///< Error code if error or WEBQC_SUCCESS on success
        char error_message[MAX_WEBQC_ERROR_MESSAGE_LEN]; ///< Human readable error description

        int line ; ///< Line number where error occurred
        const char *file; ///< File name where error occurred
        const char *func;///< Function name where error occurred
} ;

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
