/*********************************************************************
 * @file    api_extend_test.cpp
 * @brief
 * @details 属于 api extend 示例
 * @author  CastaneaG
 * @date    2024.5.22
 *********************************************************************/
// 测试用头文件
#include <gtest/gtest.h>

// ACIS
#include <acis/cstrapi.hxx>
#include <acis_utils/acis_utils.hpp>
#include <acis_utils/same_entity.hpp>

// GME
#include "api_extend/gme_cstrapi.hxx"

// ====================================================================

class api_extend_test : public ::testing::Test {
    int level = 0;

  protected:
    void SetUp() override { level = initialize_acis(); }

    void TearDown() override { terminate_acis(level); }
};

TEST_F(api_extend_test, api_extend) {
    BODY* acis_body = nullptr;
    BODY* gme_body = nullptr;

    API_BEGIN
    api_make_cuboid(2.0, 2.0, 2.0, acis_body);
    api_gme_sweep_make_cuboid(2.0, 2.0, 2.0, gme_body);

    API_END

    EXPECT_TRUE(same_entity(acis_body, gme_body));
}
