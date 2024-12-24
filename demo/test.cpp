#include "test.h"

#include <windows.h>

#include <iostream>

#include "PrivateInterfaces/ofst_bad_intersections_remover.hxx"
#include "PublicInterfaces/gme_ofstapi.hxx"
#include "access.hpp"
#include "acis/acismath.h"
#include "acis/boolapi.hxx"
#include "acis/ckoutcom.hxx"
#include "acis/cstrapi.hxx"
#include "acis/ellipse.hxx"
#include "acis/eulerapi.hxx"
#include "acis/geom_utl.hxx"
#include "acis/geometry.hxx"
#include "acis/getbox.hxx"
#include "acis/kernapi.hxx"
#include "acis/law.hxx"
#include "acis/main_law.hxx"
#include "acis/ofstapi.hxx"
#include "acis/point.hxx"
#include "acis/rgbcolor.hxx"
#include "acis/rnd_api.hxx"
#include "acis/sps2crtn.hxx"
#include "acis/straight.hxx"
#include "acis/wire_offset_options.hxx"
#include "sweep_examples.hxx"

outcome aei_TEST_1(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt) {
    APOINT* a10 = ACIS_NEW APOINT(2 * sqrt(2), 2 * sqrt(2), 0);
    APOINT* a11 = ACIS_NEW APOINT(2 * sqrt(2), -2 * sqrt(2), 0);
    VERTEX* v10 = ACIS_NEW VERTEX(a10);
    VERTEX* v11 = ACIS_NEW VERTEX(a11);
    ELLIPSE* E = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 1);
    EDGE* e1 = ACIS_NEW EDGE(v10, v11, E, 1);
    ELLIPSE* E1 = ACIS_NEW ELLIPSE(SPAposition(0, 0, 0), SPAunit_vector(0, 0, 1), SPAvector(4, 0, 0), 1);
    APOINT* a20 = ACIS_NEW APOINT(0, 4, 0);
    VERTEX* v20 = ACIS_NEW VERTEX(a20);
    APOINT* a21 = ACIS_NEW APOINT(0, -4, 0);
    VERTEX* v21 = ACIS_NEW VERTEX(a21);
    EDGE* e2 = ACIS_NEW EDGE(v20, v21, E1, 1);
    SPAinterval inter = E1->equation().param_range();
    double a = 4.6 * M_PI;
    map_param_into_interval(e2->param_range(), a);

    int same_sense;
    int full_ellipse;
    int special = special_case_ellipses2(e1, e2, 1e-6, 1e-10, same_sense, full_ellipse);
    output_ents.add(e1);
    output_ents.add(e2);
    int m = 0;

    return 0;
}

double reduce_to_range(SPAinterval& range, double period, double param) {
    double parama = param;
    if(SPAresnor > period) return param;
    double mid = range.mid_pt();
    while(mid - period / 2.0 > parama) parama = parama + period;
    while(parama >= mid + period / 2.0) parama = parama - period;
    return parama;
}
curve_curve_int* int_ellipses(ellipse* ell1, SPAinterval& rng1, ellipse* ell2, SPAinterval& rng2, double dist_tol) {
    SPAunit_vector v2 = ell1->normal;
    const SPAvector& v5 = ell2->centre - ell1->centre;
    double v6 = v5 % v2;
    if(fabs(v6) > dist_tol) return 0;
    double rad_1 = ell1->major_axis.len();
    double rad_2 = ell2->major_axis.len();
    if(same_point(ell1->centre, ell2->centre, dist_tol)) return 0;
    SPAposition int_pt1;
    SPAposition int_pt2;
    int num_ints = intersect_2_circles(ell1->centre, rad_1, ell2->centre, rad_2, ell1->normal, int_pt1, int_pt2);
    if(num_ints < 1) return 0;
    double v8 = ell1->param(int_pt1, SpaAcis::NullObj::get_parameter());
    double param1 = reduce_to_range(rng1, 6.283185307179586, v8);
    curve_curve_int* result;
    if(rng1 >> param1) {
        double v9 = ell2->param(int_pt1, SpaAcis::NullObj::get_parameter());
        double param2 = reduce_to_range(rng2, 6.283185307179586, v9);
        if(rng2 >> param2) {
            curve_curve_int* v21 = ACIS_NEW curve_curve_int(nullptr, int_pt1, param1, param2, SpaAcis::NullObj::get_par_pos());
            result = v21;
        }
    }
    if(num_ints > 1) {
        double v11 = ell1->param(int_pt2, SpaAcis::NullObj::get_parameter());
        double param1a = reduce_to_range(rng1, 6.283185307179586, v11);
        if(rng1 >> param1a) {
            double v12 = ell2->param(int_pt2, SpaAcis::NullObj::get_parameter());
            double param2a = reduce_to_range(rng2, 6.283185307179586, v12);
            if(rng2 >> param2a) {
                curve_curve_int* v22 = ACIS_NEW curve_curve_int(result, int_pt2, param1a, param2a, SpaAcis::NullObj::get_par_pos());
                return v22;
            }
        }
    }
    return result;
}
// 两个曲线都是圆，圆心不同，相同平面
int special_case_ellipses(EDGE* edge1, EDGE* edge2, double dist_tol, double ang_tol) {
    CURVE* CURV1 = edge1->geometry();
    CURVE* CURV2 = edge2->geometry();
    if(!is_ELLIPSE(CURV1) || !is_ELLIPSE(CURV2)) return 0;
    const curve& cur1 = CURV1->equation();
    const curve& cur2 = CURV2->equation();
    if(cur1 == SpaAcis::NullObj::get_curve()) return 0;
    if(cur2 == SpaAcis::NullObj::get_curve()) return 0;
    double v15 = fabs(((ellipse&)cur1).radius_ratio - 1.0);
    if(v15 > SPAresmch) return 0;
    double v16 = fabs(((ellipse&)cur2).radius_ratio - 1.0);
    if(v16 > SPAresmch) return 0;
    if(same_point(((ellipse&)cur1).centre, ((ellipse&)cur2).centre, dist_tol)) return 0;
    if(coinc_end_pts(edge1, edge2, dist_tol)) return 0;
    if(!biparallel(((ellipse&)cur1).major_axis, ((ellipse&)cur2).major_axis, ang_tol)) return 0;
    SPAvector v5 = ((ellipse&)cur1).centre - ((ellipse&)cur2).centre;
    double v6 = v5 % ((ellipse&)cur2).normal;
    if(fabs(v6) > SPAresnor) return 0;
    return 1;
}
int coinc_end_pts(EDGE* ed1, EDGE* ed2, long double dist_tol) {
    SPAtransf v12;
    SPAtransf v13;
    SPAposition p10 = edge_start_pos(ed1, v12);
    SPAposition p20 = edge_start_pos(ed2, v13);
    if(same_point(p10, p20, dist_tol)) return 1;
    SPAtransf v14;
    SPAposition p11 = edge_end_pos(ed1, v14);
    if(same_point(p11, p20, dist_tol)) return 1;
    SPAtransf v15;
    SPAposition p21 = edge_end_pos(ed2, v15);
    return same_point(p10, p21, dist_tol) || same_point(p11, p21, dist_tol);
}
int special_case_ellipses2(EDGE* edge1, EDGE* edge2, double dist_tol, double ang_tol, int& same_sense, int& full_ellipse) {
    CURVE* CURV1 = edge1->geometry();
    CURVE* CURV2 = edge2->geometry();
    if(!is_ELLIPSE(CURV1) || !is_ELLIPSE(CURV2)) return 0;
    SPAtransf v71;
    SPAposition p1s = edge_start_pos(edge1, v71);
    SPAtransf v72;
    SPAposition p1e = edge_end_pos(edge1, v72);
    SPAtransf v73;
    SPAposition p2s = edge_start_pos(edge2, v73);
    SPAtransf v74;
    SPAposition p2e = edge_end_pos(edge2, v74);
    full_ellipse = 0;
    if(same_point(p1s, p1e, SPAresabs)) {
        if(same_point(p2s, p2e, SPAresabs)) return 0;
    }
    if(same_point(p1s, p1e, SPAresabs)) full_ellipse = 1;
    if(same_point(p2s, p2e, SPAresabs)) full_ellipse = 2;
    int share_vertex = 1;  // 有共享顶点
    if(!same_point(p1s, p2s, SPAresabs)) {
        if(!same_point(p1s, p2e, SPAresabs)) {
            if(!same_point(p1e, p2s, SPAresabs)) {
                share_vertex = same_point(p1e, p2e, SPAresabs);
            }
        }
    }
    ellipse& cur1 = (ellipse&)CURV1->equation();
    ellipse& cur2 = (ellipse&)CURV2->equation();
    if((curve&)cur1 == SpaAcis::NullObj::get_curve()) return 0;
    if((curve&)cur2 == SpaAcis::NullObj::get_curve()) return 0;
    int overlap = full_ellipse > 0;
    if(!share_vertex && !full_ellipse) {
        SPAparameter started;
        if(edge1->sense() == 1) {
            SPAparameter v46 = edge1->end_param();
            started = -v46;
        } else {
            started = edge1->start_param();
        }
        double d = started.operator double();
        SPAparameter v39;
        if(edge1->sense() == 1) {
            v39 = -edge1->start_param();
        } else {
            v39 = edge1->end_param();
        }
        double ep = v39.operator double();
        SPAposition foot1;
        SPAposition foot2;
        SPAposition foot3;
        SPAposition foot4;
        SPAparameter param1;
        SPAparameter param2;
        SPAparameter param3;
        SPAparameter param4;
        SPAparameter param_guess = (d + ep) / 2.0;
        cur1.point_perp(p2s, foot1, param_guess, param1, 0);
        SPAparameter v51 = (d + ep) / 2.0;
        cur1.point_perp(p2e, foot2, v51, param2, 0);
        int p2s_in = 0;
        int p2e_in = 0;
        if(same_point(p2s, foot1, SPAresabs) && param1 > d) p2s_in = param1 < ep != 0;
        if(same_point(p2e, foot2, SPAresabs) && param2 > d && param2 < ep) p2e_in = 1;
        if(p2s_in || p2e_in) overlap = 1;
    }
    if(!share_vertex && !overlap) return 0;
    double v52 = fabs(cur1.radius_ratio - cur2.radius_ratio);
    if(v52 > SPAresmch) return 0;
    if(!same_point(cur1.centre, cur2.centre, dist_tol)) return 0;
    if(parallel(cur1.normal, cur2.normal, ang_tol)) {
        same_sense = 1;
    } else {
        if(!antiparallel(cur1.normal, cur2.normal, SPAresnor)) return 0;
        same_sense = 0;
    }
    double rad_1 = cur1.major_axis.len();
    double rad_2 = cur2.major_axis.len();
    double v53 = fabs(rad_1 - rad_2);
    if(v53 > SPAresmch) return 0;
    if(dist_tol > fabs(rad_1) || dist_tol > fabs(rad_2)) return 0;
    double v54 = fabs(cur1.radius_ratio - 1.0);
    if(v54 > SPAresmch) {
        if(!biparallel(cur1.major_axis, cur2.major_axis, SPAresnor)) return 0;
    }
    return 1;
}
int intersect_2_circles(SPAposition& C1, double r1, SPAposition& C2, double r2, SPAvector& norm, SPAposition& pt1, SPAposition& pt2) {
    SPAvector diff = C2 - C1;  // 两圆心坐标相减
    double d = diff.len();
    if(SPAresabs > d) return 0;
    double a = (d * d + r1 * r1 - r2 * r2) / (2.0 * d);
    SPAposition perp_pt = C1 + diff * a / d;
    double h2 = r1 * r1 - a * a;
    double v18 = safe_fabs(h2);
    if(SPAresabs * SPAresabs > v18) h2 = 0.0;
    if(h2 < 0.0) return 0;
    double h = acis_sqrt(h2);
    SPAunit_vector perp_dir = normalise(diff * norm);
    int num_ints;
    if(SPAresabs <= h) {
        num_ints = 2;
        pt1 = perp_pt + h * perp_dir;
        pt2 = perp_pt - h * perp_dir;
    } else {
        num_ints = 1;
        pt2 = perp_pt;
        pt1 = pt2;
    }
    return num_ints;
}
void point_perp(ellipse* e, SPAposition point, SPAposition* foot, SPAunit_vector* tangent, SPAvector* curv, SPAparameter* param_guess, SPAparameter* param_actual) {
    double cosang;
    double sinang;
    double param1;
    double tol = 0.001 * SPAresabs;
    double param = 0.0;
    SPAposition foot_current(0.0, 0.0, 0.0);
    SPAunit_vector tan_current(0.0, 0.0, 0.0);
    SPAvector foot_dt;
    SPAvector foot_ddt;
    SPAparameter param_best(0.0);
    SPAposition foot_best(0.0, 0.0, 0.0);
    SPAunit_vector tan_best(0.0, 0.0, 0.0);
    SPAvector foot_dt_best(0.0, 0.0, 0.0);
    SPAvector foot_ddt_best(0.0, 0.0, 0.0);
    int point_found = 0;
    double param0;
    SPAvector offset = point - e->centre;
    for(int method = 0;; ++method) {
        if(point_found) return;
        if(method >= 6) {
            if(GET_ALGORITHMIC_VERSION() >= AcisVersion(15, 0, 0)) return;
        }
        if(!method || method == 2 || method == 4) {
            if(!param_guess || SpaAcis::NullObj::check_parameter(*param_guess)) continue;
            param = (*param_guess).operator double();
        } else {
            // 初值：
            cosang = offset % e->major_axis;
            sinang = (offset % (e->normal * e->major_axis)) / e->radius_ratio;
            param1 = safe_atan2(sinang, cosang);
            SPAvector perp_off = point - e->centre;
            double v133 = perp_off % e->major_axis;
            double v47 = e->major_axis % e->major_axis;
            cosang = v133 / v47;
            if(fabs(v133 / v47) < 1.0 - e->radius_ratio * e->radius_ratio) {
                sinang = sqrt(1.0 - cosang * cosang);
                SPAvector v48 = e->normal * e->major_axis;
                if(perp_off % v48 < 0.0) sinang = -sinang;
                param0 = safe_atan2(sinang, cosang);
                double mix = e->radius_ratio * e->radius_ratio;
                param = (1.0 - mix) * param0 + mix * param1;
            }

            else {
                param = param1;
            }
            // param = reduce_to_range_0(param, &this->subset_range);
        }
        if(method >= 2) {
            step_from_guess(e, point, &param);
        }

        BOOL v73 = method >= 4;
        double v137 = tol;
        point_perp_internal(e, &point, &param, &param_best, &foot_best, &tan_best, &foot_dt_best, &foot_ddt_best, v73, v137, &point_found);

        if(e->subset_range.bounded()) {
            if(GET_ALGORITHMIC_VERSION() > AcisVersion(33, 0, 0) && (!param_guess || SpaAcis::NullObj::check_parameter(*param_guess))) {
                SPAposition p1(foot_best);
                SPAposition v161 = e->eval_position(e->subset_range.start_pt());
                SPAposition v162 = e->eval_position(e->subset_range.end_pt());
                SPAvector v52 = operator-(p1 - point);
                double v91 = v52.len_sq();
                double v84 = (v161 - point).len_sq();
                double v83 = (v162 - point).len_sq();
                int v65 = 0;
                int v69 = 0;
                if(e->subset_range >> param_best) {
                    if(v83 < v84) {
                        if(v91 > v83) v69 = 1;
                    } else if(v91 > v84) {
                        v65 = 1;
                    }
                } else if(v83 < v84) {
                    v69 = 1;
                } else {
                    v65 = 1;
                }
                if(v65 || v69) {
                    if(v65)
                        param_best = e->subset_range.start_pt();
                    else
                        param_best = e->subset_range.end_pt();
                    e->eval(param_best, foot_best, foot_dt_best, foot_ddt_best, 0, 0);
                    tan_best = normalise(foot_dt_best);
                }
            } else if(!param_best << e->subset_range) {
                if(param_best < e->subset_range)
                    param_best = e->subset_range.start_pt();
                else
                    param_best = e->subset_range.end_pt();
                e->eval(param_best, foot_best, foot_dt_best, foot_ddt_best, 0, 0);
                tan_best = normalise(foot_dt_best);
            }
        }

        if(foot && !SpaAcis::NullObj::check_position(*foot)) *foot = foot_best;
        if(tangent && !SpaAcis::NullObj::check_unit_vector(*tangent)) *tangent = tan_best;
        if(curv && !SpaAcis::NullObj::check_vector(*curv)) {
            double foot_dt2 = foot_dt_best % foot_dt_best;
            double v148 = foot_dt2 * foot_dt2;
            SPAvector v57 = foot_dt_best * foot_ddt_best;
            SPAvector v58 = v57 * foot_dt_best;
            *curv = v58 / v148;
        }
        if(param_actual && !SpaAcis::NullObj::check_parameter(*param_actual)) {
            if(!param_guess || SpaAcis::NullObj::check_parameter(*param_guess)) *param_actual = param_best;
        }
    }
}
void step_from_guess(curve* e, const SPAposition point, double* param_wanted) {
    SPAposition foot_current(0.0, 0.0, 0.0);
    SPAvector foot_dt;
    e->eval(*param_wanted, foot_current, foot_dt);
    SPAvector offset = point - foot_current;
    double init_err = offset % foot_dt;
    double hi_parm = *param_wanted;
    double lo_parm = *param_wanted;
    double step = 0.1;
    double lo_err;
    double hi_err;
    do {
        while(1) {
            hi_parm = hi_parm + step;
            e->eval(hi_parm, foot_current, foot_dt);
            offset = point - foot_current;
            hi_err = offset % foot_dt;
            lo_parm = lo_parm - step;
            e->eval(lo_parm, foot_current, foot_dt);
            offset = point - foot_current;
            lo_err = offset % foot_dt;
            if(hi_err * init_err > 0.0) break;
            if(lo_err * init_err > 0.0) {
                *param_wanted = hi_parm - step * 0.5;
                return;
            }
            if(step < 0.0001) {
                *param_wanted = lo_parm + step * 0.5;
                return;
            }
            hi_parm = hi_parm - step;
            lo_parm = lo_parm + step;
            step = step * 0.1;
        }
    } while(lo_err * init_err > 0.0);
    *param_wanted = lo_parm + step * 0.5;
}
void point_perp_internal(curve* e, SPAposition* point, double* param_wanted, SPAparameter* param_best, SPAposition* foot_best, SPAunit_vector* tan_best, SPAvector* foot_dt_best, SPAvector* foot_ddt_best, int linear_search, double tol, int* point_found) {
    int alt_method = 0;
    int need_to_bracket = 1;
    double param_prev = 0.0;
    double error_prev = 0.0;
    double param1 = 0.0;
    double param2 = 0.0;
    double error1 = 0.0;
    double error2 = 0.0;
    double error_best = SPAresabs;
    SPAposition foot_current(0.0, 0.0, 0.0);
    SPAvector foot_dt;
    SPAvector foot_ddt;
    SPAunit_vector tan_current(0.0, 0.0, 0.0);
    double curv_factor;
    double err_factor;
    double dparam;
    double step;
    double v38;
    double v14;
    double v13;
    double v39;
    double v42;
    double v16;
    double v43;
    double v17;
    double new_param;
    int iter;
    for(iter = 0; iter < 200 && !*point_found; ++iter) {
        e->eval(*param_wanted, foot_current, foot_dt, foot_ddt, 0, 0);
        SPAvector offset = *point - foot_current;
        tan_current = normalise(foot_dt);
        double error = offset % tan_current;
        double v12 = fabs(error);
        if(error_best > v12) {
            *param_best = *param_wanted;
            error_best = fabs(error);
            *foot_best = foot_current;
            *tan_best = tan_current;
            *foot_dt_best = foot_dt;
            *foot_ddt_best = foot_ddt;
        }
        new_param = *param_wanted;
        if(!alt_method) {
            if(tol > fabs(error)) {
                *point_found = 1;
                return;
            }
            if(linear_search) {
                curv_factor = foot_dt % foot_dt;
            } else {
                v38 = foot_dt % foot_dt;
                v13 = offset % foot_ddt;
                curv_factor = v38 - v13;
            }
            err_factor = offset % foot_dt;
            v39 = fabs(err_factor);
            v14 = fabs(curv_factor);
            if(0.3926990816987241 * v14 <= v39) {
                if(err_factor < 0.0 == curv_factor < 0.0)
                    new_param = *param_wanted + 0.1;
                else
                    new_param = *param_wanted - 0.1;
            } else {
                new_param = *param_wanted + err_factor / curv_factor;
            }
            if(iter > 3) {
                if(GET_ALGORITHMIC_VERSION() >= AcisVersion(15, 0, 0)) {
                    double conv_limit = 0.99 * param_prev + 0.01 * (*param_wanted);
                    if(*param_wanted <= param_prev) {
                        if(new_param >= conv_limit) alt_method = 1;
                    } else if(conv_limit >= new_param) {
                        alt_method = 1;
                    }
                }
            }
        }
        if(alt_method) {
            if(need_to_bracket) {
                if((error <= 0.0 || error_prev <= 0.0) && (error >= 0.0 || error_prev >= 0.0)) {
                    need_to_bracket = 0;
                    param1 = param_prev;
                    param2 = *param_wanted;
                    error1 = error_prev;
                    error2 = error;
                    new_param = (param_prev + (*param_wanted)) / 2.0;
                } else {
                    dparam = *param_wanted - param_prev;
                    if(fabs(*param_wanted - param_prev) >= 0.0001) {
                        step = 10.0 * dparam;
                    } else {
                        step = 0.001;
                        if(param_prev > *param_wanted) step = -0.001;
                    }
                    v42 = fabs(error);
                    v16 = fabs(error_prev);
                    if(v16 <= v42)
                        new_param = new_param - step;
                    else
                        new_param = new_param + step;
                }
            } else {
                if((error1 <= 0.0 || error <= 0.0) && (error1 >= 0.0 || error >= 0.0)) {
                    param2 = *param_wanted;
                    error2 = error;
                } else {
                    param1 = *param_wanted;
                    error1 = error;
                }
                v43 = fabs(param1 - param2);
                v17 = foot_dt.len();
                if(tol > v43 * v17) {
                    *point_found = 1;
                    return;
                }
                new_param = (param1 + param2) / 2.0;
            }
        }
        param_prev = *param_wanted;
        error_prev = error;
        *param_wanted = new_param;
    }
}
double find_angle(SPAvector& vec1, SPAvector& vec2) {
    double ang = 180.0;
    long double v2 = double(SPAresabs);
    if(!vec1.is_zero(v2)) {
        long double v3 = double(SPAresabs);
        if(!vec2.is_zero(v3)) {
            const SPAvector& v4 = vec1 * vec2;
            SPAunit_vector normal = normalise(v4);
            double anga = angle_between(vec1, vec2, normal);
            if(anga == -1.0) anga = M_PI;
            ang = anga * 57.29577951308232;
            if(ang <= 90.0 || ang > 180.0) {
                if(ang <= 180.0 || ang > 270.0) {
                    if(ang > 270.0) return 360.0 - ang;
                } else {
                    return ang - 180.0;
                }
            } else {
                return 180.0 - ang;
            }
        }
    }
    return ang;
}

