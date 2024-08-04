
#include "PublicInterfaces/gme_sgcofrtn.hxx"
#include "acis/law_base.hxx"
#include "acis/main_law.hxx"
#include "acis/dmexcept.hxx"


BODY* sg_offset_planar_wire(WIRE* wire, TRANSFORM* wire_transf, double offset_dist, SPAunit_vector& wire_normal, sg_gap_type close_type, int add_attribs, int trim, int zero_length, int overlap) {
    acis_exception error_info_holder(0, (error_info_base*)0);
    exception_save exception_save_mark;
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    constant_law* v13 = ACIS_NEW constant_law(offset_dist);
    constant_law* v15 = ACIS_NEW constant_law(0.0);
    //BODY* result = sg_offset_planar_wire(wire, wire_transf, v13, v15, wire_normal, close_type, add_attribs, trim, zero_length, overlap, 0);
    if (acis_interrupted())
    {
        sys_error(0, NULL);
    }
    v13->remove();
    v15->remove();
    return result;
}
