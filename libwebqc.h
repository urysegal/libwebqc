#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "webqc-errors.h"
#include "webqc-options.h"
#include <stdio.h>

#define TWO_ELECTRONS_INTEGRAL_SERVICE_ENDPOINT "eri"
#define NEW_JOB_SERVICE_ENDPOINT "job"
#define PARAMETERS_SERVICE_ENDPOINT "params"

typedef double wqc_real;

#define DEFAULT_WEBQC_SERVER_NAME "webqc.urysegal.com"
#define DEFAULT_WEBQC_SERVER_PORT (5000)


#define WQC_PRECISION_UNKNOWN ((wqc_real)0.0)  /// A number is of unknown precision
#define WQC_PRECISION_EXACT ((wqc_real)1.0)  /// A number is "exact" -- with as good a precision a wqc_real can hold

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define MAX_ELEMENT_NAME (14) /// That would be rutherfordium
#define MAX_BASIS_FUNCTION_LABEL (32) /// e.g. d_x^2-y^2

/// When sending data to the cloud service, use these constants to specify what type is each parameter
enum wqc_data_type
{
    WQC_STRING_TYPE = 1, /// WebQC cloud call parameter is a string
    WQC_REAL_TYPE = 2 /// WebQC cloud call parameter is a real number
};

/// Types of coordinate system
enum wqc_coodinate_system
{
    WQC_CARTESIAN = 1,
    WQC_SPHERICAL = 2
};

typedef struct webqc_handler_t WQC; ///< A handler to a WQC operation. When starting an asynchronous job, a WQC is returned to the caller.

typedef double wqc_location_t[3]; /// A 3D location in the system

typedef int eri_shell_index_t[4]; /// An index of an ERI


/// List of all the possible calls to WecQC service
enum wqc_job_type {
    WQC_NULL_JOB = 0, /// Bad job type to specify unset value
    WQC_JOB_NEW_JOB = 1, /// Create a new generic job
    WQC_JOB_SET_PARAMETERS = 2, /// Set parameters for a job
    WQC_JOB_TWO_ELECTRONS_INTEGRALS = 3 /// Calculate all two-electrons repulsion integrals
};

/// Status of a job
enum job_status_t {
    WQC_JOB_STATUS_UNKNOWN = 0, /// Job staus unknown (was not fetched form WebQC server yet)
    WQC_JOB_STATUS_PENDING = 1, /// Job did not start executing
    WQC_JOB_STATUS_DONE = 2, /// Job finished successfully
    WQC_JOB_STATUS_PROCESSING = 3, /// Job executing now
    WQC_JOB_STATUS_ERROR = 4 /// Job finished with error
};

/// Status of one sub-task of the overall ERI calculation
struct ERI_item_status {
    enum job_status_t status; /// Sub-Task status
    int id ; /// Sub-Task id
    char *output_blob_name; /// If task is done, where to download the ERIs from
    int range_begin[4]; /// Beginning of range of integrals to calculate
    int range_end[4]; /// End of range of integrals to calculate
};

/// information about ERI values, and the values fetched from the server
struct ERI_values {
    eri_shell_index_t begin_eri_index; /// Index of first ERI value we have in RAM
    eri_shell_index_t end_eri_index; /// Index of end ERI (one after last) value we have in RAM
    size_t eri_data_size; /// How much memory is used, in bytes, for storing the ERIs
    double *eri_values; /// The ERI values themselves, in C ordering
    double eri_precision; /// Precision of the ERIs in RAM
};

/// Information about the system being solved
struct ERI_information {
    unsigned int number_of_atoms; /// Number of atoms in the system
    unsigned int number_of_electrons;  /// Number of electrons in the system
    unsigned int number_of_functions;  ///  Overall number of functions, including all electrons on all atoms with all orientations. E.g. P orbital gives 3 functions.
    unsigned int number_of_integrals; /// Number of ERI integrals ( number_of_functions to the power of 4 )
    unsigned int number_of_shells; /// Number of shells ( similar to number_of_functions, but not counting different orientations). E.g. P orbital is one shell.
    unsigned int number_of_primitives; /// Overall number of primitives in the entire system
    struct basis_function_instance *basis_functions; /// All basis functions instances
    struct radial_function_info *basis_function_primitives; /// All the primitives involved in the system
    unsigned int *shell_to_function; /// Map shell index to function index
    struct ERI_values eri_values; ///  ERI values fetched from the WQC server
    unsigned int next_primitive; /// Next primitive to fill up
    unsigned int next_function; /// Next function to fill up
};

/// Information about ONE function, including specific orientation. For GTO , this also includes contractions
struct basis_function_instance {
    unsigned int angular_moment_l; /// Angular moment quantum number
    char angular_moment_symbol[2]; ///  s,p,d etc.
    unsigned int atom_index; /// Running index of atoms in the system
    unsigned int shell_index; /// Running index of shells in the system
    unsigned int atomic_number; /// Atomic number of the element this function is centered on
    char element_name[MAX_ELEMENT_NAME+1]; /// Element name, e.g. Iron
    char element_symbol[4]; /// Element symbol, e.g. Fe
    char function_label[MAX_BASIS_FUNCTION_LABEL+1]; /// Full function label, e.g. "p_x^2-y^2"
    enum wqc_coodinate_system coordinate_type; /// Coordinates for the function - spherical or cartesian
    unsigned int number_of_primitives; /// Number of primitives in the contraction
    unsigned int first_primitives; /// First primitives in the contraction, index into the handler's array of primitives
    wqc_location_t origin; /// Origin of the function co-ordinates. This is where the atom is located in the system.
};

