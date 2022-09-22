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
        .geometry_units = "angstrom",
        .shell_set_per_file = 0
        };

    // Submit the request to perform the calculation
    bool res = wqc_submit_job(
        handler,
        WQC_JOB_TWO_ELECTRONS_INTEGRALS,
        &parameters
    ) ;


    if ( ! res ) {
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

        eri_shell_index_t eri_index = {1, 1, 0, 3};
        res = wqc_fetch_ERI_values(handler, &eri_index);

        if (res) {
            // Print out the integrals received and their accuracy
            const double *eri_values = NULL;
            double eri_precision = WQC_PRECISION_UNKNOWN;

            eri_shell_index_t eri_shell_index, eri_shell_range_end;

            wqc_get_shell_set_range(handler, &eri_shell_index, &eri_shell_range_end);

            res = wqc_get_eri_values(handler, &eri_values, &eri_precision);
            if (res) {
                int dpos = 0;

                for (; !wqc_indices_equal(&eri_shell_index, &eri_shell_range_end);
                       wqc_next_shell_index(handler, &eri_shell_index)
                    ) {

                    int shells_count[4];
                    wqc_get_number_of_functions_in_shells(handler, eri_shell_index, shells_count , 4);

                    for (int b1 = 0U; b1 < shells_count[0]; b1++) {
                        for (int b2 = 0U; b2 < shells_count[1]; b2++) {
                            for (int k1 = 0U; k1 < shells_count[2]; k1++) {
                                for (int k2 = 0U; k2 < shells_count[3]; k2++) {
                                    fprintf(stdout," %.9e\n", eri_values[dpos++]);
                                }
                            }
                        }
                    }

                }

            } else {
                struct wqc_return_value error;
                wqc_get_last_error(handler, &error);
                fprintf(stderr, "Error retrieving ERI values : %s", error.error_message);
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
