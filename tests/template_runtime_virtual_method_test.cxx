/*********************************************************************
 * @file    template_runtime_virtual_method_test.cxx
 * @brief
 * @details 属于 runtime virtual method 示例
 * @author  CastaneaG
 * @todo    Not usable
 * @date    2024.5.21
 *********************************************************************/

// 测试用头文件
#include <gtest/gtest.h>

// GME
#include "template/template_edge_count_args.hxx"

// ACIS
#include "acis/cstrapi.hxx"
#include "acis/get_top.hxx"
#include "acis_utils.hpp"

// ====================================================================

class runtime_virtual_method_test : public ::testing::Test {
    int level = 0;

  protected:
    void SetUp() override { level = initialize_acis(); }

    void TearDown() override { terminate_acis(level); }
};

// TEST_F(runtime_virtual_method_test, EDGE_COUNT) {
//     BODY* body = nullptr;
//     int gme_edge_num = 0;
//     int acis_edge_num = 0;
//
//     API_BEGIN
//     api_make_cuboid(10.0, 10.0, 10.0, body);
//     // 调用count方法
//     logical ok = body->call_method(METHOD_ID("count", "edge_count_args"), EDGE_COUNT_ARGS(gme_edge_num));
//
//     ENTITY_LIST edge_list;
//     get_edges(body, edge_list);
//     acis_edge_num = edge_list.count();
//
//     API_END
//
//     EXPECT_TRUE(gme_edge_num == acis_edge_num);
// }
