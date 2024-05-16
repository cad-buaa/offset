/*********************************************************************
 * @file    template1.cpp
 * @brief
 * @details
 * @author  Shivelino
 * @date    2024.5.16
 *********************************************************************/
// 标准库或者第三方库
#include <stdio.h>

// 自定义头文件
#include "template/template1.hxx"

// ACIS
#include <acis/dcl_kern.h>
#include <acis/logical.h>

#include <acis/acis.hxx>
#include <acis/api.err>
#include <acis/api.hxx>
#include <acis/body.hxx>
#include <acis/check.hxx>
#include <acis/cstrapi.hxx>
#include <acis/module.hxx>
#include <acis/primtive.hxx>

// ************************************************************
outcome gme_api_make_cuboid(double width, double depth, double height, BODY*& body) {
    DEBUG_LEVEL(DEBUG_CALLS)                               // debug等级
    fprintf(debug_file_ptr, "calling api_make_cuboid\n");  // debug日志。通过debug_file_ptr记录日志

    // API_BEGIN与API_END是ACIS的错误处理机制，函数主体逻辑应该包含于其中。ACIS文档链接：https://doc.spatial.com/get_doc_page/articles/a/p/i/API_Macros_b22b.html
    API_BEGIN
    if(api_checking_on) {  // 函数输入参数所需的检查工作，可自定义处理
        check_pos_length(width, "width");
        check_pos_length(depth, "depth");
        check_non_neg_length(height, "height");
    }

    // FUNCTIONAL_BEGIN: 函数核心功能起始
    body = make_parallelepiped(width, depth, height);
    // FUNCTIONAL_END: 函数核心功能终止

    result = outcome(body == NULL ? API_FAILED : 0);  // 错误处理，在不同情况下可以自定义构造不同的outcome值
    API_END
    DEBUG_LEVEL(DEBUG_FLOW) {
        fprintf(debug_file_ptr, "leaving api_make_cuboid : % s\n", find_err_ident(result.error_number()));
    }
    return result;
}
