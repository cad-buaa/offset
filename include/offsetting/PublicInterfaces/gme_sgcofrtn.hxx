#pragma once
#include "acis/alltop.hxx"
#include "acis/off_wire.hxx"
#include "acis/transfrm.hxx"
#include "acis/unitvec.hxx"

BODY* sg_offset_planar_wire(WIRE* wire, TRANSFORM* wire_transf, double offset_dist, SPAunit_vector& wire_normal, sg_gap_type close_type, int add_attribs, int trim, int zero_length, int overlap);