void map_param_into_interval(SPAinterval& this_int, double& param) {
    double low;
    double high;

    double period = this_int.length();
    if(period != 0.0) {
        low = this_int.start_pt() - SPAresabs;
        high = this_int.end_pt() + SPAresabs;
        if(low <= param) {
            if(param > high) {
                param = param - period;
                if(param > high) param = param - ceil((param - high) / period) * period;
            }
        } else {
            param = param + period;
            if(low > param) param = param + ceil((low - param) / period) * period;
        }
    }
}
// curve_curve_int*  int_ellipses2(ellipse* ell1, SPAinterval& rng1, ellipse* ell2, SPAinterval& rng2, int same_sense, int full_ellipse, double dist_tol)
//{
//    v7;                     // rax
//    ;                       // rax
//    long double started;    // xmm0_8
//    long double v10;        // xmm0_8
//    __int64 v11;            // r8
//    __int64 v12;            // r8
//    __int64 v13;            // r8
//    __int64 v14;            // r8
//    long double v15;        // xmm0_8
//    long double v16;        // xmm0_8
//    long double v17;        // xmm0_8
//    long double v18;        // xmm0_8
//    long double v19;        // xmm0_8
//    long double v20;        // xmm0_8
//    long double v21;        // xmm0_8
//    long double v22;        // xmm0_8
//    long double v23;        // xmm0_8
//    curve_curve_int* v24;   // rax
//    long double v25;        // xmm0_8
//    long double v26;        // xmm0_8
//    long double v27;        // xmm0_8
//    long double v28;        // xmm0_8
//    long double v29;        // xmm0_8
//    long double v30;        // xmm0_8
//    long double v31;        // xmm0_8
//    curve_curve_int* v32;   // rax
//    _DWORD* v33;            // rax
//    long double v34;        // xmm0_8
//    long double v35;        // xmm0_8
//    long double v36;        // xmm0_8
//    long double v37;        // xmm0_8
//    long double v38;        // xmm0_8
//    long double v39;        // xmm0_8
//    long double v40;        // xmm0_8
//    long double v41;        // xmm0_8
//    curve_curve_int* v42;   // rax
//    curve_curve_int* v43;   // rax
//    _DWORD* v44;            // rax
//    _DWORD* v45;            // rax
//    long double v46;        // xmm0_8
//    long double v47;        // xmm0_8
//    curve_curve_int* v48;   // rax
//    long double v49;        // xmm0_8
//    long double v50;        // xmm0_8
//    long double v51;        // xmm0_8
//    long double v52;        // xmm0_8
//    long double v53;        // xmm0_8
//    long double v54;        // xmm0_8
//    __int64 v55;            // rax
//    long double v56;        // xmm0_8
//    long double v57;        // xmm0_8
//    long double v58;        // xmm0_8
//    long double v59;        // xmm0_8
//    long double v60;        // xmm0_8
//    long double v61;        // xmm0_8
//    __int64 v62;            // rax
//    long double v63;        // xmm0_8
//    long double v64;        // xmm0_8
//    long double v65;        // xmm0_8
//    long double v66;        // xmm0_8
//    long double v67;        // xmm0_8
//    long double v68;        // xmm0_8
//    long double v69;        // xmm0_8
//    long double v70;        // xmm0_8
//    curve_curve_int* v71;   // rax
//    curve_curve_int* v72;   // rax
//    _DWORD* v73;            // rax
//    __int64 v74;            // rax
//    long double v75;        // xmm0_8
//    long double v76;        // xmm0_8
//    long double v77;        // xmm0_8
//    long double v78;        // xmm0_8
//    long double v79;        // xmm0_8
//    long double v80;        // xmm0_8
//    long double v81;        // xmm0_8
//    long double v82;        // xmm0_8
//    curve_curve_int* v83;   // rax
//    curve_curve_int* v84;   // rax
//    _DWORD* v85;            // rax
//    _DWORD* v86;            // rax
//    long double v87;        // xmm0_8
//    long double v88;        // xmm0_8
//    long double v89;        // xmm0_8
//    long double v90;        // xmm0_8
//    long double v91;        // xmm0_8
//    long double v92;        // xmm0_8
//    long double v93;        // xmm0_8
//    curve_curve_int* v94;   // rax
//    long double v95;        // xmm0_8
//    long double v96;        // xmm0_8
//    long double v97;        // xmm0_8
//    long double v98;        // xmm0_8
//    long double v99;        // xmm0_8
//    curve_curve_int* v100;  // rax
//    _DWORD* v101;           // rax
//    long double v102;       // xmm0_8
//    long double v103;       // xmm0_8
//    long double v104;       // xmm0_8
//    long double v105;       // xmm0_8
//    long double v106;       // xmm0_8
//    long double v107;       // xmm0_8
//    long double v108;       // xmm0_8
//    long double v109;       // xmm0_8
//    curve_curve_int* v110;  // rax
//    curve_curve_int* v111;  // rax
//    _DWORD* v112;           // rax
//    __int64 v113;           // rax
//    long double v114;       // xmm0_8
//    long double v115;       // xmm0_8
//    curve_curve_int* v116;  // rax
//    long double v117;       // xmm0_8
//    long double v118;       // xmm0_8
//    long double v119;       // xmm0_8
//    long double v120;       // xmm0_8
//    long double v121;       // xmm0_8
//    __int64 v122;           // rax
//    long double v123;       // xmm0_8
//    long double v124;       // xmm0_8
//    long double v125;       // xmm0_8
//    long double v126;       // xmm0_8
//    long double v127;       // xmm0_8
//    __int64 v128;           // rax
//    curve_curve_int* v129;  // rax
//    _DWORD* v130;           // rax
//    curve_curve_int* v131;  // rax
//    _DWORD* v132;           // rax
//    long double v133;       // xmm0_8
//    long double v134;       // xmm0_8
//    long double v135;       // xmm0_8
//    long double v136;       // xmm0_8
//    long double v137;       // xmm0_8
//    long double v138;       // xmm0_8
//    long double v139;       // xmm0_8
//    long double v140;       // xmm0_8
//    long double v141;       // xmm0_8
//    long double v142;       // xmm0_8
//    long double v143;       // xmm0_8
//    long double v144;       // xmm0_8
//    long double v145;       // xmm0_8
//    long double v146;       // xmm0_8
//    long double v147;       // xmm0_8
//    long double v148;       // xmm0_8
//    long double v149;       // xmm0_8
//    long double v150;       // xmm0_8
//    long double v151;       // xmm0_8
//    long double v152;       // xmm0_8
//    long double v153;       // xmm0_8
//    long double v154;       // xmm0_8
//    long double v155;       // xmm0_8
//    long double v156;       // xmm0_8
//    long double v157;       // xmm0_8
//    curve_curve_int* v158;  // rax
//    curve_curve_int* v159;  // rax
//    curve_curve_int* v160;  // rax
//    __int64 v161;           // rax
//    long double v162;       // xmm0_8
//    curve_curve_int* v163;  // rax
//    __int64 v164;           // rax
//    long double v165;       // xmm0_8
//    curve_curve_int* v166;  // rax
//    __int64 v167;           // rax
//    long double v168;       // xmm0_8
//    long double v169;       // xmm0_8
//    curve_curve_int* v170;  // rax
//    __int64 v171;           // rax
//    long double v172;       // xmm0_8
//    curve_curve_int* v173;  // rax
//    __int64 v174;           // rax
//    long double v176;       // xmm0_8
//    long double v177;       // xmm0_8
//    long double v178;       // xmm0_8
//    long double v179;       // xmm0_8
//    long double v180;       // xmm0_8
//    curve_curve_int* v181;  // rax
//    curve_curve_int* v182;  // rax
//    curve_curve_int* v183;  // rax
//    __int64 v184;           // rax
//    long double v185;       // xmm0_8
//    curve_curve_int* v186;  // rax
//    __int64 v187;           // rax
//    long double v188;       // xmm0_8
//    long double v189;       // xmm0_8
//    curve_curve_int* v190;  // rax
//    __int64 v191;           // rax
//    long double v192;       // xmm0_8
//    long double v193;       // xmm0_8
//    curve_curve_int* v194;  // rax
//    __int64 v195;           // rax
//    long double v196;       // xmm0_8
//    curve_curve_int* v197;  // rax
//    __int64 v198;           // rax
//    long v199;              // xmm0_8
//    long double v200;       // xmm0_8
//    curve_curve_int* v201;  // rax
//    __int64 v202;           // rax
//    long double v203;       // xmm0_8
//    curve_curve_int* v204;  // rax
//    __int64 v205;           // rax
//    long double v206;       // xmm0_8
//    long double v207;       // xmm0_8
//    curve_curve_int* v208;  // rax
//    __int64 v209;           // rax
//    long double v210;       // xmm0_8
//    curve_curve_int* v211;  // rax
//    __int64 v212;           // rax
//    long double v213;       // xmm0_8
//    long double v214;       // xmm0_8
//    long double v215;       // xmm0_8
//    curve_curve_int* v216;  // rax
//    __int64 v217;           // rax
//    long double v218;       // xmm0_8
//    long double v219;       // xmm0_8
//    long double v220;       // xmm0_8
//    curve_curve_int* v221;  // rax
//    __int64 v222;           // rax
//    long double v223;       // xmm0_8
//    long double v224;       // xmm0_8
//    long double v225;       // xmm0_8
//    curve_curve_int* v226;  // rax
//    __int64 v227;           // rax
//    long double v228;       // xmm0_8
//    long double v229;       // xmm0_8
//    long double v230;       // xmm0_8
//    curve_curve_int* v231;  // rax
//    __int64 v232;           // rax
//    long double v233;       // xmm0_8
//    long double v234;       // xmm0_8
//    long double v235;       // xmm0_8
//    long double v236;       // xmm0_8
//    curve_curve_int* v237;  // rax
//    __int64 v238;           // rax
//    curve_curve_int* cci;   // [rsp+30h] [rbp-1308h]
//
//    SPAparameter p2;                                                                                 // [rsp+58h] [rbp-12E0h] BYREF
//    SPAparameter p1;                                                                                 // [rsp+60h] [rbp-12D8h] BYREF
//    SPAparameter param4;                                                                             // [rsp+68h] [rbp-12D0h] BYREF
//    SPAparameter param3;                                                                             // [rsp+70h] [rbp-12C8h] BYREF
//    int p2s_in;                                                                                      // [rsp+78h] [rbp-12C0h]
//    int p1s_in;                                                                                      // [rsp+7Ch] [rbp-12BCh]
//    int p1e_in;                                                                                      // [rsp+80h] [rbp-12B8h]
//    int p2e_in;                                                                                      // [rsp+84h] [rbp-12B4h]
//    int coin_ss;                                                                                     // [rsp+88h] [rbp-12B0h]
//    int coin_se;                                                                                     // [rsp+8Ch] [rbp-12ACh]
//    int coin_es;                                                                                     // [rsp+90h] [rbp-12A8h]
//    int coin_ee;                                                                                     // [rsp+94h] [rbp-12A4h]
//    SPAposition p1e;                                                                                 // [rsp+98h] [rbp-12A0h] BYREF
//    p1s;                                                                                             // [rsp+B0h] [rbp-1288h] BYREF
//    curve_curve_int* v258;                                                                           // [rsp+C8h] [rbp-1270h]
//    curve_curve_int* v259;                                                                           // [rsp+D0h] [rbp-1268h]
//    SPAparameter v260;                                                                               // [rsp+D8h] [rbp-1260h] BYREF
//    SPAposition p2s;                                                                                 // [rsp+E0h] [rbp-1258h] BYREF
//    SPAposition p2e;                                                                                 // [rsp+F8h] [rbp-1240h] BYREF
//    SPAparameter v263;                                                                               // [rsp+110h] [rbp-1228h] BYREF
//    SPAparameter v264;                                                                               // [rsp+118h] [rbp-1220h] BYREF
//    SPAparameter v265;                                                                               // [rsp+120h] [rbp-1218h] BYREF
//    SPAparameter v266;                                                                               // [rsp+128h] [rbp-1210h] BYREF
//    SPAparameter v267;                                                                               // [rsp+130h] [rbp-1208h] BYREF
//    SPAparameter v268;                                                                               // [rsp+138h] [rbp-1200h] BYREF
//    SPAparameter v269;                                                                               // [rsp+140h] [rbp-11F8h] BYREF
//    SPAparameter v270;                                                                               // [rsp+148h] [rbp-11F0h] BYREF
//    curve_curve_int* v271;                                                                           // [rsp+150h] [rbp-11E8h]
//    SPAparameter param1;                                                                             // [rsp+158h] [rbp-11E0h] BYREF
//    SPAparameter param2;                                                                             // [rsp+160h] [rbp-11D8h] BYREF
//    SPAparameter p;                                                                                  // [rsp+168h] [rbp-11D0h] BYREF
//    SPAparameter v275;                                                                               // [rsp+170h] [rbp-11C8h] BYREF
//    SPAparameter v276;                                                                               // [rsp+178h] [rbp-11C0h] BYREF
//    curve_curve_int* v277;                                                                           // [rsp+180h] [rbp-11B8h]
//    SPAparameter param;                                                                              // [rsp+188h] [rbp-11B0h] BYREF
//    v279;                                                                                            // [rsp+190h] [rbp-11A8h]
//    SPAparameter v280;                                                                               // [rsp+198h] [rbp-11A0h] BYREF
//    v281;                                                                                            // [rsp+1A0h] [rbp-1198h]
//    curve_curve_int* n;                                                                              // [rsp+1A8h] [rbp-1190h]
//    curve_curve_int* v283;                                                                           // [rsp+1B0h] [rbp-1188h]
//    SPAparameter v284;                                                                               // [rsp+1B8h] [rbp-1180h] BYREF
//    SPAparameter v285;                                                                               // [rsp+1C0h] [rbp-1178h] BYREF
//    SPAparameter v286;                                                                               // [rsp+1C8h] [rbp-1170h] BYREF
//    curve_curve_int* cci_next;                                                                       // [rsp+1D0h] [rbp-1168h]
//    curve_curve_int* v288;                                                                           // [rsp+1D8h] [rbp-1160h]
//    SPAparameter v289;                                                                               // [rsp+1E0h] [rbp-1158h] BYREF
//    curve_curve_int* cci2;                                                                           // [rsp+1E8h] [rbp-1150h]
//    curve_curve_int* v291;                                                                           // [rsp+1F0h] [rbp-1148h]
//    SPAparameter v292;                                                                               // [rsp+1F8h] [rbp-1140h] BYREF
//    curve_curve_int* v293;                                                                           // [rsp+200h] [rbp-1138h]
//    SPAparameter v294;                                                                               // [rsp+208h] [rbp-1130h] BYREF
//    curve_curve_int* v295;                                                                           // [rsp+210h] [rbp-1128h]
//    curve_curve_int* v296;                                                                           // [rsp+218h] [rbp-1120h]
//    curve_curve_int* v297;                                                                           // [rsp+220h] [rbp-1118h]
//    SPAparameter v298;                                                                               // [rsp+228h] [rbp-1110h] BYREF
//    curve_curve_int* v299;                                                                           // [rsp+230h] [rbp-1108h]
//    curve_curve_int* v300;                                                                           // [rsp+238h] [rbp-1100h]
//    curve_curve_int* v301;                                                                           // [rsp+240h] [rbp-10F8h]
//    SPAparameter v302;                                                                               // [rsp+248h] [rbp-10F0h] BYREF
//    curve_curve_int* v303;                                                                           // [rsp+250h] [rbp-10E8h]
//    curve_curve_int* v304;                                                                           // [rsp+258h] [rbp-10E0h]
//    _DWORD* v305;                                                                                    // [rsp+260h] [rbp-10D8h]
//    curve_curve_int* v306;                                                                           // [rsp+268h] [rbp-10D0h]
//    curve_curve_int* v307;                                                                           // [rsp+270h] [rbp-10C8h]
//    curve_curve_int* v308;                                                                           // [rsp+278h] [rbp-10C0h]
//    curve_curve_int* v309;                                                                           // [rsp+280h] [rbp-10B8h]
//    curve_curve_int* v310;                                                                           // [rsp+288h] [rbp-10B0h]
//    _DWORD* v311;                                                                                    // [rsp+290h] [rbp-10A8h]
//    curve_curve_int* cci3;                                                                           // [rsp+298h] [rbp-10A0h]
//    curve_curve_int* v313;                                                                           // [rsp+2A0h] [rbp-1098h]
//    _DWORD* v314;                                                                                    // [rsp+2A8h] [rbp-1090h]
//    curve_curve_int* v315;                                                                           // [rsp+2B0h] [rbp-1088h]
//    curve_curve_int* v316;                                                                           // [rsp+2B8h] [rbp-1080h]
//    curve_curve_int* v317;                                                                           // [rsp+2C0h] [rbp-1078h]
//    curve_curve_int* v318;                                                                           // [rsp+2C8h] [rbp-1070h]
//    curve_curve_int* v319;                                                                           // [rsp+2D0h] [rbp-1068h]
//    curve_curve_int* v320;                                                                           // [rsp+2D8h] [rbp-1060h]
//    curve_curve_int* v321;                                                                           // [rsp+2E0h] [rbp-1058h]
//    curve_curve_int* v322;                                                                           // [rsp+2E8h] [rbp-1050h]
//    curve_curve_int* v323;                                                                           // [rsp+2F0h] [rbp-1048h]
//    curve_curve_int* v324;                                                                           // [rsp+2F8h] [rbp-1040h]
//    curve_curve_int* v325;                                                                           // [rsp+300h] [rbp-1038h]
//    curve_curve_int* v326;                                                                           // [rsp+308h] [rbp-1030h]
//    _DWORD* v327;                                                                                    // [rsp+310h] [rbp-1028h]
//    curve_curve_int* v328;                                                                           // [rsp+318h] [rbp-1020h]
//    curve_curve_int* v329;                                                                           // [rsp+320h] [rbp-1018h]
//    __int64 v330;                                                                                    // [rsp+328h] [rbp-1010h]
//    curve_curve_int* v331;                                                                           // [rsp+330h] [rbp-1008h]
//    curve_curve_int* v332;                                                                           // [rsp+338h] [rbp-1000h]
//    curve_curve_int* v333;                                                                           // [rsp+340h] [rbp-FF8h]
//    curve_curve_int* v334;                                                                           // [rsp+348h] [rbp-FF0h]
//    curve_curve_int* v335;                                                                           // [rsp+350h] [rbp-FE8h]
//    _DWORD* v336;                                                                                    // [rsp+358h] [rbp-FE0h]
//    curve_curve_int* v337;                                                                           // [rsp+360h] [rbp-FD8h]
//    curve_curve_int* v338;                                                                           // [rsp+368h] [rbp-FD0h]
//    _DWORD* v339;                                                                                    // [rsp+370h] [rbp-FC8h]
//    curve_curve_int* v340;                                                                           // [rsp+378h] [rbp-FC0h]
//    curve_curve_int* v341;                                                                           // [rsp+380h] [rbp-FB8h]
//    curve_curve_int* v342;                                                                           // [rsp+388h] [rbp-FB0h]
//    curve_curve_int* v343;                                                                           // [rsp+390h] [rbp-FA8h]
//    curve_curve_int* v344;                                                                           // [rsp+398h] [rbp-FA0h]
//    _DWORD* v345;                                                                                    // [rsp+3A0h] [rbp-F98h]
//    curve_curve_int* v346;                                                                           // [rsp+3A8h] [rbp-F90h]
//    curve_curve_int* v347;                                                                           // [rsp+3B0h] [rbp-F88h]
//    curve_curve_int* v348;                                                                           // [rsp+3B8h] [rbp-F80h]
//    curve_curve_int* v349;                                                                           // [rsp+3C0h] [rbp-F78h]
//    curve_curve_int* v350;                                                                           // [rsp+3C8h] [rbp-F70h]
//    _DWORD* v351;                                                                                    // [rsp+3D0h] [rbp-F68h]
//    curve_curve_int* v352;                                                                           // [rsp+3D8h] [rbp-F60h]
//    curve_curve_int* v353;                                                                           // [rsp+3E0h] [rbp-F58h]
//    __int64 v354;                                                                                    // [rsp+3E8h] [rbp-F50h]
//    curve_curve_int* v355;                                                                           // [rsp+3F0h] [rbp-F48h]
//    curve_curve_int* v356;                                                                           // [rsp+3F8h] [rbp-F40h]
//    curve_curve_int* v357;                                                                           // [rsp+400h] [rbp-F38h]
//    curve_curve_int* v358;                                                                           // [rsp+408h] [rbp-F30h]
//    curve_curve_int* v359;                                                                           // [rsp+410h] [rbp-F28h]
//    curve_curve_int* v360;                                                                           // [rsp+418h] [rbp-F20h]
//    curve_curve_int* v361;                                                                           // [rsp+420h] [rbp-F18h]
//    curve_curve_int* v362;                                                                           // [rsp+428h] [rbp-F10h]
//    curve_curve_int* v363;                                                                           // [rsp+430h] [rbp-F08h]
//    curve_curve_int* v364;                                                                           // [rsp+438h] [rbp-F00h]
//    _DWORD* v365;                                                                                    // [rsp+440h] [rbp-EF8h]
//    curve_curve_int* v366;                                                                           // [rsp+448h] [rbp-EF0h]
//    curve_curve_int* v367;                                                                           // [rsp+450h] [rbp-EE8h]
//    curve_curve_int* v368;                                                                           // [rsp+458h] [rbp-EE0h]
//    _DWORD* v369;                                                                                    // [rsp+460h] [rbp-ED8h]
//    curve_curve_int* v370;                                                                           // [rsp+468h] [rbp-ED0h]
//    curve_curve_int* v371;                                                                           // [rsp+470h] [rbp-EC8h]
//    curve_curve_int* v372;                                                                           // [rsp+478h] [rbp-EC0h]
//    curve_curve_int* v373;                                                                           // [rsp+480h] [rbp-EB8h]
//    curve_curve_int* v374;                                                                           // [rsp+488h] [rbp-EB0h]
//    curve_curve_int* v375;                                                                           // [rsp+490h] [rbp-EA8h]
//    curve_curve_int* v376;                                                                           // [rsp+498h] [rbp-EA0h]
//    curve_curve_int* v377;                                                                           // [rsp+4A0h] [rbp-E98h]
//    curve_curve_int* cci4;                                                                           // [rsp+4A8h] [rbp-E90h]
//    curve_curve_int* v379;                                                                           // [rsp+4B0h] [rbp-E88h]
//    curve_curve_int* v380;                                                                           // [rsp+4B8h] [rbp-E80h]
//    curve_curve_int* v381;                                                                           // [rsp+4C0h] [rbp-E78h]
//    curve_curve_int* v382;                                                                           // [rsp+4C8h] [rbp-E70h]
//    curve_curve_int* v383;                                                                           // [rsp+4D0h] [rbp-E68h]
//    curve_curve_int* v384;                                                                           // [rsp+4D8h] [rbp-E60h]
//    curve_curve_int* v385;                                                                           // [rsp+4E0h] [rbp-E58h]
//    curve_curve_int* v386;                                                                           // [rsp+4E8h] [rbp-E50h]
//    curve_curve_int* v387;                                                                           // [rsp+4F0h] [rbp-E48h]
//    curve_curve_int* v388;                                                                           // [rsp+4F8h] [rbp-E40h]
//    curve_curve_int* v389;                                                                           // [rsp+500h] [rbp-E38h]
//    curve_curve_int* v390;                                                                           // [rsp+508h] [rbp-E30h]
//    curve_curve_int* v391;                                                                           // [rsp+510h] [rbp-E28h]
//    curve_curve_int* v392;                                                                           // [rsp+518h] [rbp-E20h]
//    curve_curve_int* v393;                                                                           // [rsp+520h] [rbp-E18h]
//    curve_curve_int* v394;                                                                           // [rsp+528h] [rbp-E10h]
//    curve_curve_int* v395;                                                                           // [rsp+530h] [rbp-E08h]
//    curve_curve_int* v396;                                                                           // [rsp+538h] [rbp-E00h]
//    curve_curve_int* v397;                                                                           // [rsp+540h] [rbp-DF8h]
//    curve_curve_int* v398;                                                                           // [rsp+548h] [rbp-DF0h]
//    curve_curve_int* v399;                                                                           // [rsp+550h] [rbp-DE8h]
//    curve_curve_int* v400;                                                                           // [rsp+558h] [rbp-DE0h]
//    curve_curve_int* v401;                                                                           // [rsp+560h] [rbp-DD8h]
//    curve_curve_int* v402;                                                                           // [rsp+568h] [rbp-DD0h]
//    curve_curve_int* v403;                                                                           // [rsp+570h] [rbp-DC8h]
//    curve_curve_int* v404;                                                                           // [rsp+578h] [rbp-DC0h]
//    curve_curve_int* v405;                                                                           // [rsp+580h] [rbp-DB8h]
//    curve_curve_int* v406;                                                                           // [rsp+588h] [rbp-DB0h]
//    curve_curve_int* v407;                                                                           // [rsp+590h] [rbp-DA8h]
//    curve_curve_int* v408;                                                                           // [rsp+598h] [rbp-DA0h]
//    curve_curve_int* v409;                                                                           // [rsp+5A0h] [rbp-D98h]
//    curve_curve_int* v410;                                                                           // [rsp+5A8h] [rbp-D90h]
//    curve_curve_int* v411;                                                                           // [rsp+5B0h] [rbp-D88h]
//    curve_curve_int* v412;                                                                           // [rsp+5B8h] [rbp-D80h]
//    curve_curve_int* v413;                                                                           // [rsp+5C0h] [rbp-D78h]
//    curve_curve_int* v414;                                                                           // [rsp+5C8h] [rbp-D70h]
//    curve_curve_int* v415;                                                                           // [rsp+5D0h] [rbp-D68h]
//    curve_curve_int* v416;                                                                           // [rsp+5D8h] [rbp-D60h]
//    curve_curve_int* v417;                                                                           // [rsp+5E0h] [rbp-D58h]
//    curve_curve_int* v418;                                                                           // [rsp+5E8h] [rbp-D50h]
//    curve_curve_int* v419;                                                                           // [rsp+5F0h] [rbp-D48h]
//    curve_curve_int* v420;                                                                           // [rsp+5F8h] [rbp-D40h]
//    curve_curve_int* v421;                                                                           // [rsp+600h] [rbp-D38h]
//    curve_curve_int* v422;                                                                           // [rsp+608h] [rbp-D30h]
//    curve_curve_int* v423;                                                                           // [rsp+610h] [rbp-D28h]
//    curve_curve_int* v424;                                                                           // [rsp+618h] [rbp-D20h]
//    curve_curve_int* v425;                                                                           // [rsp+620h] [rbp-D18h]
//    curve_curve_int* v426;                                                                           // [rsp+628h] [rbp-D10h]
//    curve_curve_int* v427;                                                                           // [rsp+630h] [rbp-D08h]
//    curve_curve_int* v428;                                                                           // [rsp+638h] [rbp-D00h]
//    curve_curve_int* v429;                                                                           // [rsp+640h] [rbp-CF8h]
//    curve_curve_int* v430;                                                                           // [rsp+648h] [rbp-CF0h]
//    curve_curve_int* v431;                                                                           // [rsp+650h] [rbp-CE8h]
//    curve_curve_int* v432;                                                                           // [rsp+658h] [rbp-CE0h]
//    curve_curve_int* v433;                                                                           // [rsp+660h] [rbp-CD8h]
//    curve_curve_int* v434;                                                                           // [rsp+668h] [rbp-CD0h]
//    curve_curve_int* v435;                                                                           // [rsp+670h] [rbp-CC8h]
//    curve_curve_int* v436;                                                                           // [rsp+678h] [rbp-CC0h]
//    curve_curve_int* v437;                                                                           // [rsp+680h] [rbp-CB8h]
//    curve_curve_int* v438;                                                                           // [rsp+688h] [rbp-CB0h]
//    curve_curve_int* v439;                                                                           // [rsp+690h] [rbp-CA8h]
//    curve_curve_int* v440;                                                                           // [rsp+698h] [rbp-CA0h]
//    curve_curve_int* v441;                                                                           // [rsp+6A0h] [rbp-C98h]
//    curve_curve_int* v442;                                                                           // [rsp+6A8h] [rbp-C90h]
//    curve_curve_int* v443;                                                                           // [rsp+6B0h] [rbp-C88h]
//    curve_curve_int* v444;                                                                           // [rsp+6B8h] [rbp-C80h]
//    curve_curve_int* v445;                                                                           // [rsp+6C0h] [rbp-C78h]
//    curve_curve_int* v446;                                                                           // [rsp+6C8h] [rbp-C70h]
//    curve_curve_int* v447;                                                                           // [rsp+6D0h] [rbp-C68h]
//    curve_curve_int* v448;                                                                           // [rsp+6D8h] [rbp-C60h]
//    curve_curve_int* v449;                                                                           // [rsp+6E0h] [rbp-C58h]
//    curve_curve_int* v450;                                                                           // [rsp+6E8h] [rbp-C50h]
//    curve_curve_int* v451;                                                                           // [rsp+6F0h] [rbp-C48h]
//    curve_curve_int* v452;                                                                           // [rsp+6F8h] [rbp-C40h]
//    curve_curve_int* v453;                                                                           // [rsp+700h] [rbp-C38h]
//    curve_curve_int* v454;                                                                           // [rsp+708h] [rbp-C30h]
//    curve_curve_int* v455;                                                                           // [rsp+710h] [rbp-C28h]
//    curve_curve_int* v456;                                                                           // [rsp+718h] [rbp-C20h]
//    curve_curve_int* v457;                                                                           // [rsp+720h] [rbp-C18h]
//    curve_curve_int* v458;                                                                           // [rsp+728h] [rbp-C10h]
//    curve_curve_int* v459;                                                                           // [rsp+730h] [rbp-C08h]
//    SPAposition*(__fastcall * eval_position)(struct ellipse*, SPAposition*, long double, int, int);  // [rsp+738h] [rbp-C00h]
//    SPAposition*(__fastcall * v461)(struct ellipse*, SPAposition*, long double, int, int);           // [rsp+740h] [rbp-BF8h]
//    SPAposition*(__fastcall * v462)(struct ellipse*, SPAposition*, long double, int, int);           // [rsp+748h] [rbp-BF0h]
//    SPAposition*(__fastcall * v463)(struct ellipse*, SPAposition*, long double, int, int);           // [rsp+750h] [rbp-BE8h]
//    SPAparameter* v464;                                                                              // [rsp+758h] [rbp-BE0h]
//    long double(__fastcall * v465)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+760h] [rbp-BD8h]
//    SPAparameter* v466;                                                                              // [rsp+768h] [rbp-BD0h]
//    long double(__fastcall * v467)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+770h] [rbp-BC8h]
//    SPApar_pos* _uv;                                                                                 // [rsp+778h] [rbp-BC0h]
//    long double d1;                                                                                  // [rsp+780h] [rbp-BB8h]
//    curve_curve_int* v470;                                                                           // [rsp+788h] [rbp-BB0h]
//    SPApar_pos* v471;                                                                                // [rsp+790h] [rbp-BA8h]
//    long double d2;                                                                                  // [rsp+798h] [rbp-BA0h]
//    curve_curve_int* v473;                                                                           // [rsp+7A0h] [rbp-B98h]
//    SPApar_pos* v474;                                                                                // [rsp+7A8h] [rbp-B90h]
//    long double v475;                                                                                // [rsp+7B0h] [rbp-B88h]
//    long double v476;                                                                                // [rsp+7B8h] [rbp-B80h]
//    _DWORD* v477;                                                                                    // [rsp+7C0h] [rbp-B78h]
//    SPAparameter* v478;                                                                              // [rsp+7C8h] [rbp-B70h]
//    long double(__fastcall * v479)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+7D0h] [rbp-B68h]
//    SPAparameter* v480;                                                                              // [rsp+7D8h] [rbp-B60h]
//    long double(__fastcall * v481)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+7E0h] [rbp-B58h]
//    SPApar_pos* v482;                                                                                // [rsp+7E8h] [rbp-B50h]
//    long double v483;                                                                                // [rsp+7F0h] [rbp-B48h]
//    curve_curve_int* v484;                                                                           // [rsp+7F8h] [rbp-B40h]
//    SPApar_pos* v485;                                                                                // [rsp+800h] [rbp-B38h]
//    long double v486;                                                                                // [rsp+808h] [rbp-B30h]
//    curve_curve_int* v487;                                                                           // [rsp+810h] [rbp-B28h]
//    SPApar_pos* v488;                                                                                // [rsp+818h] [rbp-B20h]
//    long double v489;                                                                                // [rsp+820h] [rbp-B18h]
//    long double v490;                                                                                // [rsp+828h] [rbp-B10h]
//    _DWORD* v491;                                                                                    // [rsp+830h] [rbp-B08h]
//    SPApar_pos* v492;                                                                                // [rsp+838h] [rbp-B00h]
//    long double v493;                                                                                // [rsp+840h] [rbp-AF8h]
//    long double v494;                                                                                // [rsp+848h] [rbp-AF0h]
//    _DWORD* v495;                                                                                    // [rsp+850h] [rbp-AE8h]
//    SPAparameter* v496;                                                                              // [rsp+858h] [rbp-AE0h]
//    long double(__fastcall * v497)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+860h] [rbp-AD8h]
//    SPAparameter* v498;                                                                              // [rsp+868h] [rbp-AD0h]
//    long double(__fastcall * v499)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+870h] [rbp-AC8h]
//    SPApar_pos* v500;                                                                                // [rsp+878h] [rbp-AC0h]
//    curve_curve_int* v501;                                                                           // [rsp+880h] [rbp-AB8h]
//    SPApar_pos* v502;                                                                                // [rsp+888h] [rbp-AB0h]
//    long double v503;                                                                                // [rsp+890h] [rbp-AA8h]
//    long double v504;                                                                                // [rsp+898h] [rbp-AA0h]
//    curve_curve_int* v505;                                                                           // [rsp+8A0h] [rbp-A98h]
//    SPApar_pos* v506;                                                                                // [rsp+8A8h] [rbp-A90h]
//    long double v507;                                                                                // [rsp+8B0h] [rbp-A88h]
//    long double v508;                                                                                // [rsp+8B8h] [rbp-A80h]
//    curve_curve_int* v509;                                                                           // [rsp+8C0h] [rbp-A78h]
//    void* v510;                                                                                      // [rsp+8C8h] [rbp-A70h]
//    SPAparameter* v511;                                                                              // [rsp+8D0h] [rbp-A68h]
//    long double(__fastcall * v512)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+8D8h] [rbp-A60h]
//    SPAparameter* v513;                                                                              // [rsp+8E0h] [rbp-A58h]
//    long double(__fastcall * v514)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+8E8h] [rbp-A50h]
//    SPApar_pos* v515;                                                                                // [rsp+8F0h] [rbp-A48h]
//    curve_curve_int* v516;                                                                           // [rsp+8F8h] [rbp-A40h]
//    SPApar_pos* v517;                                                                                // [rsp+900h] [rbp-A38h]
//    long double v518;                                                                                // [rsp+908h] [rbp-A30h]
//    long double v519;                                                                                // [rsp+910h] [rbp-A28h]
//    curve_curve_int* v520;                                                                           // [rsp+918h] [rbp-A20h]
//    SPApar_pos* v521;                                                                                // [rsp+920h] [rbp-A18h]
//    long double v522;                                                                                // [rsp+928h] [rbp-A10h]
//    long double v523;                                                                                // [rsp+930h] [rbp-A08h]
//    _DWORD* v524;                                                                                    // [rsp+938h] [rbp-A00h]
//    SPApar_pos* v525;                                                                                // [rsp+940h] [rbp-9F8h]
//    __int64 v526;                                                                                    // [rsp+948h] [rbp-9F0h]
//    SPAparameter* v527;                                                                              // [rsp+950h] [rbp-9E8h]
//    long double(__fastcall * v528)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+958h] [rbp-9E0h]
//    SPAparameter* v529;                                                                              // [rsp+960h] [rbp-9D8h]
//    long double(__fastcall * v530)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+968h] [rbp-9D0h]
//    SPApar_pos* v531;                                                                                // [rsp+970h] [rbp-9C8h]
//    long double v532;                                                                                // [rsp+978h] [rbp-9C0h]
//    curve_curve_int* v533;                                                                           // [rsp+980h] [rbp-9B8h]
//    SPApar_pos* v534;                                                                                // [rsp+988h] [rbp-9B0h]
//    long double v535;                                                                                // [rsp+990h] [rbp-9A8h]
//    long double v536;                                                                                // [rsp+998h] [rbp-9A0h]
//    curve_curve_int* v537;                                                                           // [rsp+9A0h] [rbp-998h]
//    SPApar_pos* v538;                                                                                // [rsp+9A8h] [rbp-990h]
//    long double v539;                                                                                // [rsp+9B0h] [rbp-988h]
//    long double v540;                                                                                // [rsp+9B8h] [rbp-980h]
//    _DWORD* v541;                                                                                    // [rsp+9C0h] [rbp-978h]
//    SPApar_pos* v542;                                                                                // [rsp+9C8h] [rbp-970h]
//    long double v543;                                                                                // [rsp+9D0h] [rbp-968h]
//    _DWORD* v544;                                                                                    // [rsp+9D8h] [rbp-960h]
//    SPAparameter* v545;                                                                              // [rsp+9E0h] [rbp-958h]
//    long double(__fastcall * v546)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+9E8h] [rbp-950h]
//    SPAparameter* v547;                                                                              // [rsp+9F0h] [rbp-948h]
//    long double(__fastcall * v548)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+9F8h] [rbp-940h]
//    SPApar_pos* v549;                                                                                // [rsp+A00h] [rbp-938h]
//    long double v550;                                                                                // [rsp+A08h] [rbp-930h]
//    long double v551;                                                                                // [rsp+A10h] [rbp-928h]
//    curve_curve_int* v552;                                                                           // [rsp+A18h] [rbp-920h]
//    SPApar_pos* v553;                                                                                // [rsp+A20h] [rbp-918h]
//    long double v554;                                                                                // [rsp+A28h] [rbp-910h]
//    curve_curve_int* v555;                                                                           // [rsp+A30h] [rbp-908h]
//    SPApar_pos* v556;                                                                                // [rsp+A38h] [rbp-900h]
//    long double v557;                                                                                // [rsp+A40h] [rbp-8F8h]
//    _DWORD* v558;                                                                                    // [rsp+A48h] [rbp-8F0h]
//    SPAparameter* v559;                                                                              // [rsp+A50h] [rbp-8E8h]
//    long double(__fastcall * v560)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+A58h] [rbp-8E0h]
//    SPAparameter* v561;                                                                              // [rsp+A60h] [rbp-8D8h]
//    long double(__fastcall * v562)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+A68h] [rbp-8D0h]
//    SPApar_pos* v563;                                                                                // [rsp+A70h] [rbp-8C8h]
//    curve_curve_int* v564;                                                                           // [rsp+A78h] [rbp-8C0h]
//    SPApar_pos* v565;                                                                                // [rsp+A80h] [rbp-8B8h]
//    long double v566;                                                                                // [rsp+A88h] [rbp-8B0h]
//    curve_curve_int* v567;                                                                           // [rsp+A90h] [rbp-8A8h]
//    SPApar_pos* v568;                                                                                // [rsp+A98h] [rbp-8A0h]
//    long double v569;                                                                                // [rsp+AA0h] [rbp-898h]
//    long double v570;                                                                                // [rsp+AA8h] [rbp-890h]
//    _DWORD* v571;                                                                                    // [rsp+AB0h] [rbp-888h]
//    SPApar_pos* v572;                                                                                // [rsp+AB8h] [rbp-880h]
//    long double v573;                                                                                // [rsp+AC0h] [rbp-878h]
//    __int64 v574;                                                                                    // [rsp+AC8h] [rbp-870h]
//    SPAparameter* v575;                                                                              // [rsp+AD0h] [rbp-868h]
//    long double(__fastcall * v576)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+AD8h] [rbp-860h]
//    SPAparameter* v577;                                                                              // [rsp+AE0h] [rbp-858h]
//    long double(__fastcall * v578)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+AE8h] [rbp-850h]
//    SPApar_pos* v579;                                                                                // [rsp+AF0h] [rbp-848h]
//    curve_curve_int* v580;                                                                           // [rsp+AF8h] [rbp-840h]
//    SPApar_pos* v581;                                                                                // [rsp+B00h] [rbp-838h]
//    long double v582;                                                                                // [rsp+B08h] [rbp-830h]
//    curve_curve_int* v583;                                                                           // [rsp+B10h] [rbp-828h]
//    SPApar_pos* v584;                                                                                // [rsp+B18h] [rbp-820h]
//    long double v585;                                                                                // [rsp+B20h] [rbp-818h]
//    long double v586;                                                                                // [rsp+B28h] [rbp-810h]
//    curve_curve_int* v587;                                                                           // [rsp+B30h] [rbp-808h]
//    void* v588;                                                                                      // [rsp+B38h] [rbp-800h]
//    SPApar_pos* v589;                                                                                // [rsp+B40h] [rbp-7F8h]
//    long double v590;                                                                                // [rsp+B48h] [rbp-7F0h]
//    curve_curve_int* v591;                                                                           // [rsp+B50h] [rbp-7E8h]
//    SPApar_pos* v592;                                                                                // [rsp+B58h] [rbp-7E0h]
//    long double v593;                                                                                // [rsp+B60h] [rbp-7D8h]
//    _DWORD* v594;                                                                                    // [rsp+B68h] [rbp-7D0h]
//    SPApar_pos* v595;                                                                                // [rsp+B70h] [rbp-7C8h]
//    curve_curve_int* v596;                                                                           // [rsp+B78h] [rbp-7C0h]
//    SPApar_pos* v597;                                                                                // [rsp+B80h] [rbp-7B8h]
//    long double v598;                                                                                // [rsp+B88h] [rbp-7B0h]
//    long double v599;                                                                                // [rsp+B90h] [rbp-7A8h]
//    _DWORD* v600;                                                                                    // [rsp+B98h] [rbp-7A0h]
//    SPAparameter* v601;                                                                              // [rsp+BA0h] [rbp-798h]
//    long double(__fastcall * v602)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+BA8h] [rbp-790h]
//    SPAparameter* v603;                                                                              // [rsp+BB0h] [rbp-788h]
//    long double(__fastcall * v604)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+BB8h] [rbp-780h]
//    SPAparameter* v605;                                                                              // [rsp+BC0h] [rbp-778h]
//    long double(__fastcall * v606)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+BC8h] [rbp-770h]
//    SPAparameter* v607;                                                                              // [rsp+BD0h] [rbp-768h]
//    long double(__fastcall * v608)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+BD8h] [rbp-760h]
//    SPApar_pos* v609;                                                                                // [rsp+BE0h] [rbp-758h]
//    long double v610;                                                                                // [rsp+BE8h] [rbp-750h]
//    curve_curve_int* v611;                                                                           // [rsp+BF0h] [rbp-748h]
//    SPApar_pos* v612;                                                                                // [rsp+BF8h] [rbp-740h]
//    long double v613;                                                                                // [rsp+C00h] [rbp-738h]
//    long double v614;                                                                                // [rsp+C08h] [rbp-730h]
//    curve_curve_int* v615;                                                                           // [rsp+C10h] [rbp-728h]
//    SPApar_pos* v616;                                                                                // [rsp+C18h] [rbp-720h]
//    long double v617;                                                                                // [rsp+C20h] [rbp-718h]
//    curve_curve_int* v618;                                                                           // [rsp+C28h] [rbp-710h]
//    SPApar_pos* v619;                                                                                // [rsp+C30h] [rbp-708h]
//    long double v620;                                                                                // [rsp+C38h] [rbp-700h]
//    long double v621;                                                                                // [rsp+C40h] [rbp-6F8h]
//    curve_curve_int* v622;                                                                           // [rsp+C48h] [rbp-6F0h]
//    SPApar_pos* v623;                                                                                // [rsp+C50h] [rbp-6E8h]
//    long double v624;                                                                                // [rsp+C58h] [rbp-6E0h]
//    curve_curve_int* v625;                                                                           // [rsp+C60h] [rbp-6D8h]
//    SPApar_pos* v626;                                                                                // [rsp+C68h] [rbp-6D0h]
//    long double v627;                                                                                // [rsp+C70h] [rbp-6C8h]
//    long double v628;                                                                                // [rsp+C78h] [rbp-6C0h]
//    curve_curve_int* v629;                                                                           // [rsp+C80h] [rbp-6B8h]
//    SPApar_pos* v630;                                                                                // [rsp+C88h] [rbp-6B0h]
//    long double v631;                                                                                // [rsp+C90h] [rbp-6A8h]
//    curve_curve_int* v632;                                                                           // [rsp+C98h] [rbp-6A0h]
//    SPApar_pos* v633;                                                                                // [rsp+CA0h] [rbp-698h]
//    long double v634;                                                                                // [rsp+CA8h] [rbp-690h]
//    long double v635;                                                                                // [rsp+CB0h] [rbp-688h]
//    curve_curve_int* v636;                                                                           // [rsp+CB8h] [rbp-680h]
//    SPApar_pos* v637;                                                                                // [rsp+CC0h] [rbp-678h]
//    long double v638;                                                                                // [rsp+CC8h] [rbp-670h]
//    curve_curve_int* v639;                                                                           // [rsp+CD0h] [rbp-668h]
//    SPApar_pos* v640;                                                                                // [rsp+CD8h] [rbp-660h]
//    long double v641;                                                                                // [rsp+CE0h] [rbp-658h]
//    long double v642;                                                                                // [rsp+CE8h] [rbp-650h]
//    curve_curve_int* v643;                                                                           // [rsp+CF0h] [rbp-648h]
//    SPApar_pos* v644;                                                                                // [rsp+CF8h] [rbp-640h]
//    long double v645;                                                                                // [rsp+D00h] [rbp-638h]
//    curve_curve_int* v646;                                                                           // [rsp+D08h] [rbp-630h]
//    SPApar_pos* v647;                                                                                // [rsp+D10h] [rbp-628h]
//    long double v648;                                                                                // [rsp+D18h] [rbp-620h]
//    long double v649;                                                                                // [rsp+D20h] [rbp-618h]
//    curve_curve_int* v650;                                                                           // [rsp+D28h] [rbp-610h]
//    SPApar_pos* v651;                                                                                // [rsp+D30h] [rbp-608h]
//    long double v652;                                                                                // [rsp+D38h] [rbp-600h]
//    curve_curve_int* v653;                                                                           // [rsp+D40h] [rbp-5F8h]
//    SPApar_pos* v654;                                                                                // [rsp+D48h] [rbp-5F0h]
//    long double v655;                                                                                // [rsp+D50h] [rbp-5E8h]
//    curve_curve_int* v656;                                                                           // [rsp+D58h] [rbp-5E0h]
//    SPApar_pos* v657;                                                                                // [rsp+D60h] [rbp-5D8h]
//    long double v658;                                                                                // [rsp+D68h] [rbp-5D0h]
//    long double v659;                                                                                // [rsp+D70h] [rbp-5C8h]
//    curve_curve_int* v660;                                                                           // [rsp+D78h] [rbp-5C0h]
//    SPApar_pos* v661;                                                                                // [rsp+D80h] [rbp-5B8h]
//    long double v662;                                                                                // [rsp+D88h] [rbp-5B0h]
//    long double v663;                                                                                // [rsp+D90h] [rbp-5A8h]
//    curve_curve_int* v664;                                                                           // [rsp+D98h] [rbp-5A0h]
//    SPApar_pos* v665;                                                                                // [rsp+DA0h] [rbp-598h]
//    long double v666;                                                                                // [rsp+DA8h] [rbp-590h]
//    long double v667;                                                                                // [rsp+DB0h] [rbp-588h]
//    curve_curve_int* v668;                                                                           // [rsp+DB8h] [rbp-580h]
//    SPApar_pos* v669;                                                                                // [rsp+DC0h] [rbp-578h]
//    long double v670;                                                                                // [rsp+DC8h] [rbp-570h]
//    curve_curve_int* v671;                                                                           // [rsp+DD0h] [rbp-568h]
//    SPApar_pos* v672;                                                                                // [rsp+DD8h] [rbp-560h]
//    long double v673;                                                                                // [rsp+DE0h] [rbp-558h]
//    curve_curve_int* v674;                                                                           // [rsp+DE8h] [rbp-550h]
//    SPApar_pos* v675;                                                                                // [rsp+DF0h] [rbp-548h]
//    long double v676;                                                                                // [rsp+DF8h] [rbp-540h]
//    curve_curve_int* v677;                                                                           // [rsp+E00h] [rbp-538h]
//    SPApar_pos* v678;                                                                                // [rsp+E08h] [rbp-530h]
//    long double v679;                                                                                // [rsp+E10h] [rbp-528h]
//    long double v680;                                                                                // [rsp+E18h] [rbp-520h]
//    curve_curve_int* v681;                                                                           // [rsp+E20h] [rbp-518h]
//    SPApar_pos* v682;                                                                                // [rsp+E28h] [rbp-510h]
//    long double v683;                                                                                // [rsp+E30h] [rbp-508h]
//    long double v684;                                                                                // [rsp+E38h] [rbp-500h]
//    curve_curve_int* v685;                                                                           // [rsp+E40h] [rbp-4F8h]
//    SPApar_pos* v686;                                                                                // [rsp+E48h] [rbp-4F0h]
//    long double v687;                                                                                // [rsp+E50h] [rbp-4E8h]
//    curve_curve_int* v688;                                                                           // [rsp+E58h] [rbp-4E0h]
//    SPApar_pos* v689;                                                                                // [rsp+E60h] [rbp-4D8h]
//    long double v690;                                                                                // [rsp+E68h] [rbp-4D0h]
//    long double v691;                                                                                // [rsp+E70h] [rbp-4C8h]
//    curve_curve_int* v692;                                                                           // [rsp+E78h] [rbp-4C0h]
//    SPAparameter* parameter;                                                                         // [rsp+E80h] [rbp-4B8h]
//    long double(__fastcall * v694)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+E88h] [rbp-4B0h]
//    SPApar_pos* par_pos;                                                                             // [rsp+E90h] [rbp-4A8h]
//    long double v696;                                                                                // [rsp+E98h] [rbp-4A0h]
//    curve_curve_int* v697;                                                                           // [rsp+EA0h] [rbp-498h]
//    SPApar_pos* v698;                                                                                // [rsp+EA8h] [rbp-490h]
//    long double v699;                                                                                // [rsp+EB0h] [rbp-488h]
//    long double v700;                                                                                // [rsp+EB8h] [rbp-480h]
//    curve_curve_int* v701;                                                                           // [rsp+EC0h] [rbp-478h]
//    SPApar_pos* v702;                                                                                // [rsp+EC8h] [rbp-470h]
//    long double v703;                                                                                // [rsp+ED0h] [rbp-468h]
//    curve_curve_int* v704;                                                                           // [rsp+ED8h] [rbp-460h]
//    SPApar_pos* v705;                                                                                // [rsp+EE0h] [rbp-458h]
//    long double v706;                                                                                // [rsp+EE8h] [rbp-450h]
//    curve_curve_int* v707;                                                                           // [rsp+EF0h] [rbp-448h]
//    SPAparameter* v708;                                                                              // [rsp+EF8h] [rbp-440h]
//    long double(__fastcall * v709)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+F00h] [rbp-438h]
//    SPApar_pos* v710;                                                                                // [rsp+F08h] [rbp-430h]
//    curve_curve_int* v711;                                                                           // [rsp+F10h] [rbp-428h]
//    SPApar_pos* v712;                                                                                // [rsp+F18h] [rbp-420h]
//    long double v713;                                                                                // [rsp+F20h] [rbp-418h]
//    long double v714;                                                                                // [rsp+F28h] [rbp-410h]
//    curve_curve_int* v715;                                                                           // [rsp+F30h] [rbp-408h]
//    SPApar_pos* v716;                                                                                // [rsp+F38h] [rbp-400h]
//    long double v717;                                                                                // [rsp+F40h] [rbp-3F8h]
//    long double v718;                                                                                // [rsp+F48h] [rbp-3F0h]
//    curve_curve_int* v719;                                                                           // [rsp+F50h] [rbp-3E8h]
//    SPApar_pos* v720;                                                                                // [rsp+F58h] [rbp-3E0h]
//    long double v721;                                                                                // [rsp+F60h] [rbp-3D8h]
//    curve_curve_int* v722;                                                                           // [rsp+F68h] [rbp-3D0h]
//    SPAparameter* v723;                                                                              // [rsp+F70h] [rbp-3C8h]
//    long double(__fastcall * v724)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+F78h] [rbp-3C0h]
//    SPAparameter v725;                                                                               // [rsp+F80h] [rbp-3B8h] BYREF
//    SPAparameter* v726;                                                                              // [rsp+F88h] [rbp-3B0h]
//    long double(__fastcall * v727)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+F90h] [rbp-3A8h]
//    SPAparameter v728;                                                                               // [rsp+F98h] [rbp-3A0h] BYREF
//    SPApar_pos* v729;                                                                                // [rsp+FA0h] [rbp-398h]
//    long double v730;                                                                                // [rsp+FA8h] [rbp-390h]
//    curve_curve_int* v731;                                                                           // [rsp+FB0h] [rbp-388h]
//    SPApar_pos* v732;                                                                                // [rsp+FB8h] [rbp-380h]
//    long double v733;                                                                                // [rsp+FC0h] [rbp-378h]
//    long double v734;                                                                                // [rsp+FC8h] [rbp-370h]
//    curve_curve_int* v735;                                                                           // [rsp+FD0h] [rbp-368h]
//    SPAparameter* v736;                                                                              // [rsp+FD8h] [rbp-360h]
//    long double(__fastcall * v737)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+FE0h] [rbp-358h]
//    SPAparameter v738;                                                                               // [rsp+FE8h] [rbp-350h] BYREF
//    SPAparameter* v739;                                                                              // [rsp+FF0h] [rbp-348h]
//    long double(__fastcall * v740)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+FF8h] [rbp-340h]
//    SPAparameter v741;                                                                               // [rsp+1000h] [rbp-338h] BYREF
//    SPApar_pos* v742;                                                                                // [rsp+1008h] [rbp-330h]
//    long double v743;                                                                                // [rsp+1010h] [rbp-328h]
//    long double v744;                                                                                // [rsp+1018h] [rbp-320h]
//    curve_curve_int* v745;                                                                           // [rsp+1020h] [rbp-318h]
//    SPApar_pos* v746;                                                                                // [rsp+1028h] [rbp-310h]
//    long double v747;                                                                                // [rsp+1030h] [rbp-308h]
//    curve_curve_int* v748;                                                                           // [rsp+1038h] [rbp-300h]
//    SPAparameter* v749;                                                                              // [rsp+1040h] [rbp-2F8h]
//    long double(__fastcall * v750)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+1048h] [rbp-2F0h]
//    SPApar_pos* v751;                                                                                // [rsp+1050h] [rbp-2E8h]
//    long double v752;                                                                                // [rsp+1058h] [rbp-2E0h]
//    curve_curve_int* v753;                                                                           // [rsp+1060h] [rbp-2D8h]
//    SPApar_pos* v754;                                                                                // [rsp+1068h] [rbp-2D0h]
//    long double v755;                                                                                // [rsp+1070h] [rbp-2C8h]
//    long double v756;                                                                                // [rsp+1078h] [rbp-2C0h]
//    curve_curve_int* v757;                                                                           // [rsp+1080h] [rbp-2B8h]
//    SPAparameter* v758;                                                                              // [rsp+1088h] [rbp-2B0h]
//    long double(__fastcall * v759)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+1090h] [rbp-2A8h]
//    SPApar_pos* v760;                                                                                // [rsp+1098h] [rbp-2A0h]
//    curve_curve_int* v761;                                                                           // [rsp+10A0h] [rbp-298h]
//    SPApar_pos* v762;                                                                                // [rsp+10A8h] [rbp-290h]
//    long double v763;                                                                                // [rsp+10B0h] [rbp-288h]
//    long double v764;                                                                                // [rsp+10B8h] [rbp-280h]
//    curve_curve_int* v765;                                                                           // [rsp+10C0h] [rbp-278h]
//    SPAparameter* v766;                                                                              // [rsp+10C8h] [rbp-270h]
//    long double(__fastcall * v767)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+10D0h] [rbp-268h]
//    SPAparameter* v768;                                                                              // [rsp+10D8h] [rbp-260h]
//    long double(__fastcall * v769)(struct ellipse*, const SPAposition*, const SPAparameter*);        // [rsp+10E0h] [rbp-258h]
//    SPApar_pos* v770;                                                                                // [rsp+10E8h] [rbp-250h]
//    long double v771;                                                                                // [rsp+10F0h] [rbp-248h]
//    curve_curve_int* v772;                                                                           // [rsp+10F8h] [rbp-240h]
//    SPApar_pos* v773;                                                                                // [rsp+1100h] [rbp-238h]
//    long double v774;                                                                                // [rsp+1108h] [rbp-230h]
//    long double v775;                                                                                // [rsp+1110h] [rbp-228h]
//    curve_curve_int* v776;                                                                           // [rsp+1118h] [rbp-220h]
//    SPAbox* box;                                                                                     // [rsp+1120h] [rbp-218h]
//    SPAinterval*(__fastcall * param_range)(struct ellipse*, SPAinterval*, const SPAbox*);            // [rsp+1128h] [rbp-210h]
//    SPAbox* v779;                                                                                    // [rsp+1130h] [rbp-208h]
//    SPAinterval*(__fastcall * v780)(struct ellipse*, SPAinterval*, const SPAbox*);                   // [rsp+1138h] [rbp-200h]
//    SPAparameter v781;                                                                               // [rsp+1140h] [rbp-1F8h] BYREF
//    SPAparameter v782;                                                                               // [rsp+1148h] [rbp-1F0h] BYREF
//    SPAparameter v783;                                                                               // [rsp+1150h] [rbp-1E8h] BYREF
//    SPAparameter v784;                                                                               // [rsp+1158h] [rbp-1E0h] BYREF
//    SPAparameter v785;                                                                               // [rsp+1160h] [rbp-1D8h] BYREF
//    SPAparameter v786;                                                                               // [rsp+1168h] [rbp-1D0h] BYREF
//    SPAparameter v787;                                                                               // [rsp+1170h] [rbp-1C8h] BYREF
//    SPAparameter v788;                                                                               // [rsp+1178h] [rbp-1C0h] BYREF
//    SPAparameter v789;                                                                               // [rsp+1180h] [rbp-1B8h] BYREF
//    SPAparameter v790;                                                                               // [rsp+1188h] [rbp-1B0h] BYREF
//    SPAparameter v791;                                                                               // [rsp+1190h] [rbp-1A8h] BYREF
//    SPAparameter v792;                                                                               // [rsp+1198h] [rbp-1A0h] BYREF
//    SPAparameter v793;                                                                               // [rsp+11A0h] [rbp-198h] BYREF
//    SPAparameter v794;                                                                               // [rsp+11A8h] [rbp-190h] BYREF
//    SPAparameter v795;                                                                               // [rsp+11B0h] [rbp-188h] BYREF
//    SPAparameter v796;                                                                               // [rsp+11B8h] [rbp-180h] BYREF
//    SPAparameter v797;                                                                               // [rsp+11C0h] [rbp-178h] BYREF
//    SPAparameter v798;                                                                               // [rsp+11C8h] [rbp-170h] BYREF
//    SPAparameter v799;                                                                               // [rsp+11D0h] [rbp-168h] BYREF
//    SPAparameter v800;                                                                               // [rsp+11D8h] [rbp-160h] BYREF
//    SPAparameter v801;                                                                               // [rsp+11E0h] [rbp-158h] BYREF
//    SPAparameter v802;                                                                               // [rsp+11E8h] [rbp-150h] BYREF
//    SPAparameter v803;                                                                               // [rsp+11F0h] [rbp-148h] BYREF
//    SPAparameter v804;                                                                               // [rsp+11F8h] [rbp-140h] BYREF
//    SPAparameter v805;                                                                               // [rsp+1200h] [rbp-138h] BYREF
//    SPAparameter v806;                                                                               // [rsp+1208h] [rbp-130h] BYREF
//    SPAparameter v807;                                                                               // [rsp+1210h] [rbp-128h] BYREF
//    SPAparameter v808;                                                                               // [rsp+1218h] [rbp-120h] BYREF
//    SPAparameter v809;                                                                               // [rsp+1220h] [rbp-118h] BYREF
//    SPAparameter v810;                                                                               // [rsp+1228h] [rbp-110h] BYREF
//    SPAparameter v811;                                                                               // [rsp+1230h] [rbp-108h] BYREF
//    SPAparameter v812;                                                                               // [rsp+1238h] [rbp-100h] BYREF
//    SPAparameter v813;                                                                               // [rsp+1240h] [rbp-F8h] BYREF
//    v814;                                                                                            // [rsp+1248h] [rbp-F0h] BYREF
//    SPAparameter v815;                                                                               // [rsp+1250h] [rbp-E8h] BYREF
//    SPAparameter v816;                                                                               // [rsp+1258h] [rbp-E0h] BYREF
//    SPAparameter v817;                                                                               // [rsp+1260h] [rbp-D8h] BYREF
//    SPAparameter v818;                                                                               // [rsp+1268h] [rbp-D0h] BYREF
//    SPAparameter v819;                                                                               // [rsp+1270h] [rbp-C8h] BYREF
//    SPAparameter v820;                                                                               // [rsp+1278h] [rbp-C0h] BYREF
//    SPAparameter v821;                                                                               // [rsp+1280h] [rbp-B8h] BYREF
//    SPAparameter v822;                                                                               // [rsp+1288h] [rbp-B0h] BYREF
//    SPAparameter v823;                                                                               // [rsp+1290h] [rbp-A8h] BYREF
//    SPAparameter v824;                                                                               // [rsp+1298h] [rbp-A0h] BYREF
//    SPAparameter v825;                                                                               // [rsp+12A0h] [rbp-98h] BYREF
//    SPAparameter v826;                                                                               // [rsp+12A8h] [rbp-90h] BYREF
//    SPAparameter result;                                                                             // [rsp+12B0h] [rbp-88h] BYREF
//    SPAparameter v828;                                                                               // [rsp+12B8h] [rbp-80h] BYREF
//    SPAparameter v829;                                                                               // [rsp+12C0h] [rbp-78h] BYREF
//    SPAparameter v830;                                                                               // [rsp+12C8h] [rbp-70h] BYREF
//    SPAposition int1;                                                                                // [rsp+12D0h] [rbp-68h] BYREF
//    SPAposition int2;                                                                                // [rsp+12E8h] [rbp-50h] BYREF
//    SPAinterval v833;                                                                                // [rsp+1300h] [rbp-38h] BYREF
//    char v834;                                                                                       // [rsp+1318h] [rbp-20h] BYREF
//
//    SPAposition int1;
//    SPAposition int2;
//
//    // rng代表EDGE的区间
//    double par1s = rng1.start_pt();
//    double par2s = rng2.start_pt();
//    double par1e = rng1.end_pt();
//    double par2e = rng2.end_pt();
//    const SPAinterval& v7 = ell1->param_range(SpaAcis::NullObj::get_box());
//    map_param_into_interval(v7, par1s);
//    const SPAinterval& v8 = ell2->param_range(SpaAcis::NullObj::get_box());
//    map_param_into_interval(v8, par2s);
//
//    double started = rng1.start_pt();
//    par1e = par1s + par1e - started;
//    double v10 = rng2.start_pt();
//    par2e = par2s + par2e - v10;
//    SPAposition p1s = ell1->eval_position(par1s);
//    SPAposition p1e = ell1->eval_position(par1e);
//    SPAposition p2s = ell2->eval_position(par2s);
//    SPAposition p2e = ell2->eval_position(par2e);
//    int coin_ss = same_point(p1s, p2s, dist_tol);
//    int coin_ee = same_point(p1e, p2e, dist_tol);
//    int coin_se = same_point(p1s, p2e, dist_tol);
//    int coin_es = same_point(p1e, p2s, dist_tol);
//    curve_curve_int* cci = nullptr;
//    curve_curve_int* v279 = nullptr;
//    if(full_ellipse) {
//        if(full_ellipse == 1) {
//            // if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Full ellipse - Partial ellipse Intersection\n");
//            if(coin_ss || coin_es) {
//                double v199 = ell1->param(p2e, SpaAcis::NullObj::get_parameter());
//                SPAparameter param(v199);
//                curve_curve_int* v279 = nullptr;
//                if(same_sense) {
//                    if(param < par1s) {
//                        double v200 = ell1->param_period();
//                        param += v200;
//                    }
//                    curve_curve_int* v421 = ACIS_NEW curve_curve_int(nullptr, p2e, param.operator double(), par2e, SpaAcis::NullObj::get_par_pos());
//                    v279 = v421;
//                    curve_curve_int* v423 = ACIS_NEW curve_curve_int(v279, p1s, par1s, par2s, SpaAcis::NullObj::get_par_pos());
//                    cci = v423;
//                } else {
//                    if(param > par1e) {
//                        v203 = ell1->param_period();
//                        param -= v203;
//                    }
//                    curve_curve_int* v425 = ACIS_NEW curve_curve_int(nullptr, p1e, par1e, par2s, SpaAcis::NullObj::get_par_pos());
//                    v279 = v425;
//                    curve_curve_int* v427 = ACIS_NEW curve_curve_int(v279, p2e, param.operator double(), par2e, SpaAcis::NullObj::get_par_pos());
//                    cci = v427;
//                }
//                cci->low_rel = cur_cur_tangent;
//                cci->high_rel = cur_cur_coin;
//                v279->low_rel = cur_cur_coin;
//                v279->high_rel = cur_cur_tangent;
//            } else if(coin_se || coin_ee) {
//                double v206 = ell1->param(p2s, SpaAcis::NullObj::get_parameter());
//                SPAparameter v280(v206);
//                curve_curve_int* v281 = nullptr;
//                if(same_sense) {
//                    if(v280 > par1e) {
//                        v207 = ell1->param_period(ell1);
//                        SPAparameter::operator-=(&v280, &v816, v207);
//                    }
//                    v428 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3908, &alloc_file_index_2127);
//                    if(v428) {
//                        v710 = SpaAcis::NullObj::get_par_pos();
//                        curve_curve_int::curve_curve_int(v428, 0i64, &p1e, par1e, par2e, v710);
//                        v429 = v208;
//                    } else {
//                        v429 = 0i64;
//                    }
//                    v711 = v429;
//                    v281 = v429;
//                    v430 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3909, &alloc_file_index_2127);
//                    if(v430) {
//                        v712 = SpaAcis::NullObj::get_par_pos();
//                        v713 = par2s;
//                        v714 = SPAparameter::operator double(&v280);
//                        curve_curve_int::curve_curve_int(v430, v281, &p2s, v714, v713, v712);
//                        v431 = (curve_curve_int*)v209;
//                    } else {
//                        v431 = 0i64;
//                    }
//                    v715 = v431;
//                    cci = v431;
//                } else {
//                    if(operator<(&v280, par1s)) {
//                        v210 = ell1->param_period(ell1);
//                        SPAparameter::operator+=(&v280, &v817, v210);
//                    }
//                    v459 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3919, &alloc_file_index_2127);
//                    if(v459) {
//                        v716 = SpaAcis::NullObj::get_par_pos();
//                        v717 = par2s;
//                        v718 = SPAparameter::operator double(&v280);
//                        curve_curve_int::curve_curve_int(v459, 0i64, &p2s, v718, v717, v716);
//                        v433 = v211;
//                    } else {
//                        v433 = 0i64;
//                    }
//                    v719 = v433;
//                    v281 = v433;
//                    v434 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3920, &alloc_file_index_2127);
//                    if(v434) {
//                        v720 = SpaAcis::NullObj::get_par_pos();
//                        v721 = par1s;
//                        curve_curve_int::curve_curve_int(v434, v281, &p1s, par1s, par2e, v720);
//                        v435 = (curve_curve_int*)v212;
//                    } else {
//                        v435 = 0i64;
//                    }
//                    v722 = v435;
//                    cci = v435;
//                }
//                cci->low_rel = cur_cur_tangent;
//                cci->high_rel = cur_cur_coin;
//                v281->low_rel = cur_cur_coin;
//                v281->high_rel = cur_cur_tangent;
//            } else {
//                v277 = nullptr;
//                SPAparameter v818;
//                SPAparameter v276;
//                SPAparameter v260;
//                if(same_sense) {
//                    double v213 = ell1->param(p2s, SpaAcis::NullObj::get_parameter());
//                    SPAparameter v725(v213);
//                    v276 = v725.operator double();
//                    v214 = ell1->param(p2e, SpaAcis::NullObj::get_parameter());
//                    SPAparameter v728(v214);
//                    v260 = v728.operator double();
//                    if(v260 < v276) {
//                        double v215 = ell1->param_period();
//                        v260 += v215;
//                    }
//                    v436 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3944, &alloc_file_index_2127);
//                    if(v436) {
//                        v729 = SpaAcis::NullObj::get_par_pos();
//                        v730 = SPAparameter::operator double(&v260);
//                        curve_curve_int::curve_curve_int(v436, 0i64, &p2e, v730, par2e, v729);
//                        v437 = v216;
//                    } else {
//                        v437 = 0i64;
//                    }
//                    v731 = v437;
//                    v277 = v437;
//                    v438 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3945, &alloc_file_index_2127);
//                    if(v438) {
//                        v732 = SpaAcis::NullObj::get_par_pos();
//                        v733 = par2s;
//                        v734 = SPAparameter::operator double(&v276);
//                        curve_curve_int::curve_curve_int(v438, v277, &p2s, v734, v733, v732);
//                        v439 = (curve_curve_int*)v217;
//                    } else {
//                        v439 = 0i64;
//                    }
//                    v735 = v439;
//                    cci = v439;
//                } else {
//                    v737 = ell1->param;
//                    v736 = SpaAcis::NullObj::get_parameter();
//                    v218 = v737(ell1, &p2e, v736);
//                    SPAparameter::SPAparameter(&v738, v218);
//                    v276.val = v738.val;
//                    v740 = ell1->param;
//                    v739 = SpaAcis::NullObj::get_parameter();
//                    v219 = v740(ell1, &p2s, v739);
//                    SPAparameter::SPAparameter(&v741, v219);
//                    v260.val = v741.val;
//                    if(operator<(&v260, &v276)) {
//                        v220 = ell1->param_period(ell1);
//                        SPAparameter::operator+=(&v260, &v820, v220);
//                    }
//                    v440 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3958, &alloc_file_index_2127);
//                    if(v440) {
//                        v742 = SpaAcis::NullObj::get_par_pos();
//                        v743 = par2s;
//                        v744 = SPAparameter::operator double(&v260);
//                        curve_curve_int::curve_curve_int(v440, 0i64, &p2s, v744, v743, v742);
//                        v441 = v221;
//                    } else {
//                        v441 = 0i64;
//                    }
//                    v745 = v441;
//                    v277 = v441;
//                    v442 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3959, &alloc_file_index_2127);
//                    if(v442) {
//                        v746 = SpaAcis::NullObj::get_par_pos();
//                        v747 = SPAparameter::operator double(&v276);
//                        curve_curve_int::curve_curve_int(v442, v277, &p2e, v747, par2e, v746);
//                        v443 = (curve_curve_int*)v222;
//                    } else {
//                        v443 = 0i64;
//                    }
//                    v748 = v443;
//                    cci = v443;
//                }
//                cci->low_rel = cur_cur_tangent;
//                cci->high_rel = cur_cur_coin;
//                v277->low_rel = cur_cur_coin;
//                v277->high_rel = cur_cur_tangent;
//            }
//        } else if(full_ellipse == 2) {
//            if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Partial ellipse - Full ellipse Intersection\n");
//            v271 = 0i64;
//            if(coin_ss || coin_se) {
//                v750 = ell2->param;
//                v749 = SpaAcis::NullObj::get_parameter();
//                v223 = v750(ell2, &p1e, v749);
//                SPAparameter::SPAparameter(&v284, v223);
//                if(same_sense && operator<(&v284, par2s)) {
//                    v224 = ell2->param_period(ell2);
//                    SPAparameter::operator+=(&v284, &v821, v224);
//                }
//                if(!same_sense && operator>(&v284, par2s)) {
//                    v225 = ell2->param_period(ell2);
//                    SPAparameter::operator-=(&v284, &v822, v225);
//                }
//                v444 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3983, &alloc_file_index_2127);
//                if(v444) {
//                    v751 = SpaAcis::NullObj::get_par_pos();
//                    v752 = SPAparameter::operator double(&v284);
//                    curve_curve_int::curve_curve_int(v444, 0i64, &p1e, par1e, v752, v751);
//                    v445 = v226;
//                } else {
//                    v445 = 0i64;
//                }
//                v753 = v445;
//                v271 = v445;
//                v446 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3984, &alloc_file_index_2127);
//                if(v446) {
//                    v754 = SpaAcis::NullObj::get_par_pos();
//                    v755 = par2s;
//                    v756 = par1s;
//                    curve_curve_int::curve_curve_int(v446, v271, &p1s, par1s, par2s, v754);
//                    v447 = (curve_curve_int*)v227;
//                } else {
//                    v447 = 0i64;
//                }
//                v757 = v447;
//                cci = v447;
//            } else if(coin_es || coin_ee) {
//                v759 = ell2->param;
//                v758 = SpaAcis::NullObj::get_parameter();
//                v228 = v759(ell2, &p1s, v758);
//                SPAparameter::SPAparameter(&v285, v228);
//                if(same_sense && operator>(&v285, par2e)) {
//                    v229 = ell2->param_period(ell2);
//                    SPAparameter::operator-=(&v285, &v823, v229);
//                }
//                if(!same_sense && operator<(&v285, par2e)) {
//                    v230 = ell2->param_period(ell2);
//                    SPAparameter::operator+=(&v285, &v824, v230);
//                }
//                v448 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3998, &alloc_file_index_2127);
//                if(v448) {
//                    v760 = SpaAcis::NullObj::get_par_pos();
//                    curve_curve_int::curve_curve_int(v448, 0i64, &p1e, par1e, par2e, v760);
//                    v449 = v231;
//                } else {
//                    v449 = 0i64;
//                }
//                v761 = v449;
//                v271 = v449;
//                v450 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3999, &alloc_file_index_2127);
//                if(v450) {
//                    v762 = SpaAcis::NullObj::get_par_pos();
//                    v763 = SPAparameter::operator double(&v285);
//                    v764 = par1s;
//                    curve_curve_int::curve_curve_int(v450, v271, &p1s, par1s, v763, v762);
//                    v451 = (curve_curve_int*)v232;
//                } else {
//                    v451 = 0i64;
//                }
//                v765 = v451;
//                cci = v451;
//            } else {
//                v767 = ell2->param;
//                v766 = SpaAcis::NullObj::get_parameter();
//                v233 = v767(ell2, &p1s, v766);
//                SPAparameter::SPAparameter(&v302, v233);
//                v769 = ell2->param;
//                v768 = SpaAcis::NullObj::get_parameter();
//                v234 = v769(ell2, &p1e, v768);
//                SPAparameter::SPAparameter(&v286, v234);
//                if(same_sense && operator<(&v286, &v302)) {
//                    v235 = ell2->param_period(ell2);
//                    SPAparameter::operator+=(&v286, &v825, v235);
//                }
//                if(!same_sense && operator>(&v286, &v302)) {
//                    v236 = ell2->param_period(ell2);
//                    SPAparameter::operator-=(&v286, &v826, v236);
//                }
//                v452 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 4021, &alloc_file_index_2127);
//                if(v452) {
//                    v770 = SpaAcis::NullObj::get_par_pos();
//                    v771 = SPAparameter::operator double(&v286);
//                    curve_curve_int::curve_curve_int(v452, 0i64, &p1e, par1e, v771, v770);
//                    v453 = v237;
//                } else {
//                    v453 = 0i64;
//                }
//                v772 = v453;
//                v271 = v453;
//                v454 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 4022, &alloc_file_index_2127);
//                if(v454) {
//                    v773 = SpaAcis::NullObj::get_par_pos();
//                    v774 = SPAparameter::operator double(&v302);
//                    v775 = par1s;
//                    curve_curve_int::curve_curve_int(v454, v271, &p1s, par1s, v774, v773);
//                    v455 = (curve_curve_int*)v238;
//                } else {
//                    v455 = 0i64;
//                }
//                v776 = v455;
//                cci = v455;
//            }
//            cci->high_rel = cur_cur_coin;
//            v271->low_rel = cur_cur_coin;
//        }
//    } else {
//        if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Partial ellipse - Full ellipse Intersection\n");
//        if(!coin_ss || coin_ee) {
//            if(coin_ss || !coin_ee) {
//                if(!coin_se || coin_es) {
//                    if(coin_se || !coin_es) {
//                        if(coin_se && coin_es) {
//                            v362 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3638, &alloc_file_index_2127);
//                            if(v362) {
//                                v589 = SpaAcis::NullObj::get_par_pos();
//                                v590 = par2s;
//                                curve_curve_int::curve_curve_int(v362, 0i64, &p1e, par1e, par2s, v589);
//                                v363 = v129;
//                            } else {
//                                v363 = 0i64;
//                            }
//                            v591 = v363;
//                            v296 = v363;
//                            v364 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3639, &alloc_file_index_2127);
//                            if(v364) {
//                                v592 = SpaAcis::NullObj::get_par_pos();
//                                v593 = par1s;
//                                curve_curve_int::curve_curve_int(v364, v296, &p1s, par1s, par2e, v592);
//                                v365 = v130;
//                            } else {
//                                v365 = 0i64;
//                            }
//                            v594 = v365;
//                            cci = (curve_curve_int*)v365;
//                            if(same_sense) {
//                                *v365 = 2;
//                                v296->low_rel = cur_cur_tangent;
//                            } else {
//                                *v365 = 3;
//                                v296->low_rel = cur_cur_coin;
//                            }
//                        } else if(coin_ss && coin_ee) {
//                            v366 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3658, &alloc_file_index_2127);
//                            if(v366) {
//                                v595 = SpaAcis::NullObj::get_par_pos();
//                                curve_curve_int::curve_curve_int(v366, 0i64, &p1e, par1e, par2e, v595);
//                                v367 = v131;
//                            } else {
//                                v367 = 0i64;
//                            }
//                            v596 = v367;
//                            v297 = v367;
//                            v368 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3659, &alloc_file_index_2127);
//                            if(v368) {
//                                v597 = SpaAcis::NullObj::get_par_pos();
//                                v598 = par2s;
//                                v599 = par1s;
//                                curve_curve_int::curve_curve_int(v368, v297, &p1s, par1s, par2s, v597);
//                                v369 = v132;
//                            } else {
//                                v369 = 0i64;
//                            }
//                            v600 = v369;
//                            cci = (curve_curve_int*)v369;
//                            if(same_sense) {
//                                *v369 = 3;
//                                v297->low_rel = cur_cur_coin;
//                            } else {
//                                v297->low_rel = cur_cur_tangent;
//                                cci->high_rel = cur_cur_tangent;
//                            }
//                        } else if(!coin_ss && !coin_ee && !coin_se && !coin_es) {
//                            v602 = ell1->param;
//                            v601 = SpaAcis::NullObj::get_parameter();
//                            v133 = v602(ell1, &p2s, v601);
//                            SPAparameter::SPAparameter(&p1, v133);
//                            v604 = ell1->param;
//                            v603 = SpaAcis::NullObj::get_parameter();
//                            v134 = v604(ell1, &p2e, v603);
//                            SPAparameter::SPAparameter(&p2, v134);
//                            v606 = ell2->param;
//                            v605 = SpaAcis::NullObj::get_parameter();
//                            v135 = v606(ell2, &p1s, v605);
//                            SPAparameter::SPAparameter(&param3, v135);
//                            v608 = ell2->param;
//                            v607 = SpaAcis::NullObj::get_parameter();
//                            v136 = v608(ell2, &p1e, v607);
//                            SPAparameter::SPAparameter(&param4, v136);
//                            p2s_in = 0;
//                            p2e_in = 0;
//                            p1s_in = 0;
//                            p1e_in = 0;
//                            if(operator>(&p1, par1s) && operator<(&p1, par1e) || (v137 = ell1->param_period(ell1), v138 = operator+(&p1, v137), v138 > par1s) && (v139 = ell1->param_period(ell1), v140 = operator+(&p1, v139), par1e > v140)) {
//                                p2s_in = 1;
//                            }
//                            if(operator>(&p2, par1s) && operator<(&p2, par1e) || (v141 = ell1->param_period(ell1), v142 = operator+(&p2, v141), v142 > par1s) && (v143 = ell1->param_period(ell1), v144 = operator+(&p2, v143), par1e > v144)) {
//                                p2e_in = 1;
//                            }
//                            if(operator>(&param3, par2s) && operator<(&param3, par2e) || (v145 = ell2->param_period(ell2), v146 = operator+(&param3, v145), v146 > par2s) && (v147 = ell2->param_period(ell2), v148 = operator+(&param3, v147), par2e > v148))
//                            {
//                                p1s_in = 1;
//                            }
//                            if(operator>(&param4, par2s) && operator<(&param4, par2e) || (v149 = ell2->param_period(ell2), v150 = operator+(&param4, v149), v150 > par2s) && (v151 = ell2->param_period(ell2), v152 = operator+(&param4, v151), par2e > v152))
//                            {
//                                p1e_in = 1;
//                            }
//                            if(same_sense) {
//                                if(p1s_in && p1e_in && p2s_in && p2e_in) {
//                                    if(operator<(&p1, &p2)) {
//                                        v153 = ell1->param_period(ell1);
//                                        SPAparameter::operator+=(&p1, &v793, v153);
//                                    }
//                                    if(operator<(&p2, par1s)) {
//                                        v154 = ell1->param_period(ell1);
//                                        SPAparameter::operator+=(&p2, &v794, v154);
//                                        v155 = ell1->param_period(ell1);
//                                        SPAparameter::operator+=(&p1, &v795, v155);
//                                    }
//                                    if(operator<(&param4, par2s)) {
//                                        v156 = ell2->param_period(ell2);
//                                        SPAparameter::operator+=(&param4, &v796, v156);
//                                    }
//                                    if(operator>(&p2, par2e)) {
//                                        v157 = ell2->param_period(ell2);
//                                        SPAparameter::operator-=(&param3, &v797, v157);
//                                    }
//                                    v370 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3720, &alloc_file_index_2127);
//                                    if(v370) {
//                                        v609 = SpaAcis::NullObj::get_par_pos();
//                                        v610 = SPAparameter::operator double(&param4);
//                                        curve_curve_int::curve_curve_int(v370, 0i64, &p1e, par1e, v610, v609);
//                                        v371 = v158;
//                                    } else {
//                                        v371 = 0i64;
//                                    }
//                                    v611 = v371;
//                                    cci4 = v371;
//                                    v372 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3721, &alloc_file_index_2127);
//                                    if(v372) {
//                                        v612 = SpaAcis::NullObj::get_par_pos();
//                                        v613 = par2s;
//                                        v614 = SPAparameter::operator double(&p1);
//                                        curve_curve_int::curve_curve_int(v372, cci4, &p2s, v614, v613, v612);
//                                        v373 = v159;
//                                    } else {
//                                        v373 = 0i64;
//                                    }
//                                    v615 = v373;
//                                    v303 = v373;
//                                    v374 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3722, &alloc_file_index_2127);
//                                    if(v374) {
//                                        v616 = SpaAcis::NullObj::get_par_pos();
//                                        v617 = SPAparameter::operator double(&p2);
//                                        curve_curve_int::curve_curve_int(v374, v303, &p2e, v617, par2e, v616);
//                                        v375 = v160;
//                                    } else {
//                                        v375 = 0i64;
//                                    }
//                                    v618 = v375;
//                                    v299 = v375;
//                                    v376 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3723, &alloc_file_index_2127);
//                                    if(v376) {
//                                        v619 = SpaAcis::NullObj::get_par_pos();
//                                        v620 = SPAparameter::operator double(&param3);
//                                        v621 = par1s;
//                                        curve_curve_int::curve_curve_int(v376, v299, &p1s, par1s, v620, v619);
//                                        v377 = (curve_curve_int*)v161;
//                                    } else {
//                                        v377 = 0i64;
//                                    }
//                                    v622 = v377;
//                                    cci = v377;
//                                    cci4->low_rel = cur_cur_coin;
//                                    v303->high_rel = cur_cur_coin;
//                                    v303->low_rel = cur_cur_tangent;
//                                    v299->high_rel = cur_cur_tangent;
//                                    v299->low_rel = cur_cur_coin;
//                                    cci->high_rel = cur_cur_coin;
//                                } else {
//                                    v258 = 0i64;
//                                    if(p2s_in && p2e_in) {
//                                        if(operator<(&p2, &p1)) {
//                                            v162 = ell1->param_period(ell1);
//                                            SPAparameter::operator+=(&p2, &v798, v162);
//                                        }
//                                        v379 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3739, &alloc_file_index_2127);
//                                        if(v379) {
//                                            v623 = SpaAcis::NullObj::get_par_pos();
//                                            v624 = SPAparameter::operator double(&p2);
//                                            curve_curve_int::curve_curve_int(v379, 0i64, &p2e, v624, par2e, v623);
//                                            v380 = v163;
//                                        } else {
//                                            v380 = 0i64;
//                                        }
//                                        v625 = v380;
//                                        v258 = v380;
//                                        v381 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3740, &alloc_file_index_2127);
//                                        if(v381) {
//                                            v626 = SpaAcis::NullObj::get_par_pos();
//                                            v627 = par2s;
//                                            v628 = SPAparameter::operator double(&p1);
//                                            curve_curve_int::curve_curve_int(v381, v258, &p2s, v628, v627, v626);
//                                            v382 = (curve_curve_int*)v164;
//                                        } else {
//                                            v382 = 0i64;
//                                        }
//                                        v629 = v382;
//                                        cci = v382;
//                                    } else if(p2s_in && p1e_in) {
//                                        if(operator<(&param4, par2s)) {
//                                            v165 = ell2->param_period(ell2);
//                                            SPAparameter::operator+=(&param4, &v799, v165);
//                                        }
//                                        v383 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3748, &alloc_file_index_2127);
//                                        if(v383) {
//                                            v630 = SpaAcis::NullObj::get_par_pos();
//                                            v631 = SPAparameter::operator double(&param4);
//                                            curve_curve_int::curve_curve_int(v383, 0i64, &p1e, par1e, v631, v630);
//                                            v384 = v166;
//                                        } else {
//                                            v384 = 0i64;
//                                        }
//                                        v632 = v384;
//                                        v258 = v384;
//                                        v385 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3749, &alloc_file_index_2127);
//                                        if(v385) {
//                                            v633 = SpaAcis::NullObj::get_par_pos();
//                                            v634 = par2s;
//                                            v635 = SPAparameter::operator double(&p1);
//                                            curve_curve_int::curve_curve_int(v385, v258, &p2s, v635, v634, v633);
//                                            v386 = (curve_curve_int*)v167;
//                                        } else {
//                                            v386 = 0i64;
//                                        }
//                                        v636 = v386;
//                                        cci = v386;
//                                    } else if(p1s_in && p2e_in) {
//                                        if(operator<(&p2, par1s)) {
//                                            v168 = ell1->param_period(ell1);
//                                            SPAparameter::operator+=(&p2, &v800, v168);
//                                        }
//                                        if(operator>(&param3, par2e)) {
//                                            v169 = ell2->param_period(ell2);
//                                            SPAparameter::operator-=(&param3, &v801, v169);
//                                        }
//                                        v387 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3759, &alloc_file_index_2127);
//                                        if(v387) {
//                                            v637 = SpaAcis::NullObj::get_par_pos();
//                                            v638 = SPAparameter::operator double(&p2);
//                                            curve_curve_int::curve_curve_int(v387, 0i64, &p2e, v638, par2e, v637);
//                                            v388 = v170;
//                                        } else {
//                                            v388 = 0i64;
//                                        }
//                                        v639 = v388;
//                                        v258 = v388;
//                                        v389 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3760, &alloc_file_index_2127);
//                                        if(v389) {
//                                            v640 = SpaAcis::NullObj::get_par_pos();
//                                            v641 = SPAparameter::operator double(&param3);
//                                            v642 = par1s;
//                                            curve_curve_int::curve_curve_int(v389, v258, &p1s, par1s, v641, v640);
//                                            v390 = (curve_curve_int*)v171;
//                                        } else {
//                                            v390 = 0i64;
//                                        }
//                                        v643 = v390;
//                                        cci = v390;
//                                    } else {
//                                        if(!p1s_in || !p1e_in) return 0i64;
//                                        if(operator<(&param4, &param3)) {
//                                            v172 = ell2->param_period(ell2);
//                                            SPAparameter::operator+=(&param4, &v802, v172);
//                                        }
//                                        v391 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3768, &alloc_file_index_2127);
//                                        if(v391) {
//                                            v644 = SpaAcis::NullObj::get_par_pos();
//                                            v645 = SPAparameter::operator double(&param4);
//                                            curve_curve_int::curve_curve_int(v391, 0i64, &p1e, par1e, v645, v644);
//                                            v392 = v173;
//                                        } else {
//                                            v392 = 0i64;
//                                        }
//                                        v646 = v392;
//                                        v258 = v392;
//                                        v393 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3769, &alloc_file_index_2127);
//                                        if(v393) {
//                                            v647 = SpaAcis::NullObj::get_par_pos();
//                                            v648 = SPAparameter::operator double(&param3);
//                                            v649 = par1s;
//                                            curve_curve_int::curve_curve_int(v393, v258, &p1s, par1s, v648, v647);
//                                            v394 = (curve_curve_int*)v174;
//                                        } else {
//                                            v394 = 0i64;
//                                        }
//                                        v650 = v394;
//                                        cci = v394;
//                                    }
//                                    v258->low_rel = cur_cur_coin;
//                                    cci->high_rel = cur_cur_coin;
//                                }
//                            } else if(p1s_in && p1e_in && p2s_in && p2e_in) {
//                                if(operator<(&p2, &p1)) {
//                                    v176 = ell1->param_period(ell1);
//                                    SPAparameter::operator+=(&p2, &v803, v176);
//                                }
//                                if(operator<(&p1, par1s)) {
//                                    v177 = ell1->param_period(ell1);
//                                    SPAparameter::operator+=(&p1, &v804, v177);
//                                    v178 = ell1->param_period(ell1);
//                                    SPAparameter::operator+=(&p2, &v805, v178);
//                                }
//                                if(operator>(&param4, par2e)) {
//                                    v179 = ell2->param_period(ell2);
//                                    SPAparameter::operator-=(&param4, &v806, v179);
//                                }
//                                if(operator<(&param3, par2s)) {
//                                    v180 = ell2->param_period(ell2);
//                                    SPAparameter::operator+=(&param3, &v807, v180);
//                                }
//                                v395 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3799, &alloc_file_index_2127);
//                                if(v395) {
//                                    v651 = SpaAcis::NullObj::get_par_pos();
//                                    v652 = SPAparameter::operator double(&param4);
//                                    curve_curve_int::curve_curve_int(v395, 0i64, &p1e, par1e, v652, v651);
//                                    v396 = v181;
//                                } else {
//                                    v396 = 0i64;
//                                }
//                                v653 = v396;
//                                v403 = v396;
//                                v397 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3800, &alloc_file_index_2127);
//                                if(v397) {
//                                    v654 = SpaAcis::NullObj::get_par_pos();
//                                    v655 = SPAparameter::operator double(&p2);
//                                    curve_curve_int::curve_curve_int(v397, v403, &p2e, v655, par2e, v654);
//                                    v398 = v182;
//                                } else {
//                                    v398 = 0i64;
//                                }
//                                v656 = v398;
//                                v300 = v398;
//                                v399 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3801, &alloc_file_index_2127);
//                                if(v399) {
//                                    v657 = SpaAcis::NullObj::get_par_pos();
//                                    v658 = par2s;
//                                    v659 = SPAparameter::operator double(&p1);
//                                    curve_curve_int::curve_curve_int(v399, v300, &p2s, v659, v658, v657);
//                                    v400 = v183;
//                                } else {
//                                    v400 = 0i64;
//                                }
//                                v660 = v400;
//                                v301 = v400;
//                                v401 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3802, &alloc_file_index_2127);
//                                if(v401) {
//                                    v661 = SpaAcis::NullObj::get_par_pos();
//                                    v662 = SPAparameter::operator double(&param3);
//                                    v663 = par1s;
//                                    curve_curve_int::curve_curve_int(v401, v301, &p1s, par1s, v662, v661);
//                                    v402 = (curve_curve_int*)v184;
//                                } else {
//                                    v402 = 0i64;
//                                }
//                                v664 = v402;
//                                cci = v402;
//                                v403->low_rel = cur_cur_coin;
//                                v300->high_rel = cur_cur_coin;
//                                v300->low_rel = cur_cur_tangent;
//                                v301->high_rel = cur_cur_tangent;
//                                v301->low_rel = cur_cur_coin;
//                                cci->high_rel = cur_cur_coin;
//                            } else {
//                                v259 = 0i64;
//                                if(p2s_in && p2e_in) {
//                                    if(operator<(&p1, &p2)) {
//                                        v185 = ell1->param_period(ell1);
//                                        SPAparameter::operator+=(&p1, &v808, v185);
//                                    }
//                                    v404 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3818, &alloc_file_index_2127);
//                                    if(v404) {
//                                        v665 = SpaAcis::NullObj::get_par_pos();
//                                        v666 = par2s;
//                                        v667 = SPAparameter::operator double(&p1);
//                                        curve_curve_int::curve_curve_int(v404, 0i64, &p2s, v667, v666, v665);
//                                        v432 = v186;
//                                    } else {
//                                        v432 = 0i64;
//                                    }
//                                    v668 = v432;
//                                    v259 = v432;
//                                    v406 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3819, &alloc_file_index_2127);
//                                    if(v406) {
//                                        v669 = SpaAcis::NullObj::get_par_pos();
//                                        v670 = SPAparameter::operator double(&p2);
//                                        curve_curve_int::curve_curve_int(v406, v259, &p2e, v670, par2e, v669);
//                                        v407 = (curve_curve_int*)v187;
//                                    } else {
//                                        v407 = 0i64;
//                                    }
//                                    v671 = v407;
//                                    cci = v407;
//                                } else if(p1e_in && p2e_in) {
//                                    if(operator>(&p2, par1e)) {
//                                        v188 = ell1->param_period(ell1);
//                                        SPAparameter::operator-=(&p2, &v809, v188);
//                                    }
//                                    if(operator>(&param4, par2e)) {
//                                        v189 = ell2->param_period(ell2);
//                                        SPAparameter::operator-=(&param4, &v810, v189);
//                                    }
//                                    v408 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3829, &alloc_file_index_2127);
//                                    if(v408) {
//                                        v672 = SpaAcis::NullObj::get_par_pos();
//                                        v673 = SPAparameter::operator double(&param4);
//                                        curve_curve_int::curve_curve_int(v408, 0i64, &p1e, par1e, v673, v672);
//                                        v409 = v190;
//                                    } else {
//                                        v409 = 0i64;
//                                    }
//                                    v674 = v409;
//                                    v259 = v409;
//                                    v410 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3830, &alloc_file_index_2127);
//                                    if(v410) {
//                                        v675 = SpaAcis::NullObj::get_par_pos();
//                                        v676 = SPAparameter::operator double(&p2);
//                                        curve_curve_int::curve_curve_int(v410, v259, &p2e, v676, par2e, v675);
//                                        v411 = (curve_curve_int*)v191;
//                                    } else {
//                                        v411 = 0i64;
//                                    }
//                                    v677 = v411;
//                                    cci = v411;
//                                } else if(p1s_in && p2s_in) {
//                                    if(operator<(&p1, par1s)) {
//                                        v192 = ell1->param_period(ell1);
//                                        SPAparameter::operator+=(&p1, &v811, v192);
//                                    }
//                                    if(operator<(&param3, par2s)) {
//                                        v193 = ell2->param_period(ell2);
//                                        SPAparameter::operator+=(&param3, &v812, v193);
//                                    }
//                                    v412 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3840, &alloc_file_index_2127);
//                                    if(v412) {
//                                        v678 = SpaAcis::NullObj::get_par_pos();
//                                        v679 = par2s;
//                                        v680 = SPAparameter::operator double(&p1);
//                                        curve_curve_int::curve_curve_int(v412, 0i64, &p2s, v680, v679, v678);
//                                        v413 = v194;
//                                    } else {
//                                        v413 = 0i64;
//                                    }
//                                    v681 = v413;
//                                    v259 = v413;
//                                    v414 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3841, &alloc_file_index_2127);
//                                    if(v414) {
//                                        v682 = SpaAcis::NullObj::get_par_pos();
//                                        v683 = SPAparameter::operator double(&param3);
//                                        v684 = par1s;
//                                        curve_curve_int::curve_curve_int(v414, v259, &p1s, par1s, v683, v682);
//                                        v415 = (curve_curve_int*)v195;
//                                    } else {
//                                        v415 = 0i64;
//                                    }
//                                    v685 = v415;
//                                    cci = v415;
//                                } else {
//                                    if(!p1s_in || !p1e_in) return 0i64;
//                                    if(operator>(&param4, &param3)) {
//                                        v196 = ell2->param_period(ell2);
//                                        SPAparameter::operator-=(&param3, &v813, v196);
//                                    }
//                                    v416 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3849, &alloc_file_index_2127);
//                                    if(v416) {
//                                        v686 = SpaAcis::NullObj::get_par_pos();
//                                        v687 = SPAparameter::operator double(&param4);
//                                        curve_curve_int::curve_curve_int(v416, 0i64, &p1e, par1e, v687, v686);
//                                        v417 = v197;
//                                    } else {
//                                        v417 = 0i64;
//                                    }
//                                    v688 = v417;
//                                    v259 = v417;
//                                    v418 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3850, &alloc_file_index_2127);
//                                    if(v418) {
//                                        v689 = SpaAcis::NullObj::get_par_pos();
//                                        v690 = SPAparameter::operator double(&param3);
//                                        v691 = par1s;
//                                        curve_curve_int::curve_curve_int(v418, v259, &p1s, par1s, v690, v689);
//                                        v419 = (curve_curve_int*)v198;
//                                    } else {
//                                        v419 = 0i64;
//                                    }
//                                    v692 = v419;
//                                    cci = v419;
//                                }
//                                v259->low_rel = cur_cur_coin;
//                                cci->high_rel = cur_cur_coin;
//                            }
//                        }
//                    } else if(same_sense) {
//                        v560 = ell1->param;
//                        v559 = SpaAcis::NullObj::get_parameter();
//                        v102 = v560(ell1, &p2e, v559);
//                        SPAparameter::SPAparameter(&v268, v102);
//                        if(operator<(&v268, par1e) && operator>(&v268, par1s) || (v103 = ell1->param_period(ell1), v104 = operator+(&v268, v103), par1e > v104) && (v105 = ell1->param_period(ell1), v106 = operator+(&v268, v105), v106 > par1s)) {
//                            v562 = ell2->param;
//                            v561 = SpaAcis::NullObj::get_parameter();
//                            v107 = v562(ell2, &p1s, v561);
//                            SPAparameter::SPAparameter(&v294, v107);
//                            if(operator<(&v268, par1s)) {
//                                v108 = ell1->param_period(ell1);
//                                SPAparameter::operator+=(&v268, &v789, v108);
//                            }
//                            if(operator>(&v294, par2e)) {
//                                v109 = ell2->param_period(ell2);
//                                SPAparameter::operator-=(&v294, &v790, v109);
//                            }
//                            v346 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3585, &alloc_file_index_2127);
//                            if(v346) {
//                                v563 = SpaAcis::NullObj::get_par_pos();
//                                curve_curve_int::curve_curve_int(v346, 0i64, &p1e, par1e, par2e, v563);
//                                v347 = v110;
//                            } else {
//                                v347 = 0i64;
//                            }
//                            v564 = v347;
//                            v352 = v347;
//                            v348 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3586, &alloc_file_index_2127);
//                            if(v348) {
//                                v565 = SpaAcis::NullObj::get_par_pos();
//                                v566 = SPAparameter::operator double(&v268);
//                                curve_curve_int::curve_curve_int(v348, v352, &p2e, v566, par2e, v565);
//                                v349 = v111;
//                            } else {
//                                v349 = 0i64;
//                            }
//                            v567 = v349;
//                            v295 = v349;
//                            v350 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3587, &alloc_file_index_2127);
//                            if(v350) {
//                                v568 = SpaAcis::NullObj::get_par_pos();
//                                v569 = SPAparameter::operator double(&v294);
//                                v570 = par1s;
//                                curve_curve_int::curve_curve_int(v350, v295, &p1s, par1s, v569, v568);
//                                v351 = v112;
//                            } else {
//                                v351 = 0i64;
//                            }
//                            v571 = v351;
//                            cci = (curve_curve_int*)v351;
//                            *v351 = 3;
//                            v295->low_rel = cur_cur_coin;
//                            v295->high_rel = cur_cur_tangent;
//                            v352->low_rel = cur_cur_tangent;
//                        } else {
//                            v353 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3596, &alloc_file_index_2127);
//                            if(v353) {
//                                v572 = SpaAcis::NullObj::get_par_pos();
//                                v573 = par2s;
//                                curve_curve_int::curve_curve_int(v353, 0i64, &p1e, par1e, par2s, v572);
//                                v354 = v113;
//                            } else {
//                                v354 = 0i64;
//                            }
//                            v574 = v354;
//                            cci = (curve_curve_int*)v354;
//                            *(_DWORD*)(v354 + 32) = 2;
//                        }
//                    } else {
//                        v283 = 0i64;
//                        v576 = ell1->param;
//                        v575 = SpaAcis::NullObj::get_parameter();
//                        v114 = v576(ell1, &p2e, v575);
//                        SPAparameter::SPAparameter(&v269, v114);
//                        v578 = ell2->param;
//                        v577 = SpaAcis::NullObj::get_parameter();
//                        v115 = v578(ell2, &p1s, v577);
//                        SPAparameter::SPAparameter(&v270, v115);
//                        v355 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3609, &alloc_file_index_2127);
//                        if(v355) {
//                            v579 = SpaAcis::NullObj::get_par_pos();
//                            curve_curve_int::curve_curve_int(v355, 0i64, &p1e, par1e, par2e, v579);
//                            v356 = v116;
//                        } else {
//                            v356 = 0i64;
//                        }
//                        v580 = v356;
//                        v283 = v356;
//                        if(operator<(&v269, par1e) && operator>(&v269, par1s) || (v117 = ell1->param_period(ell1), v118 = operator+(&v269, v117), par1e > v118) && (v119 = ell1->param_period(ell1), v120 = operator+(&v269, v119), v120 > par1s)) {
//                            if(operator>(&v269, par1e)) {
//                                v121 = ell1->param_period(ell1);
//                                SPAparameter::operator-=(&v269, &v791, v121);
//                            }
//                            v357 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3615, &alloc_file_index_2127);
//                            if(v357) {
//                                v581 = SpaAcis::NullObj::get_par_pos();
//                                v582 = SPAparameter::operator double(&v269);
//                                curve_curve_int::curve_curve_int(v357, v283, &p2e, v582, par2e, v581);
//                                v358 = (curve_curve_int*)v122;
//                            } else {
//                                v358 = 0i64;
//                            }
//                            v583 = v358;
//                            cci = v358;
//                        } else if(operator<(&v270, par2e) && operator>(&v270, par2s) || (v123 = ell2->param_period(ell2), v124 = operator+(&v270, v123), par2e > v124) && (v125 = ell2->param_period(ell2), v126 = operator+(&v270, v125), v126 > par2s)) {
//                            if(operator<(&v270, par2e)) {
//                                v127 = ell2->param_period(ell2);
//                                SPAparameter::operator+=(&v270, &v792, v127);
//                            }
//                            v359 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3622, &alloc_file_index_2127);
//                            if(v359) {
//                                v584 = SpaAcis::NullObj::get_par_pos();
//                                v585 = SPAparameter::operator double(&v270);
//                                v586 = par1s;
//                                curve_curve_int::curve_curve_int(v359, v283, &p1s, par1s, v585, v584);
//                                v360 = (curve_curve_int*)v128;
//                            } else {
//                                v360 = 0i64;
//                            }
//                            v587 = v360;
//                            cci = v360;
//                        }
//                        if(cci) {
//                            cci->high_rel = cur_cur_coin;
//                            v283->low_rel = cur_cur_coin;
//                        } else {
//                            v361 = v283;
//                            if(v283) v588 = curve_curve_int::`scalar deleting destructor'(v361, 1u); else v588 = 0i64;
//                        }
//                    }
//                } else if(same_sense) {
//                    v528 = ell1->param;
//                    v527 = SpaAcis::NullObj::get_parameter();
//                    v75 = v528(ell1, &p2s, v527);
//                    SPAparameter::SPAparameter(&v265, v75);
//                    if(operator<(&v265, par1e) && operator>(&v265, par1s) || (v76 = ell1->param_period(ell1), v77 = operator+(&v265, v76), par1e > v77) && (v78 = ell1->param_period(ell1), v79 = operator+(&v265, v78), v79 > par1s)) {
//                        v530 = ell2->param;
//                        v529 = SpaAcis::NullObj::get_parameter();
//                        v80 = v530(ell2, &p1e, v529);
//                        SPAparameter::SPAparameter(&v292, v80);
//                        if(operator<(&v265, par1s)) {
//                            v81 = ell1->param_period(ell1);
//                            SPAparameter::operator+=(&v265, &v785, v81);
//                        }
//                        if(operator<(&v292, par2s)) {
//                            v82 = ell2->param_period(ell2);
//                            SPAparameter::operator+=(&v292, &v786, v82);
//                        }
//                        v331 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3519, &alloc_file_index_2127);
//                        if(v331) {
//                            v531 = SpaAcis::NullObj::get_par_pos();
//                            v532 = SPAparameter::operator double(&v292);
//                            curve_curve_int::curve_curve_int(v331, 0i64, &p1e, par1e, v532, v531);
//                            v332 = v83;
//                        } else {
//                            v332 = 0i64;
//                        }
//                        v533 = v332;
//                        v337 = v332;
//                        v333 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3520, &alloc_file_index_2127);
//                        if(v333) {
//                            v534 = SpaAcis::NullObj::get_par_pos();
//                            v535 = par2s;
//                            v536 = SPAparameter::operator double(&v265);
//                            curve_curve_int::curve_curve_int(v333, v337, &p2s, v536, v535, v534);
//                            v334 = v84;
//                        } else {
//                            v334 = 0i64;
//                        }
//                        v537 = v334;
//                        v293 = v334;
//                        v335 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3521, &alloc_file_index_2127);
//                        if(v335) {
//                            v538 = SpaAcis::NullObj::get_par_pos();
//                            v539 = par2s;
//                            v540 = par1s;
//                            curve_curve_int::curve_curve_int(v335, v293, &p1s, par1s, par2s, v538);
//                            v336 = v85;
//                        } else {
//                            v336 = 0i64;
//                        }
//                        v541 = v336;
//                        cci = (curve_curve_int*)v336;
//                        *v336 = 2;
//                        v293->low_rel = cur_cur_tangent;
//                        v293->high_rel = cur_cur_coin;
//                        v337->low_rel = cur_cur_coin;
//                    } else {
//                        v338 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3529, &alloc_file_index_2127);
//                        if(v338) {
//                            v542 = SpaAcis::NullObj::get_par_pos();
//                            v543 = par1s;
//                            curve_curve_int::curve_curve_int(v338, 0i64, &p1s, par1s, par2e, v542);
//                            v339 = v86;
//                        } else {
//                            v339 = 0i64;
//                        }
//                        v544 = v339;
//                        cci = (curve_curve_int*)v339;
//                        *v339 = 2;
//                    }
//                } else {
//                    v288 = 0i64;
//                    v546 = ell1->param;
//                    v545 = SpaAcis::NullObj::get_parameter();
//                    v87 = v546(ell1, &p2s, v545);
//                    SPAparameter::SPAparameter(&v266, v87);
//                    v548 = ell2->param;
//                    v547 = SpaAcis::NullObj::get_parameter();
//                    v88 = v548(ell2, &p1e, v547);
//                    SPAparameter::SPAparameter(&v267, v88);
//                    if(operator<(&v266, par1e) && operator>(&v266, par1s) || (v89 = ell1->param_period(ell1), v90 = operator+(&v266, v89), par1e > v90) && (v91 = ell1->param_period(ell1), v92 = operator+(&v266, v91), v92 > par1s)) {
//                        if(operator<(&v266, par1s)) {
//                            v93 = ell1->param_period(ell1);
//                            SPAparameter::operator+=(&v266, &v787, v93);
//                        }
//                        v340 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3547, &alloc_file_index_2127);
//                        if(v340) {
//                            v549 = SpaAcis::NullObj::get_par_pos();
//                            v550 = par2s;
//                            v551 = SPAparameter::operator double(&v266);
//                            curve_curve_int::curve_curve_int(v340, 0i64, &p2s, v551, v550, v549);
//                            v341 = v94;
//                        } else {
//                            v341 = 0i64;
//                        }
//                        v552 = v341;
//                        v288 = v341;
//                    } else if(operator<(&v267, par2e) && operator>(&v267, par2s) || (v95 = ell2->param_period(ell2), v96 = operator+(&v267, v95), par2e > v96) && (v97 = ell2->param_period(ell2), v98 = operator+(&v267, v97), v98 > par2s)) {
//                        if(operator>(&v267, par2e)) {
//                            v99 = ell2->param_period(ell2);
//                            SPAparameter::operator-=(&v267, &v788, v99);
//                        }
//                        v342 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3554, &alloc_file_index_2127);
//                        if(v342) {
//                            v553 = SpaAcis::NullObj::get_par_pos();
//                            v554 = SPAparameter::operator double(&v267);
//                            curve_curve_int::curve_curve_int(v342, 0i64, &p1e, par1e, v554, v553);
//                            v343 = v100;
//                        } else {
//                            v343 = 0i64;
//                        }
//                        v555 = v343;
//                        v288 = v343;
//                    }
//                    v344 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3557, &alloc_file_index_2127);
//                    if(v344) {
//                        v556 = SpaAcis::NullObj::get_par_pos();
//                        v557 = par1s;
//                        curve_curve_int::curve_curve_int(v344, v288, &p1s, par1s, par2e, v556);
//                        v345 = v101;
//                    } else {
//                        v345 = 0i64;
//                    }
//                    v558 = v345;
//                    cci = (curve_curve_int*)v345;
//                    *v345 = 3;
//                    v288->low_rel = cur_cur_coin;
//                }
//            } else if(same_sense) {
//                n = 0i64;
//                v497 = ell1->param;
//                v496 = SpaAcis::NullObj::get_parameter();
//                v46 = v497(ell1, &p2s, v496);
//                SPAparameter::SPAparameter(&v275, v46);
//                v499 = ell2->param;
//                v498 = SpaAcis::NullObj::get_parameter();
//                v47 = v499(ell2, &p1s, v498);
//                SPAparameter::SPAparameter(&v263, v47);
//                v315 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3433, &alloc_file_index_2127);
//                if(v315) {
//                    v500 = SpaAcis::NullObj::get_par_pos();
//                    curve_curve_int::curve_curve_int(v315, 0i64, &p1e, par1e, par2e, v500);
//                    v316 = v48;
//                } else {
//                    v316 = 0i64;
//                }
//                v501 = v316;
//                n = v316;
//                v49 = safe_function_type<double>::operator double(&SPAresmch);
//                if(operator<(&v275, par1e + v49) && operator>(&v275, par1s) || (v50 = ell1->param_period(ell1), v51 = operator+(&v275, v50), par1e > v51) && (v52 = ell1->param_period(ell1), v53 = operator+(&v275, v52), v53 > par1s)) {
//                    if(operator>(&v275, par1e)) {
//                        v54 = ell1->param_period(ell1);
//                        SPAparameter::operator-=(&v275, &v781, v54);
//                    }
//                    v317 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3439, &alloc_file_index_2127);
//                    if(v317) {
//                        v502 = SpaAcis::NullObj::get_par_pos();
//                        v503 = par2s;
//                        v504 = SPAparameter::operator double(&v275);
//                        curve_curve_int::curve_curve_int(v317, n, &p2s, v504, v503, v502);
//                        v318 = (curve_curve_int*)v55;
//                    } else {
//                        v318 = 0i64;
//                    }
//                    v505 = v318;
//                    cci = v318;
//                } else {
//                    v56 = safe_function_type<double>::operator double(&SPAresmch);
//                    if(operator<(&v263, par2e + v56) && operator>(&v263, par2s) || (v57 = ell2->param_period(ell2), v58 = operator+(&v263, v57), par2e > v58) && (v59 = ell2->param_period(ell2), v60 = operator+(&v263, v59), v60 > par2s)) {
//                        if(operator>(&v263, par2e)) {
//                            v61 = ell2->param_period(ell2);
//                            SPAparameter::operator-=(&v263, &v782, v61);
//                        }
//                        v319 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3446, &alloc_file_index_2127);
//                        if(v319) {
//                            v506 = SpaAcis::NullObj::get_par_pos();
//                            v507 = SPAparameter::operator double(&v263);
//                            v508 = par1s;
//                            curve_curve_int::curve_curve_int(v319, n, &p1s, par1s, v507, v506);
//                            v320 = (curve_curve_int*)v62;
//                        } else {
//                            v320 = 0i64;
//                        }
//                        v509 = v320;
//                        cci = v320;
//                    }
//                }
//                if(cci) {
//                    cci->high_rel = cur_cur_coin;
//                    n->low_rel = cur_cur_coin;
//                } else {
//                    v321 = n;
//                    if(n) v510 = curve_curve_int::`scalar deleting destructor'(v321, 1u); else v510 = 0i64;
//                }
//            } else {
//                v512 = ell1->param;
//                v511 = SpaAcis::NullObj::get_parameter();
//                v63 = v512(ell1, &p2s, v511);
//                SPAparameter::SPAparameter(&v264, v63);
//                if(operator<(&v264, par1e) && operator>(&v264, par1s) || (v64 = ell1->param_period(ell1), v65 = operator+(&v264, v64), par1e > v65) && (v66 = ell1->param_period(ell1), v67 = operator+(&v264, v66), v67 > par1s)) {
//                    v514 = ell2->param;
//                    v513 = SpaAcis::NullObj::get_parameter();
//                    v68 = v514(ell2, &p1s, v513);
//                    SPAparameter::SPAparameter(&v298, v68);
//                    if(operator>(&v264, par1e)) {
//                        v69 = ell1->param_period(ell1);
//                        SPAparameter::operator-=(&v264, &v783, v69);
//                    }
//                    if(operator<(&v298, par2s)) {
//                        v70 = ell2->param_period(ell2);
//                        SPAparameter::operator+=(&v298, &v784, v70);
//                    }
//                    v322 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3477, &alloc_file_index_2127);
//                    if(v322) {
//                        v515 = SpaAcis::NullObj::get_par_pos();
//                        curve_curve_int::curve_curve_int(v322, 0i64, &p1e, par1e, par2e, v515);
//                        v323 = v71;
//                    } else {
//                        v323 = 0i64;
//                    }
//                    v516 = v323;
//                    v328 = v323;
//                    v324 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3478, &alloc_file_index_2127);
//                    if(v324) {
//                        v517 = SpaAcis::NullObj::get_par_pos();
//                        v518 = par2s;
//                        v519 = SPAparameter::operator double(&v264);
//                        curve_curve_int::curve_curve_int(v324, v328, &p2s, v519, v518, v517);
//                        v325 = v72;
//                    } else {
//                        v325 = 0i64;
//                    }
//                    v520 = v325;
//                    v291 = v325;
//                    v326 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3479, &alloc_file_index_2127);
//                    if(v326) {
//                        v521 = SpaAcis::NullObj::get_par_pos();
//                        v522 = SPAparameter::operator double(&v298);
//                        v523 = par1s;
//                        curve_curve_int::curve_curve_int(v326, v291, &p1s, par1s, v522, v521);
//                        v327 = v73;
//                    } else {
//                        v327 = 0i64;
//                    }
//                    v524 = v327;
//                    cci = (curve_curve_int*)v327;
//                    *v327 = 3;
//                    v291->low_rel = cur_cur_coin;
//                    v291->high_rel = cur_cur_tangent;
//                    v328->low_rel = cur_cur_tangent;
//                } else {
//                    v329 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3488, &alloc_file_index_2127);
//                    if(v329) {
//                        v525 = SpaAcis::NullObj::get_par_pos();
//                        curve_curve_int::curve_curve_int(v329, 0i64, &p1e, par1e, par2e, v525);
//                        v330 = v74;
//                    } else {
//                        v330 = 0i64;
//                    }
//                    v526 = v330;
//                    cci = (curve_curve_int*)v330;
//                    *(_DWORD*)(v330 + 32) = 2;
//                }
//            }
//        } else if(same_sense) {
//            cci_next = 0i64;
//            v465 = ell1->param;
//            v464 = SpaAcis::NullObj::get_parameter();
//            v15 = v465(ell1, &p2e, v464);
//            SPAparameter::SPAparameter(&param1, v15);
//            v467 = ell2->param;
//            v466 = SpaAcis::NullObj::get_parameter();
//            v16 = v467(ell2, &p1e, v466);
//            SPAparameter::SPAparameter(&param2, v16);
//            if(operator<(&param1, par1e) && (v17 = safe_function_type<double>::operator double(&SPAresmch), v18 = operator+(&param1, v17), v18 > par1s) ||
//               (v19 = ell1->param_period(ell1), v20 = operator+(&param1, v19), par1e > v20) && (v21 = ell1->param_period(ell1), v22 = operator+(&param1, v21), v22 > par1s)) {
//                if(operator<(&param1, par1s)) {
//                    v23 = ell1->param_period(ell1);
//                    SPAparameter::operator+=(&param1, &result, v23);
//                }
//                v405 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3372, &alloc_file_index_2127);
//                if(v405) {
//                    _uv = SpaAcis::NullObj::get_par_pos();
//                    d1 = SPAparameter::operator double(&param1);
//                    curve_curve_int::curve_curve_int(v405, 0i64, &p2e, d1, par2e, _uv);
//                    v456 = v24;
//                } else {
//                    v456 = 0i64;
//                }
//                v470 = v456;
//                cci_next = v456;
//            } else if(operator<(&param2, par2e) && (v25 = safe_function_type<double>::operator double(&SPAresmch), v26 = operator+(&param2, v25), v26 > par2s) ||
//                      (v27 = ell2->param_period(ell2), v28 = operator+(&param2, v27), par2e > v28) && (v29 = ell2->param_period(ell2), v30 = operator+(&param2, v29), v30 > par2s)) {
//                if(operator<(&param2, par2s)) {
//                    v31 = ell2->param_period(ell2);
//                    SPAparameter::operator+=(&param2, &v828, v31);
//                }
//                v457 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3378, &alloc_file_index_2127);
//                if(v457) {
//                    v471 = SpaAcis::NullObj::get_par_pos();
//                    d2 = SPAparameter::operator double(&param2);
//                    curve_curve_int::curve_curve_int(v457, 0i64, &p1e, par1e, d2, v471);
//                    v458 = v32;
//                } else {
//                    v458 = 0i64;
//                }
//                v473 = v458;
//                cci_next = v458;
//            }
//            v304 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3380, &alloc_file_index_2127);
//            if(v304) {
//                v474 = SpaAcis::NullObj::get_par_pos();
//                v475 = par2s;
//                v476 = par1s;
//                curve_curve_int::curve_curve_int(v304, cci_next, &p1s, par1s, par2s, v474);
//                v305 = v33;
//            } else {
//                v305 = 0i64;
//            }
//            v477 = v305;
//            cci = (curve_curve_int*)v305;
//            *v305 = 3;
//            cci_next->low_rel = cur_cur_coin;
//        } else {
//            v479 = ell1->param;
//            v478 = SpaAcis::NullObj::get_parameter();
//            v34 = v479(ell1, &p2e, v478);
//            SPAparameter::SPAparameter(&p, v34);
//            if(operator<(&p, par1e) && operator>(&p, par1s) || (v35 = ell1->param_period(ell1), v36 = operator+(&p, v35), par1e > v36) && (v37 = ell1->param_period(ell1), v38 = operator+(&p, v37), v38 > par1s)) {
//                if(operator<(&p, par1s)) {
//                    v39 = ell1->param_period(ell1);
//                    SPAparameter::operator+=(&p, &v829, v39);
//                }
//                v481 = ell2->param;
//                v480 = SpaAcis::NullObj::get_parameter();
//                v40 = v481(ell2, &p1e, v480);
//                SPAparameter::SPAparameter(&v289, v40);
//                if(operator>(&v289, par2e)) {
//                    v41 = ell2->param_period(ell2);
//                    SPAparameter::operator-=(&v289, &v830, v41);
//                }
//                v306 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3404, &alloc_file_index_2127);
//                if(v306) {
//                    v482 = SpaAcis::NullObj::get_par_pos();
//                    v483 = SPAparameter::operator double(&v289);
//                    curve_curve_int::curve_curve_int(v306, 0i64, &p1e, par1e, v483, v482);
//                    v307 = v42;
//                } else {
//                    v307 = 0i64;
//                }
//                v484 = v307;
//                cci3 = v307;
//                v308 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3405, &alloc_file_index_2127);
//                if(v308) {
//                    v485 = SpaAcis::NullObj::get_par_pos();
//                    v486 = SPAparameter::operator double(&p);
//                    curve_curve_int::curve_curve_int(v308, cci3, &p2e, v486, par2e, v485);
//                    v309 = v43;
//                } else {
//                    v309 = 0i64;
//                }
//                v487 = v309;
//                cci2 = v309;
//                v310 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3406, &alloc_file_index_2127);
//                if(v310) {
//                    v488 = SpaAcis::NullObj::get_par_pos();
//                    v489 = par2s;
//                    v490 = par1s;
//                    curve_curve_int::curve_curve_int(v310, cci2, &p1s, par1s, par2s, v488);
//                    v311 = v44;
//                } else {
//                    v311 = 0i64;
//                }
//                v491 = v311;
//                cci = (curve_curve_int*)v311;
//                *v311 = 2;
//                cci2->low_rel = cur_cur_tangent;
//                cci2->high_rel = cur_cur_coin;
//                cci3->low_rel = cur_cur_coin;
//            } else {
//                v313 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 3416, &alloc_file_index_2127);
//                if(v313) {
//                    v492 = SpaAcis::NullObj::get_par_pos();
//                    v493 = par2s;
//                    v494 = par1s;
//                    curve_curve_int::curve_curve_int(v313, 0i64, &p1s, par1s, par2s, v492);
//                    v314 = v45;
//                } else {
//                    v314 = 0i64;
//                }
//                v495 = v314;
//                cci = (curve_curve_int*)v314;
//                *v314 = 2;
//            }
//        }
//    }
//    return cci;
//}