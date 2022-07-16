
Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.0.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

include (Catch)

include(CTest)

SET(CMAKE_CXX_FLAGS  "${CMAKE_C_FLAGS} ${GCC_WERR_COMPILE_FLAGS} -fsanitize=address")
SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -pthread")

add_executable(test-all test-options.cpp test-eri.cpp)

target_include_directories(test-all PUBLIC ${CMAKE_SOURCE_DIR})
add_dependencies(test-all libwebqc)
target_link_libraries(test-all libwebqc Catch2::Catch2WithMain ${CURL_LIBRARIES})

add_test(NAME test-all
        COMMAND test-all)

catch_discover_tests(test-all)
