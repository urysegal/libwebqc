include(CTest)

SET(CMAKE_CXX_FLAGS  "${CMAKE_C_FLAGS} ${GCC_WERR_COMPILE_FLAGS} -fsanitize=address")
SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -pthread")

Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.0.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

include (Catch)

add_executable(test-options test-options.cpp)

target_include_directories(test-options PUBLIC ${CMAKE_SOURCE_DIR})
add_dependencies(test-options libwebqc)
target_link_libraries(test-options libwebqc Catch2::Catch2WithMain)

add_test(NAME test-options
        COMMAND test-options)

catch_discover_tests(test-options)
