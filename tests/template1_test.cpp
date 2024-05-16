/*********************************************************************
 * @file    template1_test.cpp
 * @brief
 * @details
 * @author  Shivelino
 * @date    2024.5.16
 *********************************************************************/
// 测试用头文件
#include <gtest/gtest.h>

// ACIS
#include <acis/cstrapi.hxx>
#include <acis_utils/acis_utils.hpp>
#include <acis_utils/same_entity.hpp>  // same_entity所在头文件

// GME
#include "template/template1.hxx"

// ====================================================================

class Template1_Test : public ::testing::Test {
    int level = 0;

  protected:
    void SetUp() override { level = initialize_acis(); }

    void TearDown() override { terminate_acis(level); }
};

TEST_F(Template1_Test, api_make_cuboid) {
    BODY* acis_en = nullptr;
    BODY* gme_en = nullptr;

    api_make_cuboid(1.0, 2.0, 3.0, acis_en);
    gme_api_make_cuboid(1.0, 2.0, 3.0, gme_en);

    EXPECT_TRUE(same_entity(acis_en, gme_en));  // same_entity可用于比较ENTITY之间的等价性。如果您负责的部分不存在已有的判等接口，则您需要自行提供判等函数
}
