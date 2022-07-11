#include <stdio.h>
#include <stdlib.h>

#include "libwebqc.h"

/*
 * Example program that loads all the two-electron integrals needed for the water molecule and saves them in
 * a local file.
 *
 * Parameters are the access token and a filename to save the integrals in.
 *
 */


void
verify_arguments(int argc, const char *argv[])
{
    if ( argc != 2 )
    {
        fprintf(stderr, "Usage: %s output-filename\n", argv[0]);
        exit(1);
    }
}


void
calculate_integrals(WQC *job_handler)
{
    const char *water_xyz_geometry =
            "3\n"
            "H2O\n"
            "O 0.00000000 0.00000000 -0.07223463\n"
            "H 0.83020871 0.00000000  0.53109206\n"
            "H 0.00000000 0.53109206  0.56568542\n";


    bool res = wqc_submit_two_electron_integrals_job(
            job_handler,
            "sto-3g",
            water_xyz_geometry
    ) ;


    if ( ! res )
    {
        webqc_return_value_t error;
        wqc_get_last_error(job_handler, &error);
        fprintf(stderr, "Error %lu: %s\n", error.error_code, error.error_message);
        exit(1);
    }
}

void
save_integrals( WQC *job_handler, const char *output_filename)
{
}


int
main(int argc, const char *argv[])
{

    verify_arguments(argc, argv);
    const char *output_filename = argv[1];

    WQC *handler = wqc_init();

    wqc_set_option(handler, WQC_OPTION_ACCESS_TOKEN, WQC_FREE_ACCESS_TOKEN);

    calculate_integrals(handler);

    save_integrals(handler, output_filename);

    wqc_cleanup(handler);

    return 0;
}