/// Information about the radial part of a basis function (eg one primitive in a contraction )
/// in Case of GTO, the radial part will have elements coefficient*exp(-exponent*x^2)
/// in Case of STO, the radial part will have elements coefficient*exp(-exponent*x)

struct radial_function_info {
    double coefficient; /// Coefficient for one primitives term
    double exponent; /// Also known as "alpha" and "zetta"
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


//! Find if the handler was called to do a job is actually a duplicate of another job which computes the same
//! calculation and is already done or running.
//! \param handler Han
//! \return
bool wqc_job_is_duplicate(
    WQC *handler /// Hanlder to check for submitting a duplicate job
);


/// @brief Parameters for a jobs that calculates the two-electrons repulsion integrals in the given
/// basis set on the given geormtry. The Geometry is in XYZ format. Units can be "angstrom" , "SI" for
/// picometers or "au" (atomic units).
struct two_electron_integrals_job_parameters {
    const char *basis_set_name; /// basis set name. It must be present on the Basis Set Exchange at https://www.basissetexchange.org/
    const char *geometry; /// XYZ-file formatted molecular system. See for example https://openbabel.org/wiki/XYZ_(format)
    wqc_real geometry_precision;  /// A number between 0 to 1 specifying how accurate the geometry is
    const char *geometry_units; /// What units are the geometry X/Y/Z positions in.
    int shell_set_per_file; /// How many shell sets to store in a file. 0 means the default (set at the server side).
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


/// Get the status of a job that was sent on the handler
/// \param handler handler the job was submitted on
/// \return true on successful retrieval of the job status, false otherwise
bool wqc_get_status
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

//! @brief Get the value of an option.
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


//! Find if a job is done on the WebQC server side.
//! \param handler handler that the job was submitted on
//! \return true if the job is done (either successfully or not), false if it's still processing
bool wqc_job_done(
    WQC *handler
);


//! @brief Wait for a job to be done, up to a given amount of time. When the function returns, you can call wqc_get_status to get the status of the job.
//! The function will return early (with false value) if an error was encountered. It will call the WebQC server with an exponential backoff,
//! every 1,2,4,8,16,32, 64 seconds, and then again in this sequence.
//! \param handler Handler to the job. Job should have been submitted with wqc_submit_job()
//! \param milliseconds_to_wait how many milliseconds to wait for the job to be done.
//! \return true if the job is done by the time seconds_to_wait has passed.
bool wqc_wait_for_job(
        WQC *handler,
        int64_t milliseconds_to_wait
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

//! Get the parameter set used for the job.
//! \param handler Hanlder that were used to submit the job
//! \return pointer to a read-only null-terminated paramter set id string.

const char *
wqc_get_parameter_set_id(
    WQC *handler
);

//! Get details about integrals calculated by any integral-related job that was submitted with the handler. You
//! have to call wqc_reset on the handler between submitting the integrals job and calling this function.
//! \param handler hanlder that an integral job was called on
//! \return true on success, false otherwise

bool
wqc_get_integrals_details(
    WQC *handler
);


//! Print full details about ERIs
//! \param handler Hanlder where the integrals details were fetched
//! \param fp File pointer to print to
void
wqc_print_integrals_details(
    WQC *handler,
    FILE *fp
);

//! Fetch from the WQC server some range of ERI that were calculated using the handler, that was calculated for
//! the functions at eri_index.
//! \param handler A handler where a ERI calculation was submitted on
//! \param eri_index the 4 centers for which to fetch the ERI
//! \return true on success, false on failure and set up the error description in the handler
bool
wqc_fetch_ERI_values(
    WQC *handler,
    const eri_shell_index_t *eri_index
);


//! When iterating over shells sequentially, use this function to increment the shell index to the next position.
//! \param handler Handler where a call to calculate ERIs was made on
//! \param eri_index  in - Current index ; out - next index position
//! \return true if the index reached is actually stored in the handler.
bool
wqc_next_shell_index(
    WQC *handler,
    eri_shell_index_t *eri_index
);


//! Check if two ERI indices are equal. Used to check end of iteration over a range of ERIs.
//! \param eri_index_a  first index to compare
//! \param eri_index_b second index to compare
//! \return true if the indices are equal.
bool
wqc_indices_equal(
    const eri_shell_index_t *eri_index_a,
    const eri_shell_index_t *eri_index_b
);

/// Get the begin position and end position of the ERI that were calculated and downloaded.
/// \param handler Handler the ERI calculation was called on
/// \param begin output - first shell index whose ERI are available to use
/// \param end output - end shell index whose ERI are available to use
//! \return false if there were now ERIs downloaded at all. Else, returns true.
bool wqc_get_shell_set_range(WQC *handler,
     eri_shell_index_t *begin,
     eri_shell_index_t *end);


//! Get the values of the ERIs, after it was fetched from the WQC server
//! \param handler Handler the ERI calculation was called on
//! \param eri_value output - the values of the integrals - all possible orientations in each shell (see documentation)
//! \param eri_precision output - the precision of the value returned
//! \return true if the integral value exists in the handler and is returned. False otherwise, and error set the handler
bool
wqc_get_eri_values(
    WQC *handler,
    const double **eri_values,
    double *eri_precision
);

/// Get the number of functions that are in each of the n shell. For example, in a p shell there are 3. This shell indices
/// are listen in the ERI information structure.
/// \param handler Handler the ERI calculation was called on
/// \param shells List of shell indices that you are interested in
/// \param number_of_functions output - per each shell, how many functions
/// \param n how many shells you are interested in getting information about
/// \return true if the information was fetched from the WQC server and all shell sized found. False otherwise
bool wqc_get_number_of_functions_in_shells(
    WQC *handler,
    const int *shells,
    int *number_of_functions,
    int n
);

#ifdef __cplusplus
} // "extern C"
#endif
