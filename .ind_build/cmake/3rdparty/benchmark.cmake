function(find_benchmark)
    message(
        STATUS
            "=================================================================")
    message(STATUS "Start finding third party: benchmark.")

    # find in system
    find_package(benchmark CONFIG QUIET)

    if(NOT ${benchmark_FOUND})
        message(WARNING "Benchmark NOT found in system.")

        # Options
        set(BENCHMARK_ENABLE_TESTING OFF)
        set(BENCHMARK_ENABLE_INSTALL OFF)
        set(BENCHMARK_INSTALL_DOCS OFF)

        # Fetch
        include(FetchContent)
        message(STATUS "Start FetchContent_Declare: benchmark.")
        FetchContent_Declare(
            benchmark
            PREFIX "${CMAKE_BINARY_DIR}/_deps/benchmark"
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG v1.8.0) # OVERRIDE_FIND_PACKAGE
        message(STATUS "Start FetchContent_MakeAvailable: benchmark.")
        FetchContent_MakeAvailable(benchmark)
        message(STATUS "Fetch Over: benchmark.")
    else()
        message(STATUS "Third party found: benchmark.")
    endif()

    message(
        STATUS
            "=================================================================")
endfunction(find_benchmark)
