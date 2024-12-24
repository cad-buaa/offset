﻿#pragma once
#include "PublicInterfaces/gme_dcl_ofst.h"
#include "acis/alltop.hxx"
#include "acis/law_base.hxx"
#include "acis/off_wire.hxx"
#include "acis/transfrm.hxx"
#include "acis/unitvec.hxx"
#include "acis/wire_offset_options.hxx"


extern DECL_GME_OFST option_header gme_check_offst_self_intersection;

BODY* gme_sg_offset_planar_wire(WIRE* wire, const TRANSFORM* wire_transf, double offset_dist, const SPAunit_vector& wire_normal, sg_gap_type close_type, int add_attribs, int trim, int zero_length, int overlap);
BODY* gme_sg_offset_planar_wire(WIRE* wire, const TRANSFORM* wire_transf, law* dist_law, law* twist_law, const SPAunit_vector& wire_normal, sg_gap_type close_type, int add_attribs, int trim, int zero_length, int overlap, int iKeepMiniTopo);
BODY* gme_sg_offset_planar_wire(BODY* wire_body, double offset_dist,const SPAunit_vector& wire_normal, sg_gap_type close_type, int add_attribs, int trim, int zero_length, int overlap);
BODY* gme_sg_offset_planar_wire(BODY* iBaseWireBody, wire_offset_options* iOptions);
BODY* gme_sg_offset_planar_wire(BODY* wire_body, law* offset_law, law* twist_law, SPAunit_vector& wire_normal, sg_gap_type close_type, int add_attribs, int trim, int zero_length, int overlap, int iKeepMiniTopo);

curve* gme_offset_geometry(curve const* geometry, SPAunit_vector const& in_normal, law* dist_law, law* twist_law, SPAinterval const& in_off_domain);