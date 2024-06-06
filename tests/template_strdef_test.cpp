/*********************************************************************
 * @file    template_strdef_test.cpp
 * @brief
 * @details
 * @author  Shivelino
 * @date    2024.6.6
 * @version
 * @par Copyright(c): Concloud Software Corporation
 * @par todo:
 * @par history:
 *********************************************************************/
// 测试用头文件
#include <gtest/gtest.h>

// ACIS
#include <acis/cstrapi.hxx>
#include <acis_utils/acis_utils.hpp>
#include <acis_utils/same_entity.hpp>  // same_entity所在头文件

// GME
#include "template/template_strdef.hxx"

// ====================================================================

class Template_Strdef_Test : public ::testing::Test {
    int level = 0;

  protected:
    void SetUp() override { level = initialize_acis(); }

    void TearDown() override { terminate_acis(level); }
};

TEST_F(Template_Strdef_Test, closed) {
    straight str(SPAposition(0, 0, 0), SPAunit_vector(1, 0, 0));

    EXPECT_EQ(str.closed(), str.gme_closed());
}
