#include <libwebqc.h>
#include <catch2/catch_test_macros.hpp>

static const char *water_xyz_geometry =
        "3\n"
        "H2O\n"
        "O 0.00000000 0.00000000 -0.07223463\n"
        "H 0.83020871 0.00000000  0.53109206\n"
        "H 0.00000000 0.53109206  0.56568542\n";

struct two_electron_integrals_job_parameters parameters = {"sto-3g", water_xyz_geometry, WQC_PRECISION_UNKNOWN, "angstrom"};

TEST_CASE( "submit integrals job", "[eri]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    SECTION("Do REST Call") {

        REQUIRE(wqc_set_option(handler, WQC_OPTION_ACCESS_TOKEN, WQC_FREE_ACCESS_TOKEN) == true);


        CHECK(wqc_submit_job(handler, WQC_JOB_TWO_ELECTRONS_INTEGRALS, &parameters) == true);
    }
    wqc_cleanup(handler);
}

TEST_CASE( "submit integrals job no SSL", "[eri]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    SECTION("Do REST Call") {

        REQUIRE(wqc_set_option(handler, WQC_OPTION_ACCESS_TOKEN, WQC_FREE_ACCESS_TOKEN) == true);
        REQUIRE(wqc_set_option(handler, WQC_OPTION_INSECURE_SSL, 1) == true);

        CHECK(wqc_submit_job(handler, WQC_JOB_TWO_ELECTRONS_INTEGRALS, &parameters) == true);
    }
    wqc_cleanup(handler);
}

TEST_CASE( "submit nonexistent job type", "[eri]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    REQUIRE( wqc_submit_job(handler, (enum wqc_job_type)432432, nullptr ) == false ) ;
    wqc_cleanup(handler);
}
