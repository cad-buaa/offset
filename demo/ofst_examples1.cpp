#include "ofst_examples1.hxx"

#include "PublicInterfaces/gme_ofstapi.hxx"
#include "acis/boolapi.hxx"
#include "acis/ckoutcom.hxx"
#include "acis/cstrapi.hxx"
#include "acis/ellipse.hxx"
#include "acis/eulerapi.hxx"
#include "acis/kernapi.hxx"
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
    // FACE* givenface = NULL;
    // FACE* offsetface = NULL;
    // API_BEGIN
    // check_outcome(api_clear_annotations());
    // SPAposition center(0.0, 0.0, 0.0);
    // SPAvector normal(0.0, 0.0, 1.0);
    // double width = 10.0;
    // double height = 15.0;
    // check_outcome(api_face_plane(center, width, height, &normal, givenface));
    // double distance = -10.0;
    // check_outcome(gme_api_offset_face(givenface, distance, offsetface));
    //// check_outcome(api_offset_face(givenface, distance, offsetface));

    // rgb_color Red(1.0, 0.0, 0.0);
    // check_outcome(api_rh_set_entity_rgb(offsetface, Red));
    // check_outcome(api_clear_annotations());
    // API_END
    // if(result.ok()) {
    //     output_ents.add(givenface);
    //     output_ents.add(offsetface);
    // }
    //
    //  BODY* acis_en = nullptr;
    BODY* gme_en = nullptr;
    ENTITY_LIST output_bodies1 = NULL;
    ENTITY_LIST to_be_stitched1 = NULL;
    ENTITY* api_stitch1 = NULL;

    aei_SWEEP_WEDGE(to_be_stitched1, NULL);
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
    check_outcome(gme_api_offset_face(givenface, distance, offsetface));
    // check_outcome(api_offset_face(givenface, distance, offsetface));

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
    check_outcome(gme_api_offset_face(givenface, distance, offsetface));
    // check_outcome(api_offset_face(givenface, distance, offsetface));

    rgb_color Red(1.0, 0.0, 0.0);
    check_outcome(api_rh_set_entity_rgb(offsetface, Red));
    check_outcome(api_clear_annotations());
    API_END
    if(result.ok()) {
        output_ents.add(givenface);
        output_ents.add(offsetface);
    }
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
    check_outcome(gme_api_offset_face(givenface, distance, offsetface));
    // check_outcome(api_offset_face(givenface, distance, offsetface));

    rgb_color Red(1.0, 0.0, 0.0);
    check_outcome(api_rh_set_entity_rgb(offsetface, Red));
    check_outcome(api_clear_annotations());
    API_END
    if(result.ok()) {
        output_ents.add(givenface);
        output_ents.add(offsetface);
    }
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
    check_outcome(gme_api_offset_face(givenface, distance, offsetface));
    // check_outcome(api_offset_face(givenface, distance, offsetface));
    rgb_color Red(1.0, 0.0, 0.0);
    check_outcome(api_rh_set_entity_rgb(offsetface, Red));
    check_outcome(api_clear_annotations());
    API_END
    if(result.ok()) {
        output_ents.add(givenface);
        output_ents.add(offsetface);
    }
    return result;
}

outcome aei_OFFSET_FACE_6(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt)
{
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
    check_outcome(gme_api_offset_face(givenface, distance, offsetface));
    rgb_color Red(1.0, 0.0, 0.0);
    check_outcome(api_rh_set_entity_rgb(offsetface, Red));
    API_END
    if(result.ok()) {
        output_ents.add(givenface);
        output_ents.add(offsetface);
    }
    return result;
}

