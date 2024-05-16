cmake_minimum_required(VERSION 3.27)

include(config.cmake)

# ---------------------------------------------------------------------------------------
# Config Preprocess
# ---------------------------------------------------------------------------------------
string(TOLOWER ${MODULE_NAME} MODULE_NAME_LOWER)
string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)
set(MODULE_VERSION
    "${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}.${MODULE_VERSION_PATCH}")

# build option
option(MODULE_IND_BUILD "build as module(OFF) or as independent project(ON)" ON
)# module独立编译选项

if(${PROJ_SUPER_BUILD})
    set(MODULE_IND_BUILD OFF) # 如果通过上层构建，则强制更改为“作为模块build”
endif()

set(MODULE_SUPER_BUILD ${MODULE_IND_BUILD}) # independent build IS super-build

# ---------------------------------------------------------------------------------------
# Start project
# ---------------------------------------------------------------------------------------
project(
    ${MODULE_NAME_LOWER}
    VERSION ${MODULE_VERSION}
    DESCRIPTION ""
    HOMEPAGE_URL ""
    LANGUAGES CXX C)
message(STATUS "Build ${PROJECT_NAME}: ${PROJECT_VERSION}")
set(MODULE_TARGET_NAME ${PROJECT_NAME})

# ---------------------------------------------------------------------------------------
# Create module header folder and Generate version.h
# ---------------------------------------------------------------------------------------
file(MAKE_DIRECTORY "${MODULE_INCLUDE_DIR}/${MODULE_NAME_LOWER}")
configure_file("${CMAKE_CURRENT_LIST_DIR}/version.h.in"
               "${MODULE_INCLUDE_DIR}/${MODULE_NAME_LOWER}_version.h" @ONLY)

# ---------------------------------------------------------------------------------------
# Options and vars
# ---------------------------------------------------------------------------------------
set(PROPERTY_PREFIX "${PROJECT_NAME}")

# ---------------------------------------------------------------------------------------
# Set build and c++ options
# ---------------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE
        "Release"
        CACHE STRING "Choose Release or Debug" FORCE)
endif()

if(NOT CMAKE_CONFIGURATION_TYPES)
    # Debug;Release;RelWithDebInfo;MinSizeRel
    set(CMAKE_CONFIGURATION_TYPES
        "Debug;Release"
        CACHE STRING "Release and Debug" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # force C++20
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX d)

# make sure __cplusplus is defined when using msvc and enable parallel build
if(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " /Zc:__cplusplus /MP")
endif()

# 编译器选项
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # todo
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_definitions(-Wno-error)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # todo
endif()

