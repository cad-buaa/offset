/*********************************************************************
 * @file    gme_cstrapi.hxx
 * @brief
 * @details 属于 api extend 示例
 * @author  CastaneaG
 * @date    2024.5.21
 *********************************************************************/
#pragma once

#include <acis/dcl_cstr.h>

#include <acis/acis.hxx>
#include <acis/api.hxx>

/**
 * @brief     	使用 sweep 操作构建 cuboid
 * @details     使用 sweep 操作构建 cuboid, 该长方体中心点位于原点
 * @param[in]	x x 方向的尺寸
 * @param[in]	y y 方向的尺寸
 * @param[in]	z z 方向的尺寸
 * @param[out]	body 用于返回生成的body
 * @return		函数执行结果
 * @related     BODY
 */
DECL_CSTR outcome gme_api_sweep_make_cuboid(double x, double y, double z, BODY*& body);