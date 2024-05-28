function(find_googletest)
    message(
        STATUS
            "=================================================================")
    option(DOWNLOAD_GOOGLETEST "Download googletest library" OFF) # 选项：强制下载
    message(STATUS "Start finding third party: googletest.")
    # 判断 gtest 是否存在
    if(DOWNLOAD_GOOGLETEST OR (NOT EXISTS ${gtest_SOURCE_DIR}) OR (NOT EXISTS ${gtest_BINARY_DIR}))
        message(WARNING "Need Gtest.")

        # Options
        set(INSTALL_GTEST OFF)
        set(BUILD_SHARED_LIBS OFF)

        # Fetch
        include(FetchContent) # CMake 的 FetchContent 模块，用于远程获取第三方库
        message(STATUS "Start FetchContent_Declare: gtest.")
        FetchContent_Declare( # 声明第三方依赖
            googletest
            PREFIX "${CMAKE_BINARY_DIR}/_deps/gtest"
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0) # OVERRIDE_FIND_PACKAGE
        message(STATUS "Start FetchContent_MakeAvailable: gtest.")
        FetchContent_MakeAvailable(googletest) # 声明后完成下载与配置
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
