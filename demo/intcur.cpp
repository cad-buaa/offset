#include "intcur.h"

#include "acis/acismath.h"
#include "acis/boolapi.hxx"
#include "acis/bs3c_def.hxx"
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
#include "acis/sp3crtn.hxx"
#include "acis/sps3crtn.hxx"

outcome aei_INTCUR_1(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt) {
}

SPAunit_vector gme_offset_int_cur::point_direction(SPAposition& point, SPAparameter& param_guess) {  // [rsp+118h] [rbp-30h] BYRE
    bs3_curve_def* cur = this->cur();
    double __formal = this->fitol();
    double v4 = bs3_curve_invert(point, __formal, cur, param_guess);
    SPAparameter new_guess(v4);
    SPAunit_vector result;
    SPAparameter extParam = new_guess.operator double();
    if(param_guess && !SpaAcis::NullObj::check_parameter(param_guess)) extParam = param_guess.operator double();
    ofstintcur_linear_extender* v14 = this->mStartExtender;
    ofstintcur_linear_extender* v17 = this->mEndExtender;
    if(this->mStartExtender && v14->inside_extension(extParam.operator double())) {
        ofstintcur_linear_extender* mStartExtender = this->mStartExtender;
        result = mStartExtender->get_direction();
    } else if(this->mEndExtender && v17->inside_extension(extParam.operator double())) {
        ofstintcur_linear_extender* mEndExtender = this->mEndExtender;
        result = mEndExtender->get_direction();
    } else {
        SPAparameter actual_param;
        curve* orig_curve = this->orig_curve;
        SPAposition position = SpaAcis::NullObj::get_position();
        SPAunit_vector unit_vector = SpaAcis::NullObj::get_unit_vector();
        SPAvector vector = SpaAcis::NullObj::get_vector();
        this->orig_curve->point_perp(point, position, unit_vector, vector, new_guess, actual_param, 0);

        const SPAvector v6 = this->eval_deriv(actual_param.operator double());
        result = normalise(v6);
    }
    // if(curve_module_header.debug_level >= 0x1E) {
    //     acis_fprintf(debug_file_ptr, "point_direction returns : ");
    //     SPAvector::debug(result, debug_file_ptr);
    //     debug_newline(debug_file_ptr);
    // }
    return result;
}

double bs3_curve_invert(SPAposition& pos, double __formal, bs3_curve_def* cur, SPAparameter& param_guess) {
    SPAposition foot;
    SPAparameter param_actual;
    SPAunit_vector unit_vector = SpaAcis::NullObj::get_unit_vector();
    bs3_curve_perp(pos, cur, foot, unit_vector, param_guess, param_actual, 0.0);
    return param_actual.operator double();
}