# ---------------------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------------------
set(MODULE_INDBUILD_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/.ind_build")

# # 预定义宏
add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DCUSTOM_ROOT_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
add_definitions(-DCUSTOM_LOG_DIR="${CMAKE_CURRENT_SOURCE_DIR}/log")
add_definitions(-DCUSTOM_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
add_definitions(-DCUSTOM_CONFIG_DIR="${CMAKE_CURRENT_SOURCE_DIR}/config")

# # 框架路径
set(MODULE_CMAKE_DIR ${MODULE_INDBUILD_PREFIX}/cmake)
set(MODULE_DEPS_DIR ${MODULE_INDBUILD_PREFIX}/3rdparty)
set(MODULE_SAMPLES_DIR ${MODULE_INDBUILD_PREFIX}/samples)
set(MODULE_SCRIPTS_DIR ${MODULE_INDBUILD_PREFIX}/scripts)
set(MODULE_DOCS_DIR ${MODULE_INDBUILD_PREFIX}/docs)
set(MODULE_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(MODULE_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/out)
set(MODULE_BUILD_DEPS_DIR ${MODULE_BUILD_DIR}/_deps) # FetchContent_Declare的工作路径
set(MODULE_COMMON_INCLUDE_DIRS ${MODULE_INCLUDE_DIR} ${MODULE_DEPS_DIR})

# 构建输出directory
set(MODULE_OUT_DIR_DIFF_BUILD_TYPE
    "$<$<CONFIG:Debug>:"
    "${MODULE_OUT_DIR}/Debug"
    ">"
    "$<$<CONFIG:Release>:"
    "${MODULE_OUT_DIR}/Release"
    ">"
    "$<$<CONFIG:RelWithDebInfo>:"
    "${MODULE_OUT_DIR}/RelWithDebInfo"
    ">"
    "$<$<CONFIG:MinSizeRel>:"
    "${MODULE_OUT_DIR}/MinSizeRel"
    ">")
string(REPLACE ";" "" MODULE_OUT_DIR_DIFF_BUILD_TYPE
               ${MODULE_OUT_DIR_DIFF_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MODULE_OUT_DIR_DIFF_BUILD_TYPE}
)# 可执行Target文件的输出路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MODULE_OUT_DIR_DIFF_BUILD_TYPE}
)# 共享库Target文件的输出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MODULE_OUT_DIR_DIFF_BUILD_TYPE}
)# 静态库Target文件的输出路径
set(MODULE_DLL_OUTPUT_DIR ${MODULE_OUT_DIR_DIFF_BUILD_TYPE}) # DLL文件移动目标路径

# # 框架项目选项
option(MODULE_BUILD_TESTS "Whether to build the module: test" ON)
option(MODULE_BUILD_BENCHS "Whether to build the module: benchmark" ON)
option(MODULE_BUILD_FORMAT "Whether to build the format tool" ON)
option(MODULE_BUILD_INSTALL "Generate the install target" OFF)
option(MODULE_BUILD_SAMPLES "Whether to build the module: sample" OFF) # sample
                                                                       # option
option(MODULE_BUILD_DOXYGEN "Whether to build the module: DocGenerate" ON
)# doxygen编译开关

# ---------------------------------------------------------------------------------------
# 3rdparty
# ---------------------------------------------------------------------------------------
include("${MODULE_CMAKE_DIR}/3rdparty/acis.cmake")
set(MODULE_ACIS_VERSION
    "R34"
    CACHE STRING "ACIS version linked")
find_acis(${MODULE_ACIS_VERSION})

# ---------------------------------------------------------------------------------------
# Library
# ---------------------------------------------------------------------------------------
include("${MODULE_CMAKE_DIR}/frame_utils.cmake")
get_all_headers(${MODULE_INCLUDE_DIR} TARGET_HEADER_FILES)
set(TARGET_HEADER_FILES ${TARGET_HEADER_FILES} ${MODULE_HEADERS})
get_all_sources(${MODULE_SOURCE_DIR} TARGET_SOURCE_FILES)

if("${TARGET_SOURCE_FILES}" STREQUAL "")
    message(WARNING "This folder has no sources!!!")
else()
    add_library(${MODULE_TARGET_NAME} SHARED ${TARGET_HEADER_FILES}
                                             ${TARGET_SOURCE_FILES}) # 添加链接库输出

    set_property(TARGET ${MODULE_TARGET_NAME} PROPERTY FOLDER
                                                       ${PROPERTY_PREFIX})

    # ACIS
    target_include_acis(${MODULE_TARGET_NAME})
    target_link_acis(${MODULE_TARGET_NAME} ${MODULE_DLL_OUTPUT_DIR})

    # 设置目标引用
    target_include_directories(
        ${MODULE_TARGET_NAME}
        PUBLIC "$<BUILD_INTERFACE:${MODULE_COMMON_INCLUDE_DIRS}>"
               "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

endif()

# ---------------------------------------------------------------------------------------
# Binary
# ---------------------------------------------------------------------------------------
if(${MODULE_BUILD_SAMPLES})
    add_subdirectory("${MODULE_INDBUILD_PREFIX}/samples")
endif()

if(${MODULE_BUILD_TESTS})
    add_subdirectory("${MODULE_INDBUILD_PREFIX}/tests")
endif()

if(${MODULE_BUILD_BENCHS})
    add_subdirectory("${MODULE_INDBUILD_PREFIX}/benchs")
endif()

# ---------------------------------------------------------------------------------------
# Chore
# ---------------------------------------------------------------------------------------
if(${MODULE_BUILD_DOXYGEN})
    add_subdirectory("${MODULE_INDBUILD_PREFIX}/docs")
endif()

# cmake-format
if(${MODULE_BUILD_FORMAT})
    find_package(Python REQUIRED)

    add_custom_target(
        format
        COMMAND ${Python_EXECUTABLE} ${MODULE_SCRIPTS_DIR}/format.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set_property(TARGET format PROPERTY FOLDER "${PROPERTY_PREFIX}")

    add_custom_target(
        format-check
        COMMAND ${Python_EXECUTABLE} ${MODULE_SCRIPTS_DIR}/format_check.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set_property(TARGET format-check PROPERTY FOLDER "${PROPERTY_PREFIX}")
endif()

# ---------------------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------------------
# ATTENTION: ind-build should not support Install
