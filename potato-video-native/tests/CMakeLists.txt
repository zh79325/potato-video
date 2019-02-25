project(${CMAKE_PROJECT_NAME}_tests)



# Clone Google Test
set(GOOGLETEST_DIR ${CMAKE_SOURCE_DIR}/tests/lib/googletest)
if(NOT EXISTS ${GOOGLETEST_DIR})
    find_package(Git REQUIRED)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} clone https://github.com/google/googletest ${GOOGLETEST_DIR}
    )
endif()

add_subdirectory(lib/googletest)
add_subdirectory(unit_tests)

#target_link_libraries(${CMAKE_PROJECT_NAME}_tests potato_video_native_lib)