void bs3_curve_perp(SPAposition& pos, bs3_curve_def* cur, SPAposition& foot, SPAunit_vector& tangent, SPAparameter& param_guess, SPAparameter& param_actual, double quick_exit_dist_tol) {
    int v7;                           // eax
    int v8;                           // eax
    long double started;              // xmm0_8
    long double v10;                  // xmm0_8
    *v11;                             // rax
    long double v12;                  // xmm0_8
    SPAvector* v13;                   // rax
    long double v14;                  // xmm0_8
    ag_spline* v15;                   // rax
    SPAvector* v16;                   // rax
    SPAvector* v17;                   // rax
    double v18;                       // xmm0_8
    long double v19;                  // xmm0_8
    const SPAvector* v20;             // rax
    const SPAvector* v21;             // rax
    long double v22;                  // xmm0_8
    long double v23;                  // xmm0_8
    long double v24;                  // xmm0_8
    long double v25;                  // xmm0_8
    long double v26;                  // xmm0_8
    long double v27;                  // xmm0_8
    AcisVersion* v28;                 // rax
    long double v29;                  // xmm0_8
    long double v30;                  // xmm0_8
    double v31;                       // xmm0_8
    long double param;                // [rsp+30h] [rbp-318h] BYREF
    int i;                            // [rsp+38h] [rbp-310h]
    long double delta;                // [rsp+40h] [rbp-308h]
    long double curv_factor;          // [rsp+48h] [rbp-300h]
    long double new_param;            // [rsp+50h] [rbp-2F8h]
    long double term_tol;             // [rsp+58h] [rbp-2F0h]
    long double last_error;           // [rsp+60h] [rbp-2E8h]
    int v39;                          // [rsp+68h] [rbp-2E0h]
    int v40;                          // [rsp+6Ch] [rbp-2DCh]
    long double last_delta;           // [rsp+70h] [rbp-2D8h]
    long double error;                // [rsp+78h] [rbp-2D0h]
    long double test_param;           // [rsp+80h] [rbp-2C8h] BYREF
    long double small_norm_factor;    // [rsp+88h] [rbp-2C0h]
    SPAinterval curve_range;          // [rsp+90h] [rbp-2B8h] BYREF
    int ERR;                          // [rsp+A8h] [rbp-2A0h] BYREF
    int no_iteration;                 // [rsp+ACh] [rbp-29Ch]
    int curve_periodic;               // [rsp+B0h] [rbp-298h]
    long double test_error;           // [rsp+B8h] [rbp-290h]
    long double norm_factor;          // [rsp+C0h] [rbp-288h]
    long double new_delta;            // [rsp+C8h] [rbp-280h]
    long double relaxed_tol;          // [rsp+D0h] [rbp-278h]
    SPAvector foot_dt;                // [rsp+D8h] [rbp-270h] BYREF
    SPAposition foot_current;         // [rsp+F0h] [rbp-258h] BYREF
    long double value;                // [rsp+108h] [rbp-240h]
    double v56;                       // [rsp+110h] [rbp-238h]
    double v57;                       // [rsp+118h] [rbp-230h]
    double v58;                       // [rsp+120h] [rbp-228h]
    double v59;                       // [rsp+128h] [rbp-220h]
    double v60;                       // [rsp+130h] [rbp-218h]
    AcisVersion* vt2;                 // [rsp+138h] [rbp-210h]
    AcisVersion* vt1;                 // [rsp+140h] [rbp-208h]
    long double the_guess;            // [rsp+148h] [rbp-200h]
    SPAparameter v64;                 // [rsp+150h] [rbp-1F8h] BYREF
    SPAvector* xdotdot;               // [rsp+158h] [rbp-1F0h]
    long double v66;                  // [rsp+160h] [rbp-1E8h]
    double v67;                       // [rsp+168h] [rbp-1E0h]
    double v68;                       // [rsp+170h] [rbp-1D8h]
    double v69;                       // [rsp+178h] [rbp-1D0h]
    SPAparameter v70;                 // [rsp+180h] [rbp-1C8h] BYREF
    AcisVersion v71;                  // [rsp+188h] [rbp-1C0h] BYREF
    AcisVersion v72;                  // [rsp+18Ch] [rbp-1BCh] BYREF
    _iobuf* fp;                       // [rsp+190h] [rbp-1B8h]
    SPAvector foot_ddt;               // [rsp+198h] [rbp-1B0h] BYREF
    SPAunit_vector tan_current;       // [rsp+1B0h] [rbp-198h] BYREF
    SPAvector offset;                 // [rsp+1C8h] [rbp-180h] BYREF
    SPAposition test_foot_current;    // [rsp+1E0h] [rbp-168h] BYREF
    SPAposition start_pos;            // [rsp+1F8h] [rbp-150h] BYREF
    SPAposition end_pos;              // [rsp+210h] [rbp-138h] BYREF
    SPAvector test_foot_dt;           // [rsp+228h] [rbp-120h] BYREF
    SPAunit_vector test_tan_current;  // [rsp+240h] [rbp-108h] BYREF
    ag_vec p1;                        // [rsp+258h] [rbp-F0h] BYREF
    SPAvector v1;                     // [rsp+270h] [rbp-D8h] BYREF
    SPAvector result;                 // [rsp+288h] [rbp-C0h] BYREF
    SPAvector v85;                    // [rsp+2A0h] [rbp-A8h] BYREF
    SPAvector v86;                    // [rsp+2B8h] [rbp-90h] BYREF
    SPAvector v87;                    // [rsp+2D0h] [rbp-78h] BYREF
    SPAunit_vector v88;               // [rsp+2E8h] [rbp-60h] BYREF
    SPAvector v89;                    // [rsp+300h] [rbp-48h] BYREF
    SPAvector v90;                    // [rsp+318h] [rbp-30h] BYREF

    if(cur) {
        if(bs3_curve_degree(cur) <= 25) {
            curve_range = bs3_curve_range(cur);
            curve_periodic = bs3_curve_periodic(cur);
            int no_iteration = 0;
            /*  if(bs3_curve_module_header.debug_level >= 0x1E)
              {
                  acis_fprintf(debug_file_ptr, "Find perpendicular from ");
                  SPAposition::debug(pos, debug_file_ptr);
                  acis_fprintf(debug_file_ptr, " to spline\n");
                  if(param_guess && !SpaAcis::NullObj::check_parameter(param_guess)) {
                      acis_fprintf(debug_file_ptr, "initial guess ");
                      fp = debug_file_ptr;
                      value = SPAparameter::operator double(param_guess);
                      debug_real(value, fp);
                      debug_newline(debug_file_ptr);
                  } else {
                      acis_fprintf(debug_file_ptr, "no initial guess\n");
                  }
                  if(bs3_curve_module_header.debug_level >= 0x23) {
                      acis_fprintf(debug_file_ptr, "Curve details\n\t");
                      bs3_curve_debug(cur, "\t", debug_file_ptr);
                      debug_newline(debug_file_ptr);
                  }
              }*/
            if(!param_guess || SpaAcis::NullObj::check_parameter(param_guess)) {
                double started = curve_range.start_pt();
                SPAposition start_pos = bs3_curve_position(started, cur);
                double v10 = curve_range.end_pt();
                SPAposition end_pos = bs3_curve_position(v10, cur);
                SPAvector v11 = pos - start_pos;
                double v57 = v11.len_sq();
                if(SPAresabs * SPAresabs <= v57) {
                    SPAvector v13 = pos - end_pos;
                    double v59 = v13.len_sq();
                    if(SPAresabs * SPAresabs <= v59) {
                        int ERR = 0;
                        ag_vec p1(pos);
                        v15 = cur->get_cur();
                        if(ag_cls_pt_bs(p1.P, &param, &ERR)) {
                            if(bs3_curve_module_header.debug_level >= 0x1E) {
                                acis_fprintf(debug_file_ptr, "\tresult from ag_cls_pt_bs ");
                                debug_real(param, debug_file_ptr);
                                debug_newline(debug_file_ptr);
                            }
                        } else {
                            v16 = operator-(&v86, pos, &start_pos);
                            v60 = SPAvector::len_sq(v16);
                            v17 = operator-(&v87, pos, &end_pos);
                            v18 = SPAvector::len_sq(v17);
                            if(v18 < v60)
                                param = SPAinterval::end_pt(&curve_range);
                            else
                                param = SPAinterval::start_pt(&curve_range);
                            if(bs3_curve_module_header.debug_level >= 0x1E) {
                                acis_fprintf(debug_file_ptr, "\tno result from ag_cls_pt_bs; use ");
                                debug_real(param, debug_file_ptr);
                                debug_newline(debug_file_ptr);
                            }
                        }
                    } else {
                        param = SPAinterval::end_pt(&curve_range);
                    }
                } else {
                    param = SPAinterval::start_pt(&curve_range);
                }
            } else {
                param = SPAparameter::operator double(param_guess);
            }
            the_guess = param;
            term_tol = 0.001 * safe_function_type<double>::operator double(&SPAresabs);
            if(quick_exit_dist_tol >= 0.0) {
                if(quick_exit_dist_tol != 0.0) quick_exit_dist_tol = quick_exit_dist_tol * quick_exit_dist_tol;
            } else {
                quick_exit_dist_tol = 0.0;
            }
            SPAposition::SPAposition(&foot_current, 0.0, 0.0, 0.0);
            SPAunit_vector::SPAunit_vector(&tan_current, 0.0, 0.0, 0.0);
            last_delta = 0.0;
            last_error = 1.0 / safe_function_type<double>::operator double(&SPAresnor);
            for(i = 0; i < 20; ++i) {
                SPAvector::SPAvector(&foot_dt);
                SPAvector::SPAvector(&foot_ddt);
                bs3_curve_eval(param, cur, &foot_current, &foot_dt, &foot_ddt);
                if(bs3_curve_module_header.debug_level >= 0x1E) {
                    acis_fprintf(debug_file_ptr, "\tfoot (");
                    SPAposition::debug(&foot_current, debug_file_ptr);
                    acis_fprintf(debug_file_ptr, "), param (");
                    debug_real(param, debug_file_ptr);
                    acis_fprintf(debug_file_ptr, ")\n");
                    acis_fprintf(debug_file_ptr, "\tfirst derivative ");
                    SPAvector::debug(&foot_dt, debug_file_ptr);
                    debug_newline(debug_file_ptr);
                    if(bs3_curve_module_header.debug_level >= 0x23) {
                        acis_fprintf(debug_file_ptr, "\tsecond derivative ");
                        SPAvector::debug(&foot_ddt, debug_file_ptr);
                        debug_newline(debug_file_ptr);
                    }
                }
                operator-(&offset, pos, &foot_current);
                if(quick_exit_dist_tol != 0.0 && quick_exit_dist_tol > SPAvector::len_sq(&offset)) {
                    if(bs3_curve_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "\tdistance tol met - successful");
                    break;
                }
                qmemcpy(&tan_current, normalise(&v88, &foot_dt), sizeof(tan_current));
                error = operator%(&offset, &tan_current);
                if(no_iteration || term_tol > error && error > COERCE_DOUBLE(*(_QWORD*)&term_tol ^ _xmm)) {
                    if(bs3_curve_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "\tsuccessful");
                    break;
                }
                norm_factor = operator%(&foot_dt, &foot_dt);
                v19 = operator%(&foot_ddt, &foot_dt);
                v20 = operator*(&v89, v19, &foot_dt);
                v21 = operator/(&v90, v20, norm_factor);
                SPAvector::operator-=(&foot_ddt, v21);
                v22 = operator%(&offset, &foot_ddt);
                curv_factor = norm_factor - v22;
                small_norm_factor = norm_factor * 0.1;
                if(norm_factor * 0.1 > norm_factor - v22 && curv_factor > COERCE_DOUBLE(*(_QWORD*)&small_norm_factor ^ _xmm)) {
                    if(curv_factor >= 0.0)
                        curv_factor = small_norm_factor;
                    else
                        *(_QWORD*)&curv_factor = *(_QWORD*)&small_norm_factor ^ _xmm;
                }
                v23 = operator%(&offset, &foot_dt);
                delta = v23 / curv_factor;
                if(i <= 0 || (v40 = delta < 0.0, v39 = last_delta < 0.0, v40 == v39)) {
                    if(error * error > last_error * last_error) {
                        if(bs3_curve_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "****bs3_curve_perp diverging: reset step size!\n");
                        delta = 0.5 * last_delta;
                        param = param - last_delta;
                    }
                } else {
                    new_delta = -0.5 * last_delta;
                    if(delta > 0.0 && delta > new_delta || delta < 0.0 && new_delta > delta) {
                        if(bs3_curve_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "****bs3_curve_perp oscillating: reset step size!\n");
                        delta = new_delta;
                    }
                }
                new_param = param + delta;
                if(!curve_periodic) {
                    v24 = SPAinterval::start_pt(&curve_range);
                    if(v24 <= new_param) {
                        v26 = SPAinterval::end_pt(&curve_range);
                        if(new_param > v26) {
                            v27 = SPAinterval::end_pt(&curve_range);
                            if(param == v27) {
                                if(bs3_curve_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "\ttop param limit");
                                break;
                            }
                            new_param = SPAinterval::end_pt(&curve_range);
                        }
                    } else {
                        v25 = SPAinterval::start_pt(&curve_range);
                        if(param == v25) {
                            if(bs3_curve_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "\tbottom param limit");
                            break;
                        }
                        new_param = SPAinterval::start_pt(&curve_range);
                    }
                }
                last_delta = new_param - param;
                param = new_param;
                last_error = error;
            }
            if(i >= 20) {
                if(bs3_curve_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "***bs3_curve_perp didn't converge\n");
                relaxed_tol = 0.1 * safe_function_type<double>::operator double(&SPAresabs);
                if(last_error > relaxed_tol || COERCE_DOUBLE(*(_QWORD*)&relaxed_tol ^ _xmm) > last_error) {
                    SPAposition::SPAposition(&test_foot_current);
                    SPAvector::SPAvector(&test_foot_dt);
                    AcisVersion::AcisVersion(&v71, 23, 0, 2);
                    vt2 = v28;
                    vt1 = GET_ALGORITHMIC_VERSION(&v72);
                    if(operator>=(vt1, vt2)) {
                        SPAparameter::SPAparameter(&v64, the_guess);
                        bs3_curve_min_dist(cur, pos, term_tol, &v64, &test_param);
                    } else {
                        bs3_curve_min_dist(cur, pos, term_tol, param_guess, &test_param);
                    }
                    xdotdot = SpaAcis::NullObj::get_vector();
                    v66 = test_param;
                    bs3_curve_eval(test_param, cur, &test_foot_current, &test_foot_dt, xdotdot);
                    normalise(&test_tan_current, &test_foot_dt);
                    operator-(&v1, pos, &test_foot_current);
                    test_error = operator%(&v1, &test_tan_current);
                    if(term_tol > test_error && test_error > COERCE_DOUBLE(*(_QWORD*)&term_tol ^ _xmm)) goto LABEL_94;
                    v67 = SPAinterval::start_pt(&curve_range);
                    v29 = safe_function_type<double>::operator double(&SPAresnor);
                    if(v67 + v29 > test_param || (v68 = SPAinterval::end_pt(&curve_range), v30 = safe_function_type<double>::operator double(&SPAresnor), test_param > v68 - v30) || (v69 = fabs_0(test_error), v31 = fabs_0(last_error), v31 > v69)) {
                    LABEL_94:
                        if(bs3_curve_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "***Law attempt result better\n");
                        param = test_param;
                        qmemcpy(&foot_current, &test_foot_current, sizeof(foot_current));
                        qmemcpy(&tan_current, &test_tan_current, sizeof(tan_current));
                    }
                }
            }
            if(bs3_curve_module_header.debug_level >= 0x1E) {
                acis_fprintf(debug_file_ptr, ", returning param ");
                debug_real(param, debug_file_ptr);
                acis_fprintf(debug_file_ptr, "\n\t\tfoot ");
                SPAposition::debug(&foot_current, debug_file_ptr);
                acis_fprintf(debug_file_ptr, "\n\t\ttangent ");
                SPAvector::debug(&tan_current, debug_file_ptr);
                debug_newline(debug_file_ptr);
            }
            if(foot && !SpaAcis::NullObj::check_position(foot)) qmemcpy(foot, &foot_current, sizeof(SPAposition));
            if(tangent && !SpaAcis::NullObj::check_unit_vector(tangent)) qmemcpy(tangent, &tan_current, sizeof(SPAunit_vector));
            if(param_actual) {
                if(!SpaAcis::NullObj::check_parameter(param_actual)) {
                    SPAparameter::SPAparameter(&v70, param);
                    param_actual->val = v70.val;
                }
            }
        } else {
            v8 = message_module::message_code(&spaacisds_bs3_crv_errmod, 20);
            sys_error(v8);
        }
    } else {
        v7 = message_module::message_code(&spaacisds_bs3_crv_errmod, 4);
        sys_error(v7);
    }
}

