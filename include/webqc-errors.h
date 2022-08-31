#pragma once

#include <stdint.h>

typedef struct webqc_handler_t WQC;

#define MAX_WEBQC_ERROR_MESSAGE_LEN (1024) ///< Maximum size of error message in webqc_return_value_t;

#define WEBQC_SUCCESS (0) ///< Call was successful
#define WEBQC_ERROR_UNKNOWN_OPTION (1) ///< An unknown option was passed to the API
#define WEBQC_BAD_OPTION_VALUE (2) ///< An illegal option value was passed
#define WEBQC_NOT_IMPLEMENTED (3) ///< The operation is not yet implemented
#define WEBQC_OUT_OF_MEMORY (4) ///< Run out of memory
#define WEBQC_WEB_CALL_ERROR (5) ///< Error calling a web service
#define WEBQC_NOT_FETCHED (6) ///< A value called for was not yet fetched from the WQC server
#define WEBQC_IO_ERROR (7) ///< A Some file-related error

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

//! Set the error code on the handler. Error message is also set automatically.
//! \param handler Handler to set the error on
//! \param code the error code to set
void wqc_set_error(
        WQC *handler,
        error_code_t code
);

//! Set the error code on the handler. Error message is also set automatically, plus an
//! additional message
//! \param handler  Handler to set the error on
//! \param code the error code to set
//! \param extra_message NULL-terminated message
void wqc_set_error_with_message(
        WQC *handler,
        error_code_t code,
        const char *extra_message
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

