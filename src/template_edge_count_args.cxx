/*********************************************************************
 * @file    template_edge_count_args.cxx
 * @brief
 * @details 属于 runtime virtual method 示例
 * @author  CastaneaG
 * @date    2024.5.21
 * @todo    ATTENTION: this template file is not usable for library develop
 *********************************************************************/
#include <stdio.h>

// ACIS 头文件
#include "acis/alltop.hxx"
#include "acis/api.hxx"
#include "acis/check.hxx"
#include "acis/ckoutcom.hxx"
#include "acis/cstrapi.hxx"
#include "acis/get_top.hxx"
#include "acis/lists.hxx"

// 自定义头文件
#include "template_edge_count_args.hxx"

/**
 * @brief			edge_count
 * @details			由 argref.num 传回 body 中 edge 的数量
 * @param[in] 		entptr	要操作的Entity的指针,在函数内部可以转换为具体的实体类型指针
 * @param[in,out]	argref	对应方法参数列表的常量引用，函数内可转换为对应的 METHOD_ARGS 派生类的引用
 * @return   		logical
 */
static logical edge_count(void* entptr, METHOD_ARGS const& argref) {
    BODY* body = (BODY*)entptr;
    EDGE_COUNT_ARGS const& args = *(EDGE_COUNT_ARGS*)&argref;

    // 检查entptr指向的是否为body。
    if(api_checking_on) {
        check_body(body);
    } else {
        return FALSE;
    }

    ENTITY_LIST edge_list;
    get_edges(body, edge_list);
    args.num = edge_list.count();

    return TRUE;
}

// Must regist in exe, not dll
// static METHOD_ID edge_count_method_id("count", "edge_count_args");               // 关联方法名 count 和参数类型 edge_count_args, 前者为自定方法名，后者为参数列表标识符。
// static MethodFunction xxx = BODY::add_method(edge_count_method_id, edge_count);  // 将 edge_count 函数注册为 BODY 类的 "count" 方法的实现。xxx的作用是确保在程序启动时自动完成方法注册。
