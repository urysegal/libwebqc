#include <libwebqc.h>
#include <catch2/catch_test_macros.hpp>

TEST_CASE( "Unknown options result in error", "[options]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);
    REQUIRE(wqc_set_option(handler, (wqc_option_t)(5748349), "hello") == false );
    wqc_cleanup(handler);
}

TEST_CASE( "Retrieving an error", "[options]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);
    REQUIRE(wqc_set_option(handler, (wqc_option_t)(5748349), "hello") == false );
    webqc_return_value_t error_info;
    REQUIRE(wqc_get_last_error(handler, &error_info) == true);
    REQUIRE(error_info.error_code == WEBQC_ERROR_UNKNOWN_OPTION );
    REQUIRE(error_info.error_message[0] );
    wqc_cleanup(handler);
}


TEST_CASE( "access token get and set", "[options]" ) {
    const char *sample_token = "hello";
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);
    REQUIRE(wqc_set_option(handler, WQC_OPTION_ACCESS_TOKEN, sample_token) == true );
    const char *token_value = nullptr;
    REQUIRE(wqc_get_option(handler, WQC_OPTION_ACCESS_TOKEN, &token_value) == true );
    REQUIRE(strcmp(token_value, sample_token) == 0 );
    wqc_cleanup(handler);
}
