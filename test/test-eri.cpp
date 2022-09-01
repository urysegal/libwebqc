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

TEST_CASE("Parse Illegal integrals info replies", "[eri]") {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    SECTION("Parse illegal integrals info") {

        const char *illegals_replies[] = {
            R"json(
{ "system": {
     "number_of_atoms":1,
     "number_of_electrons" : 3,
     "number_of_functions" : 3,
     "number_of_integrals" : 13,
     "number_of_shells" : 5,
     "number_of_primitives" : 13,
     "functions": [
        { "origin":[1,2,3] ,
          "angular_moment_symbol" : "s",
          "element_name" : "element_name",
          "element_symbol" : "element_symbol",
          "function_label" : "function_label",
          "angular_moment_l" : 0,
          "atom_index" : 0,
          "shell_index" : 0,
          "atomic_number":3,
          "number_of_primitives":1,
          "spherical":true,
          "primitives":[  { "exponent:":1, "coCOefficient":5 }]
        }
      ]
    }
}
            )json",
            R"json(
{ "system": {
     "number_of_atoms":1,
     "number_of_electrons" : 3,
     "number_of_functions" : 3,
     "number_of_integrals" : 13,
     "number_of_shells" : 5,
     "number_of_primitives" : 13,
     "functions": [
        { "origin":[1,2,"FOO"] ,
          "angular_moment_symbol" : "s",
          "element_name" : "element_name",
          "element_symbol" : "element_symbol",
          "function_label" : "function_label",
          "angular_moment_l" : 0,
          "atom_index" : 0,
          "shell_index" : 0,
          "atomic_number":3,
          "number_of_primitives":1,
          "spherical":true,
          "primitives":[ { "exponent:":1, "coefficient":5 } ]
        }
      ]
    }
}
            )json",



            "{\"system\": {} }",
            "{\"functions\": [{}]}",
            "{\"system\": {\"number_of_functions\":3, \"functions\": [{}]}}",
            "{\"system\": { \"number_of_atoms\":1, \"number_of_electrons\":3, \"number_of_functions\":3, \"number_of_integrals\":13, \"number_of_shells\":5, \"number_of_primitives\":13,\"functions\": [{}]}}",

            NULL
        };

        handler->job_type = WQC_JOB_TWO_ELECTRONS_INTEGRALS;
        strncpy(handler->job_id, "job-a", sizeof(handler->job_id));

        for ( int  i = 0 ; illegals_replies[i] ; ++i ) {
            const char *weird_ERI_reply = illegals_replies[i];

            size_t total_size = strlen(weird_ERI_reply);

            CHECK(wqc_set_downloaded_data((void *)weird_ERI_reply, total_size, &handler->web_call_info.web_reply) ==
                  total_size);

            struct wqc_return_value error_structure = init_webqc_return_value();

            CHECK(update_eri_details(handler) == false);

            CHECK(wqc_get_last_error(handler, &error_structure) == true);
            CHECK(error_structure.error_code != 0);
            CHECK(error_structure.error_message[0] != '\0');
        }
    }

    wqc_cleanup(handler);
}

