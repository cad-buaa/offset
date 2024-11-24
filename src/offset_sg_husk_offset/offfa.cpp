#include "ProtectedInterfaces/off_cu.hxx"
#include "ProtectedInterfaces/offsetsf.hxx"
#include "ProtectedInterfaces/ofsttools.hxx"
#include "ProtectedInterfaces/sgoffrtn_prot.hxx"
#include "acis/DSYWarningManager.h"
#include "acis/SPA_approx_options.hxx"
#include "acis/SPA_edge_line_arc_options.hxx"
#include "acis/acis_options.hxx"
#include "acis/acistype.hxx"
#include "acis/add_pcu.hxx"
#include "acis/box.hxx"
#include "acis/calctol.hxx"
#include "acis/check.hxx"
#include "acis/ckoutcom.hxx"
#include "acis/cnc.hxx"
#include "acis/condef.hxx"
#include "acis/cone.hxx"
#include "acis/copyent.hxx"
#include "acis/cucuint.hxx"
#include "acis/cur.hxx"
#include "acis/curdef.hxx"
#include "acis/curve.hxx"
#include "acis/elem1.hxx"
#include "acis/err_info.hxx"
#include "acis/errmsg.hxx"
#include "acis/exchg_funcs.hxx"
#include "acis/exct_int.hxx"
#include "acis/faceutil.hxx"
#include "acis/gen_rendobj.hxx"
#include "acis/geometry.hxx"
#include "acis/get_top.hxx"
#include "acis/getbox.hxx"
#include "acis/importexport.h"
#include "acis/intdef.hxx"
#include "acis/interval.hxx"
#include "acis/kernopts.hxx"
#include "acis/law.hxx"
#include "acis/lists_iterator.hxx"
#include "acis/math.hxx"
#include "acis/method.hxx"
#include "acis/module.hxx"
#include "acis/param.hxx"
#include "acis/pcudef.hxx"
#include "acis/pcurve.hxx"
#include "acis/point.hxx"
#include "acis/res.hxx"
#include "acis/rm_pcu.hxx"
#include "acis/rot_spl.hxx"
#include "acis/sp2crtn.hxx"
#include "acis/spa_null_base.hxx"
#include "acis/spa_progress_info.hxx"
#include "acis/spldef.hxx"
#include "acis/sps2crtn.hxx"
#include "acis/strdef.hxx"
#include "acis/sur.hxx"
#include "acis/surextnd.hxx"
#include "acis/transfrm.hxx"
#include "acis/unitvec.hxx"
#include "acis/vlists.hxx"

class breakpoint_callback : public ACIS_OBJECT {
    enum enumMsgType { msgInfo = 0, msgWarning = 1, msgError = 2, msgStatus = 3 };
    virtual void init() = 0;
    virtual int is_debugging() = 0;
    virtual int is_enabled() = 0;
    virtual int is_above_current_level() = 0;
    virtual int get_bkp_level() = 0;
    virtual void suspend() = 0;
    virtual void output() = 0;
    virtual outcome get_rgb_default() = 0;
    virtual RenderingObject* new_render_object() = 0;
    virtual void delete_render_object() = 0;
    virtual void delete_render_objects() = 0;
    virtual int count_render_objects() = 0;
    breakpoint_callback(breakpoint_callback* __that);
    breakpoint_callback(breakpoint_callback& __that);
    breakpoint_callback();
    breakpoint_callback& operator=(breakpoint_callback* __that);
    breakpoint_callback& operator=(breakpoint_callback& __that);
    // vfptr + 0x00 [sizeof = 8]
};

