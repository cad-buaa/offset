#include "ofst_examples1.hxx"

#include <windows.h>

#include <iostream>

#include "PrivateInterfaces/ofst_bad_intersections_remover.hxx"
#include "PublicInterfaces/gme_ofstapi.hxx"
#include "access.hpp"
#include "acis/acismath.h"
#include "acis/boolapi.hxx"
#include "acis/ckoutcom.hxx"
#include "acis/cstrapi.hxx"
#include "acis/ellipse.hxx"
#include "acis/eulerapi.hxx"
#include "acis/geom_utl.hxx"
#include "acis/geometry.hxx"
#include "acis/getbox.hxx"
#include "acis/kernapi.hxx"
#include "acis/law.hxx"
#include "acis/main_law.hxx"
#include "acis/ofstapi.hxx"
#include "acis/point.hxx"
#include "acis/rgbcolor.hxx"
#include "acis/rnd_api.hxx"
#include "acis/sps2crtn.hxx"
#include "acis/straight.hxx"
#include "acis/wire_offset_options.hxx"
#include "sweep_examples.hxx"

outcome aei_OFFSET_FACE_1(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt) {
    FACE* givenface = NULL;
    FACE* offsetface = NULL;
    API_BEGIN
    check_outcome(api_clear_annotations());
    SPAposition center(0.0, 0.0, 0.0);
    SPAvector normal(0.0, 0.0, 1.0);
    double width = 10.0;
    double height = 15.0;
    check_outcome(api_face_plane(center, width, height, &normal, givenface));
    double distance = -10.0;

    LARGE_INTEGER frequency;   // 频率
    LARGE_INTEGER start, end;  // 起始和结束时间
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    check_outcome(api_offset_face(givenface, distance, offsetface));
    QueryPerformanceCounter(&end);
    double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    double elapsedmilliseconds = elapsedseconds * 1000.0;
    double elapsedmicroseconds = elapsedseconds * 1e6;
    double elapsednanoseconds = elapsedseconds * 1e9;
    AllocConsole();
    freopen("conout$", "w", stdout);
    printf("running time is %f\n", elapsedmilliseconds);
    printf("running time is %f\n", elapsedmicroseconds);
    printf("running time is %f\n", elapsednanoseconds);

    // check_outcome(api_offset_face(givenface, distance, offsetface));

    rgb_color Red(1.0, 0.0, 0.0);
    check_outcome(api_rh_set_entity_rgb(offsetface, Red));
    check_outcome(api_clear_annotations());
    API_END
    if(result.ok()) {
        output_ents.add(givenface);
        output_ents.add(offsetface);
    }

    BODY* acis_en = nullptr;
    BODY* gme_en = nullptr;
    ENTITY_LIST output_bodies1 = NULL;
    ENTITY_LIST to_be_stitched1 = NULL;
    ENTITY* api_stitch1 = NULL;
    aei_SWEEP_WEDGE(to_be_stitched1, NULL);

    /*ENTITY_LIST LIST;
    LIST.add(offsetface);
    acis_api_save_entity_list("D:\\11月底里程碑考核\\gme_offsetface1_plane", LIST);*/

    return 0;
}

outcome aei_OFFSET_FACE_2(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt) {
    FACE* givenface = NULL;
    FACE* offsetface = NULL;
    API_BEGIN
    check_outcome(api_clear_annotations());
    // TrackAnnotations(TRUE);

    //(define face (face:sphere (position 0 0 0) 40 -45 45 0 90))
    double R = 40.0;
    /*double latitude_start_angle = degrees_to_radians(-45.0);
    double latitude_end_angle = degrees_to_radians(45.0);
    double longitude_start_angle = degrees_to_radians(0.0);
    double longitude_end_angle = degrees_to_radians(90.0);*/
    double latitude_start_angle = -45.0;
    double latitude_end_angle = 45.0;
    double longitude_start_angle = 0.0;
    double longitude_end_angle = 90.0;
    SPAvector normal(0.0, 0.0, 1.0);
    SPAposition center(0.0, 0.0, 0.0);
    check_outcome(api_face_sphere(center, R, latitude_start_angle, latitude_end_angle, longitude_start_angle, longitude_end_angle, &normal, givenface));
    //(define offset (face:offset face -20))
    double distance = -20.0;

    LARGE_INTEGER frequency;   // 频率
    LARGE_INTEGER start, end;  // 起始和结束时间
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    check_outcome(gme_api_offset_face(givenface, distance, offsetface));

    // check_outcome(api_offset_face(givenface, distance, offsetface));
    QueryPerformanceCounter(&end);
    double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    double elapsedmilliseconds = elapsedseconds * 1000.0;
    double elapsedmicroseconds = elapsedseconds * 1e6;
    double elapsednanoseconds = elapsedseconds * 1e9;
    AllocConsole();
    freopen("conout$", "w", stdout);
    printf("running time is %f\n", elapsedmilliseconds);
    printf("running time is %f\n", elapsedmicroseconds);
    printf("running time is %f\n", elapsednanoseconds);

    //(entity:set-color offset RED)
    rgb_color Red(1.0, 0.0, 0.0);
    check_outcome(api_rh_set_entity_rgb(offsetface, Red));

    // test this api for 100 times
    /*clock_t start_time = clock();
    for(int n = 0; n < 100; n++) {
        FACE* testOffset = NULL;
        check_outcome(api_offset_face(givenface, distance, testOffset));
    }
    clock_t end_time = clock();
    double total_time = (double)(end_time - start_time);

    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    printf("100次总共耗时 %lf ms，平均每次耗时 %lf ms。\n", total_time, total_time / 100.0);*/

    // TrackAnnotations(FALSE);
    check_outcome(api_clear_annotations());
    API_END
    if(result.ok()) {
        output_ents.add(givenface);
        output_ents.add(offsetface);
    }
    // ENTITY_LIST LIST;
    // LIST.add(offsetface);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\gme_offsetface2_cylinder", LIST);
    return result;
}

outcome aei_OFFSET_FACE_3(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt) {
    FACE* givenface = NULL;
    FACE* offsetface = NULL;
    API_BEGIN
    check_outcome(api_clear_annotations());
    SPAposition center(0.0, 0.0, 0.0);
    SPAvector normal(0.0, 0.0, 10.0);
    double bottom = 10.0;
    double top = 10.0;
    double ratio = 1.0;
    SPAposition pt(10.0, 0.0, 0.0);
    check_outcome(api_face_cylinder_cone(center, normal, bottom, top, 0.0, 360.0, ratio, &pt, givenface));
    double distance = 10.0;

    LARGE_INTEGER frequency;   // 频率
    LARGE_INTEGER start, end;  // 起始和结束时间
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    /* check_outcome(gme_api_offset_face(givenface, distance, offsetface));*/
    check_outcome(gme_api_offset_face(givenface, distance, offsetface));

    QueryPerformanceCounter(&end);
    double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    double elapsedmilliseconds = elapsedseconds * 1000.0;
    double elapsedmicroseconds = elapsedseconds * 1e6;
    double elapsednanoseconds = elapsedseconds * 1e9;
    AllocConsole();
    freopen("conout$", "w", stdout);
    printf("running time is %f\n", elapsedmilliseconds);
    printf("running time is %f\n", elapsedmicroseconds);
    printf("running time is %f\n", elapsednanoseconds);

    rgb_color Red(1.0, 0.0, 0.0);
    check_outcome(api_rh_set_entity_rgb(offsetface, Red));
    check_outcome(api_clear_annotations());
    API_END
    if(result.ok()) {
        output_ents.add(givenface);
        output_ents.add(offsetface);
    }
    // ENTITY_LIST LIST;
    // LIST.add(offsetface);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\offsetface3_cone", LIST);
    return result;
}

outcome aei_OFFSET_FACE_4(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt) {
    FACE* givenface = NULL;
    FACE* offsetface = NULL;
    API_BEGIN
    check_outcome(api_clear_annotations());
    SPAposition center(0.0, 0.0, 0.0);
    SPAvector normal(0.0, 0.0, 10.0);
    double bottom = 10.0;
    double top = 0.0;
    double ratio = 1.0;
    SPAposition pt(10.0, 0.0, 0.0);
    check_outcome(api_face_cylinder_cone(center, normal, bottom, top, 0.0, 360.0, ratio, &pt, givenface));
    double distance = 10.0;

    LARGE_INTEGER frequency;   // 频率
    LARGE_INTEGER start, end;  // 起始和结束时间
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    // check_outcome(gme_api_offset_face(givenface, distance, offsetface));
    check_outcome(api_offset_face(givenface, distance, offsetface));
    QueryPerformanceCounter(&end);
    double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    double elapsedmilliseconds = elapsedseconds * 1000.0;
    double elapsedmicroseconds = elapsedseconds * 1e6;
    double elapsednanoseconds = elapsedseconds * 1e9;
    AllocConsole();
    freopen("conout$", "w", stdout);
    printf("running time is %f\n", elapsedmilliseconds);
    printf("running time is %f\n", elapsedmicroseconds);
    printf("running time is %f\n", elapsednanoseconds);

    rgb_color Red(1.0, 0.0, 0.0);
    check_outcome(api_rh_set_entity_rgb(offsetface, Red));
    check_outcome(api_clear_annotations());
    API_END
    if(result.ok()) {
        output_ents.add(givenface);
        output_ents.add(offsetface);
    }
    // ENTITY_LIST LIST;
    // LIST.add(offsetface);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\offsetface4_cylindercone", LIST);
    return result;
}

outcome aei_OFFSET_FACE_5(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt) {
    FACE* givenface = NULL;
    FACE* offsetface = NULL;
    API_BEGIN
    check_outcome(api_clear_annotations());
    SPAposition center(0.0, 0.0, 0.0);
    SPAvector normal(0.0, 0.0, 1.0);
    double major = 15.0;
    double minor = 5.0;
    double tu_start = 0.0;
    double tu_end = 270.0;
    double sv_start = 0.0;
    double sv_end = 180.0;
    check_outcome(api_face_torus(center, major, minor, tu_start, tu_end, sv_start, sv_end, &normal, givenface));
    double distance = 5.0;

    LARGE_INTEGER frequency;   // 频率
    LARGE_INTEGER start, end;  // 起始和结束时间
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    // check_outcome(gme_api_offset_face(givenface, distance, offsetface));
    gme_api_offset_face(givenface, distance, offsetface);
    QueryPerformanceCounter(&end);
    double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    double elapsedmilliseconds = elapsedseconds * 1000.0;
    double elapsedmicroseconds = elapsedseconds * 1e6;
    double elapsednanoseconds = elapsedseconds * 1e9;
    AllocConsole();
    freopen("conout$", "w", stdout);
    printf("running time is %f\n", elapsedmilliseconds);
    printf("running time is %f\n", elapsedmicroseconds);
    printf("running time is %f\n", elapsednanoseconds);

    rgb_color Red(1.0, 0.0, 0.0);
    check_outcome(api_rh_set_entity_rgb(offsetface, Red));
    check_outcome(api_clear_annotations());
    API_END
    if(result.ok()) {
        output_ents.add(givenface);
        output_ents.add(offsetface);
    }
    /* ENTITY_LIST LIST;
     LIST.add(offsetface);
     acis_api_save_entity_list("D:\\11月底里程碑考核\\offsetface5_torus", LIST);*/
    return result;
}

