#include "ofst_examples1.hxx"

#include "PublicInterfaces/gme_ofstapi.hxx"
#include "acis/ckoutcom.hxx"
#include "acis/cstrapi.hxx"
#include "acis/kernapi.hxx"
#include "acis/ofstapi.hxx"
#include "acis/rgbcolor.hxx"
#include "acis/rnd_api.hxx"

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
    check_outcome(api_face_cylinder_cone(center, normal, bottom, top, 0.0, 180.0, ratio, &pt, givenface));
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
    check_outcome(api_face_cylinder_cone(center, normal, bottom, top, 0.0, 180.0, ratio, &pt, givenface));
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

outcome aei_OFFSET_WIRE_1(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt) {
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
    EDGE* edges[3];
    edges[0] = e1;
    edges[1] = e2;
    edges[2] = e3;
    api_make_ewire(3, edges, wirebody);
    COEDGE* c0 = new COEDGE(e1, FORWARD, NULL, NULL);
    WIRE* wire = NULL;
    WIRE* wire1 = new WIRE(c0, wire);

    // output_ents.add(wire1);

    SPAunit_vector normal(0, 0, 1);

    ENTITY_LIST wires;
    api_get_wires(wirebody, wires);
    BODY* wirebody2 = NULL;

    WIRE* oriwire = (WIRE*)wires[0];

    gme_api_offset_planar_wire(oriwire, NULL, 5, normal, wirebody2);

    output_ents.add(wirebody2);

    return 0;
}