int check_failed_full_surface(FACE* face_copy, surface* offset_geom, SPAbox& face_box, SPApar_box& face_pb) {
    SPAinterval face_urange = face_pb.u_range();
    SPAinterval face_vrange = face_pb.v_range();
    int failed_full_surface = 0;
    SURFACE* original_surf = face_copy->geometry();
    SPApar_box pb_old = original_surf->equation().param_range(face_box);
    SPApar_box pb_offs = offset_geom->param_range();
    SPAinterval offs_urange = pb_offs.u_range();
    SPAinterval offs_vrange = pb_offs.v_range();
    if(pb_old.u_range().length() - SPAresnor > offs_urange.length() && !offset_geom->closed_u()) {
        if(offs_urange.bounded_below()) {
            double started = offs_urange.start_pt();
            failed_full_surface = started > face_urange.start_pt() + 10.0 * SPAresnor;
        }
        if(offs_urange.bounded_above()) {
            if(face_urange.end_pt() - 10.0 * SPAresnor > offs_urange.end_pt()) {
                failed_full_surface = 1;
            }
        }
    }
    if(pb_old.v_range().length() - SPAresnor > offs_vrange.length() && !offset_geom->closed_v()) {
        if(offs_vrange.bounded_below()) {
            if(offs_vrange.start_pt() > face_vrange.start_pt() + 10.0 * SPAresnor) {
                failed_full_surface = 1;
            }
        }
        if(offs_vrange.bounded_above()) {
            if(face_vrange.end_pt() - 10.0 * SPAresnor > offs_vrange.end_pt()) {
                return 1;
            }
        }
    }
    return failed_full_surface;
}

SPApar_vec invert_vec(SPAvector& off, SPAvector* dpos, int* well_conditioned) {
    SPApar_vec result(0.0, 0.0);
    double u_comp = off % *dpos;
    double v_comp = off % *(dpos + 1);
    double dot_uu = *dpos % *dpos;
    double dot_uv = *dpos % *(dpos + 1);
    double dot_vv = *(dpos + 1) % *(dpos + 1);
    double denom = dot_uu * dot_vv - dot_uv * dot_uv;
    double du_numer = u_comp * dot_vv - v_comp * dot_uv;
    double dv_numer = v_comp * dot_uu - u_comp * dot_uv;
    // v9 = RES_significant(denom, du_numer) && RES_significant(denom, dv_numer);
    // v21 = dpos->len();
    bool zero_du = SPAresabs > dpos->len();
    // v22 = (dpos + 1)->len();
    bool zero_dv = SPAresabs > (dpos + 1)->len();
    int value = 1;
    if(!well_conditioned) {
        well_conditioned = &value;
    }
    if(zero_du && zero_dv) {
        *well_conditioned = 0;
        result = SPApar_vec(0.0, 0.0);
    } else if(zero_du) {
        *well_conditioned = 0;
        result = SPApar_vec(0.0, v_comp / dot_vv);
    } else if(zero_dv) {
        *well_conditioned = 0;
        result = SPApar_vec(u_comp / dot_uu, 0.0);
    } else {
        if(RES_significant(denom, du_numer) && RES_significant(denom, dv_numer)) {
            *well_conditioned = 1;
            result = SPApar_vec(du_numer / denom, dv_numer / denom);
        } else {
            *well_conditioned = 0;
            result = SPApar_vec(0.0, 0.0);
        }
    }
    return result;
}

int on_isoparam_curve(surface* sf, SPAposition& ctrlpt, int i, const double const_val, SPApar_vec& off, double tol) {
    double u_val, v_val;
    if(i) {
        u_val = ctrlpt.coordinate(0);
        v_val = const_val;
    } else {
        u_val = const_val;
        v_val = ctrlpt.coordinate(1);
    }
    SPApar_pos uv(u_val, v_val);
    // SPApar_pos::operator+=(&uv, off);
    uv += off;
    SPAposition pos;
    SPAvector deriv[2];
    // v6 = ctrlpt.coordinate(i);
    double diff = D3_fabs(ctrlpt.coordinate(i) - const_val);
    // v10 = deriv[i].len();
    return (SPAresnor > deriv[i].len() || tol / deriv[i].len() > diff);
}

