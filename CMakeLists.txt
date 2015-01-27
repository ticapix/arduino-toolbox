cmake_minimum_required (VERSION 2.6)
enable_testing()
project(arduino-toolbox C CXX)

SET(TOOLBOX_DIRECTORIES AsyncComm Coroutine GPRS RingBuffer)

add_subdirectory(tests)

find_package(Doxygen)
if(DOXYGEN_FOUND)

    string(REPLACE ";" " " TOOLBOX_DIRECTORIES_DOXYGEN "${TOOLBOX_DIRECTORIES}")
#    foreach(dir ${DIRECTORIES})
#        file(RELATIVE_PATH _tmp ${CMAKE_CURRENT_BINARY_DIR} ${dir})
#        set(REL_DIRECTORIES "${REL_DIRECTORIES} ${_tmp}")
#    endforeach(dir ${DIRECTORIES})

    execute_process(COMMAND git rev-parse --abbrev-ref HEAD OUTPUT_VARIABLE GIT_BRANCH)
    string(REPLACE "\n" "" GIT_BRANCH "${GIT_BRANCH}")

    execute_process(COMMAND git rev-parse HEAD OUTPUT_VARIABLE GIT_COMMIT)
    string(REPLACE "\n" "" GIT_COMMIT "${GIT_COMMIT}")

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc-api.doxygen.in
        ${CMAKE_CURRENT_BINARY_DIR}/doc-api.doxygen
        IMMEDIATE @ONLY)

    add_custom_target(doc
        COMMAND ${CMAKE_COMMAND} -E echo_append "Building API Documentation..."
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doc-api.doxygen
        COMMAND ${CMAKE_COMMAND} -E echo "Done in ${CMAKE_CURRENT_BINARY_DIR}."
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)
