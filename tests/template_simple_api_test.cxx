﻿/*********************************************************************
 * @file    template_simple_api_test.cxx
 * @brief
 * @details
 * @author  Shivelino
 * @date    2024.5.16
 *********************************************************************/
// 测试用头文件
#include <gtest/gtest.h>

// GME
#include "template_simple_api.hxx"

// ACIS
#include "access.hpp"
#include "acis/cstrapi.hxx"
#include "acis_utils.hpp"
#include "same_entity.hpp"  // same_entity所在头文件

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

    // 存取后判等
    acis_api_save_entity("C:\\Users\\Shivelino\\Desktop\\gme_en.sat", gme_en);
    ENTITY* gme_en_restore = nullptr;
    acis_api_restore_entity("C:\\Users\\Shivelino\\Desktop\\gme_en.sat", gme_en_restore);
    acis_api_save_entity("C:\\Users\\Shivelino\\Desktop\\gme_en_restore.sat", gme_en_restore);
    EXPECT_TRUE(same_entity(acis_en, gme_en_restore));
}
