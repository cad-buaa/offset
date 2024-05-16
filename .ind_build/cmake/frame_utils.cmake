# 获取目录下的所有头文件（非递归）
function(get_hearers_in_folder _dir_in, _list_out)
    file(GLOB __tmp_list "${ARGV0}/*.h" "${ARGV0}/*.hpp" "${ARGV0}/*.hxx")
    set(${ARGV1}
        ${__tmp_list}
        PARENT_SCOPE)
endfunction()

# 获取目录下的所有源文件（非递归）
function(get_sources_in_folder _dir_in, _list_out)
    file(GLOB __tmp_list "${ARGV0}/*.c" "${ARGV0}/*.cpp" "${ARGV0}/*.cxx")
    set(${ARGV1}
        ${__tmp_list}
        PARENT_SCOPE)
endfunction()

# 获取目录下的所有头文件
function(get_all_headers _dir_in, _list_out)
    file(GLOB_RECURSE __tmp_list "${ARGV0}/*.h" "${ARGV0}/*.hpp"
         "${ARGV0}/*.hxx")
    set(${ARGV1}
        ${__tmp_list}
        PARENT_SCOPE)
endfunction()

# 获取目录下的所有源文件
function(get_all_sources _dir_in, _list_out)
    file(GLOB_RECURSE __tmp_list "${ARGV0}/*.c" "${ARGV0}/*.cpp"
         "${ARGV0}/*.cxx")
    set(${ARGV1}
        "${__tmp_list}"
        PARENT_SCOPE)
endfunction()

# 获取目录下的所有lib文件
function(get_all_libs _dir_in, _list_out)
    file(GLOB_RECURSE __tmp_list "${ARGV0}/*.lib")
    set(${ARGV1}
        "${__tmp_list}"
        PARENT_SCOPE)
endfunction()

# 获取目录下的所有dll文件
function(get_all_dlls _dir_in, _list_out)
    file(GLOB_RECURSE __tmp_list "${ARGV0}/*.dll")
    set(${ARGV1}
        "${__tmp_list}"
        PARENT_SCOPE)
endfunction()
