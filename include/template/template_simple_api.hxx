/*********************************************************************
 * @file    template_simple_api.hxx
 * @brief
 * @details
 * @author  Shivelino
 * @date    2024.5.16
 *********************************************************************/
#pragma once

// 声明接口所属模块
#include <acis/dcl_kern.h>  // DECL_KERN宏位于此文件夹

// 声明所需头文件
#include <acis/api.hxx>
#include <acis/body.hxx>

/**
 * @brief   构造cuboid.
 * @details ACIS中的接口名为api_make_cuboid，因此自行定义的函数名加上"gme_"前缀即可。
 *			另外，我们完全推荐本文件的文件名也与ACIS的文件名保持一致，以保证后续GME引擎对ACIS的适配性。
 *			事实上，最推荐的一种做法是，直接将本模块在ACIS的对应头文件拷贝至模块头文件夹，然后给ACIS的接口名添加"gme_"前缀即可（另外注意更改头文件的引用）。
 * @param[in] width cuboid的宽度
 * @param[in] depth cuboid的深度
 * @param[in] height cuboid的高度
 * @param[out] body 返回实体
 * @return 函数执行结果
 */
DECL_KERN outcome gme_api_make_cuboid(double width, double depth, double height, BODY*& body);
