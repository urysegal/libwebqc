#include <libwebqc.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <catch2/interfaces/catch_interfaces_reporter.hpp>

class testRunListener : public Catch::EventListenerBase {
public:
    using Catch::EventListenerBase::EventListenerBase;

    virtual void testRunStarting(Catch::TestRunInfo const&) override {
        wqc_global_init();
    }

    void testRunEnded( Catch::TestRunStats const& testRunStats ) override {
        wqc_global_cleanup();
    }

};

CATCH_REGISTER_LISTENER(testRunListener)

TEST_CASE( "Unknown options get and set result in error", "[options]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);
    REQUIRE(wqc_set_option(handler, (wqc_option_t)(5748349), "hello") == false );
    REQUIRE(wqc_get_option(handler, (wqc_option_t)(5748349), "hello") == false );
    wqc_cleanup(handler);
}

TEST_CASE( "Retrieving an error", "[options]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);
    REQUIRE(wqc_set_option(handler, (wqc_option_t)(5748349), "hello") == false );
    struct wqc_return_value error_info;
    REQUIRE(wqc_get_last_error(handler, &error_info) == true);
    REQUIRE(error_info.error_code == WEBQC_ERROR_UNKNOWN_OPTION );
    REQUIRE(error_info.error_message[0] );
    wqc_cleanup(handler);
}


TEST_CASE( "string values get and set", "[options]" ) {
    const char *sample_string = "hello";
    unsigned int i = 0;
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    wqc_option_t string_options[] = {WQC_OPTION_ACCESS_TOKEN, WQC_OPTION_SERVER_NAME};

    for ( i = 0 ; i < sizeof(string_options)/sizeof(wqc_option_t) ; ++i) {
        REQUIRE(wqc_set_option(handler, string_options[i], sample_string) == true);
        const char *value = nullptr;
        REQUIRE(wqc_get_option(handler, string_options[i], &value) == true);
        REQUIRE(strcmp(value, sample_string) == 0);
    }
    wqc_cleanup(handler);
}

TEST_CASE( "bool values get and set", "[options]" ) {
    unsigned int i = 0;
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);

    wqc_option_t string_options[] = {WQC_OPTION_INSECURE_SSL};

    for ( i = 0 ; i < sizeof(string_options)/sizeof(wqc_option_t) ; ++i) {

        REQUIRE(wqc_set_option(handler, string_options[i], true) == true);
        int value = false;
        REQUIRE(wqc_get_option(handler, string_options[i], &value) == true);
        REQUIRE(value == true );

        REQUIRE(wqc_set_option(handler, string_options[i], false) == true);
        value = true;
        REQUIRE(wqc_get_option(handler, string_options[i], &value) == true);
        REQUIRE(value == false );

    }
    wqc_cleanup(handler);
}
