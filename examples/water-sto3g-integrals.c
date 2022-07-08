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
    if ( argc != 3 )
    {
        fprintf(stderr, "Usage: %s access-token output-filename\n", argv[0]);
        exit(1);
    }
}


void
calculate_integrals(const char *access_token, webqc_job_t *job_handler)
{
    const char *water_xyz_geometry =
            "3\n"
            "H2O\n"
            "O 0.00000000 0.00000000 -0.07223463\n"
            "H 0.83020871 0.00000000  0.53109206\n"
            "H 0.00000000 0.53109206  0.56568542\n";

    webqc_return_value_t errors = init_webqc_return_value();

    bool res = submit_two_electron_integrals_job(
            "sto-3g",
            water_xyz_geometry,
            access_token,
            job_handler,
            &errors
    ) ;


    if ( ! res )
    {
        exit(1);
    }
}

void
save_integrals( webqc_job_t job_handler, const char *output_filename)
{
    webqc_return_value_t errors = init_webqc_return_value();
    errors.error_code = 0;
    if ( errors.error_code != 0 )
    {
        exit(1);
    }
}


int
main(int argc, const char *argv[])
{
    webqc_job_t job_handler = BAD_JOB_HANDLER;

    verify_arguments(argc, argv);
    const char *access_token = argv[1];
    const char *output_filename = argv[2];

    calculate_integrals(access_token, &job_handler);

    save_integrals(job_handler, output_filename);

    return 0;
}