int spline_isoparam(surface* sf, pcurve* pcur, int i, double* iso_val, double tol) {
    int result = 1;
    bs2_curve_def* bs2_cur = pcur->cur();
    if(bs2_cur) {
        SPAposition* ctrlpts = nullptr;
        double* weights = nullptr;
        double* knots = nullptr;
        int num_ctrlpts;
        int num_knots;
        int rat;
        int deg;
        int dim;
        bs2_curve_to_array(bs2_cur, dim, deg, rat, num_ctrlpts, ctrlpts, weights, num_knots, knots, 0);
        double const_val = ctrlpts->coordinate(i);
        SPApar_vec off = pcur->offset();
        for(int j = 1; j < num_ctrlpts; ++j) {
            double val = ctrlpts[j].coordinate(i);
            if(D3_fabs(val - const_val) > SPAresmch && !on_isoparam_curve(sf, ctrlpts[j], i, const_val, off, tol)) {
                result = 0;
                break;
            }
        }
        if(result && iso_val) {
            *iso_val = const_val;
        }
        if(ctrlpts) {
            // alloc_ptr = ctrlpts;
            SPAposition::operator delete[](ctrlpts);
        }
        if(weights) {
            // v24 = weights;
            ACIS_STD_TYPE_OBJECT::operator delete[](weights);
        }
        if(knots) {
            // v25 = knots;
            ACIS_STD_TYPE_OBJECT::operator delete[](knots);
        }
    } else {
        return 0;
    }
    return result;
}

bool check_if_face_is_full_surface(FACE* original_face, SURFACE* original_SUR, surface* offset_geom) {
    AcisVersion vt2 = AcisVersion(25, 0, 0);
    AcisVersion vt1 = GET_ALGORITHMIC_VERSION();
    if(vt1 <= vt2) {
        return 0;
    }
    // const surface& v5 = original_SUR->equation();
    if(!SUR_is_spline(original_SUR->equation()) || !SUR_is_spline(*offset_geom)) {
        return 0;
    }
    // const surface& v28 = original_SUR->equation();
    SPApar_box sf_pb = original_SUR->equation().param_range();
    ENTITY_LIST edges;
    get_edges(original_face, edges, PAT_CAN_CREATE);
    if(edges.iteration_count() == 4) {
        int retval = 1;
        EDGE* tmped;
        for(tmped = (EDGE*)edges.first(); tmped; tmped = (EDGE*)edges.next()) {
            COEDGE* tmpcoed = tmped->coedge();
            if(!tmpcoed->geometry()) {
                sg_add_pcurve_to_coedge((TCOEDGE*)tmpcoed, 0, bndy_unknown, 0, 1);
            }
            if(!tmpcoed->geometry()) {
                retval = 0;
                break;
            }
            double b = tmped->get_tolerance();
            double a = SPAresfit;
            double tol = D3_max(a, b);
            if(tol > tmped->length()) {
                tol = SPAresabs;
            }
            SPAunit_vector ed_tan = edge_mid_dir(tmped);
            SPAunit_vector ed_norm = edge_mid_norm(tmped, SPAtransf(), original_face);//不确
            SPAvector ed_cross = tol * (ed_tan * ed_norm);
            SPAposition pos;
            //`vector constructor iterator'(dpos, 0x18ui64, 2ui64, (void *(__fastcall *)(void *))SPAvector::SPAvector); 

            SPAvector dpos[2];
            SPApar_pos uv = original_SUR->equation().param(edge_mid_pos(tmped));
            original_SUR->equation().eval(uv, pos, dpos);
            SPApar_vec pvec = invert_vec(ed_cross, dpos,nullptr);
            // iso_val = *(double*)& parallel_get_tolerance::`vftable'[1];
            double iso_val = INFINITY;
            pcurve temp_pcurve = tmpcoed->geometry()->equation(1);
            if(spline_isoparam((surface*)(&original_SUR->equation()), &temp_pcurve, 0, &iso_val, tol)) {
                double ptol = fabs(pvec.du.operator double());
                double started = sf_pb.u_range().start_pt();
                if(fabs(iso_val - started) > ptol) {
                    if(fabs(iso_val - sf_pb.u_range().end_pt()) > ptol) {                 
                    goto LABEL_18;
                    }
                }
            } else {
                if(!spline_isoparam((surface*)(&original_SUR->equation()), &temp_pcurve, 1, &iso_val, tol)) {
                    goto LABEL_18;
                }
                if(fabs(iso_val - sf_pb.v_range().start_pt()) > fabs(pvec.dv.operator double())) {
                    if(fabs(iso_val - sf_pb.v_range().end_pt()) > fabs(pvec.dv)) {
                    LABEL_18:
                        retval = 0;
                        break;
                    }
                }
            };
        }
        return retval;
    } 
    else {
        return 0;
    }
}

