#pragma once
#include <stdbool.h>
#include <cjson/cJSON.h>
#include "libwebqc.h"

#ifdef __cplusplus
extern "C" {
#endif


/// Type of JSON fields, to be used to extract values from HTTP replies.
enum json_field_types {
    WQC_JSON_INT = 0, /// Integer field
    WQC_JSON_STRING = 1,  /// String field
    WQC_JSON_BOOL = 2, /// Boolean field
    WQC_JSON_ARRAY = 3, /// Array field
    WQC_JSON_NUMBER = 4 // Number field
};

/// Instructions for extracting fields from a JSON, used to extract values from HTTP replies.
struct json_field_info {
    const char *field_name; /// JSON field name
    enum json_field_types field_type; /// type of field to expect
    void *target; /// Where to put the extracted value
    unsigned int max_size; /// maximum number of bytes to copy into target. Only used for string
};


//! Extract a list of fields of given types from a JSON object returned as an HTTP reply. If there is an error, it is set on the handler.
//! the function attempts to extract all fields, even when any of them is missing.
//! \param handler Hanlder where the HTTP call was made on. Errors are set on this handler.
//! \param json_object JSON Object to extract the fields from
//! \param fields List of fields to extract
//! \return true on sucess, else false and error set on the handler.
bool extract_json_fields(
    WQC *handler,
    const cJSON *json_object,
    const struct json_field_info *fields
);


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

//! Update the handler structure with the information about ERI calculation progress, and mark the job done if all ERI are done
//! \param handler handler that just completed successfuly a call to GET on the eri endpoint
//! \return true on success, false on failure (and sets error on the handler)
bool update_eri_job_status(
        WQC *handler
);

//! Update the handler structure with the information about the ERIs.
//! \param handler handler that just completed successfully any integral-related call
//! \return true on success, false on failure (and sets error on the handler)
bool update_eri_details(
    WQC *handler
);

//! Update the handler structure with the values of the calculated ERIs.
//! \param handler handler that just completed successfully ERI calculation job
//! \return true on success, false on failure (and sets error on the handler)
bool update_eri_values(
    WQC *handler
);


//! Parse a JSON that we returned as the reply body of the most most recent HTTP call
//! \param handler Handler that call was made on
//! \param reply_json Parsed JSON will be returned in this pointer if there was no error
//! \return true on success, false on failure (and sets error on the handler)
bool parse_JSON_reply(
    WQC *handler,
    cJSON **reply_json
);


//! Get a JSON object with the given name from another JSON object
//! \param json JSON object to extract the object from
//! \param field_name field name of an object inside
//! \param obj on success, pointer will set to the field
//! \return true on success - there was an object with the given name
bool get_object_from_reply(
    const cJSON *json,
    const char *field_name,
    cJSON **obj
);

//! Get a JSON array with the given name from a JSON object
//! \param json JSON object to extract the array from
//! \param field_name field name of an array inside
//! \param obj on success, pointer will set to the field
//! \return true on success - there was an array with the given name
bool get_array_from_JSON(
    const cJSON *json,
    const char *field_name,
    cJSON **obj
);


//! Get an integer with the given name from a JSON object
//! \param json JSON object to extract the integer from
//! \param field_name field name of an array inside
//! \param value on success, pointer will set to the field
//! \return true on success - there was an integer with the given name
bool get_int_from_JSON(
    const cJSON *json,
    const char *field_name,
    int *value
);

//! Get a double precision number with the given name from a JSON object
//! \param json JSON object to extract the number from
//! \param field_name field name of an array inside
//! \param value on success, pointer will set to the field
//! \return true on success - there was an number with the given name
bool get_number_from_JSON(
    const cJSON *json,
    const char *field_name,
    double *value
);


//! Get a string with the given name from a JSON object, up to a given size
//! \param json JSON object to extract the integer from
//! \param field_name field name of an array inside
//! \param dest output buffer for the string
//! \param max_size buffer size that dest points to
//! \return true on success - there was a string with the given name
bool get_string_from_JSON(
    const cJSON *json,
    const char *field_name,
    char *dest,
    unsigned int max_size
);

//! Get a boolean with the given name from a JSON object
//! \param json JSON object to extract the boolean from
//! \param field_name field name of an array inside
//! \param value on success, pointer will set to the field
//! \return true on success - there was a boolean with the given name
bool get_bool_from_JSON(
    const cJSON *json,
    const char *field_name,
    bool *value
);


//! Parse an array of integers, optionally making sure it has at least array_size elements
//! \param handler Hanlder the reply can on, used to set the error message in case of error
//! \param array_json JSON of the array
//! \param array_out pointer to enough memory to contain array_size integers
//! \param array_size if -1, there is no limit on the array size. If not, read this number of ints from the start of the
//! array. Less than that will set an error on the handler.
//! \return true if the array was parsed, else false and error set on the handler.
bool parse_int_array(
    WQC *handler,
    const cJSON *array_json,
    int *array_out,
    int array_size
);



#ifdef __cplusplus
} // "extern C"
#endif