outcome aei_OFFSET_WIRE_1(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt) {
    ////例子1  闭合三角形
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
    // //SPAposition p4 = e1->start()->geometry()->coords();
    // const curve& eq1 = e1->geometry()->equation();
    // double a = ((straight&)eq1).subset_range.start_pt();
    // SPAposition p = ((straight&)eq1).root_point;
    // //SPAposition foot;
    // //eq1.point_perp(p4, foot);
    // //eq1.param()
    // //int a = 0;
    // EDGE* edges[3];
    // edges[0] = e1;
    // edges[1] = e2;
    // edges[2] = e3;
    // api_make_ewire(3, edges, wirebody);
    // /* COEDGE* c0 = ACIS_NEW COEDGE(e1, REVERSED, NULL, NULL);
    //    VERTEX* t = c0->end();*/
    // SPAunit_vector normal(0, 0, 1);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // BODY* wirebody2 = NULL;
    // BODY* wirebody3 = NULL;
    // WIRE* oriwire = (WIRE*)wires[0];
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -3, normal, wirebody2);
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 3, normal, wirebody3);
    // rgb_color Red(1.0, 0.0, 0.0);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // //gme_api_offset_planar_wire(wirebody, 2, normal, wirebody2);
    // output_ents.add(wirebody);
    // output_ents.add(wirebody2);
    // output_ents.add(wirebody3);

    // //例子2  闭合四边形
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
    //
    //   EDGE* edges[4];
    //   edges[0] = e1;
    //   edges[1] = e2;
    //   edges[2] = e3;
    //   edges[3] = e4;
    //   api_make_ewire(4, edges, wirebody);
    ///* COEDGE* c0 = ACIS_NEW COEDGE(e1, REVERSED, NULL, NULL);
    // VERTEX* t = c0->end();*/
    // SPAunit_vector normal(0, 0, 1);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // BODY* wirebody2 = NULL;
    // WIRE* oriwire = (WIRE*)wires[0];
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 3, normal, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // //gme_api_offset_planar_wire(wirebody, 2, normal, wirebody2);
    // output_ents.add(wirebody);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -3, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);

    ////例子3 非闭合折线
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
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -5, normal, wirebody2);
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

    //// 例子4 非闭合折线
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
    // int b = wirebody->lump()->shell()->wire()->coedge()->edge()->geometry()->equation().type();
    //
    // SPAunit_vector normal(0, 0, 1);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // BODY* wirebody2 = NULL;
    // WIRE* oriwire = (WIRE*)wires[0];
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 2, normal, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // // gme_api_offset_planar_wire(wirebody, 2, normal, wirebody2);
    // output_ents.add(wirebody);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // BODY* wirebody4 = NULL;
    // //gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -2, normal, wirebody3);
    // gme_api_offset_planar_wire(wirebody, -2, normal, wirebody3);
    // gme_api_offset_planar_wire(wirebody3, -2, normal, wirebody4);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);
    // int a = wirebody4->lump()->shell()->wire()->coedge()->edge()->geometry()->equation().type();
    // int c;

    // 例子测试
    /*   SPAposition p1(-10, -5, 0);
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

    // 例子6  B样条曲线2
    /* SPAposition ctrlpts[6];
     ctrlpts[0] = SPAposition(-10, 5, 0.0);
     ctrlpts[1] = SPAposition(-5, -3, 0.0);
     ctrlpts[2] = SPAposition(-2, 3, 0.0);
     ctrlpts[3] = SPAposition(2, -2, 0.0);
     ctrlpts[4] = SPAposition(5, 5.5, 0.0);
     ctrlpts[5] = SPAposition(10, 2, 0.0);
     double weight[9] = {0, 0, 0, 0, 0, 0};
     int num_knots_u = 9;
     double knots_u[9] = {0.0, 0.0, 0.0, 0.0, 2.0, 4.0, 4.0, 4.0, 4.0};
     int degree = 3;
     logical national = 0;
     bs2_curve bs2 = bs2_curve_from_ctrlpts(degree, national, 0, 0, 7, ctrlpts, weight, SPAresabs, num_knots_u, knots_u, SPAresabs);
     EDGE* e = NULL;
     api_curve_spline(6, ctrlpts, NULL, NULL, e);
     EDGE* edge[1];
     edge[0] = e;
     BODY* wirebody;
     api_make_ewire(1, edge, wirebody);
     ENTITY_LIST wires;
     api_get_wires(wirebody, wires);
     SPAunit_vector normal(0, 0, 1);
     BODY* wirebody2 = NULL;
     api_offset_planar_wire((WIRE*)wires[0], NULL, 0.5, normal, wirebody2);
     rgb_color Red(1.0, 0.0, 0.0);
     api_rh_set_entity_rgb(wirebody2, Red);
     output_ents.add(wirebody);
     output_ents.add(wirebody2);
     rgb_color Blue(0.0, 0.0, 1.0);
     BODY* wirebody3 = NULL;
     gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -0.5, normal, wirebody3);
     api_rh_set_entity_rgb(wirebody3, Blue);
     output_ents.add(wirebody3);*/

    // //例子7  闭合B样条曲线
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
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 0.4, normal, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -0.5, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);

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
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 0.4, normal, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);   output_ents.add(wirebody);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -0.5, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);

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
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 0.4, normal, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -0.5, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);

    // 例子10   椭圆

    // ELLIPSE* e1 = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 2);
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
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -0.5, normal, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 0.5, normal, wirebody3);
    // int a = wirebody3->wire()->coedge()->edge()->geometry()->equation().type();
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);

    ////
    //// 例子11   椭圆
    // ellipse* e1 = ACIS_NEW ellipse(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(5, 6, 0), 2);
    // // ELLIPSE* e1 = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 2);
    // EDGE* e = NULL;
    //
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
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -2, normal, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 2, normal, wirebody3);
    // int b = wirebody3->wire()->coedge()->edge()->geometry()->equation().type();
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);

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

    // 例子12   两椭圆复合曲线

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
    //
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 1, normal, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -1, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);

    // 例子13  直线与椭圆复合曲线
    ////椭圆
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
    //
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 0.5, normal, wirebody2);
    // output_ents.add(wirebody);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -1, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);

    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 0.5, normal, wirebody2);
    // output_ents.add(wirebody2);

    // //例子14  直线与椭圆复合曲线
    // //椭圆
    //   APOINT* a10 = ACIS_NEW APOINT(-4, 0, 0);
    //   APOINT* a11 = ACIS_NEW APOINT(4,0,0);
    //   VERTEX* v10 = ACIS_NEW VERTEX(a10);
    //   VERTEX* v11 = ACIS_NEW VERTEX(a11);
    //   ELLIPSE* e = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 0.5);
    //   EDGE* e1 = ACIS_NEW EDGE(v10, v11, e, 1);
    //   SPAposition p1(-4, 0, 0);
    //   SPAposition p2(4, 0, 0);
    //// 直线
    // EDGE* e2 = NULL;
    // api_curve_line(p1, p2, e2);
    // EDGE* edge1[2];
    // edge1[0] = e1;
    // edge1[1] = e2;
    // BODY* wirebody = NULL;
    // api_make_ewire(2, edge1, wirebody);
    //
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // SPAunit_vector normal(0, 0, 1);
    // BODY* wirebody2 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 0.5, normal, wirebody2);
    // output_ents.add(wirebody);
    //
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -1, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);

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
    //// SPAposition p4 = e1->start()->geometry()->coords();
    // const curve& eq1 = e1->geometry()->equation();
    // double a = ((straight&)eq1).subset_range.start_pt();
    // SPAposition p = ((straight&)eq1).root_point;
    //// SPAposition foot;
    //// eq1.point_perp(p4, foot);
    //// eq1.param()
    //// int a = 0;
    // EDGE* edges[3];
    // edges[0] = e1;
    // edges[1] = e2;
    // edges[2] = e3;
    // api_make_ewire(3, edges, wirebody);
    ///* COEDGE* c0 = ACIS_NEW COEDGE(e1, REVERSED, NULL, NULL);
    // VERTEX* t = c0->end();*/
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

    // SPAposition ctrlpts[4];
    //  ctrlpts[0] = SPAposition(-5.0, 0.0, 0.0);
    //  ctrlpts[1] = SPAposition(-5.0, 5.0, 0.0);
    //  ctrlpts[2] = SPAposition(5.0,  5.0, 0.0);
    //  ctrlpts[3] = SPAposition(5.0,  0.0, 0.0);
    //  EDGE* e = NULL;
    //  EDGE* e1 = NULL;
    //  double weight[4] = {1, 0.5, 0.5, 1};
    //  int num_knots_u = 7;
    //  double knots_u[7] = {0.0, 0.0, 0.0, 0.5, 1.0, 1.0, 1.0};
    //  int degree = 2;
    //  logical national = 1;
    //  api_mk_ed_int_ctrlpts(degree, 1, 0, 0, 4, ctrlpts, weight, SPAresabs, num_knots_u, knots_u, SPAresabs, e);
    //   APOINT* a10 = ACIS_NEW APOINT(ctrlpts[0]);
    //   APOINT* a11 = ACIS_NEW APOINT(ctrlpts[3]);
    //   VERTEX* v10 = ACIS_NEW VERTEX(a10);
    //   VERTEX* v11 = ACIS_NEW VERTEX(a11);
    //   ELLIPSE* ee = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(5, 0, 0), 0.5);
    //   e1 = ACIS_NEW EDGE(v11, v10, ee, 1);
    //  EDGE* edge[2];
    //  edge[0] = e;
    //  edge[1] = e1;
    //  BODY* wirebody=NULL;
    //  api_make_ewire(2, edge, wirebody);
    //  ENTITY_LIST wires;
    //  api_get_wires(wirebody, wires);
    //  SPAunit_vector normal(0, 0, 1);
    //  BODY* wirebody2 = NULL;
    //
    //   wire_offset_options option;
    //   char my_function_string[50];
    //   strcpy(my_function_string, "(2*x)+2*y");
    //   law* twist;
    //   api_str_to_law(my_function_string, &twist, 0, NULL);
    //   //law* x = new identity_law(0);
    //   //law* d = new cos_law(x);
    //   option.set_twist_law(twist);
    //   option.set_distance(1);
    //   option.set_plane_normal(normal);
    //   option.set_gap_type(arc);
    //   api_offset_planar_wire(wirebody, &option, wirebody2);
    //
    //
    //  output_ents.add(wirebody);
    //  rgb_color Red(1.0, 0.0, 0.0);
    //  api_rh_set_entity_rgb(wirebody2, Red);
    //  output_ents.add(wirebody2);

    // 变距
    // APOINT* a = ACIS_NEW APOINT(SPAposition(10.0, 0.0, 0.0));
    // VERTEX* v = ACIS_NEW VERTEX(a);
    // ELLIPSE* ee = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(10, 0, 0), 1);
    // EDGE* e1 = ACIS_NEW EDGE(v, v, ee, 1);
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
    // option.set_twist_law(twist);
    // option.set_distance(5);
    // option.set_plane_normal(normal);
    // option.set_gap_type(arc);
    // api_offset_planar_wire(wirebody, &option, wirebody2);
    // output_ents.add(wirebody);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);

    // 变距
    // APOINT* a = ACIS_NEW APOINT(SPAposition(20.0, 0.0, 0.0));
    // VERTEX* v = ACIS_NEW VERTEX(a);
    // ELLIPSE* ee = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(20, 0, 0), 1);
    // EDGE* e1 = ACIS_NEW EDGE(v, v, ee, 1);
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
    // strcpy(my_function_string, "-10-5*cos(x*2)");
    // law* d;
    // api_str_to_law(my_function_string, &d, 0, NULL);

    // option.set_distance(d);
    // option.set_plane_normal(normal);
    // option.set_twist_law(0);
    ////option.set_gap_type(arc);
    // api_offset_planar_wire(wirebody, &option, wirebody2);
    // output_ents.add(wirebody);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // output_ents.add(wirebody2);

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
    //
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // api_offset_planar_wire((WIRE*)wires[0], NULL, -5, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);

    // APOINT* a10 = ACIS_NEW APOINT(-4, 0, 0);
    // APOINT* a11 = ACIS_NEW APOINT(4, 0, 0);
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
    // EDGE* edge[1];
    // edge[0] = e11;
    // EDGE* edge1[1];
    // edge1[0] = e22;
    // EDGE* edge2[1];
    // edge2[0] = e33;
    // BODY* wirebody1 = NULL;
    // BODY* wirebody2 = NULL;
    // BODY* wirebody3 = NULL;
    // BODY* wirebody11 = NULL;
    // BODY* wirebody22 = NULL;
    // BODY* wirebody33 = NULL;
    // api_make_ewire(1, edge, wirebody1);
    // api_make_ewire(1, edge1, wirebody2);
    // api_make_ewire(1, edge2, wirebody3);
    // SPAunit_vector normal(0, 0, 1);
    // wire_offset_options option;
    // option.set_gap_type(corner);
    // char my_function_string[50];
    // strcpy(my_function_string, "1");
    // law* d;
    // api_str_to_law(my_function_string, &d, 0, NULL);
    // option.set_distance(2);
    // option.set_plane_normal(normal);
    // api_offset_planar_wire(wirebody1, &option, wirebody11);
    // api_offset_planar_wire(wirebody2, &option, wirebody22);
    // api_offset_planar_wire(wirebody3, &option, wirebody33);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody11, Red);
    // api_rh_set_entity_rgb(wirebody22, Red);
    // api_rh_set_entity_rgb(wirebody33, Red);

    // output_ents.add(wirebody1);
    // output_ents.add(wirebody2);
    // output_ents.add(wirebody3);

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

    //

    ////例子1  复杂轮廓
    // SPAposition p1(-10, 0, 0);
    // SPAposition p2(10, 0, 0);
    // SPAposition p3(2, 10, 0);
    // SPAposition p4(10, 20, 0);
    // SPAposition p5(-10, 20, 0);
    // SPAposition p6(-2, 10, 0);
    //
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
    //
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

    ///* COEDGE* c0 = ACIS_NEW COEDGE(e1, REVERSED, NULL, NULL);
    // VERTEX* t = c0->end();*/
    // SPAunit_vector normal(0, 0, 1);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // BODY* wirebody2 = NULL;
    // BODY* wirebody3 = NULL;
    // WIRE* oriwire = (WIRE*)wires[0];
    // gme_api_offset_planar_wire(wirebody, 5, normal, wirebody2);

    ////gme_api_offset_planar_wire(wirebody, 3.5, normal, wirebody3);

    ////gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -3.5, normal, wirebody2);
    ////gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 3.5, normal, wirebody3);
    // rgb_color Red(1.0, 0.0, 0.0);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    ////gme_api_offset_planar_wire(wirebody, 2, normal, wirebody2);
    // output_ents.add(wirebody);
    // output_ents.add(wirebody2);
    // output_ents.add(wirebody3);

    //
    //
    //
    //
    //// 例子  复杂非闭合折线
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
    //
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
    //
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
    //
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
    ////
    ////
    // SPAunit_vector normal(0, 0, 1);
    // ENTITY_LIST wires;
    // api_get_wires(wirebody, wires);
    // BODY* wirebody2 = NULL;
    // WIRE* oriwire = (WIRE*)wires[0];
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, 1.1, normal, wirebody2);
    // rgb_color Red(1.0, 0.0, 0.0);
    // api_rh_set_entity_rgb(wirebody2, Red);
    //// gme_api_offset_planar_wire(wirebody, 2, normal, wirebody2);
    //
    // output_ents.add(wirebody2);
    // rgb_color Blue(0.0, 0.0, 1.0);
    // BODY* wirebody3 = NULL;
    // gme_api_offset_planar_wire((WIRE*)wires[0], NULL, -1.1, normal, wirebody3);
    // api_rh_set_entity_rgb(wirebody3, Blue);
    // output_ents.add(wirebody3);

    //
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
    //
    // EDGE* e3 = NULL;
    // EDGE* e4 = NULL;
    // api_curve_line(p2, p5, e3);
    // api_curve_line(p5, p3, e4);
    //
    // EDGE* edges3[2];
    // edges3[0] = e3;
    // edges3[1] = e4;
    // BODY* wirebody3;
    // api_make_ewire(2, edges3, wirebody3);
    // rgb_color Red(0.0, 0.0, 1);
    //  api_rh_set_entity_rgb(wirebody3, Red);
    // output_ents.add(wirebody3);
    //
    //
    //
    // SPAposition p1(-6, 0, 0);
    // SPAposition p2(-3, 4, 0);
    // SPAposition point(-6, 0, 0);
    // EDGE* e1 = NULL;
    // EDGE* e2 = NULL;
    // api_curve_line(p1, p2, e1);
    // SPAposition foot1;
    // SPAposition* foot =&foot1;
    // straight& str = (straight&)e1->geometry()->equation();
    // double this_param = str.param(point);
    // bool a3 = foot && !SpaAcis::NullObj::check_position(*foot);

    // e1->geometry()->equation().point_perp(point, *foot);
    // bool a1 = this_param < str.subset_range;
    // bool a2 = this_param > str.subset_range;

    ////bool a4 = &tangent && !SpaAcis::NullObj::check_unit_vector(tangent);
    ////bool a5 = &curv && !SpaAcis::NullObj::check_vector(curv);
    ////
    ////SPAposition& foot = SpaAcis::NullObj::get_parameter()
    ////point_perp1(&(straight&)e1->geometry()->equation() ,SPAposition(0, 0, 0), foot);
    // int a = 0;

    // 例子11   椭圆
    ellipse* e0 = ACIS_NEW ellipse(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 2);

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
    a++;
    /*    ELLIPSE* e1 = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 2);
        SPAinterval a = e0->subset_range;*/

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
    //
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

    return 0;
}

void point_perp1(straight* b, const SPAposition& point, SPAposition& foot, SPAunit_vector& tangent = SpaAcis::NullObj::get_unit_vector(), SPAvector& curv = SpaAcis::NullObj::get_vector(), SPAparameter& param_actual = SpaAcis::NullObj::get_parameter()) {
    double this_param = b->param(point);
    if(this_param < b->subset_range) {
        this_param = b->subset_range.start_pt();
    } else if(this_param > b->subset_range) {
        this_param = b->subset_range.end_pt();
    }
    if(&foot && !SpaAcis::NullObj::check_position(foot)) foot = b->eval_position(this_param);
    if(&tangent && !SpaAcis::NullObj::check_unit_vector(tangent)) tangent, b->direction;
    if(&curv && !SpaAcis::NullObj::check_vector(curv)) curv, null_vector;
    if(param_actual) {
        if(!SpaAcis::NullObj::check_parameter(param_actual)) {
            param_actual = this_param;
        }
    }
}