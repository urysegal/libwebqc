#include <libwebqc.h>
#include <catch2/catch.hpp>

TEST_CASE( "access token get and set", "[options]" ) {
    WQC *handler = wqc_init();
    REQUIRE(handler != NULL);
    REQUIRE(wqc_set_option(handler, WQC_OPTION_ACCESS_TOKEN, "hello") == true );
    wqc_cleanup(handler);
}
