#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <curl/curl.h>

#include "libwebqc.h"

/*
 * Example program that loads all the two-electron integrals needed for building the hamiltonian of
 * the water molecule and saves them in a file.
 *
 * Parameter is a filename to save the integrals in.
 *
 */


void
verify_arguments(int argc, const char *argv[])
{
    if ( argc != 2 )
    {
        fprintf(stderr, "Usage: %s output-filename\n", argv[0]);
        exit(0);
    }
}

bool
calculate_integrals(WQC *job_handler)
{
    bool rv = false;

    const char *water_xyz_geometry =
            "3\n"
            "H2O\n"
            "O 0.00000000 0.00000000 -0.07223463\n"
            "H 0.83020871 0.00000000  0.53109206\n"
            "H 0.00000000 0.53109206  0.56568542\n";

    struct two_electron_integrals_job_parameters parameters = { "sto-3g", water_xyz_geometry, WQC_PRECISION_UNKNOWN,
            "angstrom"};

    bool res = wqc_submit_job(
            job_handler,
            WQC_JOB_TWO_ELECTRONS_INTEGRALS,
            &parameters
    ) ;


    if ( ! res )
    {
        struct wqc_return_value error;
        wqc_get_last_error(job_handler, &error);
        fprintf(stderr, "Error %" PRIu64 ": %s\n", error.error_code, error.error_message);
    } else {
        rv = true;
    }

    return rv;
}

bool
save_integrals( WQC *handler, const char *output_filename)
{
    bool res = wqc_get_status(handler) ;

    if ( res ) {
        if ( ! wqc_job_done(handler) ) {
            res = wqc_wait_for_job(handler, 600*1000);
        }
        if ( res ) {
            //res = wqc_get_eri_results(handler);
        } else {
            fprintf(stderr, "Could not get ERI results within 10 minutes\n");
        }
    }
    if ( ! res ) {
        struct wqc_return_value error;
        wqc_get_last_error(handler, &error);
        fprintf(stderr, "Error %" PRIu64 ": %s\n", error.error_code, error.error_message);
    }
    return res;
}

bool
get_integrals_details(const char *parameter_set)
{
    bool res = true;

    return res;
}

int
main(int argc, const char *argv[])
{

    verify_arguments(argc, argv);
    const char *output_filename = argv[1];

    wqc_global_init();

    WQC *handler = wqc_init();

    bool rv = calculate_integrals(handler);

    if ( rv ) {
        rv = save_integrals(handler, output_filename);
    }

    if ( rv ) {

        wqc_reset(handler);

        rv = wqc_get_integrals_details(handler);

        if (rv) {
            wqc_print_integrals_details(handler, stdout);
        }
    }

    wqc_cleanup(handler);

    wqc_global_cleanup();

    return 0;
}
