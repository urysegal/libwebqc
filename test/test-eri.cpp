#include <libwebqc.h>
#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>
#include <unistd.h>

#include "include/webqc-json.h"
#include "include/webqc-handler.h"

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

        CHECK(wqc_submit_job(handler, WQC_JOB_TWO_ELECTRONS_INTEGRALS, &parameters) == true);
    }
    wqc_cleanup(handler);
}


TEST_CASE( "submit duplicate job", "[eri]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    char random_name[64];

    snprintf(random_name,64,"%09u_%021d", getpid(), rand());

#define RANDOM_SPACE "random_____________________________"
    const char *geometry =
            "3\n"
            RANDOM_SPACE
            "\nO 0.00000000 0.00000000 -0.07223463\n"
            "H 0.83020871 0.00000000  0.53109206\n"
            "H 0.00000000 0.53109206  0.56568542\n";

    char *geometry1 = strdup(geometry);

    memcpy(geometry1+2, random_name, std::min(strlen(random_name),sizeof(RANDOM_SPACE)));

    struct two_electron_integrals_job_parameters parameters1 = {"sto-3g", geometry1, WQC_PRECISION_UNKNOWN, "angstrom"};

    SECTION("Do REST Call") {

        CHECK(wqc_submit_job(handler, WQC_JOB_TWO_ELECTRONS_INTEGRALS, &parameters1) == true);
        CHECK(wqc_job_is_duplicate(handler) == false);
        CHECK(wqc_submit_job(handler, WQC_JOB_TWO_ELECTRONS_INTEGRALS, &parameters1) == true);
        CHECK(wqc_job_is_duplicate(handler) == true);

    }
    free(geometry1);
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


TEST_CASE( "submit integrals job to bad server", "[eri]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    SECTION("Do REST Call") {

        REQUIRE(wqc_set_option(handler, WQC_OPTION_SERVER_NAME, "nosuchserver.cam:8732") == true);
        CHECK(wqc_submit_job(handler, WQC_JOB_TWO_ELECTRONS_INTEGRALS, &parameters) == false);
    }
    wqc_cleanup(handler);
}

TEST_CASE( "try to parse a bad reply", "[eri]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    SECTION("Parse reply") {
        char illegal_JSON[] = "{ \"a\" : 7, g: 6.6}";
        size_t total_size = sizeof(illegal_JSON);

        CHECK(wqc_collect_downloaded_data(illegal_JSON, total_size, &handler->curl_info.web_reply) == total_size);

        struct wqc_return_value error_structure = init_webqc_return_value();
        cJSON *reply_json = NULL;

        CHECK(parse_JSON_reply(handler, &reply_json) == false);
        CHECK(wqc_get_last_error(handler, &error_structure) == true );
        CHECK(error_structure.error_code != 0 );
        CHECK(error_structure.error_message[0] != '\0');

    }
    wqc_cleanup(handler);
}