FACE* sg_offset_face(FACE* original_face, double offset_distance, int& failed_full_surface, offset_options* pOffsetOptions) {
    double offset_distancea = offset_distance;
    if(!original_face) {
        return NULL;
    }
    ofst_init();
    int remake_face = 1;
    FACE* face_copy = copy_face(original_face);
    if(face_copy->sense() == REVERSED) {
        offset_distancea = -offset_distance;
    }
    CONE* original_surf = (CONE*)(face_copy->geometry());
    SPAbox face_box = get_face_box(face_copy);
    SPApar_box face_pb;
    sg_get_face_par_box(original_face, face_pb);
    SPAinterval u_range = face_pb.u_range();
    SPAinterval v_range = face_pb.v_range();
    if(!(original_surf->equation().periodic_u())) {
        SPAinterval param_range_u = original_surf->equation().param_range_u(face_box);
        u_range = param_range_u;
    }
    if(!(original_surf->equation().periodic_v())) {
        SPAinterval param_range_v = original_surf->equation().param_range_v(face_box);
        v_range = param_range_v;
    }
    SPApar_box surf_pb(u_range, v_range);
    face_pb = surf_pb;
    SPAinterval face_urange = face_pb.u_range();
    SPAinterval face_vrange = face_pb.v_range();
    if(split_periodic_splines.on() && original_surf && is_CONE(original_surf)) {
        CONE* cone_surf = original_surf;
        double u_period = original_surf->equation().param_period_u();
        double v_period = original_surf->equation().param_period_v();
        int periodic = 0;
        if(u_period > 0.0 || v_period > 0.0) {
            periodic = 1;
        }
        if(periodic && cone_surf->radius_ratio() != 1.0) {
            double u_length = face_urange.length();
            double v_length = face_vrange.length();
            // safe_function_type<double> SPAresabs;
            if(u_length > 0.0 && v_length > 0.0 && u_period > 0.0 && u_length > u_period - SPAresabs || v_period > 0.0 && v_length > v_period - SPAresabs) {
                SPApar_pos guess_pp;
                if(GET_ALGORITHMIC_VERSION() > AcisVersion(29, 0, 2)) {
                    double vval = face_vrange.mid_pt();
                    double uval = face_urange.mid_pt();
                    guess_pp = SPApar_pos(uval, vval);
                } else {
                    double vval = face_urange.mid_pt();
                    double uval = face_urange.mid_pt();
                    guess_pp = SPApar_pos(uval, vval);
                }
                SPAposition start_pos = original_face->loop()->start()->start_pos();
                SPApar_pos split = original_surf->equation().param(start_pos, guess_pp);
                double started = face_vrange.start_pt();
                double percent = (split.v - started) / v_period;
                if(percent < 0.0 && percent > -SPAresnor) {
                    percent = 0.0;
                }
                api_split_face(face_copy, TRUE, TRUE, percent);
            }
        }
    }

    int did_adaptive = 0;
    int part_inv = 0;
    int make_buff_face = 0;
    error_info* err_info = nullptr;
    standard_offsetter std_off(part_inv, pOffsetOptions, &remake_face, &did_adaptive, 1, 0, 1);
    acis_exception error_info_holder(0, NULL, NULL, 0);
    error_info_base* error_info_base_ptr = nullptr;
    exception_save exception_save_mark;
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    int error_no = 0;
    // v80 = &v140;
    // SPApar_box::SPApar_box(&v140, &face_pb);
    // v81 = v10;
    // v82 = face_copy;
    // offset_geom = standard_offsetter::offset_surface_single(&std_off, original_face, face_copy, v10, &face_box, offset_distancea, (error_info**)&NULL_REF);
    error_info* err = (error_info*)NULL_REF;
    surface* offset_geom = std_off.offset_surface_single(original_face, face_copy, &face_pb, face_box, offset_distancea, err);
    did_adaptive = std_off.did_adaptive();
    SURFACE* original_surfa = face_copy->geometry();
    if(did_adaptive) {
        if(find_option("iop_adaptive_ofst_surface")->on()) {
            if(GET_ALGORITHMIC_VERSION() > AcisVersion(26, 0, 0)) {
                SPApar_box cur_pbox = offset_geom->param_range();
                int singular = 0;
                if(offset_geom->singular_u(cur_pbox.u_range().start_pt()) || offset_geom->singular_u(cur_pbox.u_range().end_pt()) || offset_geom->singular_v(cur_pbox.v_range().start_pt()) || offset_geom->singular_v(cur_pbox.v_range().end_pt())) {
                    singular = 1;
                } else {
                    singular = 0;
                }
                if(!singular) {
                    SPAinterval new_urange;
                    SPAinterval new_vrange;
                    for(int ii = 0; ii < 2; ++ii) {
                        SPAinterval cur_range = (ii) ? (cur_pbox.v_range()) : (cur_pbox.u_range());
                        double delta = cur_range.length() * 0.05;
                        if(ii) {
                            new_vrange = SPAinterval(cur_range.start_pt() - delta, cur_range.end_pt() + delta);

                        } else {
                            new_urange = SPAinterval(cur_range.start_pt() - delta, cur_range.end_pt() + delta);
                        }
                    }
                    SPApar_box requested_pbox(new_urange, new_vrange);
                    acis_exception v121(0, NULL, NULL, 0);
                    error_info_base* e_info = nullptr;
                    exception_save v107;
                    v107.begin();
                    get_error_mark().buffer_init = 1;
                    extend_surface(*offset_geom, requested_pbox, 1);
                    // v107.~exception_save();
                    if(acis_interrupted()) {
                        sys_error(0, e_info);
                    }
                    // v121.~acis_exception();
                }
            }
        }
    }
    std_off.make_face(face_copy, original_face, original_surfa, offset_geom, face_box, failed_full_surface, face_pb, did_adaptive, offset_distancea);
    if(offset_geom) {
        ACIS_DELETE offset_geom;
    }
    // exception_save_mark.~exception_save();
    if(acis_interrupted()) {
        sys_error(0, error_info_base_ptr);
    }
    // error_info_holder.~acis_exception();
    ofst_term();
    // std_off.~standard_offsetter();
    return face_copy;
}