int ag_cls_pt_bs(ag_spline* bs, double* P, double* tmin, int* ERR) {
    double d2min[2];
    d2min[0] = -2.0;
    return ag_cls_bs(bs, ag_min_dist_f, nullptr, P, d2min, tmin, ERR);
}
int ag_cls_bs(ag_spline* bs, int (*f)(ag_spline*), char* par, double* P, double* d2min, double* tmin, int* ERR) {
    int num_sp;           // [rsp+40h] [rbp-178h]
    int i;                // [rsp+44h] [rbp-174h]
    int ia;               // [rsp+44h] [rbp-174h]
    int j;                // [rsp+48h] [rbp-170h]
    int ja;               // [rsp+48h] [rbp-170h]
    int dim;              // [rsp+4Ch] [rbp-16Ch]
    ag_cnode* cn0;        // [rsp+50h] [rbp-168h]
    unsigned int found;   // [rsp+5Ch] [rbp-15Ch]
    ag_cnode** cnd;       // [rsp+60h] [rbp-158h] BYREF
    long double* d2;      // [rsp+68h] [rbp-150h] BYREF
    BOOL v18;             // [rsp+70h] [rbp-148h]
    int* key;             // [rsp+78h] [rbp-140h] BYREF
    ag_cnode* cn;         // [rsp+80h] [rbp-138h]
    int m;                // [rsp+88h] [rbp-130h]
    int fnd;              // [rsp+8Ch] [rbp-12Ch]
    node_fnd;             // [rsp+90h] [rbp-128h]
    ag_mmbox box;         // [rsp+98h] [rbp-120h] BYREF
    long double v25;      // [rsp+A8h] [rbp-110h]
    ag_cnode* noden;      // [rsp+B0h] [rbp-108h]
    long double min[3];   // [rsp+B8h] [rbp-100h] BYREF
    long double max[3];   // [rsp+D0h] [rbp-E8h] BYREF
    int keys[10];         // [rsp+E8h] [rbp-D0h] BYREF
    long double d2s[10];  // [rsp+110h] [rbp-A8h] BYREF
    ag_cnode* cnds[10];   // [rsp+160h] [rbp-58h] BYREF

    ag_cnode* node_fnd = nullptr;
    int num_sp = ag_n_sp_in_bs(bs);
    m = bs->m;
    dim = bs->dim;
    box.min = min;
    box.max = max;
    found = 0;
    v18 = num_sp > 10;
    if(num_sp <= 10) {
        key = keys;
        d2 = d2s;
        cnd = cnds;
    } else {
        key = (int*)ag_al_mem(4 * num_sp);
        d2 = (long double*)ag_al_mem(8 * num_sp);
        cnd = (ag_cnode**)ag_al_mem(8 * num_sp);
    }
    cn0 = bs->node0;
    noden = bs->noden;
    for(i = 0; i < num_sp; ++i) {
        cn = cn0;
        ag_V_copy(cn0->Pw, min, dim);
        ag_V_copy(min, max, dim);
        for(j = 0; j < m; ++j) {
            cn = cn->next;
            ag_box_V_join(&box, cn->Pw, &box, dim);
        }
        cnd[i] = cn0;
        v25 = ag_box_Vld2(&box, P, dim);
        d2[i] = v25;
        for(cn0 = cn0->next; cn0 != noden && cn0->t == cn0->next->t; cn0 = cn0->next)
            ;
    }
    ag_heap_sort_dk(d2, key, num_sp, 1);
    for(ia = 0; ia < num_sp; ++ia) {
        ja = key[ia];
        if(*d2min > -1.0 && d2[ja] > *d2min) break;
        if(!option_header::on(&agnodefindcons) || thread_count() == 1 && option_header::on(&usenodeopt)) bs->node = cnd[ja];
        fnd = ag_cls_sp(bs, f, par, P, d2min, tmin, ERR, cnd[ja]);
        if(*ERR) {
            if(num_sp > 10) {
                ag_dal_mem((void**)&key, 4 * num_sp);
                ag_dal_mem((void**)&d2, 8 * num_sp);
                ag_dal_mem((void**)&cnd, 8 * num_sp);
            }
            return 0i64;
        }
        if(fnd) {
            found = 1;
            node_fnd = cnd[ja];
        }
    }
    if(num_sp > 10) {
        ag_dal_mem((void**)&key, 4 * num_sp);
        ag_dal_mem((void**)&d2, 8 * num_sp);
        ag_dal_mem((void**)&cnd, 8 * num_sp);
    }
    if(found) bs->node = node_fnd;
    return found;
}
int ag_cls_sp {
    unsigned int found;  // [rsp+40h] [rbp-38h]
    int dim;             // [rsp+44h] [rbp-34h]
    int rat;             // [rsp+48h] [rbp-30h]
    int m;               // [rsp+4Ch] [rbp-2Ch]
    ag_spline* bz;       // [rsp+50h] [rbp-28h] BYREF
    ag_spline* pow;      // [rsp+58h] [rbp-20h] BYREF
    ag_cnode* node;      // [rsp+60h] [rbp-18h]

    m = bs->m;
    dim = bs->dim;
    rat = bs->rat;
    node = curr_node;
    bz = ag_Bez_get(m, rat, 0, dim);
    if(ag_bs_sp_to_Bez(bs, bz, curr_node)) {
        if(*d2min > -1.0 && ag_box_Vld2(bz->bsbox, P, dim) > *d2min) {
            ag_Bez_ret(&bz);
            return 0i64;
        }
        pow = bz;
        ag_bs_Bez_to_pow(bz);
        bz->ctype = 102;
        found = ag_cls_pow(pow, f, par, P, 0i64, d2min, tmin);
        if(found) *tmin = *node->t + *tmin * (*node->next->t - *node->t);
        ag_Bez_ret(&pow);
    } else {
        ag_Bez_ret(&bz);
        *ERR = 1;
        return 0;
    }
    return found;
}

