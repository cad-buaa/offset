﻿#include "PublicInterfaces/gme_sgcofrtn.hxx"
#include "PublicInterfaces/gme_ofstapi.hxx"
#include "PublicInterfaces/off_misc.hxx"
#include "acis/api.err"
#include "acis/api.hxx"
#include "acis/check.hxx"
#include "acis/module.hxx"
#include "acis/unitvec.hxx"
#include "acis/off_utl.hxx"

#define MODULE() api
extern DECL_BASE module_debug api_module_header;


outcome gme_api_offset_planar_wire(BODY* given_wire, double offset_distance, const SPAunit_vector& wire_normal, BODY*& offset_wire, AcisOptions* ao ) {
    DEBUG_LEVEL(DEBUG_CALLS) {
        acis_fprintf(debug_file_ptr, "calling api_offset_planar_wire\n");
    }
    API_BEGIN
    if(api_check_on()) {
        check_wire_body(given_wire, 0, 0);
        check_pos_length(wire_normal.len(), "normal");
    }
    if(ao && ao->journal_on()) {
        /*          J_api_offset_planar_wire(given_wire, offset_distance, wire_normal, ao);*/
    }
    order_wire_coedges(given_wire);
    offset_wire = gme_sg_offset_planar_wire(given_wire, offset_distance, wire_normal, arc, 0, 1, 0, 0);
    if(offset_wire) {
        result = outcome(0, (error_info*)0);
    } else {
        result = outcome(API_FAILED, (error_info*)0);
    }
    API_END
    DEBUG_LEVEL(DEBUG_FLOW) {
        acis_fprintf(debug_file_ptr, "leaving api_offset_planar_wire: %s\n", find_err_ident(result.error_number()));
    }
    return result;
}