outcome aei_OFFSET_FACE_6(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt) {
    FACE* givenface = NULL;
    FACE* offsetface = NULL;
    API_BEGIN
    SPAposition ctrlpts[25];
    ctrlpts[0].set_x(0.0);
    ctrlpts[0].set_y(0.0);
    ctrlpts[0].set_z(2.0);
    ctrlpts[1].set_x(0.0);
    ctrlpts[1].set_y(1.0);
    ctrlpts[1].set_z(2.0);
    ctrlpts[2].set_x(0.0);
    ctrlpts[2].set_y(2.0);
    ctrlpts[2].set_z(2.0);
    ctrlpts[3].set_x(0.0);
    ctrlpts[3].set_y(3.0);
    ctrlpts[3].set_z(2.0);
    ctrlpts[4].set_x(0.0);
    ctrlpts[4].set_y(4.0);
    ctrlpts[4].set_z(2.0);
    ctrlpts[5].set_x(2.0);
    ctrlpts[5].set_y(0.0);
    ctrlpts[5].set_z(1.0);
    ctrlpts[6].set_x(2.0);
    ctrlpts[6].set_y(1.0);
    ctrlpts[6].set_z(1.0);
    ctrlpts[7].set_x(2.0);
    ctrlpts[7].set_y(2.0);
    ctrlpts[7].set_z(1.0);
    ctrlpts[8].set_x(2.0);
    ctrlpts[8].set_y(3.0);
    ctrlpts[8].set_z(1.0);
    ctrlpts[9].set_x(2.0);
    ctrlpts[9].set_y(4.0);
    ctrlpts[9].set_z(1.0);
    ctrlpts[10].set_x(4.0);
    ctrlpts[10].set_y(0.0);
    ctrlpts[10].set_z(2.0);
    ctrlpts[11].set_x(4.0);
    ctrlpts[11].set_y(1.0);
    ctrlpts[11].set_z(2.0);
    ctrlpts[12].set_x(4.0);
    ctrlpts[12].set_y(2.0);
    ctrlpts[12].set_z(2.0);
    ctrlpts[13].set_x(4.0);
    ctrlpts[13].set_y(3.0);
    ctrlpts[13].set_z(2.0);
    ctrlpts[14].set_x(4.0);
    ctrlpts[14].set_y(4.0);
    ctrlpts[14].set_z(2.0);
    ctrlpts[15].set_x(6.0);
    ctrlpts[15].set_y(0.0);
    ctrlpts[15].set_z(1.0);
    ctrlpts[16].set_x(6.0);
    ctrlpts[16].set_y(1.0);
    ctrlpts[16].set_z(1.0);
    ctrlpts[17].set_x(6.0);
    ctrlpts[17].set_y(2.0);
    ctrlpts[17].set_z(1.0);
    ctrlpts[18].set_x(6.0);
    ctrlpts[18].set_y(3.0);
    ctrlpts[18].set_z(1.0);
    ctrlpts[19].set_x(6.0);
    ctrlpts[19].set_y(4.0);
    ctrlpts[19].set_z(1.0);
    ctrlpts[20].set_x(8.0);
    ctrlpts[20].set_y(0.0);
    ctrlpts[20].set_z(-2.0);
    ctrlpts[21].set_x(8.0);
    ctrlpts[21].set_y(1.0);
    ctrlpts[21].set_z(-2.0);
    ctrlpts[22].set_x(8.0);
    ctrlpts[22].set_y(2.0);
    ctrlpts[22].set_z(-2.0);
    ctrlpts[23].set_x(8.0);
    ctrlpts[23].set_y(3.0);
    ctrlpts[23].set_z(-2.0);
    ctrlpts[24].set_x(8.0);
    ctrlpts[24].set_y(4.0);
    ctrlpts[24].set_z(-2.0);
    double weights[25] = {1.0, 1.0, 2.0, 1.0, 1.0, 2.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 2.0, 1.0, 2.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 2.0, 1.0, 1.0};
    double knots_u[9] = {0.0, 0.0, 0.0, 0.0, 0.5, 1.0, 1.0, 1.0, 1.0};
    double knots_v[9] = {0.0, 0.0, 0.0, 0.0, 0.5, 1.0, 1.0, 1.0, 1.0};
    check_outcome(api_mk_fa_spl_ctrlpts(3, FALSE, 0, 0, 5, 3, FALSE, 0, 0, 5, ctrlpts, weights, 1.e-6, 9, knots_u, 9, knots_v, 1.e-6, givenface));
    double distance = 3.0;

    LARGE_INTEGER frequency;   // 频率
    LARGE_INTEGER start, end;  // 起始和结束时间
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    // check_outcome(gme_api_offset_face(givenface, distance, offsetface));
    check_outcome(gme_api_offset_face(givenface, distance, offsetface));
    QueryPerformanceCounter(&end);
    double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    double elapsedmilliseconds = elapsedseconds * 1000.0;
    double elapsedmicroseconds = elapsedseconds * 1e6;
    double elapsednanoseconds = elapsedseconds * 1e9;
    AllocConsole();
    freopen("conout$", "w", stdout);
    printf("running time is %f\n", elapsedmilliseconds);
    printf("running time is %f\n", elapsedmicroseconds);
    printf("running time is %f\n", elapsednanoseconds);

    rgb_color Red(1.0, 0.0, 0.0);
    check_outcome(api_rh_set_entity_rgb(offsetface, Red));
    API_END
    if(result.ok()) {
        output_ents.add(givenface);
        output_ents.add(offsetface);
    }

    // ENTITY_LIST LIST;
    // LIST.add(offsetface);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\offsetface6_splineface", LIST);
    return result;
}