int ag_n_sp_in_bs(ag_spline* bs) {
    int nsp = 0;
    for(ag_cnode* node = bs->node0; node != bs->noden; node = node->next) {
        if(node->t != node->next->t) ++nsp;
    }
    return nsp;
}

int ag_min_dist_f(ag_spline* __formal) {
    return 2 ^ 32 - 1;
}

SPAinterval bs3_curve_range(bs3_curve_def* cur) {
    SPAinterval result;
    if(cur && cur->get_cur()) {
        if(cur->get_cur()->noden) {
            double d2 = *cur->get_cur()->noden->t;
            double* t = cur->get_cur()->node0->t;
            result = (*t, d2);
        } else {
            result = 0.0;
        }
        return result;
    } else {
        SPAinterval result1;
        return result1;
    }
}

SPAposition* __fastcall bs3_curve_position(SPAposition* result, long double param, bs3_curve_def* cur) {
    SPAvector* vector;   // rax
    SPAvector* xdotdot;  // [rsp+30h] [rbp-18h]

    SPAposition::SPAposition(result);
    xdotdot = SpaAcis::NullObj::get_vector();
    vector = SpaAcis::NullObj::get_vector();
    bs3_curve_eval(param, cur, result, vector, xdotdot);
    return result;
}

int bs3_curve_evaluate(double param, bs3_curve_def* cur, SPAposition& pos, const SPAvector** deriv, int nd, int index) {
    agspline_thread_ctx* v6;  // rax
    agspline_thread_ctx* v7;  // rax
    int v8;                   // eax
    int v10;                  // eax
    int v11;                  // eax
    int v12;                  // eax
    const void* v13;          // rax
    const void* v14;          // rax
    int i;                    // [rsp+30h] [rbp-B8h]
    int neval;                // [rsp+34h] [rbp-B4h]
    int j;                    // [rsp+3Ch] [rbp-ACh]
    double low;               // [rsp+40h] [rbp-A8h]
    double high;              // [rsp+48h] [rbp-A0h]
    double delta;             // [rsp+50h] [rbp-98h]
    ag_cpoint** values;       // [rsp+58h] [rbp-90h]
    int ind;                  // [rsp+60h] [rbp-88h]
    double knottol;           // [rsp+68h] [rbp-80h]
    ag_spline* bs;            // [rsp+70h] [rbp-78h]
    ag_cpoint* this_cp;       // [rsp+78h] [rbp-70h]
    int* max_deriv;           // [rsp+80h] [rbp-68h]
    SPAposition v27;          // [rsp+A0h] [rbp-48h] BYREF
    SPAvector v28;            // [rsp+B8h] [rbp-30h] BYREF

    v6 = agspline_context();
    values = &agspline_thread_ctx::c3curve(v6)->values;
    v7 = agspline_context();
    max_deriv = &agspline_thread_ctx::c3curve(v7)->max_deriv;
    if(cur) {
        if(bs3_curve_def::get_cur(cur)->noden) {
            int neval = bs3_curve_accurate_derivs(cur);
            if(neval > nd) neval = nd;
            bs = cur->get_cur();
            if(neval > *max_deriv) {
                if(*values) ag_db_cpts(values, 3);
                *values = ag_bld_cpts(nullptr, nullptr, neval + 1, 3);
                *max_deriv = neval;
            }
            if(bs3_curve_def::get_form(cur) == bs3_curve_periodic_ends) {
                low = *bs->node0->t;
                high = *bs->noden->t;
                delta = high - low;
                knottol = bs3_curve_knottol();
                if(index >= 0) {
                    if(index > 0) {
                        low = low - knottol;
                        high = high - knottol;
                    }
                } else {
                    low = low + knottol;
                    high = high + knottol;
                }
                if(delta != 0.0) {
                    if(low > param) {
                        param = param + delta;
                        if(low > param) param = param + ceil((low - param) / delta) * delta;
                    }
                    if(param >= high) {
                        param = param - delta;
                        if(param >= high) param = param - ceil((param - high) / delta) * delta;
                    }
                }
            }
            if(index) {
                if(index >= 0)
                    ind = 1;
                else
                    ind = -1;
                if(acis_eval_bs_l_or_r(param, neval, bs, *values, ind) == -1) {
                    v12 = message_module::message_code(&spaacisds_curve_errmod, 4);
                    sys_error(v12);
                }
            } else if(ag_eval_bs(param, neval, bs, *values, 0) == -1) {
                v11 = message_module::message_code(&spaacisds_curve_errmod, 4);
                sys_error(v11);
            }
            this_cp = *values;
            if(pos && !SpaAcis::NullObj::check_position(pos)) {
                SPAposition::SPAposition(&v27, this_cp->P);
                qmemcpy(pos, v13, sizeof(SPAposition));
            }
            for(i = 0; i < neval; ++i) {
                this_cp = this_cp->next;
                if(deriv[i]) {
                    if(!SpaAcis::NullObj::check_vector(deriv[i])) {
                        SPAvector::SPAvector(&v28, this_cp->P);
                        qmemcpy(deriv[i], v14, sizeof(SPAvector));
                    }
                }
            }
            while(i < nd) {
                if(deriv[i] && !SpaAcis::NullObj::check_vector(deriv[i])) qmemcpy(deriv[i], &null_vector, sizeof(SPAvector));
                ++i;
            }
            if(bs3_curve_module_header.debug_level >= 0x1E) {
                acis_fprintf(debug_file_ptr, "bs3_curve_evaluate() results at param ");
                debug_real(param, debug_file_ptr);
                acis_fprintf(debug_file_ptr, ", nd = %d side index = %d\n", (unsigned int)nd, (unsigned int)index);
                acis_fprintf(debug_file_ptr, "\tPosition: ");
                if(!pos || SpaAcis::NullObj::check_position(pos))
                    acis_fprintf(debug_file_ptr, "NULL argument");
                else
                    SPAposition::debug(pos, debug_file_ptr);
                debug_newline(debug_file_ptr);
                for(j = 0; j < nd; ++j) {
                    acis_fprintf(debug_file_ptr, "\tDeriv %d: ", (unsigned int)(j + 1));
                    if(!deriv[j] || SpaAcis::NullObj::check_vector(deriv[j]))
                        acis_fprintf(debug_file_ptr, "NULL argument");
                    else
                        SPAvector::debug(deriv[j], debug_file_ptr);
                    debug_newline(debug_file_ptr);
                }
                debug_newline(debug_file_ptr);
            }
            return (unsigned int)neval;
        } else {
            v10 = message_module::message_code(&spaacisds_curve_errmod, 4);
            sys_error(v10);
            return 0i64;
        }
    } else {
        v8 = message_module::message_code(&spaacisds_bs3_crv_errmod, 4);
        sys_error(v8);
        return 0i64;
    }
}

int ag_db_cpts(ag_cpoint** cptp, int dim) {
    ag_cpoint* cpt_prev;  // [rsp+30h] [rbp-18h]
    ag_cpoint* cpt0;
    if(!cptp) return 0;
    ag_cpoint* cpt = *cptp;
    if(!cpt) return 0;
    for(cpt0 = cpt;; cpt0 = cpt_prev) {
        cpt_prev = cpt0->prev;
        if(!cpt_prev || cpt_prev == cpt) break;
    }
    if(cpt_prev) {
        cpt_prev->next = nullptr;
        cpt0->prev = nullptr;
    }
    do {
        cpt = cpt0;
        cpt0 = cpt0->next;
        ag_dal_dbl(&cpt->P, dim);
        ag_dal_mem((void**)&cpt, 0x18u);
    } while(cpt0);
    *cptp = 0i64;
    return 0i64;
}

int ag_dal_dbl(long double** dbl, int n) {
    if(dbl && *dbl && n > 0) ag_dal_mem((void**)dbl, 8 * n);
    return 0i64;
}