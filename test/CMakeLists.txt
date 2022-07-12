include(CTest)


SET(CMAKE_CXX_FLAGS  "${CMAKE_C_FLAGS} ${GCC_WERR_COMPILE_FLAGS} -fsanitize=address")
SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -pthread")

find_package(Catch2 2 REQUIRED)

add_executable(test-options test-options.cpp)

target_include_directories(test-options PUBLIC ${CMAKE_SOURCE_DIR})
add_dependencies(test-options libwebqc)
target_link_libraries(test-options libwebqc Catch2::Catch2WithMain)

add_test(NAME test-options
        COMMAND test-options)
