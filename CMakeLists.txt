cmake_minimum_required(VERSION 3.16)

project(libwebqc)

include(CTest)


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

option(COVERAGE "Coverage" ON)
set(COMPILE_FLAGS -Wall -Werror -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer -pthread)
set(LINK_FLAGS -fsanitize=address)

#SET(GCC_WERR_COMPILE_FLAGS "-Wall -Werror")
#SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_WERR_COMPILE_FLAGS} -fsanitize=address")
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

find_package(PkgConfig)

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})
include_directories(${libwebqc_SOURCE_DIR} ${libwebqc_SOURCE_DIR}/include)

add_library(libwebqc SHARED src/libwebqc.c src/webqc-options.c src/webqc-errors.c src/web_access.c src/reply_parsers.c include/webqc-json.h)

target_compile_options(libwebqc PUBLIC ${COMPILE_FLAGS})
target_link_options(libwebqc PUBLIC ${LINK_FLAGS})
target_link_libraries(libwebqc ${CURL_LIBRARIES} -lcjson)

add_executable(water-sto3g-integrals examples/water-sto3g-integrals.c)
add_dependencies(water-sto3g-integrals libwebqc)
target_include_directories(water-sto3g-integrals PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(water-sto3g-integrals PUBLIC ${COMPILE_FLAGS})
target_link_options(water-sto3g-integrals PUBLIC ${LINK_FLAGS})
target_link_libraries(water-sto3g-integrals libwebqc ${CURL_LIBRARIES})

if(COVERAGE)
    message(STATUS "Enabling coverage")
    target_compile_options(libwebqc PUBLIC --coverage)
    target_link_options(libwebqc PUBLIC --coverage)

    find_program(CODECOV_GCOV gcov)
    find_program(CODECOV_LCOV lcov)
    find_program(CODECOV_GENHTML genhtml)

    set(CODECOV_OUTPUTFILE cmake_coverage.output)
    set(CODECOV_HTMLOUTPUTDIR coverage_results)

    add_custom_target(coverage_init ALL ${CODECOV_LCOV}  --base-directory .  --directory ${CMAKE_BINARY_DIR}/CMakeFiles/libwebqc.dir/src  --exclude '/usr/include/*' --output-file ${CODECOV_OUTPUTFILE} --capture --initial )
    add_custom_target(coverage ${CODECOV_LCOV}  --base-directory .  --directory ${CMAKE_BINARY_DIR}/CMakeFiles/libwebqc.dir/src --exclude '/usr/include/*' --output-file ${CODECOV_OUTPUTFILE} --capture COMMAND genhtml -o ${CODECOV_HTMLOUTPUTDIR} ${CODECOV_OUTPUTFILE} )
endif()

add_subdirectory(test)

# first we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOC "Build documentation" )

if (BUILD_DOC)
# check if Doxygen is installed
find_package(Doxygen REQUIRED dot)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )
else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
endif()
