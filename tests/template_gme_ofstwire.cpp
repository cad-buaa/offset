#include <gtest/gtest.h>

// ACIS
#include "PublicInterfaces/gme_ofstapi.hxx"
#include "acis/cstrapi.hxx"
#include "acis/ofstapi.hxx"
#include "acis_utils.hpp"
#include "same_entity.hpp"
// ====================================================================

class api_extend_test_wire : public ::testing::Test {
    int level = 0;

  protected:
    void SetUp() override { level = initialize_acis(); }

    void TearDown() override { terminate_acis(level); }
};

TEST_F(api_extend_test_wire, api_extend_wire) {
    BODY* acis_body = nullptr;
    BODY* gme_body = nullptr;

    SPAposition p1(-10, -5, 0);
    SPAposition p2(10, -5, 0);
    SPAposition p3(-4, 10, 0);
    SPAposition p4(4, 10, 0);
    EDGE* e1 = NULL;
    EDGE* e2 = NULL;
    EDGE* e3 = NULL;
    BODY* wirebody = NULL;
    api_curve_line(p1, p2, e1);
    api_curve_line(p2, p3, e2);
    api_curve_line(p3, p4, e3);
    EDGE* edges[3];
    edges[0] = e1;
    edges[1] = e2;
    edges[2] = e3;
    api_make_ewire(3, edges, wirebody);
    SPAunit_vector normal(0, 0, 1);
    ENTITY_LIST wires;
    api_get_wires(wirebody, wires);
    BODY* wirebody2 = NULL;
    BODY* wirebody3 = NULL;
    WIRE* oriwire = (WIRE*)wires[0];

    API_BEGIN

    api_offset_planar_wire(wirebody, 2, normal, wirebody2);
    gme_api_offset_planar_wire(wirebody, 2, normal, wirebody3);

    API_END

    EXPECT_TRUE(same_body(wirebody2, wirebody3));
}
