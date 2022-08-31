#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <curl/curl.h>

#include "libwebqc.h"

/*
 * Example program that calculates all the two-electron integrals needed for building the hamiltonian of
 * the water molecule, print information about the system and the ERIs, and prints some of ERI values.
 */


int
main()
{
    // Global initialization of the WebQC Library
    wqc_global_init();

    // Initialize one call handler
    WQC *handler = wqc_init();

    // Calculate all ERI integrals for the water molecule:

    // Specify geometry, in XYZ format
    const char *water_xyz_geometry =
        "3\n"
        "H2O\n"
        "O 0.00000000 0.00000000 -0.07223463\n"
        "H 0.83020871 0.00000000  0.53109206\n"
        "H 0.00000000 0.53109206  0.56568542\n";

    // Specify all the parameters required for ERI calculation:
    struct two_electron_integrals_job_parameters parameters =
        {
        .basis_set_name = "sto-3g",
        .geometry = water_xyz_geometry,
        .geometry_precision = WQC_PRECISION_UNKNOWN,
        .geometry_units = "angstrom"
        };

    // Submit the request to perform the calculation
    bool res = wqc_submit_job(
        handler,
        WQC_JOB_TWO_ELECTRONS_INTEGRALS,
        &parameters
    ) ;


    if ( ! res )
    {
        // There was an error submitting the job. Print what has happened.
        struct wqc_return_value error;
        wqc_get_last_error(handler, &error);
        fprintf(stderr, "Error submitting ERI calculation request: %s\n", error.error_message);
    } else {
        // wait for the ERI calculation job to finish
        res = wqc_wait_for_job(handler, 600*1000);
    }

    if ( ! res ) {
        // There was an error waiting for the job to finish.
        struct wqc_return_value error;
        wqc_get_last_error(handler, &error);
        fprintf(stderr, "Error waiting for job to finish : %s", error.error_message);
    } else {

        // Make another call on the same handler, to get information about the ERIs.
        wqc_reset(handler);
        res = wqc_get_integrals_details(handler);

        if (res) {
            // Print out all the information about the ERI.
            wqc_print_integrals_details(handler, stdout);
        }

        // Make another call on the same handler, to get some ERI values
        wqc_reset(handler);

        eri_index_t eri_range_begin = { 1,1,0,3 };
        eri_index_t eri_range_end = { 3,0,2,2 };
        res = wqc_fetch_ERI_values(handler, &eri_range_begin, &eri_range_end);

        if (res) {
            // Print out the integrals received and their accuracy
            double eri_value = 0;
            double eri_precision = WQC_PRECISION_UNKNOWN;

            for (
                eri_index_t eri_index = { eri_range_begin[0], eri_range_begin[1], eri_range_begin[2], eri_range_begin[3]} ;
                 ! wqc_eri_indices_equal(&eri_index, &eri_range_end)  ;
                 wqc_next_eri_index(handler, &eri_index)
                )
            {

                res = wqc_get_eri_value(handler, &eri_index, &eri_value, &eri_precision);
                if ( res ) {
                    printf("[%u %u | %u %u ] = %.9e (error: %f )\n",
                           eri_index[0], eri_index[1], eri_index[2], eri_index[3],
                           eri_value, eri_precision);
                } else {
                    struct wqc_return_value error;
                    wqc_get_last_error(handler, &error);
                    fprintf(stderr, "Error retrieving ERI value : %s", error.error_message);
                    break;
                }
            }
        } else {
            struct wqc_return_value error;
            wqc_get_last_error(handler, &error);
            fprintf(stderr, "Error downloading ERI values : %s", error.error_message);
        }
    }

    // Clean up the handler
    wqc_cleanup(handler);

    // Clean up the entire WQC library
    wqc_global_cleanup();

    return 0;
}