TEST_CASE( "submit integrals job and wait for it to finish", "[eri]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    SECTION("Do REST Call") {

        CHECK(wqc_get_status(handler) == false );
        CHECK(wqc_submit_job(handler, WQC_JOB_TWO_ELECTRONS_INTEGRALS, &parameters) == true);
        CHECK(wqc_wait_for_job(handler, 60000) == true );
        wqc_reset(handler);
        CHECK(wqc_get_integrals_details(handler) == true);
        wqc_print_integrals_details(handler, stdout);
        wqc_reset(handler);
        eri_index_t eri_range_begin = { 1,1,0,3 };
        eri_index_t eri_range_end = { 7,0,0,0 };
        CHECK(wqc_fetch_ERI_values(handler, &eri_range_begin, &eri_range_end) == true );
        double eri_value = 0;
        double eri_precision = WQC_PRECISION_UNKNOWN;

        eri_index_t out_of_range_eri_index = { 7,0,0,0};
        CHECK(wqc_get_eri_value(handler, &out_of_range_eri_index, &eri_value, &eri_precision) == false);

        struct wqc_return_value error_structure = init_webqc_return_value();
        CHECK(wqc_get_last_error(handler, &error_structure) == true );
        CHECK(error_structure.error_code == WEBQC_NOT_FETCHED );
        CHECK(error_structure.error_message[0] != '\0');

        for (
            eri_index_t eri_index = { eri_range_begin[0], eri_range_begin[1], eri_range_begin[2], eri_range_begin[3]} ;
            ! wqc_eri_indices_equal(&eri_index, &eri_range_end)  ;
            wqc_next_eri_index(handler, &eri_index)
            ) {

            CHECK( wqc_get_eri_value(handler, &eri_index, &eri_value, &eri_precision) == true );
        }

        int begin_shell_index[4] = {1,2,3,4};
        int end_shell_index[4] = {4,2,2,2};

        FILE *emptyfile= tmpfile();
        CHECK(read_ERI_values_from_file(handler, emptyfile, begin_shell_index, end_shell_index) == false);
        CHECK(wqc_get_last_error(handler, &error_structure) == true );
        CHECK(error_structure.error_code == WEBQC_IO_ERROR );
        CHECK(error_structure.error_message[0] != '\0');

        fclose(emptyfile);
    }
    wqc_cleanup(handler);
}



TEST_CASE( "submit integrals job and get status", "[eri]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    SECTION("Do REST Call") {

        CHECK(wqc_get_status(handler) == false );
        CHECK(wqc_submit_job(handler, WQC_JOB_TWO_ELECTRONS_INTEGRALS, &parameters) == true);
        CHECK(wqc_get_parameter_set_id(handler) != NULL);
        CHECK(wqc_job_done(handler) == false);
        CHECK(wqc_get_status(handler) == true );

    }
    wqc_cleanup(handler);
}


TEST_CASE( "try to parse a bad reply", "[eri]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    SECTION("Parse badly formed JSON") {
        char illegal_JSON[] = "{ \"a\" : 7, g: 6.6}";
        size_t total_size = sizeof(illegal_JSON);

        CHECK(wqc_collect_downloaded_data(illegal_JSON, total_size, &handler->web_call_info.web_reply) == total_size);

        struct wqc_return_value error_structure = init_webqc_return_value();
        cJSON *reply_json = NULL;

        CHECK(parse_JSON_reply(handler, &reply_json) == false);
        CHECK(wqc_get_last_error(handler, &error_structure) == true );
        CHECK(error_structure.error_code != 0 );
        CHECK(error_structure.error_message[0] != '\0');
    }

    SECTION("Parse illegal ERI reply") {
        char weird_ERI_reply[] = "{\"job_id\":\"job-b\" , \"job_status\" : {   }  } ";
        size_t total_size = sizeof(weird_ERI_reply);

        CHECK(wqc_collect_downloaded_data(weird_ERI_reply, total_size, &handler->web_call_info.web_reply) == total_size);

        struct wqc_return_value error_structure = init_webqc_return_value();

        handler->job_type = WQC_JOB_TWO_ELECTRONS_INTEGRALS;

        strncpy(handler->job_id, "job-a", sizeof(handler->job_id));

        CHECK(update_job_details(handler) == false);

        CHECK(wqc_get_last_error(handler, &error_structure) == true );
        CHECK(error_structure.error_code != 0 );
        CHECK(error_structure.error_message[0] != '\0');

        error_structure = init_webqc_return_value();
        strncpy(handler->job_id, "job-b", sizeof(handler->job_id));
        CHECK(update_job_details(handler) == false);

        CHECK(wqc_get_last_error(handler, &error_structure) == true );
        CHECK(error_structure.error_code != 0 );
        CHECK(error_structure.error_message[0] != '\0');

        handler->job_type = WQC_NULL_JOB;
        error_structure = init_webqc_return_value();
        CHECK(update_job_details(handler) == false);

        CHECK(wqc_get_last_error(handler, &error_structure) == true );
        CHECK(error_structure.error_code != 0 );
        CHECK(error_structure.error_message[0] != '\0');

    }

    SECTION("Parse illegal ERI item status arrays") {

        const char *illegals_replies[] = {
                "{\"job_id\": \"job-a\" , \"items\": [{\"requestor\": \"ury\", \"id\": 9, \"status\": \"processing\", \"result_blob\": null, \"begin\": [0,0,0,0], \"end\": [5, 0, 0, 0]}, \"WHATISTHAT\"]}",
                "{\"job_id\": \"job-a\" }",
                "{\"job_id\": \"job-a\" , \"items\": 8}",
                "{\"job_id\": \"job-a\", \"items\": [{\"requestor\": \"ury\", \"id\": 9, \"status\": \"error\", \"result_blob\": null, \"begin\": [\"\",0,0,0], \"end\": [5, 0, 0, 0]}]}",
                "{\"job_id\": \"job-a\", \"items\": [{\"requestor\": \"ury\", \"id\": 9, \"status\": \"pending\", \"result_blob\": null, \"begin\": [0], \"end\": [5, 0, 0, 0]}]}",
                "{\"job_id\": \"job-a\", \"items\": [{\"requestor\": \"ury\", \"id\": 9, \"status\": \"WEIRD\", \"result_blob\": null, \"begin\": [0,0,0,0]}]}",
                "{\"job_id\": \"job-a\", \"items\": [{\"requestor\": \"ury\", \"status\": \"done\", \"result_blob\": null, \"begin\": [0,0,0,0], \"end\": [5, 0, 0, 0]}]}",
                NULL
        };

        handler->job_type = WQC_JOB_TWO_ELECTRONS_INTEGRALS;
        strncpy(handler->job_id, "job-a", sizeof(handler->job_id));

        for ( int  i = 0 ; illegals_replies[i] ; ++i ) {
            const char *weird_ERI_reply = illegals_replies[i];

            size_t total_size = strlen(weird_ERI_reply);

            CHECK(wqc_set_downloaded_data((void *)weird_ERI_reply, total_size, &handler->web_call_info.web_reply) ==
                  total_size);

            struct wqc_return_value error_structure = init_webqc_return_value();

            CHECK(update_eri_job_status(handler) == false);

            CHECK(wqc_get_last_error(handler, &error_structure) == true);
            CHECK(error_structure.error_code != 0);
            CHECK(error_structure.error_message[0] != '\0');
        }
    }
    wqc_cleanup(handler);
}


