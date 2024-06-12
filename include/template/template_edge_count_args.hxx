/*********************************************************************
 * @file    template_edge_count_args.hxx
 * @brief
 * @details 属于 runtime virtual method 示例
 * @author  CastaneaG
 * @date    2024.5.21
 * @todo    ATTENTION: this template file is not usable for library develop
 *********************************************************************/
#pragma once

#include <acis/metharg.hxx>

/**
 * @brief		参数列表类
 * @details 该类需要包含除了要操作的ENTITY外的其他参数。
 *				  主要目的为用于参数检查，而不是为了封装，建议将所有数据成员声明为 public 。
 * @note	  除了构造函数，必须包含一个虚函数id()，用于返回一个标识符（与类名匹配来避免冲突）。
 *				  为减少组件间依赖，建议将该类所有方法在头文件中定义为内联函数。
 * @see     https://doc.spatial.com/get_doc_page/articles/r/u/n/Run-time_Virtual_Methods_b79e.html
 */
class EDGE_COUNT_ARGS : public METHOD_ARGS {
  public:
    int& num;
    EDGE_COUNT_ARGS(int& num): num(num) {}
    virtual const char* id() const { return "edge_count_args"; }
};