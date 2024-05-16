function(find_googletest)
    message(
        STATUS
            "=================================================================")
    message(STATUS "Start finding third party: gtest.")

    # find in system
    find_package(gtest CONFIG QUIET)

    if(NOT ${gtest_FOUND})
        message(WARNING "Gtest NOT found in system.")

        # Options
        set(INSTALL_GTEST OFF)
        set(BUILD_SHARED_LIBS OFF)

        # Fetch
        include(FetchContent)
        message(STATUS "Start FetchContent_Declare: gtest.")
        FetchContent_Declare(
            googletest
            PREFIX "${CMAKE_BINARY_DIR}/_deps/gtest"
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0) # OVERRIDE_FIND_PACKAGE
        message(STATUS "Start FetchContent_MakeAvailable: gtest.")
        FetchContent_MakeAvailable(googletest)
        message(STATUS "Fetch Over: gtest.")
    else()
        message(STATUS "Third party found: gtest.")
    endif()

    message(
        STATUS
            "=================================================================")
endfunction(find_googletest)

macro(find_gtest)
    find_googletest()
endmacro(find_gtest)