int use_pcurve_for_curve_offset(COEDGE* coed, surface* offsurf) {
    int use_pcurve = 0;
    AcisVersion vt2 = AcisVersion(19, 0, 1);
    AcisVersion vt1 = GET_ALGORITHMIC_VERSION();
    if(vt1 > vt2) {
        if(coed->edge()->geometry()) {
            if(offsurf) {
                return check_analytical_curve_offset_on_cone(coed, offsurf);
            }
        }
    }
    return use_pcurve;
}

int check_analytical_curve_offset_on_cone(COEDGE* coed, surface* offsurf) {
    int ret_val = 0;
    AcisVersion cav = GET_ALGORITHMIC_VERSION();
    const curve& base_cu = coed->edge()->geometry()->equation();
    if(!offsurf || !SUR_is_cone(*offsurf) || CUR_is_intcurve(base_cu)) {
        return NULL;
    }
    int is_tolerant = 1;
    if(!is_TEDGE(coed->edge())) {
        if(!is_TVERTEX(coed->start())) {
            if(!is_TVERTEX(coed->end())) {
                is_tolerant = 0;
            }
        }
    }
    if(!is_tolerant) {
        return NULL;
    }
    const cone& origcone = (const cone&)(coed->loop()->face()->geometry()->equation());
    if(origcone.circular()) {
        if(CUR_is_straight(base_cu)) {
            ret_val = 0;
            if(cav > AcisVersion(23, 0, 1)) {
                if(is_TEDGE(coed->edge())) {
                    if(coed->edge()->get_tolerance() > SPAresabs) {
                        ret_val = 1;
                    }
                }
            }
            if(!ret_val) {
                SPAposition spos(coed->start()->geometry()->coords());
                SPAposition epos(coed->end()->geometry()->coords());
                SPAposition sfoot;
                SPAposition efoot;
                SPApar_pos sparam;
                SPApar_pos eparam;
                origcone.point_perp(spos, sfoot, SpaAcis::NullObj::get_par_pos(), sparam, 0);
                origcone.point_perp(epos, efoot, SpaAcis::NullObj::get_par_pos(), eparam, 0);
                double pardist = fabs(sparam.v - eparam.v);
                return pardist > SPAresnor;
            }
        } else if(CUR_is_ellipse(base_cu)) {
            ret_val = (cav > AcisVersion(23, 0, 1)) && (is_TEDGE(coed->edge())) && (coed->edge()->get_tolerance() > SPAresabs);
            if(!ret_val) {
                SPAposition ell_centre(((const ellipse&)base_cu).centre);
                SPAunit_vector ell_normal(((const ellipse&)base_cu).normal);
                SPAunit_vector cone_normal(origcone.base.normal);
                SPAposition cone_base_centre(origcone.base.centre);
                if(SPAresnor <= fabs(((const ellipse&)base_cu).radius_ratio - 1.0)) {
                    straight cone_axis(cone_base_centre, cone_normal, 1.0);
                    SPAposition foot;
                    cone_axis.point_perp(ell_centre, foot);
                    ret_val = (ell_centre - foot).len() > SPAresabs;
                    // cone_axis.~straight();
                } else {
                    return biparallel(ell_normal, cone_normal) == 0;
                }
            }
        }
    }
    return ret_val;
}