outcome aei_OFFSET_WIRE_1(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt) {
    ///////例子1  闭合三角形

    SPAposition p1(-10, -5, 0);
    SPAposition p2(10, -5, 0);
    SPAposition p3(0, 10, 0);
    EDGE* e1 = NULL;
    EDGE* e2 = NULL;
    EDGE* e3 = NULL;
    BODY* wirebody = NULL;
    api_curve_line(p1, p2, e1);
    api_curve_line(p2, p3, e2);
    api_curve_line(p3, p1, e3);
    // SPAposition p4 = e1->start()->geometry()->coords();
    const curve& eq1 = e1->geometry()->equation();
    double a = ((straight&)eq1).subset_range.start_pt();
    SPAposition p = ((straight&)eq1).root_point;
    // SPAposition foot;
    // eq1.point_perp(p4, foot);
    // eq1.param()
    // int a = 0;
    EDGE* edges[3];
    edges[0] = e1;
    edges[1] = e2;
    edges[2] = e3;
    api_make_ewire(3, edges, wirebody);
    /* COEDGE* c0 = ACIS_NEW COEDGE(e1, REVERSED, NULL, NULL);
     VERTEX* t = c0->end();*/
    SPAunit_vector normal(0, 0, 1);
    ENTITY_LIST wires;
    api_get_wires(wirebody, wires);
    BODY* wirebody2 = NULL;
    BODY* wirebody3 = NULL;
    WIRE* oriwire = (WIRE*)wires[0];

    // LARGE_INTEGER frequency;   // 频率
    // LARGE_INTEGER start, end;  // 起始和结束时间
    // QueryPerformanceFrequency(&frequency);
    // QueryPerformanceCounter(&start);
    gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -3, normal, wirebody2);
    wire_offset_options option;

    // double dist = d->eval(1.0);
    // int aed = d->return_dim();
    constant_law* twist = ACIS_NEW constant_law(0);
    option.set_distance(3);
    option.set_plane_normal(normal);
    // option.set_twist_law(0);
    option.set_gap_type(natural);
    gme_api_offset_planar_wire(wirebody, &option, wirebody2);

    /* QueryPerformanceCounter(&end);*/
    // double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    // double elapsedmilliseconds = elapsedseconds * 1000.0;
    // double elapsedmicroseconds = elapsedseconds * 1e6;
    // double elapsednanoseconds = elapsedseconds * 1e9;
    // AllocConsole();
    // freopen("conout$", "w", stdout);
    // printf("running time is %f\n", elapsedmilliseconds);
    // printf("running time  is %f\n", elapsedmicroseconds);
    // printf("running time  is %f\n", elapsednanoseconds);
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -3, normal, wirebody2);
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 3, normal, wirebody3);
    //// //api_offset_planar_wire((WIRE*)wires[0], NULL, -3, normal, wirebody2);
    //// //api_offset_planar_wire((WIRE*)wires[0], NULL, 3, normal, wirebody3);
    // rgb_color Red(1.0, 0.0, 0.0);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    //// gme_api_offset_planar_wire(wirebody, 2, normal, wirebody2);
    output_ents.add(wirebody);
    output_ents.add(wirebody2);
    // output_ents.add(wirebody3);
    ENTITY_LIST LIST;
    LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\gme_closed_sanjiao.sat", LIST);

    ////例子2  闭合四边形
    // SPAposition p1(-10, -10, 0);
    // SPAposition p2(-10, 10, 0);
    // SPAposition p3(10, 10, 0);
    // SPAposition p4(10, -10, 0);
    // EDGE* e1 = NULL;
    // EDGE* e2 = NULL;
    // EDGE* e3 = NULL;
    // EDGE* e4 = NULL;
    // BODY* wirebody = NULL;
    // api_curve_line(p1, p2, e1);
    // api_curve_line(p2, p3, e2);
    // api_curve_line(p3, p4, e3);
    // api_curve_line(p4, p1, e4);
    // EDGE* edges[4];
    // edges[0] = e1;
    // edges[1] = e2;
    // edges[2] = e3;
    // edges[3] = e4;
    // api_make_ewire(4, edges, wirebody);
    // SPAunit_vector normal(0, 0, 1);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // BODY* wirebody2 = NULL;
    // WIRE* oriwire = (WIRE*)wires[0];

    // LARGE_INTEGER frequency;   // 频率
    // LARGE_INTEGER start, end;  // 起始和结束时间
    // QueryPerformanceFrequency(&frequency);
    // QueryPerformanceCounter(&start);
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 3, normal, wirebody2);
    // QueryPerformanceCounter(&end);
    // double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    // double elapsedmilliseconds = elapsedseconds * 1000.0;
    // double elapsedmicroseconds = elapsedseconds * 1e6;
    // double elapsednanoseconds = elapsedseconds * 1e9;
    // AllocConsole();
    // freopen("conout$", "w", stdout);
    // printf("running time is %f\n", elapsedmilliseconds);
    // printf("running time  is %f\n", elapsedmicroseconds);
    // printf("running time  is %f\n", elapsednanoseconds);

    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -3, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);
    ////ENTITY_LIST LIST;
    ////LIST.add(wirebody2);
    ////acis_api_save_entity_list("D:\\11月底里程碑考核\\closed_sibian.sat", LIST);

    //////例子3 非闭合折线
    // SPAposition p1(-10, -5, 0);
    // SPAposition p2(10, -5, 0);
    // SPAposition p3(-4, 10, 0);
    // SPAposition p4(4, 10, 0);
    // EDGE* e1 = NULL;
    // EDGE* e2 = NULL;
    // EDGE* e3 = NULL;
    // BODY* wirebody = NULL;
    // api_curve_line(p3, p1, e1);
    // api_curve_line(p1, p2, e2);
    // api_curve_line(p2, p4, e3);
    // EDGE* edges[3];
    // edges[0] = e1;
    // edges[1] = e2;
    // edges[2] = e3;
    // api_make_ewire(3, edges, wirebody);
    // SPAunit_vector normal(0, 0, 1);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // BODY* wirebody2 = NULL;
    // WIRE* oriwire = (WIRE*)wires[0];

    // LARGE_INTEGER frequency;   // 频率
    // LARGE_INTEGER start, end;  // 起始和结束时间
    // QueryPerformanceFrequency(&frequency);
    // QueryPerformanceCounter(&start);
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -5, normal, wirebody2);
    // QueryPerformanceCounter(&end);
    // double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    // double elapsedmilliseconds = elapsedseconds * 1000.0;
    // double elapsedmicroseconds = elapsedseconds * 1e6;
    // double elapsednanoseconds = elapsedseconds * 1e9;
    // AllocConsole();
    // freopen("conout$", "w", stdout);
    // printf("running time is %f\n", elapsedmilliseconds);
    // printf("running time  is %f\n", elapsedmicroseconds);
    // printf("running time  is %f\n", elapsednanoseconds);
    //

    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // // gme_api_offset_planar_wire(wirebody, 2, normal, wirebody2);
    // output_ents.add(wirebody);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 5, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);
    // ENTITY_LIST LIST;
    // LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\unclosed_zhexian1.sat", LIST);

    ////// 例子4 非闭合折线
    // SPAposition p1(-10, -5, 0);
    // SPAposition p2(10, -5, 0);
    // SPAposition p3(-4, 10, 0);
    // SPAposition p4(4, 10, 0);
    // EDGE* e1 = NULL;
    // EDGE* e2 = NULL;
    // EDGE* e3 = NULL;
    // BODY* wirebody = NULL;
    // api_curve_line(p1, p2, e1);
    // api_curve_line(p2, p3, e2);
    // api_curve_line(p3, p4, e3);
    // EDGE* edges[3];
    // edges[0] = e1;
    // edges[1] = e2;
    // edges[2] = e3;
    // api_make_ewire(3, edges, wirebody);
    // SPAunit_vector normal(0, 0, 1);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // BODY* wirebody2 = NULL;
    // WIRE* oriwire = (WIRE*)wires[0];
    // api_offset_planar_wire((WIRE*)wires[0], NULL, 2, normal, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    //// gme_api_offset_planar_wire(wirebody, 2, normal, wirebody2);
    // output_ents.add(wirebody);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    //// BODY* wirebody4 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -2, normal, wirebody3);
    //// gme_api_offset_planar_wire(wirebody, -2, normal, wirebody3);
    //// gme_api_offset_planar_wire(wirebody3, -2, normal, wirebody4);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);
    // ENTITY_LIST LIST;
    // LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\unclosed_zhexian2.sat", LIST);

    // 例子测试
    /* SPAposition p1(-10, -5, 0);
       SPAposition p2(10, -5, 0);
       SPAposition p3(0, 10, 0);
       EDGE* e1 = NULL;
       EDGE* e2 = NULL;
       EDGE* e3 = NULL;
       BODY* wirebody1 = NULL;
       BODY* wirebody2 = NULL;
       BODY* wirebody3 = NULL;
       BODY* wirebody11 = NULL;
       BODY* wirebody22 = NULL;
       BODY* wirebody33 = NULL;
       api_curve_line(p1, p2, e1);
       api_curve_line(p2, p3, e2);
       api_curve_line(p3, p1, e3);
       EDGE* edges1[1];
       EDGE* edges2[1];
       EDGE* edges3[1];
       edges1[0] = e1;
       edges2[0] = e2;
       edges3[0] = e3;
       api_make_ewire(1, edges1, wirebody1);
       api_make_ewire(1, edges2, wirebody2);
       api_make_ewire(1, edges3, wirebody3);
       SPAunit_vector normal(0, 0, 1);
       gme_api_offset_planar_wire(wirebody1, -3, normal, wirebody11);
       gme_api_offset_planar_wire(wirebody2, -3, normal, wirebody22);
       gme_api_offset_planar_wire(wirebody3, -3, normal, wirebody33);
       output_ents.add(wirebody1);
       output_ents.add(wirebody2);
       output_ents.add(wirebody3);
       output_ents.add(wirebody11);
       output_ents.add(wirebody22);
       output_ents.add(wirebody33);*/

    ////例子5  B样条曲线1
    // SPAposition ctrlpts[6];
    // ctrlpts[0] = SPAposition(0.0, 0.0, 0.0);
    // ctrlpts[1] = SPAposition(1.0, 1.0, 0.0);
    // ctrlpts[2] = SPAposition(2.0, 2.0, 0.0);
    // ctrlpts[3] = SPAposition(3.0, 3.5, 0.0);
    // ctrlpts[4] = SPAposition(4.0, 5.5, 0.0);
    // ctrlpts[5] = SPAposition(5.0, 4.5, 0.0);
    // double weight[9] = {0, 0, 0, 0, 0, 0};
    // int num_knots_u = 9;
    // double knots_u[9] = {0.0, 0.0, 0.0, 0.0, 2.0, 4.0, 4.0, 4.0, 4.0};
    // int degree = 3;
    // logical national = 0;
    // bs2_curve bs2 = bs2_curve_from_ctrlpts(degree, national, 0, 0, 7, ctrlpts, weight, SPAresabs, num_knots_u, knots_u, SPAresabs);
    // EDGE* e = NULL;
    // api_curve_spline(6, ctrlpts, NULL, NULL, e);
    // EDGE* edge[1];
    // edge[0] = e;
    // BODY* wirebody;
    // api_make_ewire(1, edge, wirebody);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;
    // api_offset_planar_wire((WIRE*)wires[0], NULL, 0.5, normal, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -0.5, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);
    // ENTITY_LIST LIST;
    // LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\Bspline1.sat", LIST);

    //// 例子6  B样条曲线2
    // SPAposition ctrlpts[6];
    // ctrlpts[0] = SPAposition(-10, 5, 0.0);
    // ctrlpts[1] = SPAposition(-5, -3, 0.0);
    // ctrlpts[2] = SPAposition(-2, 3, 0.0);
    // ctrlpts[3] = SPAposition(2, -2, 0.0);
    // ctrlpts[4] = SPAposition(5, 5.5, 0.0);
    // ctrlpts[5] = SPAposition(10, 2, 0.0);
    // double weight[9] = {0, 0, 0, 0, 0, 0};
    // int num_knots_u = 9;
    // double knots_u[9] = {0.0, 0.0, 0.0, 0.0, 2.0, 4.0, 4.0, 4.0, 4.0};
    // int degree = 3;
    // logical national = 0;
    // bs2_curve bs2 = bs2_curve_from_ctrlpts(degree, national, 0, 0, 7, ctrlpts, weight, SPAresabs, num_knots_u, knots_u, SPAresabs);
    // EDGE* e = NULL;
    // api_curve_spline(6, ctrlpts, NULL, NULL, e);
    // EDGE* edge[1];
    // edge[0] = e;
    // BODY* wirebody;
    // api_make_ewire(1, edge, wirebody);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;

    // LARGE_INTEGER frequency;   // 频率
    // LARGE_INTEGER start, end;  // 起始和结束时间
    // QueryPerformanceFrequency(&frequency);
    // QueryPerformanceCounter(&start);
    // api_offset_planar_wire((WIRE*)wires[0], NULL, 0.5, normal, wirebody2);
    // QueryPerformanceCounter(&end);
    // double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    // double elapsedmilliseconds = elapsedseconds * 1000.0;
    // double elapsedmicroseconds = elapsedseconds * 1e6;
    // double elapsednanoseconds = elapsedseconds * 1e9;
    // AllocConsole();
    // freopen("conout$", "w", stdout);
    // printf("running time is %f\n", elapsedmilliseconds);
    // printf("running time  is %f\n", elapsedmicroseconds);
    // printf("running time  is %f\n", elapsednanoseconds);

    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -0.5, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);
    // ENTITY_LIST LIST;
    // LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\gme_Bspline2.sat", LIST);

    ////例子7  闭合B样条曲线
    // SPAposition ctrlpts[6];
    // ctrlpts[0] = SPAposition(-10, 0, 0.0);
    // ctrlpts[1] = SPAposition(-5, 3, 0.0);
    // ctrlpts[2] = SPAposition(2, 0, 0.0);
    // ctrlpts[3] = SPAposition(4, -3, 0.0);
    // ctrlpts[4] = SPAposition(0, -5.5, 0.0);
    // ctrlpts[5] = SPAposition(-10, 0, 0.0);
    // double weight[9] = {1, 5, 1, 6, 1, 10};
    // int num_knots_u = 9;
    // double knots_u[9] = {0.0, 0.0, 0.0, 0.0, 2.0, 4.0, 4.0, 4.0, 4.0};
    // int degree = 3;
    // logical national = 0;
    // bs2_curve bs2 = bs2_curve_from_ctrlpts(degree, national, 1, 0, 7, ctrlpts, weight, SPAresabs, num_knots_u, knots_u, SPAresabs);
    // EDGE* e = NULL;
    // api_curve_spline(6, ctrlpts, NULL, NULL, e);
    // EDGE* edge[1];
    // edge[0] = e;
    // BODY* wirebody;
    // api_make_ewire(1, edge, wirebody);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;

    // LARGE_INTEGER frequency;   // 频率
    // LARGE_INTEGER start, end;  // 起始和结束时间
    // QueryPerformanceFrequency(&frequency);
    // QueryPerformanceCounter(&start);
    // api_offset_planar_wire((WIRE*)wires[0], NULL, 0.4, normal, wirebody2);
    // QueryPerformanceCounter(&end);
    // double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    // double elapsedmilliseconds = elapsedseconds * 1000.0;
    // double elapsedmicroseconds = elapsedseconds * 1e6;
    // double elapsednanoseconds = elapsedseconds * 1e9;
    // AllocConsole();
    // freopen("conout$", "w", stdout);
    // printf("running time is %f\n", elapsedmilliseconds);
    // printf("running time  is %f\n", elapsedmicroseconds);
    // printf("running time  is %f\n", elapsednanoseconds);

    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -0.5, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);
    // ENTITY_LIST LIST;
    // LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\closed_Bspline.sat", LIST);

    //
    ////  例子8  NURBS曲线表示的圆
    // SPAposition ctrlpts[4];
    // ctrlpts[0] = SPAposition(-5.0, 0.0, 0.0);
    // ctrlpts[1] = SPAposition(-5.0, 5.0, 0.0);
    // ctrlpts[2] = SPAposition(5.0,  5.0, 0.0);
    // ctrlpts[3] = SPAposition(5.0,  0.0, 0.0);
    // // double weight[5] = {1, 0.5, 1, 0.5, 1};
    // // int num_knots_u = 8;
    // // double knots_u[8] = {0.0, 0.0, 0.0, 0.5, 0.5, 1.0, 1.0, 1.0};
    // // int degree = 2;
    // // logical national = 1;
    // ///* api_curve_spline2(,) */
    // EDGE* e = NULL;
    // double weight[4] = {1, 0.5, 0.5, 1};
    // int num_knots_u = 7;
    // double knots_u[7] = {0.0, 0.0, 0.0, 0.5, 1.0, 1.0, 1.0};
    // int degree = 2;
    // logical national = 1;
    // api_mk_ed_int_ctrlpts(degree, 1, 0, 0, 4, ctrlpts, weight, SPAresabs, num_knots_u, knots_u, SPAresabs, e);
    // //api_make_edge_from_curve((curve*)bs2_curve, e)
    // //api_curve_spline(5, ctrlpts, NULL, NULL, e);
    // EDGE* edge[1];
    // edge[0] = e;
    // BODY* wirebody=NULL;
    // api_make_ewire(1, edge, wirebody);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;
    // api_offset_planar_wire((WIRE*)wires[0], NULL, 0.4, normal, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);   output_ents.add(wirebody);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -0.5, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);
    // ENTITY_LIST LIST;
    // LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\NURBS.sat", LIST);

    // //例子9  闭合NURBS曲线
    // SPAposition ctrlpts[6];
    // ctrlpts[0] = SPAposition(-10,  0,   0.0);
    // ctrlpts[1] = SPAposition(-5 , 3,  0.0);
    // ctrlpts[2] = SPAposition(0 , 0,   0.0);
    // ctrlpts[3] = SPAposition(5  , -3,  0.0);
    // ctrlpts[4] = SPAposition(0  , -10, 0.0);
    // ctrlpts[5] = SPAposition(-10, 0, 0.0);
    // EDGE* e = NULL;
    // double weight[6] = {0.5, 0.8, 0.7, 0.6, 0.5, 1};
    // int num_knots_u = 10;
    // double knots_u[10] = {0.0, 0.0, 0.0, 0.0, 0.5,0.5, 1, 1, 1, 1};
    // int degree = 3;
    // logical national = 1;
    // api_mk_ed_int_ctrlpts(degree, national, 0, 0, 6, ctrlpts, weight, SPAresabs, num_knots_u, knots_u, SPAresabs,e);
    ////bs2_curve bs2 = bs2_curve_from_ctrlpts(degree, national, 1, 0, 7, ctrlpts, weight, SPAresabs, num_knots_u, knots_u, SPAresabs);
    ////api_curve_spline(6, ctrlpts, NULL, NULL, e);
    // EDGE* edge[1];
    // edge[0] = e;
    // BODY* wirebody;
    // api_make_ewire(1, edge, wirebody);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;

    // LARGE_INTEGER frequency;   // 频率
    // LARGE_INTEGER start, end;  // 起始和结束时间
    // QueryPerformanceFrequency(&frequency);
    // QueryPerformanceCounter(&start);
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 0.4, normal, wirebody2);
    // QueryPerformanceCounter(&end);
    // double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    // double elapsedmilliseconds = elapsedseconds * 1000.0;
    // double elapsedmicroseconds = elapsedseconds * 1e6;
    // double elapsednanoseconds = elapsedseconds * 1e9;
    // AllocConsole();
    // freopen("conout$", "w", stdout);
    // printf("running time is %f\n", elapsedmilliseconds);
    // printf("running time  is %f\n", elapsedmicroseconds);
    // printf("running time  is %f\n", elapsednanoseconds);
    //
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -0.5, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);
    // ENTITY_LIST LIST;
    // LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\closed_NURBS.sat", LIST);

    //// 例子10   椭圆
    // ellipse* e1 = ACIS_NEW ellipse(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 2);
    // EDGE* e = NULL;
    // api_make_edge_from_curve(e1, e);
    // EDGE* edge[1];
    // edge[0] = e;
    // BODY* wirebody;
    // api_make_ewire(1, edge, wirebody);
    // output_ents.add(wirebody);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;
    // api_offset_planar_wire((WIRE*)wires[0], NULL, -0.5, normal, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 0.5, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);
    // ENTITY_LIST LIST;
    // LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\ellipse1.sat", LIST);

    //////
    //// 例子11   椭圆
    // ellipse* e1 = ACIS_NEW ellipse(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(5, 6, 0), 2);
    // // ELLIPSE* e1 = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 2);
    // EDGE* e = NULL;
    // api_make_edge_from_curve(e1, e);
    // int a = e->geometry()->equation().type();
    // EDGE* edge[1];
    // edge[0] = e;
    // BODY* wirebody;
    // api_make_ewire(1, edge, wirebody);
    // output_ents.add(wirebody);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;

    // LARGE_INTEGER frequency;   // 频率
    // LARGE_INTEGER start, end;  // 起始和结束时间
    // QueryPerformanceFrequency(&frequency);
    // QueryPerformanceCounter(&start);
    // api_offset_planar_wire((WIRE*)wires[0], NULL, -2, normal, wirebody2);
    // QueryPerformanceCounter(&end);
    // double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    // double elapsedmilliseconds = elapsedseconds * 1000.0;
    // double elapsedmicroseconds = elapsedseconds * 1e6;
    // double elapsednanoseconds = elapsedseconds * 1e9;
    // AllocConsole();
    // freopen("conout$", "w", stdout);
    // printf("running time is %f\n", elapsedmilliseconds);
    // printf("running time  is %f\n", elapsedmicroseconds);
    // printf("running time  is %f\n", elapsednanoseconds);

    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 2, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);

    // ENTITY_LIST LIST;
    // LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\gme_ellipse2.sat", LIST);

    // 测试
    // APOINT* a10 = ACIS_NEW APOINT(-4, 0, 0);
    // APOINT* a11 = ACIS_NEW APOINT(4,0,0);
    // VERTEX* v10 = ACIS_NEW VERTEX(a10);
    // VERTEX* v11 = ACIS_NEW VERTEX(a11);
    // ELLIPSE* e1 = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 2);
    // EDGE* e11 = ACIS_NEW EDGE(v10, v10, e1, 1);
    //////e1->equation().point_perp
    // EDGE* edge[1];
    // edge[0] = e11;
    //
    // BODY* wirebody=NULL;
    // api_make_ewire(1, edge, wirebody);
    // output_ents.add(wirebody);

    //// 例子12   两椭圆复合曲线
    // APOINT* a10 = ACIS_NEW APOINT(-4, 0, 0);
    // APOINT* a11 = ACIS_NEW APOINT(4,0,0);
    // APOINT* a20 = ACIS_NEW APOINT(-4, 0, 0);
    // APOINT* a21 = ACIS_NEW APOINT(4, 0, 0);
    // VERTEX* v20 = ACIS_NEW VERTEX(a20);
    // VERTEX* v21 = ACIS_NEW VERTEX(a21);
    // VERTEX* v10 = ACIS_NEW VERTEX(a10);
    // VERTEX* v11 = ACIS_NEW VERTEX(a11);
    // ELLIPSE* e1 = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 2);
    // ELLIPSE* e2 = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 1);
    // EDGE* e22 = ACIS_NEW EDGE(v20, v21, e2, 1);
    // EDGE* e11 = ACIS_NEW EDGE(v11, v10, e1, 1);
    ////e1->equation().point_perp
    // EDGE* edge[2];
    // edge[0] = e11;
    // edge[1] = e22;
    // BODY* wirebody=NULL;
    // api_make_ewire(2, edge, wirebody);
    // output_ents.add(wirebody);
    // //output_ents.add(wirebody1);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;
    // api_offset_planar_wire((WIRE*)wires[0], NULL, 1, normal, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -1, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);
    // ENTITY_LIST LIST;
    // LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\combine_ellipse.sat", LIST);

    //// 例子13  直线与椭圆复合曲线
    // APOINT* a10 = ACIS_NEW APOINT(-4, 0, 0);
    // APOINT* a11 = ACIS_NEW APOINT(4,0,0);
    // VERTEX* v10 = ACIS_NEW VERTEX(a10);
    // VERTEX* v11 = ACIS_NEW VERTEX(a11);
    // ELLIPSE* e = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 2);
    // EDGE* e1 = ACIS_NEW EDGE(v10, v11, e, 1);
    // SPAposition p1(-4, 0, 0);
    // SPAposition p2(4, 0, 0);
    //// 直线
    // EDGE* e2 = NULL;
    // api_curve_line(p1, p2, e2);
    // EDGE* edge1[2];
    // edge1[0] = e1;
    // edge1[1] = e2;
    // BODY* wirebody = NULL;
    // api_make_ewire(2, edge1, wirebody);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;

    // LARGE_INTEGER frequency;   // 频率
    // LARGE_INTEGER start, end;  // 起始和结束时间
    // QueryPerformanceFrequency(&frequency);
    // QueryPerformanceCounter(&start);
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 0.5, normal, wirebody2);
    // QueryPerformanceCounter(&end);
    // double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    // double elapsedmilliseconds = elapsedseconds * 1000.0;
    // double elapsedmicroseconds = elapsedseconds * 1e6;
    // double elapsednanoseconds = elapsedseconds * 1e9;
    // AllocConsole();
    // freopen("conout$", "w", stdout);
    // printf("running time is %f\n", elapsedmilliseconds);
    // printf("running time  is %f\n", elapsedmicroseconds);
    // printf("running time  is %f\n", elapsednanoseconds);
    //
    // output_ents.add(wirebody);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -1, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);
    //// ENTITY_LIST LIST;
    // LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\combine_ellipseline.sat", LIST);

    // //例子14  直线与椭圆复合曲线
    // //椭圆
    // APOINT* a10 = ACIS_NEW APOINT(-4, 0, 0);
    // APOINT* a11 = ACIS_NEW APOINT(4,0,0);
    // VERTEX* v10 = ACIS_NEW VERTEX(a10);
    // VERTEX* v11 = ACIS_NEW VERTEX(a11);
    // ELLIPSE* e = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 0.5);
    // EDGE* e1 = ACIS_NEW EDGE(v10, v11, e, 1);
    // SPAposition p1(-4, 0, 0);
    // SPAposition p2(4, 0, 0);
    //// 直线
    // EDGE* e2 = NULL;
    // api_curve_line(p1, p2, e2);
    // EDGE* edge1[2];
    // edge1[0] = e1;
    // edge1[1] = e2;
    // BODY* wirebody = NULL;
    // api_make_ewire(2, edge1, wirebody);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;
    // api_offset_planar_wire((WIRE*)wires[0], NULL, 0.5, normal, wirebody2);
    // output_ents.add(wirebody);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -1, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);
    // ENTITY_LIST LIST;
    // LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\combine_ellipseline2.sat", LIST);

    ////例子15   直线与NURBS
    // SPAposition ctrlpts[4];
    // ctrlpts[0] = SPAposition(-5.0, 0.0, 0.0);
    // ctrlpts[1] = SPAposition(-5.0, 5.0, 0.0);
    // ctrlpts[2] = SPAposition(5.0,  5.0, 0.0);
    // ctrlpts[3] = SPAposition(5.0,  0.0, 0.0);
    // EDGE* e = NULL;
    // EDGE* e1 = NULL;
    // double weight[4] = {0.2, 0.5, 0.5, 0.2};
    // int num_knots_u = 7;
    // double knots_u[7] = {0.0, 0.0, 0.0, 0.5, 1.0, 1.0, 1.0};
    // int degree = 2;
    // logical national = 1;
    // api_mk_ed_int_ctrlpts(degree, 1, 0, 0, 4, ctrlpts, weight, SPAresabs, num_knots_u, knots_u, SPAresabs, e);
    // api_curve_line(ctrlpts[0], ctrlpts[3] = SPAposition(5.0, 0.0, 0.0), e1);
    // EDGE* edge[2];
    // edge[0] = e;
    // edge[1] = e1;
    // BODY* wirebody=NULL;
    // api_make_ewire(2, edge, wirebody);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 1, normal, wirebody2);
    // output_ents.add(wirebody);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -1, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);
    // ENTITY_LIST LIST;
    // LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\gme_combine_NURBSline.sat", LIST);

    //// 例子16   椭圆与NURBS
    // SPAposition ctrlpts[4];
    // ctrlpts[0] = SPAposition(-5.0, 0.0, 0.0);
    // ctrlpts[1] = SPAposition(-5.0, 5.0, 0.0);
    // ctrlpts[2] = SPAposition(5.0,  5.0, 0.0);
    // ctrlpts[3] = SPAposition(5.0,  0.0, 0.0);
    // EDGE* e = NULL;
    // EDGE* e1 = NULL;
    // double weight[4] = {1, 0.5, 0.5, 1};
    // int num_knots_u = 7;
    // double knots_u[7] = {0.0, 0.0, 0.0, 0.5, 1.0, 1.0, 1.0};
    // int degree = 2;
    // logical national = 1;
    // api_mk_ed_int_ctrlpts(degree, 1, 0, 0, 4, ctrlpts, weight, SPAresabs, num_knots_u, knots_u, SPAresabs, e);
    // APOINT* a10 = ACIS_NEW APOINT(ctrlpts[0]);
    // APOINT* a11 = ACIS_NEW APOINT(ctrlpts[3]);
    // VERTEX* v10 = ACIS_NEW VERTEX(a10);
    // VERTEX* v11 = ACIS_NEW VERTEX(a11);
    // ELLIPSE* ee = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(5, 0, 0), 0.5);
    // e1 = ACIS_NEW EDGE(v11, v10, ee, 1);
    // EDGE* edge[2];
    // edge[0] = e;
    // edge[1] = e1;
    // BODY* wirebody=NULL;
    // api_make_ewire(2, edge, wirebody);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;

    // LARGE_INTEGER frequency;   // 频率
    // LARGE_INTEGER start, end;  // 起始和结束时间
    // QueryPerformanceFrequency(&frequency);
    // QueryPerformanceCounter(&start);
    // api_offset_planar_wire((WIRE*)wires[0], NULL, 1, normal, wirebody2);
    // QueryPerformanceCounter(&end);
    // double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    // double elapsedmilliseconds = elapsedseconds * 1000.0;
    // double elapsedmicroseconds = elapsedseconds * 1e6;
    // double elapsednanoseconds = elapsedseconds * 1e9;
    // AllocConsole();
    // freopen("conout$", "w", stdout);
    // printf("running time is %f\n", elapsedmilliseconds);
    // printf("running time  is %f\n", elapsedmicroseconds);
    // printf("running time  is %f\n", elapsednanoseconds);
    //

    // output_ents.add(wirebody);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -1, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);
    // ENTITY_LIST LIST;
    // LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\gme_combine_NURBSellipse.sat", LIST);

    //// 例子17   NURBS与NURBS
    // SPAposition ctrlpts[4];
    // ctrlpts[0] = SPAposition(-5.0, 0.0, 0.0);
    // ctrlpts[1] = SPAposition(-5.0, 5.0, 0.0);
    // ctrlpts[2] = SPAposition(5.0,  5.0, 0.0);
    // ctrlpts[3] = SPAposition(5.0,  0.0, 0.0);
    // EDGE* e = NULL;
    // double weight[4] = {1, 0.5, 0.5, 1};
    // int num_knots_u = 7;
    // double knots_u[7] = {0.0, 0.0, 0.0, 0.5, 1.0, 1.0, 1.0};
    // int degree = 2;
    // logical national = 1;
    // api_mk_ed_int_ctrlpts(degree, 1, 0, 0, 4, ctrlpts, weight, SPAresabs, num_knots_u, knots_u, SPAresabs, e);
    // SPAposition ctrlpts1[4];
    // ctrlpts1[0] = SPAposition(5.0, 0.0, 0.0);
    // ctrlpts1[1] = SPAposition(5.0, -5.0, 0.0);
    // ctrlpts1[2] = SPAposition(-5.0, -5.0, 0.0);
    // ctrlpts1[3] = SPAposition(-5.0, 0.0, 0.0);
    // EDGE* e1 = NULL;
    // double weight1[4] = {0.2, 0.5, 0.5, 0.2};
    // api_mk_ed_int_ctrlpts(degree, 1, 0, 0, 4, ctrlpts1, weight1, SPAresabs, num_knots_u, knots_u, SPAresabs, e1);
    // EDGE* edge[2];
    // edge[0] = e;
    // edge[1] = e1;
    // BODY* wirebody=NULL;
    // api_make_ewire(2, edge, wirebody);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 1, normal, wirebody2);
    // output_ents.add(wirebody);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -1, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);
    // ENTITY_LIST LIST;
    // LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\gme_combine_NURBSNURBS.sat", LIST);

    ////测试变距1
    // SPAposition p1(-10, -5, 0);
    // SPAposition p2(10, -5, 0);
    // SPAposition p3(0, 10, 0);
    // EDGE* e1 = NULL;
    // EDGE* e2 = NULL;
    // EDGE* e3 = NULL;
    // BODY* wirebody = NULL;
    // api_curve_line(p1, p2, e1);
    // api_curve_line(p2, p3, e2);
    // api_curve_line(p3, p1, e3);
    // EDGE* edges[3];
    // edges[0] = e1;
    // edges[1] = e2;
    // edges[2] = e3;
    // api_make_ewire(3, edges, wirebody);
    // SPAunit_vector normal(0, 0, 1);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // BODY* wirebody2 = NULL;
    // BODY* wirebody3 = NULL;
    // WIRE* oriwire = (WIRE*)wires[0];
    // wire_offset_options option;
    // char my_function_string[50];
    // strcpy(my_function_string, "(2*x)");
    // law* twist;
    // api_str_to_law(my_function_string, &twist, 0, NULL);
    // law* x = new identity_law(0);
    // law* d = new cos_law(x);
    // option.set_twist_law(twist);
    // option.set_distance(d);
    // option.set_plane_normal(normal);
    // option.set_gap_type(arc);
    // api_offset_planar_wire(wirebody, &option, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // api_rh_set_entity_rgb(wirebody3, Blue);

    //// gme_api_offset_planar_wire(wirebody, 2, normal, wirebody2);
    // output_ents.add(wirebody);
    // output_ents.add(wirebody2);

    ////测试变距2
    // SPAposition ctrlpts[4];
    // ctrlpts[0] = SPAposition(-5.0, 0.0, 0.0);
    // ctrlpts[1] = SPAposition(-5.0, 5.0, 0.0);
    // ctrlpts[2] = SPAposition(5.0,  5.0, 0.0);
    // ctrlpts[3] = SPAposition(5.0,  0.0, 0.0);
    // EDGE* e = NULL;
    // EDGE* e1 = NULL;
    // double weight[4] = {1, 0.5, 0.5, 1};
    // int num_knots_u = 7;
    // double knots_u[7] = {0.0, 0.0, 0.0, 0.5, 1.0, 1.0, 1.0};
    // int degree = 2;
    // logical national = 1;
    // api_mk_ed_int_ctrlpts(degree, 1, 0, 0, 4, ctrlpts, weight, SPAresabs, num_knots_u, knots_u, SPAresabs, e);
    //  APOINT* a10 = ACIS_NEW APOINT(ctrlpts[0]);
    //  APOINT* a11 = ACIS_NEW APOINT(ctrlpts[3]);
    //  VERTEX* v10 = ACIS_NEW VERTEX(a10);
    //  VERTEX* v11 = ACIS_NEW VERTEX(a11);
    //  ELLIPSE* ee = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(5, 0, 0), 0.5);
    //  e1 = ACIS_NEW EDGE(v11, v10, ee, 1);
    // EDGE* edge[2];
    // edge[0] = e;
    // edge[1] = e1;
    // BODY* wirebody=NULL;
    // api_make_ewire(2, edge, wirebody);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;
    // wire_offset_options option;
    // char my_function_string[50];
    // strcpy(my_function_string, "(2*x)+2*y");
    // law* twist;
    // api_str_to_law(my_function_string, &twist, 0, NULL);
    ////law* x = new identity_law(0);
    ////law* d = new cos_law(x);
    // option.set_twist_law(twist);
    // option.set_distance(1);
    // option.set_plane_normal(normal);
    // option.set_gap_type(arc);
    // api_offset_planar_wire(wirebody, &option, wirebody2);
    // output_ents.add(wirebody);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);

    ////变距1
    // APOINT* a = ACIS_NEW APOINT(SPAposition(10.0, 0.0, 0.0));
    // VERTEX* v111 = ACIS_NEW VERTEX(a);
    // ELLIPSE* ee = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(10, 0, 0), 1);
    // EDGE* e1 = ACIS_NEW EDGE(v111, v111, ee, 1);

    // EDGE* edge[1];
    // edge[0] = e1;
    // BODY* wirebody = NULL;
    // api_make_ewire(1, edge, wirebody);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;
    // wire_offset_options option;
    // char my_function_string[50];
    // strcpy(my_function_string, "(2*x)");
    // law* twist;
    // api_str_to_law(my_function_string, &twist, 0, NULL);
    //// law* x = new identity_law(0);
    //// law* d = new cos_law(x);
    // double d = 5;
    // option.set_twist_law(twist);
    // option.set_distance(d);
    // option.set_plane_normal(normal);
    // option.set_gap_type(arc);
    // gme_api_offset_planar_wire(wirebody, &option, wirebody2);
    // output_ents.add(wirebody);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);

    // curve_law_data* v39 = ACIS_NEW curve_law_data(*geometry, off_domain.start_pt(), off_domain.end_pt());
    // curve_law_data* cld = v39;
    // curve_law* v41 = ACIS_NEW curve_law(cld);
    /*    SPAunit_vector n(0, 0, 1);
        double param = M_PI / 8;
        SPAposition t1;
        SPAvector* t1_[3];
        SPAvector gpx(0.0, 0.0, 0.0);
        SPAvector gppx(0.0, 0.0, 0.0);
        SPAvector gpppx(0.0, 0.0, 0.0);
        t1_[0] = &gpx;
        t1_[1] = &gppx;
        t1_[2] = &gpppx;
        ee->equation().evaluate(param, t1, t1_, 1);
        double deriv_length = t1_[0]->len();
        SPAvector px = (1 / deriv_length) * (*t1_[0]);
        SPAvector v = px * n;
        double tx = -twist->eval(param);
        double costx = acis_cos(tx);
        double sintx = acis_sin(tx);
        SPAvector v2 = sintx * n;
        SPAvector v1 = costx * v;
        SPAvector v16 = v1 + v2;
        SPAvector v17 = v16 * d;
        SPAposition pos = t1 + v17;

        output_ents.add(wirebody);
        rgb_color Red(1.0, 0.0, 0.0);
        api_rh_set_entity_rgb(wirebody2, Red);
        output_ents.add(wirebody2);
        ENTITY_LIST LIST;
        LIST.add(wirebody2);
        acis_api_save_entity_list("D:\\11月底里程碑考核\\law1.sat", LIST);
        SPAposition p11;
        SPAvector v11;
        wirebody->lump()->shell()->wire()->coedge()->edge()->geometry()->equation().eval(param,p11,v11 );
        SPAposition p22;
        SPAvector v22;
        wirebody2->lump()->shell()->wire()->coedge()->edge()->geometry()->equation().eval(param,p22,v22);*/

    // // 变距2
    //  APOINT* a = ACIS_NEW APOINT(SPAposition(20.0, 0.0, 0.0));
    //  VERTEX* v = ACIS_NEW VERTEX(a);
    //  ELLIPSE* ee = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(20, 0, 0), 1);
    //  EDGE* e1 = ACIS_NEW EDGE(v, v, ee, 1);
    //  EDGE* edge[1];
    //  edge[0] = e1;
    //  BODY* wirebody = NULL;
    //  api_make_ewire(1, edge, wirebody);
    //  ENTITY_LIST wires;
    //  api_get_wires(wirebody, wires);
    //  SPAunit_vector normal(0, 0, 1);
    //  BODY* wirebody2 = NULL;
    //  wire_offset_options option;
    //  char my_function_string[50];
    //  strcpy(my_function_string, "-20-5*cos(x*2)");
    //  law* d;
    //  api_str_to_law(my_function_string, &d, 0, NULL);
    //  //double dist = d->eval(1.0);
    //  //int aed = d->return_dim();
    //  constant_law* twist = ACIS_NEW constant_law(0);

    // option.set_distance(d);
    // option.set_plane_normal(normal);
    // option.set_twist_law((law*)twist);
    // option.set_gap_type(arc);
    // api_offset_planar_wire(wirebody, &option, wirebody2);
    // //SPAposition p11 = wirebody->lump()->shell()->wire()->coedge()->edge()->geometry()->equation().eval_position(M_PI/4);
    // //SPAposition p22 = wirebody2->lump()->shell()->wire()->coedge()->edge()->geometry()->equation().eval_position(M_PI/4);
    ///*   double period = e1->geometry()->equation().param_period();
    // */
    //  output_ents.add(wirebody);
    //  rgb_color Red(1.0, 0.0, 0.0);
    //  api_rh_set_entity_rgb(wirebody2, Red);
    //  output_ents.add(wirebody2);

    //////复杂的曲线  两个半圆+y一个半圆
    // APOINT* a10 = ACIS_NEW APOINT(-4, 0, 0);
    // APOINT* a11 = ACIS_NEW APOINT(4,0,0);
    // APOINT* a20 = ACIS_NEW APOINT(4, 0, 0);
    // APOINT* a21 = ACIS_NEW APOINT(12, 0, 0);
    // VERTEX* v20 = ACIS_NEW VERTEX(a20);
    // VERTEX* v21 = ACIS_NEW VERTEX(a21);
    // VERTEX* v10 = ACIS_NEW VERTEX(a10);
    // VERTEX* v11 = ACIS_NEW VERTEX(a11);
    // ELLIPSE* e1 = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 1);
    // ELLIPSE* e2 = ACIS_NEW ELLIPSE(SPAposition(8, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 1);
    // ELLIPSE* e3 = ACIS_NEW ELLIPSE(SPAposition(4, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(8, 0, 0), 1);
    // EDGE* e22 = ACIS_NEW EDGE(v20, v21, e2, 1);
    // EDGE* e11 = ACIS_NEW EDGE(v10, v11, e1, 1);
    // EDGE* e33 = ACIS_NEW EDGE(v21, v10, e3, 1);
    // EDGE* edge[3];
    // edge[0] = e11;
    // edge[1] = e22;
    // edge[2] = e33;
    // BODY* wirebody = NULL;
    // api_make_ewire(3, edge, wirebody);
    // output_ents.add(wirebody);
    ////output_ents.add(wirebody1);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;
    // wire_offset_options option;
    // option.set_gap_type(corner);
    // char my_function_string[50];
    // strcpy(my_function_string,"1");
    // law* d;
    // api_str_to_law(my_function_string, &d, 0, NULL);
    // option.set_distance(2);
    // option.set_plane_normal(normal);
    // api_offset_planar_wire(wirebody, &option, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // api_offset_planar_wire((WIRE*)wires[0], NULL, -5, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);

    /*   APOINT* a10 = ACIS_NEW APOINT(-4, 0, 0);
       APOINT* a11 = ACIS_NEW APOINT(4, 0, 0);
       APOINT* a20 = ACIS_NEW APOINT(4, 0, 0);
       APOINT* a21 = ACIS_NEW APOINT(12, 0, 0);
       VERTEX* v20 = ACIS_NEW VERTEX(a20);
       VERTEX* v21 = ACIS_NEW VERTEX(a21);
       VERTEX* v10 = ACIS_NEW VERTEX(a10);
       VERTEX* v11 = ACIS_NEW VERTEX(a11);
       ELLIPSE* e1 = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 1);
       ELLIPSE* e2 = ACIS_NEW ELLIPSE(SPAposition(8, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 1);
       ELLIPSE* e3 = ACIS_NEW ELLIPSE(SPAposition(4, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(8, 0, 0), 1);
       EDGE* e22 = ACIS_NEW EDGE(v20, v21, e2, 1);
       EDGE* e11 = ACIS_NEW EDGE(v10, v11, e1, 1);
       EDGE* e33 = ACIS_NEW EDGE(v21, v10, e3, 1);
       EDGE* edge[1];
       edge[0] = e11;
       EDGE* edge1[1];
       edge1[0] = e22;
       EDGE* edge2[1];
       edge2[0] = e33;
       BODY* wirebody1 = NULL;
       BODY* wirebody2 = NULL;
       BODY* wirebody3 = NULL;
       BODY* wirebody11 = NULL;
       BODY* wirebody22 = NULL;
       BODY* wirebody33 = NULL;
       api_make_ewire(1, edge, wirebody1);
       api_make_ewire(1, edge1, wirebody2);
       api_make_ewire(1, edge2, wirebody3);
       SPAunit_vector normal(0, 0, 1);
       wire_offset_options option;
       option.set_gap_type(corner);
       char my_function_string[50];
       strcpy(my_function_string, "1");
       law* d;
       api_str_to_law(my_function_string, &d, 0, NULL);
       option.set_distance(2);
       option.set_plane_normal(normal);
       api_offset_planar_wire(wirebody1, &option, wirebody11);
       api_offset_planar_wire(wirebody2, &option, wirebody22);
       api_offset_planar_wire(wirebody3, &option, wirebody33);
       rgb_color Red(1.0, 0.0, 0.0);
       api_rh_set_entity_rgb(wirebody11, Red);
       api_rh_set_entity_rgb(wirebody22, Red);
       api_rh_set_entity_rgb(wirebody33, Red);
       output_ents.add(wirebody1);
       output_ents.add(wirebody2);
       output_ents.add(wirebody3);*/
    // output_ents.add(wirebody11);
    // output_ents.add(wirebody22);
    // output_ents.add(wirebody33);

    // APOINT* a10 = ACIS_NEW APOINT(-4, 0, 0);
    // APOINT* a11 = ACIS_NEW APOINT(4, 0, 0);
    // VERTEX* v10 = ACIS_NEW VERTEX(a10);
    // VERTEX* v11 = ACIS_NEW VERTEX(a11);
    // ELLIPSE* e1 = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 1);
    // EDGE* e11 = ACIS_NEW EDGE(v10, v11, e1, 1);
    // EDGE* e22 = NULL;
    // api_curve_line(SPAposition(4, 0, 0), SPAposition(5, 5, 0), e22);
    //// e1->equation().point_perp
    // EDGE* edge[2];
    // edge[0] = e11;
    // edge[1] = e22;
    // BODY* wirebody = NULL;
    // api_make_ewire(2, edge, wirebody);
    // output_ents.add(wirebody);
    //// output_ents.add(wirebody1);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;
    // wire_offset_options option;
    // option.set_gap_type(arc);
    // char my_function_string[50];
    // strcpy(my_function_string, "-0.5");
    // law* d;
    // api_str_to_law(my_function_string, &d, 0, NULL);
    // option.set_distance(d);
    // api_offset_planar_wire(wirebody, &option, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);

    ////例子1  复杂轮廓
    // SPAposition p1(-10, 0, 0);
    // SPAposition p2(10, 0, 0);
    // SPAposition p3(2, 10, 0);
    // SPAposition p4(10, 20, 0);
    // SPAposition p5(-10, 20, 0);
    // SPAposition p6(-2, 10, 0);
    // SPAposition p7(-50, -25, 0);
    // SPAposition p8(50, -25, 0);
    // SPAposition p9(0, 50, 0);
    // EDGE* e1 = NULL;
    // EDGE* e2 = NULL;
    // EDGE* e3 = NULL;
    // EDGE* e4 = NULL;
    // EDGE* e5 = NULL;
    // EDGE* e6 = NULL;
    // EDGE* e7 = NULL;
    // EDGE* e8 = NULL;
    // EDGE* e9 = NULL;
    // BODY* wirebody;
    // BODY* wirebody1;
    // api_curve_line(p1, p2, e1);
    // api_curve_line(p2, p3, e2);
    // api_curve_line(p3, p4, e3);
    // api_curve_line(p4, p5, e4);
    // api_curve_line(p5, p6, e5);
    // api_curve_line(p6, p1, e6);
    // api_curve_line(p7, p8, e7);
    // api_curve_line(p8, p9, e8);
    // api_curve_line(p9, p7, e9);
    //////SPAposition p4 = e1->start()->geometry()->coords();
    ////const curve& eq1 = e1->geometry()->equation();
    ////double a = ((straight&)eq1).subset_range.start_pt();
    ////SPAposition p = ((straight&)eq1).root_point;
    ////SPAposition foot;
    ////eq1.point_perp(p4, foot);
    ////eq1.param()
    ////int a = 0;
    // EDGE *edges[6];
    // edges[0] = e1;
    // edges[1] = e2;
    // edges[2] = e3;
    // edges[3] = e4;
    // edges[4] = e5;
    // edges[5] = e6;
    // EDGE* edges1[3];
    // edges1[0] = e7;
    // edges1[1] = e8;
    // edges1[2] = e9;
    // api_make_ewire(6, edges, wirebody);
    // api_make_ewire(3, edges1, wirebody1);
    // api_combine_body(wirebody1, wirebody);
    ////api_make_ewires(9, edges, 2, wirebody);
    ////(6, edges, wirebody);
    // output_ents.add(wirebody);
    /////* COEDGE* c0 = ACIS_NEW COEDGE(e1, REVERSED, NULL, NULL);
    //// VERTEX* t = c0->end();*/
    // SPAunit_vector normal(0, 0, 1);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // BODY* wirebody2 = NULL;
    // BODY* wirebody3 = NULL;
    // WIRE* oriwire = (WIRE*)wires[0];

    // LARGE_INTEGER frequency;   // 频率
    // LARGE_INTEGER start, end;  // 起始和结束时间
    // QueryPerformanceFrequency(&frequency);
    // QueryPerformanceCounter(&start);
    // api_offset_planar_wire(wirebody, 5, normal, wirebody2);
    // QueryPerformanceCounter(&end);
    // double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    // double elapsedmilliseconds = elapsedseconds * 1000.0;
    // double elapsedmicroseconds = elapsedseconds * 1e6;
    // double elapsednanoseconds = elapsedseconds * 1e9;
    // AllocConsole();
    // freopen("conout$", "w", stdout);
    // printf("running time is %f\n", elapsedmilliseconds);
    // printf("running time  is %f\n", elapsedmicroseconds);
    // printf("running time  is %f\n", elapsednanoseconds);

    // gme_api_offset_planar_wire(wirebody, 3.5, normal, wirebody3);
    ////gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -3.5, normal, wirebody2);
    ////gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 3.5, normal, wirebody3);
    // rgb_color Red(1.0, 0.0, 0.0);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody);
    // output_ents.add(wirebody2);
    // ENTITY_LIST LIST;
    // LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\gme_two_wires", LIST);

    // output_ents.add(wirebody3);

    //// 例子20  复杂非闭合折线
    // SPAposition p1(-5, -10, 0);
    // SPAposition p2(-5, 10, 0);
    // SPAposition p3(0, 10, 0);
    // SPAposition p4(0, 1, 0);
    // SPAposition p5(2, 1, 0);
    // SPAposition p6(2, 5, 0);
    // SPAposition p7(6, 5, 0);
    // SPAposition p8(6, -5, 0);
    // SPAposition p9(2, -5, 0);
    // SPAposition p10(2, -1, 0);
    // SPAposition p11(0, -1, 0);
    // SPAposition p12(0, -10, 0);
    // EDGE* e1 = NULL;
    // EDGE* e2 = NULL;
    // EDGE* e3 = NULL;
    // EDGE* e4 = NULL;
    // EDGE* e5 = NULL;
    // EDGE* e6 = NULL;
    // EDGE* e7 = NULL;
    // EDGE* e8 = NULL;
    // EDGE* e9 = NULL;
    // EDGE* e10 = NULL;
    // EDGE* e11 = NULL;
    // EDGE* e12 = NULL;
    // BODY* wirebody = NULL;
    // api_curve_line(p1, p2, e1 );
    // api_curve_line(p2, p3, e2 );
    // api_curve_line(p3, p4, e3 );
    // api_curve_line(p4, p5, e4 );
    // api_curve_line(p5, p6, e5 );
    // api_curve_line(p6, p7, e6 );
    // api_curve_line(p7, p8, e7 );
    // api_curve_line(p8, p9, e8 );
    // api_curve_line(p9, p10, e9 );
    // api_curve_line(p10, p11,e10);
    // api_curve_line(p11, p12,e11);
    // api_curve_line(p12, p1, e12);
    // EDGE* edges[12];
    // edges[0] = e1 ;
    // edges[1] = e2 ;
    // edges[2] = e3 ;
    // edges[3] = e4 ;
    // edges[4] = e5 ;
    // edges[5] = e6 ;
    // edges[6] = e7 ;
    // edges[7] = e8 ;
    // edges[8] = e9 ;
    // edges[9] = e10;
    // edges[10]= e11;
    // edges[11]= e12;
    // api_make_ewire(12, edges, wirebody);
    // output_ents.add(wirebody);
    // SPAunit_vector normal(0, 0, 1);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // BODY* wirebody2 = NULL;
    // WIRE* oriwire = (WIRE*)wires[0];

    // LARGE_INTEGER frequency;   // 频率
    // LARGE_INTEGER start, end;  // 起始和结束时间
    // QueryPerformanceFrequency(&frequency);
    // QueryPerformanceCounter(&start);
    // api_offset_planar_wire((WIRE*)wires[0], NULL, 1.1, normal, wirebody2);
    // QueryPerformanceCounter(&end);
    // double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    // double elapsedmilliseconds = elapsedseconds * 1000.0;
    // double elapsedmicroseconds = elapsedseconds * 1e6;
    // double elapsednanoseconds = elapsedseconds * 1e9;
    // AllocConsole();
    // freopen("conout$", "w", stdout);
    // printf("running time is %f\n", elapsedmilliseconds);
    // printf("running time  is %f\n", elapsedmicroseconds);
    // printf("running time  is %f\n", elapsednanoseconds);
    //
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    //// gme_api_offset_planar_wire(wirebody, 2, normal, wirebody2);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -1.1, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);
    // ENTITY_LIST LIST;
    // LIST.add(wirebody2);
    // acis_api_save_entity_list("D:\\11月底里程碑考核\\gme_complex_wire", LIST);

    // SPAposition p1(-6, 0, 0);
    // SPAposition p2(-3, 4, 0);
    // SPAposition p3(3, 4, 0);
    // SPAposition p4(6, 0, 0);
    // EDGE* e1 = NULL;
    // EDGE* e2 = NULL;
    // api_curve_line(p1, p2, e1);
    // api_curve_line(p4, p3, e2);
    // EDGE* edges[1];
    // edges[0] = e1 ;
    // EDGE* edges1[1];
    // edges1[0] = e2 ;
    // BODY* wirebody;
    // BODY* wirebody1;
    // api_make_ewire(1, edges, wirebody);
    // api_make_ewire(1, edges1, wirebody1);
    // output_ents.add(wirebody);
    // output_ents.add(wirebody1);
    // APOINT* a10 = ACIS_NEW APOINT(-3, 4, 0);
    // APOINT* a11 = ACIS_NEW APOINT(3, 4, 0);
    // VERTEX* v10 = ACIS_NEW VERTEX(a10);
    // VERTEX* v11 = ACIS_NEW VERTEX(a11);
    // ELLIPSE* e11 = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(5, 0, 0), 1);
    // EDGE* ee = ACIS_NEW EDGE(v10, v11, e11, 1);
    // EDGE* e22 = NULL;
    // EDGE* edges2[1];
    // edges2[0] = ee;
    // BODY* wirebody2;
    // api_make_ewire(1, edges2, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);
    // SPAposition p5(0, 8, 0);
    // EDGE* e3 = NULL;
    // EDGE* e4 = NULL;
    // api_curve_line(p2, p5, e3);
    // api_curve_line(p5, p3, e4);
    // EDGE* edges3[2];
    // edges3[0] = e3;
    // edges3[1] = e4;
    // BODY* wirebody3;
    // api_make_ewire(2, edges3, wirebody3);
    // rgb_color Blue(0.0, 0.0, 1);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);

    // 例子测试   椭圆
    // double t = 0.5*M_PI;
    // SPAinterval range(4 * M_PI, 6 * M_PI);
    // double zero_based_t = t - range.start_pt();
    // t = (fmod(zero_based_t, 2 * M_PI) + range.start_pt())/M_PI;
    // double s = sqrt(-2);

    // double mod = fmod(-3.5 * M_PI, 2 * M_PI)/M_PI;
    // double ab = fabs((-3.5 * M_PI)/ (2 * M_PI));
    // double param_best = 0.0;
    // SPAposition foot_best(0.0, 0.0, 0.0);
    // SPAunit_vector tan_best(0.0, 0.0, 0.0);
    // SPAvector foot_dt_best(0.0, 0.0, 0.0);
    // SPAvector foot_ddt_best(0.0, 0.0, 0.0);

    // SPAposition p(2, 3, 0);
    // SPAposition foot;
    // double aa = safe_atan2(1.8064929*2, 1.7164888);
    // double bb = safe_atan2(3*2,2);
    // ellipse* e0 = ACIS_NEW ellipse(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 0.5);

    // ellipse* ee0 = ACIS_NEW ellipse(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 1);

    // double aaa = e0->param(SPAposition(2.82842712, 1.41421356, 0));

    // EDGE* ee1;
    // EDGE* ee2;
    // api_curve_line(SPAposition(2 * sqrt(2), 0, 0), p, ee1);
    // api_make_edge_from_curve(e0, ee2);
    // curve_curve_int* curint;
    // sg_inter_ed_ed(ee1, ee2, curint, SPAresabs, SPAresnor);
    // SPAposition p2;
    ////= e0->eval_position(aa);
    // SPAvector dt;
    // e0->eval(0, p2, dt);
    ///*   SPAvector a(p2 - p1);*/
    //// SPAvector o(1, 0, 0);
    //// double angle = find_angle(a, o)*(M_PI/180);
    // int point_found = 0;

    // SPAparameter ell_par;
    // point_perp(e0, p, &foot, &SpaAcis::NullObj::get_unit_vector(), &SpaAcis::NullObj::get_unit_vector(), &SpaAcis::NullObj::get_parameter(), 0);
    // SPAposition foot1(foot);
    // int o = 0;

    /* point_perp_internal(e0, p, M_PI / 3, param_best, foot_best, tan_best, foot_dt_best, foot_ddt_best, 0, SPAresabs, point_found);
int kk = point_found;
int uy;*/

    // SPAposition p1(0, 0, 2);
    // SPAposition foot;
    // e0->point_perp(p1, foot);
    // SPAvector maj = e0->major_axis;
    // SPAvector min = e0->minor_axis;
    // SPAposition p(2, 3, 0);
    // SPAposition o(0, 0, 0);
    // SPAvector offset = p - o;
    // double cos = offset % maj;
    // double sin = (offset % (e0->normal * maj)) / e0->radius_ratio;
    // double tan = sin / cos;
    // int u = sqrt(-1.25);
    // int g = 0;

    /* SPAposition foot2;
     SPAposition p(2, 3, 0);
     e0->point_perp(p, foot2);
     SPAparameter t;
     t = e0->param(foot2);
     SPAvector foot_dt;
     SPAvector foot_ddt;
     e0->eval(t, foot2, foot_dt, foot_ddt);
     SPAposition p1 = e0->eval_position(0.5);
     SPAposition p2 = e0->eval_position(M_PI );
     SPAposition p3 = e0->eval_position(3*M_PI / 2);
     SPAposition p4 = e0->eval_position(M_PI *2);
     SPAvector b = e0->major_axis;
     int c = 0;
     SPAparameter* param_guess = &SpaAcis::NullObj::get_parameter();
     SPAposition foot1;
     SPAposition* foot = &foot1;
     int h = foot && !SpaAcis::NullObj::check_position(*foot);
     int a = !param_guess || SpaAcis::NullObj::check_parameter(*param_guess);
     double w = 0;
     SPAposition a1;
     SPAvector a11;
     e0->eval(w, a1, a11);
     a++;*/

    // APOINT* a10 = ACIS_NEW APOINT(-4, 0, 0);
    // APOINT* a11 = ACIS_NEW APOINT(4, 0, 0);
    // VERTEX* v10 = ACIS_NEW VERTEX(a10);
    // VERTEX* v11 = ACIS_NEW VERTEX(a11);
    // ELLIPSE* e1 = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 2);
    // EDGE* e11 = ACIS_NEW EDGE(v11, v11, e1, 1);
    // SPAinterval b = e11->geometry()->equation().subset_range;
    // int l = e11->geometry()->equation().subset_range.mid_pt();
    // int c = ((ellipse*)e1->equation()).major_axis
    // int eee = e11->geometry()->equation().subset_range.mid_pt();
    //  EDGE* e = NULL;
    //  api_make_edge_from_curve(e1, e);
    //  int a = e->geometry()->equation().type();
    //  EDGE* edge[1];
    //  edge[0] = e;
    //  BODY* wirebody;
    //  api_make_ewire(1, edge, wirebody);
    //  output_ents.add(wirebody);
    //  ENTITY_LIST wires;
    //  api_get_wires(wirebody, wires);
    //  SPAunit_vector normal(0, 0, 1);
    //  BODY* wirebody2 = NULL;
    //  gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -2, normal, wirebody2);
    //  rgb_color Red(1.0, 0.0, 0.0);
    //  api_rh_set_entity_rgb(wirebody2, Red);
    //  output_ents.add(wirebody2);
    //  rgb_color Blue(0.0, 0.0, 1.0);
    //  BODY* wirebody3 = NULL;
    //  BODY* wirebody4 = NULL;
    //  gme_api_offset_planar_wire(wirebody, 1, normal, wirebody3);
    //  gme_api_offset_planar_wire(wirebody3, 1, normal, wirebody4);
    //  int b = wirebody3->lump()->shell()->wire()->coedge()->edge()->geometry()->equation().type();
    //  int c = wirebody4->lump()->shell()->wire()->coedge()->edge()->geometry()->equation().type();
    //  api_rh_set_entity_rgb(wirebody3, Blue);
    //  output_ents.add(wirebody3);

    // SPAposition p(0, 0, 0);
    // SPAposition p1(1, 1, 0);
    // EDGE* e=NULL;
    // api_curve_line(p, p1, e);
    // const curve* geometry = &e->geometry()->equation();
    // SPAinterval off_domain = e->param_range();
    // curve_law_data* v39 = ACIS_NEW curve_law_data(*geometry, off_domain.start_pt(), off_domain.end_pt());
    // curve_law_data* cld = v39;
    // double curv = cld->curvature(0.2);
    // curve_law* v41 = ACIS_NEW curve_law(cld);
    // double s = 0.3;
    // double* eeee;
    // SPAposition pp=v41->evaluateM_P(& s);
    // int o = 9;
    // law* aaa = v41;
    // double x1 = 0.1;
    // SPAposition p2 = aaa->evaluateM_P(&x1);
    // api_make_edge_from_curve(ell,)
    // CURVE* simp_curv = test_for_line(aaa, off_domain);

    /* SPAposition p1(-10, -4, 0);
     SPAposition p2(10, -5, 0);
     SPAposition p3(13, -2, 0);
     SPAposition p4(13, 5, 0);
     EDGE* e1 = NULL;
     EDGE* e3 = NULL;
     APOINT* a1 = ACIS_NEW APOINT(10, -5, 0);
     APOINT* a2 = ACIS_NEW APOINT(13, -2, 0);
     VERTEX* v1 = ACIS_NEW VERTEX(a1);
     VERTEX* v2 = ACIS_NEW VERTEX(a2);
     BODY* wirebody = NULL;
     BODY* wirebody2 = NULL;
     api_curve_line(p1, p2, e1);
     api_curve_line(p3, p4, e3);
     SPAunit_vector normal(0, 0, 1);
     SPAvector maj(0, 3, 0);
     ELLIPSE* ell = ACIS_NEW ELLIPSE(SPAposition(10, -2, 0), normal, maj, 1.0);
     EDGE* e2 = ACIS_NEW EDGE(v1, v2, ell, FORWARD);

     EDGE* e[3];
     e[0] = e1;
     e[1] = e2;
     e[2] = e3;
     api_make_ewire(3, e, wirebody);
     gme_api_offset_planar_wire(wirebody, -3, normal, wirebody2);
     output_ents.add(wirebody);
     output_ents.add(wirebody2);*/

    return 0;
}

