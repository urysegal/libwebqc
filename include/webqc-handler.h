#pragma once
#include <stdint.h>
#include "webqc-errors.h"

/**
 * @brief Internal structure that maintains the status of an asynchrounous WQC operation.
 */
struct webqc_handler_t {
    uint64_t handler_id; /// A unique ID for this operation
    webqc_return_value_t return_value; /// Return value from last call to WQC API
    char *access_token; /// Token that authorizes access to the WEBQC web service
};