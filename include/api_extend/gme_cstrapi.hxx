#pragma once

#include <acis/dcl_cstr.h>

#include <acis/acis.hxx>
#include <acis/api.hxx>

DECL_CSTR outcome api_gme_sweep_make_cuboid(double,  // x 方向的尺寸
                                            double,  // y 方向的尺寸
                                            double,  // z 方向的尺寸
                                            BODY*&   // 构造的 Body
);