iop_modeler_state_sentry::iop_modeler_state_sentry() {
    // modeler_state* v1;  // rax
    // modeler_state* v2;  // [rsp+30h] [rbp-28h]
    // modeler_state* v3;  // [rsp+38h] [rbp-20h]

    this->ag_opt = nullptr;
    this->mdlr_state = nullptr;
    /*this->ag_opt = find_option((const char *)&com_cur::`vftable'[3]);
    if ( this->ag_opt )
      option_header::push(this->ag_opt, 1);
    v2 = (modeler_state *)ACIS_OBJECT::operator new(
                            0x28ui64,
                            eDefault,
                            "E:\\build\\acis\\NTSwin_b64_debug\\SPAAcisInterop\\iopheal_ipi.m\\src\\spax_check_edge_error_util.cpp",
                            146,
                            &alloc_file_index_24);
    if ( v2 )
    {
          modeler_state::modeler_state(v2);
          v3 = v1;
    }
    else
    {
          v3 = 0i64;
    }
    this->mdlr_state = v3;*/
}

iop_modeler_state_sentry::~iop_modeler_state_sentry() {
    if(this->ag_opt) {
        this->ag_opt->pop();
    }
    modeler_state* mdlr_state = this->mdlr_state;
    /*if (mdlr_state)
    {
        modeler_state::`scalar deleting destructor'(mdlr_state, 1u);
    }*/
    this->mdlr_state = nullptr;
}

parallel_get_tolerance::parallel_get_tolerance() {
    this->_pState = nullptr;
}

void parallel_get_tolerance::process(void* arg) {
    if(this->_pState) {
        this->_pState->mdlr_state->activate();
    }
    if(arg) {
        calculate_edge_tolerance((EDGE*)arg, *((double*)arg + 1));
    }
}

void parallel_get_tolerance::get_tolerances(SpaStdVector<TEDGE*>& tedges, SpaStdVector<std::pair<TEDGE*, double>>* tedge_tols) {
}

bool parallel_get_tolerance::is_threadsafe_tol_update(EDGE* ed) {
    return TRUE;
}
