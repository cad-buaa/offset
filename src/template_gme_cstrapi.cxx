/*********************************************************************
 * @file    template_gme_cstrapi.cxx
 * @brief
 * @details 属于 api extend 示例
 * @author  CastaneaG
 * @date    2024.5.21
 *********************************************************************/
#include "template_gme_cstrapi.hxx"

#include <stdio.h>

#include "acis/alltop.hxx"
#include "acis/api.err"
#include "acis/api.hxx"
#include "acis/check.hxx"
#include "acis/ckoutcom.hxx"
#include "acis/cstrapi.hxx"
#include "acis/get_top.hxx"
#include "acis/lists.hxx"
#include "acis/module.hxx"
#include "acis/sweepapi.hxx"

outcome gme_api_sweep_make_cuboid(double width, double depth, double height, BODY*& body) {
    //DEBUG_LEVEL(DEBUG_CALLS)
    //fprintf(debug_file_ptr, "calling api_gme_sweep_make_cuboid\n");

    API_BEGIN

    // 检查参数是否大于容差
    if(api_checking_on) {
        check_pos_length(width, "width");
        check_pos_length(depth, "depth");
        check_non_neg_length(height, "height");
    }

    BODY* sheet = NULL;
    SPAposition origin(-width / 2.0, -depth / 2.0, -height / 2);
    SPAposition left(-width / 2.0, depth / 2.0, -height / 2);
    SPAposition right(width / 2.0, -depth / 2.0, -height / 2);
    FACE* fprofile = NULL;

    // 创建 sheet body
    check_outcome(api_make_plface(origin, left, right, fprofile));
    FACE* faces[1];
    faces[0] = fprofile;
    check_outcome(api_mk_by_faces(NULL, 1, faces, sheet));
    check_outcome(api_body_to_2d(sheet));
    ENTITY_LIST face_list;
    check_outcome(api_get_faces((ENTITY*)sheet, face_list));
    ENTITY* profile = face_list[0];

    // 使用 Sweep 操作创建长方体
    sweep_options options;
    options.set_draft_angle(0.0);
    BODY* new_body = NULL;
    // 当profile是某个body的一部分时，结果将使用该body返回。其拥有的body被更改，new_body 指向 NULL。
    check_outcome(api_sweep_with_options(profile, height, &options, new_body, NULL));

    body = sheet;
    result = outcome(body == NULL ? API_FAILED : 0);
    API_END

    //DEBUG_LEVEL(DEBUG_FLOW) {
    //    fprintf(debug_file_ptr, "calling gme_api_sweep_make_cuboid\n");
    //}

    return result;
}