void exampleFunction() {
    SPAposition p1(-10, -5, 0);
    SPAposition p2(10, -5, 0);
    SPAposition p3(0, 10, 0);
    EDGE* e1 = NULL;
    EDGE* e2 = NULL;
    EDGE* e3 = NULL;
    BODY* wirebody = NULL;
    api_curve_line(p1, p2, e1);
    api_curve_line(p2, p3, e2);
    api_curve_line(p3, p1, e3);
    // SPAposition p4 = e1->start()->geometry()->coords();
    const curve& eq1 = e1->geometry()->equation();
    double a = ((straight&)eq1).subset_range.start_pt();
    SPAposition p = ((straight&)eq1).root_point;
    // SPAposition foot;
    // eq1.point_perp(p4, foot);
    // eq1.param()
    // int a = 0;
    EDGE* edges[3];
    edges[0] = e1;
    edges[1] = e2;
    edges[2] = e3;
    api_make_ewire(3, edges, wirebody);
    /* COEDGE* c0 = ACIS_NEW COEDGE(e1, REVERSED, NULL, NULL);
       VERTEX* t = c0->end();*/
    SPAunit_vector normal(0, 0, 1);
    ENTITY_LIST wires;
    api_get_wires(wirebody, wires);
    BODY* wirebody2 = NULL;
    BODY* wirebody3 = NULL;
    WIRE* oriwire = (WIRE*)wires[0];

    LARGE_INTEGER frequency;   // 频率
    LARGE_INTEGER start, end;  // 起始和结束时间
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    api_offset_planar_wire((WIRE*)wires[0], NULL, -3, normal, wirebody2);
    QueryPerformanceCounter(&end);
    double elapsedseconds = static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(frequency.QuadPart);
    double elapsedmilliseconds = elapsedseconds * 1000.0;
    double elapsedmicroseconds = elapsedseconds * 1e6;
    double elapsednanoseconds = elapsedseconds * 1e9;
    AllocConsole();
    freopen("conout$", "w", stdout);
    printf("running time is %f\n", elapsedmilliseconds);
    printf("running time  is %f\n", elapsedmicroseconds);
    printf("running time  is %f\n", elapsednanoseconds);
}