TEST_CASE( "parse ERI done reply with blob", "[eri]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    SECTION("Parse reply blobs") {

        const char *blob_replies[] = {
                "{\"job_id\": \"job-a\", \"items\": [{\"requestor\": \"ury\", \"id\": 9, \"status\": \"done\", \"result_blob\": \"bloby.blob.bin\", \"begin\": [0,0,0,0], \"end\": [5, 0, 0, 0]}]}",
                NULL
        };

        handler->job_type = WQC_JOB_TWO_ELECTRONS_INTEGRALS;
        strncpy(handler->job_id, "job-a", sizeof(handler->job_id));

        for ( int  i = 0 ; blob_replies[i] ; ++i ) {
            const char *ERI_reply = blob_replies[i];

            size_t total_size = strlen(ERI_reply);

            CHECK(wqc_set_downloaded_data((void *)ERI_reply, total_size, &handler->web_call_info.web_reply) ==
                  total_size);

            CHECK(update_eri_job_status(handler) == true);
        }
    }
    wqc_cleanup(handler);
}


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
        CHECK(wqc_job_done(handler) == false);
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

TEST_CASE( "submit integrals job bad token", "[eri]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    SECTION("Do REST Call") {

        REQUIRE(wqc_set_option(handler, WQC_OPTION_ACCESS_TOKEN, "this is not a token") == true);
        REQUIRE(wqc_set_option(handler, WQC_OPTION_INSECURE_SSL, 1) == true);

        CHECK(wqc_submit_job(handler, WQC_JOB_TWO_ELECTRONS_INTEGRALS, &parameters) == false);
    }
    wqc_cleanup(handler);
}


TEST_CASE( "submit nonexistent job type", "[eri]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    REQUIRE( wqc_submit_job(handler, (enum wqc_job_type)432432, nullptr ) == false ) ;
    REQUIRE( wqc_job_done(handler) == false );
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

