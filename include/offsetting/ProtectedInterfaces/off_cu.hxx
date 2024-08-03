#pragma once
#include "acis/alltop.hxx"
#include "acis/SPA_approx_options.hxx"

int use_pcurve_for_curve_offset(COEDGE* coed, surface* offsurf);
int check_analytical_curve_offset_on_cone(COEDGE* coed, surface* offsurf);
curve* make_offset_curve_from_pcurve(COEDGE* pCoedge, surface& rOffsetSurface);
void sg_approx(COEDGE* in_coedge, bs2_curve_def*& coedge_approx_result, SPA_internal_approx_options& approxOpts);
