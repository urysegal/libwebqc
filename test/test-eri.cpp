#include <libwebqc.h>
#include <catch2/catch_test_macros.hpp>

static const char *water_xyz_geometry =
        "3\n"
        "H2O\n"
        "O 0.00000000 0.00000000 -0.07223463\n"
        "H 0.83020871 0.00000000  0.53109206\n"
        "H 0.00000000 0.53109206  0.56568542\n";


TEST_CASE( "submit integrals job", "[eri]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    SECTION("Do REST Call") {

        REQUIRE(wqc_set_option(handler, WQC_OPTION_ACCESS_TOKEN, WQC_FREE_ACCESS_TOKEN) == true);

        struct two_electron_integrals_job_parameters parameters = {"sto-3g", water_xyz_geometry};

        CHECK(wqc_submit_job(handler, TWO_ELECTRONS_INTEGRAL, &parameters) == true);
    }
    wqc_cleanup(handler);
}

TEST_CASE( "submit integrals job no SSL", "[eri]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    SECTION("Do REST Call") {

        REQUIRE(wqc_set_option(handler, WQC_OPTION_ACCESS_TOKEN, WQC_FREE_ACCESS_TOKEN) == true);
        REQUIRE(wqc_set_option(handler, WQC_OPTION_INSECURE_SSL, 1) == true);

        struct two_electron_integrals_job_parameters parameters = {"sto-3g", water_xyz_geometry};

        CHECK(wqc_submit_job(handler, TWO_ELECTRONS_INTEGRAL, &parameters) == true);
    }
    wqc_cleanup(handler);
}

TEST_CASE( "submit nonexistant job ", "[eri]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    REQUIRE( wqc_submit_job(handler, (wqc_job_types_tag)432432, NULL ) == false ) ;
    wqc_cleanup(handler);
}