//void point_perp(ellipse* e, SPAposition point, SPAposition* foot, SPAunit_vector* tangent, SPAvector* curv, SPAparameter* param_guess, SPAparameter* param_actual) {
//    double cosang;
//    double sinang;
//    double param1;
//    double tol = 0.001 * SPAresabs;
//    double param = 0.0;
//    SPAposition foot_current(0.0, 0.0, 0.0);
//    SPAunit_vector tan_current(0.0, 0.0, 0.0);
//    SPAvector foot_dt;
//    SPAvector foot_ddt;
//    SPAparameter param_best(0.0);
//    SPAposition foot_best(0.0, 0.0, 0.0);
//    SPAunit_vector tan_best(0.0, 0.0, 0.0);
//    SPAvector foot_dt_best(0.0, 0.0, 0.0);
//    SPAvector foot_ddt_best(0.0, 0.0, 0.0);
//    int point_found = 0;
//    double param0;
//    SPAvector offset = point - e->centre;
//    for(int method = 0;; ++method) {
//        if(point_found) return;
//        if(method >= 6) {
//            if(GET_ALGORITHMIC_VERSION() >= AcisVersion(15, 0, 0)) return;
//        }
//        if(!method || method == 2 || method == 4) {
//            if(!param_guess || SpaAcis::NullObj::check_parameter(*param_guess)) continue;
//            param = (*param_guess).operator double();
//        } else {
//            // 初值：
//            cosang = offset % e->major_axis;
//            sinang = (offset % (e->normal * e->major_axis)) / e->radius_ratio;
//            param1 = safe_atan2(sinang, cosang);
//            SPAvector perp_off = point - e->centre;
//            double v133 = perp_off % e->major_axis;
//            double v47 = e->major_axis % e->major_axis;
//            cosang = v133 / v47;
//            if(fabs(v133 / v47) < 1.0 - e->radius_ratio * e->radius_ratio) {
//                sinang = sqrt(1.0 - cosang * cosang);
//                SPAvector v48 = e->normal * e->major_axis;
//                if(perp_off % v48 < 0.0) sinang = -sinang;
//                param0 = safe_atan2(sinang, cosang);
//                double mix = e->radius_ratio * e->radius_ratio;
//                param = (1.0 - mix) * param0 + mix * param1;
//            }
//
//            else {
//                param = param1;
//            }
//            // param = reduce_to_range_0(param, &this->subset_range);
//        }
//        if(method >= 2) {
//            step_from_guess(e, point, &param);
//        }
//
//        BOOL v73 = method >= 4;
//        double v137 = tol;
//        point_perp_internal(e, &point, &param, &param_best, &foot_best, &tan_best, &foot_dt_best, &foot_ddt_best, v73, v137, &point_found);
//
//        if(e->subset_range.bounded()) {
//            if(GET_ALGORITHMIC_VERSION() > AcisVersion(33, 0, 0) && (!param_guess || SpaAcis::NullObj::check_parameter(*param_guess))) {
//                SPAposition p1(foot_best);
//                SPAposition v161 = e->eval_position(e->subset_range.start_pt());
//                SPAposition v162 = e->eval_position(e->subset_range.end_pt());
//                SPAvector v52 = operator-(p1 - point);
//                double v91 = v52.len_sq();
//                double v84 = (v161 - point).len_sq();
//                double v83 = (v162 - point).len_sq();
//                int v65 = 0;
//                int v69 = 0;
//                if(e->subset_range >> param_best) {
//                    if(v83 < v84) {
//                        if(v91 > v83) v69 = 1;
//                    } else if(v91 > v84) {
//                        v65 = 1;
//                    }
//                } else if(v83 < v84) {
//                    v69 = 1;
//                } else {
//                    v65 = 1;
//                }
//                if(v65 || v69) {
//                    if(v65)
//                        param_best = e->subset_range.start_pt();
//                    else
//                        param_best = e->subset_range.end_pt();
//                    e->eval(param_best, foot_best, foot_dt_best, foot_ddt_best, 0, 0);
//                    tan_best = normalise(foot_dt_best);
//                }
//            } else if(!param_best << e->subset_range) {
//                if(param_best < e->subset_range)
//                    param_best = e->subset_range.start_pt();
//                else
//                    param_best = e->subset_range.end_pt();
//                e->eval(param_best, foot_best, foot_dt_best, foot_ddt_best, 0, 0);
//                tan_best = normalise(foot_dt_best);
//            }
//        }
//
//        if(foot && !SpaAcis::NullObj::check_position(*foot)) *foot = foot_best;
//        if(tangent && !SpaAcis::NullObj::check_unit_vector(*tangent)) *tangent = tan_best;
//        if(curv && !SpaAcis::NullObj::check_vector(*curv)) {
//            double foot_dt2 = foot_dt_best % foot_dt_best;
//            double v148 = foot_dt2 * foot_dt2;
//            SPAvector v57 = foot_dt_best * foot_ddt_best;
//            SPAvector v58 = v57 * foot_dt_best;
//            *curv = v58 / v148;
//        }
//        if(param_actual && !SpaAcis::NullObj::check_parameter(*param_actual)) {
//            if(!param_guess || SpaAcis::NullObj::check_parameter(*param_guess)) *param_actual = param_best;
//        }
//    }
//}
//
//void step_from_guess(curve* e, const SPAposition point, double* param_wanted) {
//    SPAposition foot_current(0.0, 0.0, 0.0);
//    SPAvector foot_dt;
//    e->eval(*param_wanted, foot_current, foot_dt);
//    SPAvector offset = point - foot_current;
//    double init_err = offset % foot_dt;
//    double hi_parm = *param_wanted;
//    double lo_parm = *param_wanted;
//    double step = 0.1;
//    double lo_err;
//    double hi_err;
//    do {
//        while(1) {
//            hi_parm = hi_parm + step;
//            e->eval(hi_parm, foot_current, foot_dt);
//            offset = point - foot_current;
//            hi_err = offset % foot_dt;
//            lo_parm = lo_parm - step;
//            e->eval(lo_parm, foot_current, foot_dt);
//            offset = point - foot_current;
//            lo_err = offset % foot_dt;
//            if(hi_err * init_err > 0.0) break;
//            if(lo_err * init_err > 0.0) {
//                *param_wanted = hi_parm - step * 0.5;
//                return;
//            }
//            if(step < 0.0001) {
//                *param_wanted = lo_parm + step * 0.5;
//                return;
//            }
//            hi_parm = hi_parm - step;
//            lo_parm = lo_parm + step;
//            step = step * 0.1;
//        }
//    } while(lo_err * init_err > 0.0);
//    *param_wanted = lo_parm + step * 0.5;
//}
//
//void point_perp_internal(curve* e, SPAposition* point, double* param_wanted, SPAparameter* param_best, SPAposition* foot_best, SPAunit_vector* tan_best, SPAvector* foot_dt_best, SPAvector* foot_ddt_best, int linear_search, double tol, int* point_found) {
//    int alt_method = 0;
//    int need_to_bracket = 1;
//    double param_prev = 0.0;
//    double error_prev = 0.0;
//    double param1 = 0.0;
//    double param2 = 0.0;
//    double error1 = 0.0;
//    double error2 = 0.0;
//    double error_best = SPAresabs;
//    SPAposition foot_current(0.0, 0.0, 0.0);
//    SPAvector foot_dt;
//    SPAvector foot_ddt;
//    SPAunit_vector tan_current(0.0, 0.0, 0.0);
//    double curv_factor;
//    double err_factor;
//    double dparam;
//    double step;
//    double v38;
//    double v14;
//    double v13;
//    double v39;
//    double v42;
//    double v16;
//    double v43;
//    double v17;
//    double new_param;
//    int iter;
//    for(iter = 0; iter < 200 && !*point_found; ++iter) {
//        e->eval(*param_wanted, foot_current, foot_dt, foot_ddt, 0, 0);
//        SPAvector offset = *point - foot_current;
//        tan_current = normalise(foot_dt);
//        double error = offset % tan_current;
//        double v12 = fabs(error);
//        if(error_best > v12) {
//            *param_best = *param_wanted;
//            error_best = fabs(error);
//            *foot_best = foot_current;
//            *tan_best = tan_current;
//            *foot_dt_best = foot_dt;
//            *foot_ddt_best = foot_ddt;
//        }
//        new_param = *param_wanted;
//        if(!alt_method) {
//            if(tol > fabs(error)) {
//                *point_found = 1;
//                return;
//            }
//            if(linear_search) {
//                curv_factor = foot_dt % foot_dt;
//            } else {
//                v38 = foot_dt % foot_dt;
//                v13 = offset % foot_ddt;
//                curv_factor = v38 - v13;
//            }
//            err_factor = offset % foot_dt;
//            v39 = fabs(err_factor);
//            v14 = fabs(curv_factor);
//            if(0.3926990816987241 * v14 <= v39) {
//                if(err_factor < 0.0 == curv_factor < 0.0)
//                    new_param = *param_wanted + 0.1;
//                else
//                    new_param = *param_wanted - 0.1;
//            } else {
//                new_param = *param_wanted + err_factor / curv_factor;
//            }
//            if(iter > 3) {
//                if(GET_ALGORITHMIC_VERSION() >= AcisVersion(15, 0, 0)) {
//                    double conv_limit = 0.99 * param_prev + 0.01 * (*param_wanted);
//                    if(*param_wanted <= param_prev) {
//                        if(new_param >= conv_limit) alt_method = 1;
//                    } else if(conv_limit >= new_param) {
//                        alt_method = 1;
//                    }
//                }
//            }
//        }
//        if(alt_method) {
//            if(need_to_bracket) {
//                if((error <= 0.0 || error_prev <= 0.0) && (error >= 0.0 || error_prev >= 0.0)) {
//                    need_to_bracket = 0;
//                    param1 = param_prev;
//                    param2 = *param_wanted;
//                    error1 = error_prev;
//                    error2 = error;
//                    new_param = (param_prev + (*param_wanted)) / 2.0;
//                } else {
//                    dparam = *param_wanted - param_prev;
//                    if(fabs(*param_wanted - param_prev) >= 0.0001) {
//                        step = 10.0 * dparam;
//                    } else {
//                        step = 0.001;
//                        if(param_prev > *param_wanted) step = -0.001;
//                    }
//                    v42 = fabs(error);
//                    v16 = fabs(error_prev);
//                    if(v16 <= v42)
//                        new_param = new_param - step;
//                    else
//                        new_param = new_param + step;
//                }
//            } else {
//                if((error1 <= 0.0 || error <= 0.0) && (error1 >= 0.0 || error >= 0.0)) {
//                    param2 = *param_wanted;
//                    error2 = error;
//                } else {
//                    param1 = *param_wanted;
//                    error1 = error;
//                }
//                v43 = fabs(param1 - param2);
//                v17 = foot_dt.len();
//                if(tol > v43 * v17) {
//                    *point_found = 1;
//                    return;
//                }
//                new_param = (param1 + param2) / 2.0;
//            }
//        }
//        param_prev = *param_wanted;
//        error_prev = error;
//        *param_wanted = new_param;
//    }
//}
//
//double find_angle(SPAvector& vec1, SPAvector& vec2) {
//    double ang = 180.0;
//    long double v2 = double(SPAresabs);
//    if(!vec1.is_zero(v2)) {
//        long double v3 = double(SPAresabs);
//        if(!vec2.is_zero(v3)) {
//            const SPAvector& v4 = vec1 * vec2;
//            SPAunit_vector normal = normalise(v4);
//            double anga = angle_between(vec1, vec2, normal);
//            if(anga == -1.0) anga = M_PI;
//            ang = anga * 57.29577951308232;
//            if(ang <= 90.0 || ang > 180.0) {
//                if(ang <= 180.0 || ang > 270.0) {
//                    if(ang > 270.0) return 360.0 - ang;
//                } else {
//                    return ang - 180.0;
//                }
//            } else {
//                return 180.0 - ang;
//            }
//        }
//    }
//    return ang;
//}
//
// curve_curve_int* line_ell_int(EDGE* edge1, EDGE* edge2, double dist_tol_in, double angle_tol_in, bool* difficult_config) {
//    curve* ed_cur1 = edge1->geometry()->equation().make_copy();
//    curve* ed_cur2 = edge2->geometry()->equation().make_copy();
//    straight* st_eq = (straight*)ed_cur1;
//    ellipse* ell_eq = (ellipse*)ed_cur2;
//
//    SPAvector centre_offset = ell_eq->centre - st_eq->root_point;
//    SPAunit_vector line_dir(st_eq->direction);
//    double circle_radius = ell_eq->major_axis.len();
//    double ratio = ell_eq->radius_ratio;
//    double v286 = fabs(ratio - 1.0);
//    bool not_circle = v286 > SPAresmch;
//    SPAvector maj_dir;
//    SPAvector min_dir;
//    if(not_circle) {
//        maj_dir = ell_eq->major_axis / circle_radius;
//        min_dir = ell_eq->normal * maj_dir;
//        SPAvector v287 = (centre_offset % min_dir / ratio) * min_dir;
//        SPAvector v1 = (centre_offset % maj_dir) * maj_dir;
//        centre_offset = v1 + v287;
//        SPAvector v289 = (line_dir % min_dir / ratio) * min_dir;
//        SPAvector v290 = line_dir % maj_dir * maj_dir;
//        SPAvector v26 = v290 + v289;
//        line_dir = normalise(v26);
//    }
//    SPAvector parallel_offset = (centre_offset % line_dir) * line_dir;
//    SPAvector perp_offset = centre_offset - parallel_offset;
//    double perp_dist = perp_offset.len();
//    if(not_circle) {
//        SPAvector v291 = (perp_offset % min_dir * ratio) * min_dir;
//        SPAvector v292 = (perp_offset % maj_dir) * maj_dir;
//        perp_offset = v292 + v291;
//    }
//    SPAposition perp_foot = ell_eq->centre - perp_offset;
//    // if(sg_inter_module_header.debug_level >= 0x28) {
//    //     acis_fprintf(debug_file_ptr, "Circle Radius : %0.15f\tPerp Dist : %0.15f\n", (double)circle_radius, (double)perp_dist);
//    //     acis_fprintf(debug_file_ptr, "Dist Tolerance : %.15f\n", (double)dist_tol);
//    //     acis_fprintf(debug_file_ptr, "perp_offset : ");
//    //     SPAvector::debug(&perp_offset, debug_file_ptr);
//    //     debug_newline(debug_file_ptr);
//    // }
//    int intersects = 0;
//    SPAposition inter1;
//    SPAposition inter2;
//    double dist_tol = dist_tol_in;
//    if(perp_dist <= circle_radius + dist_tol) {
//        if(dist_tol <= fabs(circle_radius - perp_dist)) {
//            intersects = 1;
//            int tang_root = 0;
//            double FUZZ_TOL = 3.0 * dist_tol;
//            if(not_circle) {
//                SPAparameter param1;
//                if(ell_eq->test_point_tol(perp_foot, FUZZ_TOL, SpaAcis::NullObj::get_parameter(), param1)) {
//                    SPAposition v468 = ell_eq->eval_position(param1.operator double());
//                    SPAvector v58 = v468 - perp_foot;
//                    bool ellipse_tangent = dist_tol > v58.len();
//                    if(ellipse_tangent) {
//                        SPAposition v481 = interpolate(0.5, perp_foot, v468);
//                        perp_foot = v481;
//                        SPAbox v559 = get_edge_box(edge1);
//                        SPAbox v560 = get_edge_box(edge2);
//                        SPAposition v340 = v559.low();
//                        SPAposition v341 = v559.high();
//                        SPAvector v59 = v341 - v340;
//                        double v209 = v59.len();
//                        SPAposition v342 = v560.low();
//                        SPAposition v343 = v560.high();
//                        SPAvector v60 = v343 - v342;
//                        double v210 = v60.len();
//                        // v210椭圆
//                        double v211;
//                        if(v210 <= v209)
//                            v211 = v210;
//                        else
//                            v211 = v209;
//                        double v213 = v211 / 20.0;
//                        double v212 = 25.0 * dist_tol;
//                        double v214;
//                        if(v211 / 20.0 <= 25.0 * dist_tol)
//                            v214 = v213;
//                        else
//                            v214 = v212;
//                        SPAunit_vector v469(st_eq->direction);
//                        const SPAvector v61 = v214 * v469;
//                        SPAposition v482 = perp_foot - v61;
//                        const SPAvector v62 = v214 * v469;
//                        SPAposition v483 = perp_foot + v62;
//                        int v158 = !edge2->closed() && (ell_str_sol || ell_end_sol) || str_str_sol || str_end_sol;
//                        bool pt_at_str_end[2];
//                        pt_at_str_end[1] = v158;
//                        if(v158 && ell_eq->test_point_tol(v482, SPAresabs) && ell_eq->test_point_tol(v483, SPAresabs)) {
//                            *difficult_config = 1;
//                        } else {
//                            ;
//                            if(biparallel(st_eq->direction, ell_eq->eval_deriv(param1), SPAresnor)) {
//                                tang_root = 1;
//                            } else {
//                                /*                              if(sg_inter_module_header.debug_level >= 0x1E)
//                                                                  acis_fprintf(debug_file_ptr,
//                                                                               "Tangency point is a solution, but its not a tangent classification, discarding it and relying on"
//                                                                               " general intersectors.\n");*/
//                                *difficult_config = 1;
//                            }
//                        }
//                    } else {
//                        *difficult_config = 1;
//                    }
//                }
//            } else if(FUZZ_TOL > circle_radius - perp_dist) {
//                *difficult_config = 1;
//            }
//            if(*difficult_config) {
//                intersects = 0;
//            } else if(tang_root) {
//                inter2 = perp_foot;
//                inter1 = inter2;
//            } else {
//                double half_chord = acis_sqrt(circle_radius * circle_radius - perp_dist * perp_dist);
//                SPAvector chord_offset = half_chord * line_dir;
//                if(not_circle) {
//                    SPAvector v354 = (chord_offset % min_dir * ratio) * min_dir;
//                    SPAvector v355 = (chord_offset % maj_dir) * maj_dir;
//                    chord_offset = v355 + v354;
//                }
//                inter1 = perp_foot - chord_offset;
//                inter2 = perp_foot + chord_offset;
//            }
//        }
//    }
//}


