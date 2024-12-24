#pragma once
#include "acis/alltop.hxx"
#include "acis/kernapi.hxx"
#include "acis/module.hxx"



//
//void order_wire_coedges(ENTITY* wire_body);
curve* gme_sg_offset_planar_curve(curve& geom, SPAinterval& range, double fit_data, law* dist_law, law* twist_law, SPAunit_vector& off_nor, int all_spline, double tol);
