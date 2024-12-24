#pragma once

#include "acis/acis_options.hxx"
#include "acis/alltop.hxx"
#include "acis/api.hxx"
#include "acis/ellipse.hxx"
#include "acis/intcucu.hxx"
#include "acis/lists.hxx"


void point_perp(ellipse* e, SPAposition point, SPAposition* foot, SPAunit_vector* tangent, SPAvector* curv, SPAparameter* param_guess, SPAparameter* param_actual);
void point_perp_internal(curve* e, SPAposition* point, double* param_wanted, SPAparameter* param_best, SPAposition* foot_best, SPAunit_vector* tan_best, SPAvector* foot_dt_best, SPAvector* foot_ddt_best, int linear_search, double tol, int* point_found);
void step_from_guess(curve* e, const SPAposition point, double* param_wanted);
double find_angle(SPAvector& vec1, SPAvector& vec2);

curve_curve_int* int_ellipses(ellipse* ell1, SPAinterval& rng1, ellipse* ell2, SPAinterval& rng2, double dist_tol);
int intersect_2_circles(SPAposition& C1, double r1, SPAposition& C2, double r2, SPAvector& norm, SPAposition& pt1, SPAposition& pt2);
int special_case_ellipses2(EDGE* edge1, EDGE* edge2, double dist_tol, double ang_tol, int& same_sense, int& full_ellipse);
int special_case_ellipses(EDGE* edge1, EDGE* edge2, double dist_tol, double ang_tol);
int coinc_end_pts(EDGE* ed1, EDGE* ed2, long double dist_tol);
double reduce_to_range(SPAinterval* range, long double period, double param);

outcome aei_TEST_1(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt);
