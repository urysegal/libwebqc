cmake_minimum_required(VERSION 3.16)
project(libwebqc)

set(CMAKE_VERBOSE_MAKEFILE ON)

SET(GCC_WERR_COMPILE_FLAGS "-Wall -Werror")
SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_WERR_COMPILE_FLAGS} -fsanitize=address")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

set(CMAKE_C_STANDARD 23)

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})


add_library(libwebqc SHARED libwebqc.c webqc-options.c webqc-errors.c web_access.c)


add_executable(water-sto3g-integrals examples/water-sto3g-integrals.c)
target_include_directories(water-sto3g-integrals PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_dependencies(water-sto3g-integrals libwebqc)
target_link_libraries(water-sto3g-integrals libwebqc ${CURL_LIBRARIES})


add_subdirectory (test)

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