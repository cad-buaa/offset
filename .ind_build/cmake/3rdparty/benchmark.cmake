function(find_benchmark)
    message(
        STATUS
            "=================================================================")
    option(DOWNLOAD_BENCHMARK "Download benchmark library" OFF) # 选项：强制下载
    message(STATUS "Start finding third party: benchmark.")

    # 判断 benchmark 是否存在
    if(DOWNLOAD_BENCHMARK OR (NOT EXISTS ${benchmark_SOURCE_DIR}) OR (NOT EXISTS ${benchmark_BINARY_DIR}))
        message(WARNING "Need Benchmark")

        # Options
        set(BENCHMARK_ENABLE_TESTING OFF)
        set(BENCHMARK_ENABLE_INSTALL OFF)
        set(BENCHMARK_INSTALL_DOCS OFF)

        # Fetch
        include(FetchContent)
        message(STATUS "Start FetchContent_Declare: benchmark.")
        FetchContent_Declare( # 声明第三方依赖
            benchmark
            PREFIX "${CMAKE_BINARY_DIR}/_deps/benchmark"
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG v1.8.0) # OVERRIDE_FIND_PACKAGE
        message(STATUS "Start FetchContent_MakeAvailable: benchmark.")
        FetchContent_MakeAvailable(benchmark) # 声明后完成下载与配置
        message(STATUS "Fetch Over: benchmark.")
    else()
        message(STATUS "Third party found: benchmark.")
    endif()

    message(
        STATUS
            "=================================================================")
endfunction(find_benchmark)
