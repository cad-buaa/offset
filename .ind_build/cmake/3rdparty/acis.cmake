function(download_acis _version)
    if(NOT EXISTS ${DOWNLOAD_ACIS_DIR})
        message(STATUS "Not downloaded, start downloading ACIS.")
        execute_process(
            COMMAND
                git clone -b ${_version}
                https://github.com/gme3d/3rdparty-acis.git ${DOWNLOAD_ACIS_DIR}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE _result)

        if(NOT ${_result} EQUAL 0)
            message(FATAL_ERROR "Download package error")
        endif()

        message(STATUS "ACIS download successfully.")
    endif()
endfunction(download_acis _version)

function(find_acis _version)
    message(
        STATUS
            "=================================================================")
    message(STATUS "Start finding third party: acis. version: ${_version}")

    set(DOWNLOAD_ACIS_DIR "${CMAKE_SOURCE_DIR}/download/acis/${_version}")
    download_acis(${_version})

    # headers
    set(ACIS_HEADERS_DIR
        "${DOWNLOAD_ACIS_DIR}/include"
        PARENT_SCOPE)

    # library
    set(TEMP_LIB_DEB)
    set(TEMP_LIB_REL)
    set(TEMP_DLL_DEB)
    set(TEMP_DLL_REL)

    if(${_version} STREQUAL "R32")
        set(TEMP_LIB_DEB "${DOWNLOAD_ACIS_DIR}/lib/SpaACISd.lib")
        set(TEMP_LIB_REL "${DOWNLOAD_ACIS_DIR}/lib/SpaACIS.lib")
        set(TEMP_DLL_DEB "${DOWNLOAD_ACIS_DIR}/lib/SpaACISd.dll")
        set(TEMP_DLL_REL "${DOWNLOAD_ACIS_DIR}/lib/SpaACIS.dll")
    elseif(${_version} STREQUAL "R34")
        set(TEMP_LIB_DEB "${DOWNLOAD_ACIS_DIR}/lib/debug/SPAAcisDs.lib")
        set(TEMP_LIB_REL "${DOWNLOAD_ACIS_DIR}/lib/debug/SPAAcisDs.lib")
        set(TEMP_DLL_DEB "${DOWNLOAD_ACIS_DIR}/lib/release/SPAAcisDs.dll")
        set(TEMP_DLL_REL "${DOWNLOAD_ACIS_DIR}/lib/release/SPAAcisDs.dll")
        add_definitions(-DSPAACISDS)
    else()
        message(FATAL_ERROR "Unknown version of ACIS")
    endif()

    set(ACIS_LIB_DEB
        ${TEMP_LIB_DEB}
        PARENT_SCOPE)
    set(ACIS_LIB_REL
        ${TEMP_LIB_REL}
        PARENT_SCOPE)
    set(ACIS_DLL_DEB
        ${TEMP_DLL_DEB}
        PARENT_SCOPE)
    set(ACIS_DLL_REL
        ${TEMP_DLL_REL}
        PARENT_SCOPE)

    message(
        STATUS
            "=================================================================")
endfunction(find_acis _version)

macro(target_include_acis _target_name)
    target_include_directories(
        ${_target_name}
        PUBLIC "$<BUILD_INTERFACE:${ACIS_HEADERS_DIR}>"
               "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
endmacro(target_include_acis _target_name)

macro(target_link_acis _target_name _dest_dir)
    target_link_libraries(${_target_name} PUBLIC debug ${ACIS_LIB_DEB}
                                                 optimized ${ACIS_LIB_REL})
    set(_input_file
        "$<$<CONFIG:Debug>:${ACIS_DLL_DEB}>$<$<NOT:$<CONFIG:Debug>>:${ACIS_DLL_REL}>"
    )
    add_custom_command(
        TARGET ${_target_name}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${_input_file} ${_dest_dir})
endmacro(
    target_link_acis
    _target_name
    _dest_dir)
