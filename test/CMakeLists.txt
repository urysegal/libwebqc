include(CTest)


SET(CMAKE_CXX_FLAGS  "${CMAKE_C_FLAGS} ${GCC_WERR_COMPILE_FLAGS} -fsanitize=address")
SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -pthread")

find_package(Catch2 2 REQUIRED)

add_executable(token-option token-option.cpp)

target_include_directories(token-option PUBLIC ${CMAKE_SOURCE_DIR})
add_dependencies(token-option libwebqc)
target_link_libraries(token-option libwebqc Catch2::Catch2WithMain)

add_test(NAME token-option
        COMMAND token-option)
