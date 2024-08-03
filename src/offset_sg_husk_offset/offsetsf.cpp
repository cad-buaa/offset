#include "ProtectedInterfaces/offsetsf.hxx"

#include "PrivateInterfaces/apiFinder.hxx"
#include "ProtectedInterfaces/off_cu.hxx"
#include "ProtectedInterfaces/ofsttools.hxx"
#include "ProtectedInterfaces/protlist.hxx"
#include "acis/acistype.hxx"
#include "acis/add_pcu.hxx"
#include "acis/allsfdef.hxx"
#include "acis/annotation.hxx"
#include "acis/bcu.hxx"
#include "acis/blnd_spl.hxx"
#include "acis/bsf.hxx"
#include "acis/calctol.hxx"
#include "acis/ckoutcom.hxx"
#include "acis/cnstruct.hxx"
#include "acis/copyanno.hxx"
#include "acis/cur.hxx"
#include "acis/curdef.hxx"
#include "acis/curve.hxx"
#include "acis/cvc.hxx"
#include "acis/errorbase.hxx"
#include "acis/faceutil.hxx"
#include "acis/geometry.hxx"
#include "acis/get_top.hxx"
#include "acis/getbox.hxx"
#include "acis/intdef.hxx"
#include "acis/kernapi.hxx"
#include "acis/kernopts.hxx"
#include "acis/law.hxx"
#include "acis/math.hxx"
#include "acis/mkoffscur.hxx"
#include "acis/off_scur.hxx"
#include "acis/offserr_info.hxx"
#include "acis/par_int.hxx"
#include "acis/pcurve.hxx"
#include "acis/point.hxx"
#include "acis/repedge.hxx"
#include "acis/rm_pcu.hxx"
#include "acis/sgapi.err"
#include "acis/sgoffrtn.hxx"
#include "acis/sgquertn.hxx"
#include "acis/sliver_ents.hxx"
#include "acis/spline.hxx"
#include "acis/splsur.err"
#include "acis/sur.hxx"
#include "acis/sur_off.err"
#include "acis/surf_int.hxx"
#include "acis/surface.hxx"
#include "acis/svc.hxx"
#include "acis/tolerize_ent.hxx"
#include "acis/topol.hxx"

extern APIFINDER apiFinderACIS;
extern PROCSTATE prostate;

offset_surface_options::offset_surface_options() {
    this->exact_offset_var_blend = 1;
    this->make_approx_sf = 1;
    this->simplify = 1;
    this->do_adaptive = 0;
    this->tolerize = 1;
    this->replace_edges_with_parintcurve = 0;
    this->replace_vertex_blend = 0;
    this->allow_split = 0;
    this->need_par_int_curs = 0;
    this->trim_bad_geometry = 1;
}

standard_offsetter::standard_offsetter(standard_offsetter& __that): m_part_inv(__that.m_part_inv) {
    // this->__vftable = (standard_offsetter_vtbl*)standard_offsetter::`vftable';
    memcpy(&this->m_offset_surface_options, &__that.m_offset_surface_options, sizeof(this->m_offset_surface_options));
    this->m_part_inv = __that.m_part_inv;
    this->m_off_opts = __that.m_off_opts;
    this->m_reparam_exact_offset_surf = __that.m_reparam_exact_offset_surf;
    this->m_natural_boundary_for_closed_spline = __that.m_natural_boundary_for_closed_spline;
    this->m_need_par_int_curs = __that.m_need_par_int_curs;
}

standard_offsetter::standard_offsetter(int& part_inv, offset_options* pOffOpts, int* remake_face, int* adaptive_flag, int tolerize, int replace_vertex_blend, int allow_split): m_part_inv(part_inv) {
    // this->__vftable = (standard_offsetter_vtbl*)standard_offsetter::`vftable';
    this->m_offset_surface_options = offset_surface_options();
    this->m_remake_face = 0i64;
    this->m_adaptive_flag = 0i64;
    this->m_part_inv = part_inv;
    this->m_tolerize = tolerize;
    this->m_replace_vertex_blend = replace_vertex_blend;
    this->m_allow_split = allow_split;
    this->m_off_opts = nullptr;  // NULL
    if(pOffOpts) {
        this->m_off_opts = ACIS_NEW offset_options(*pOffOpts);
    }
    this->m_reparam_exact_offset_surf = 0;
    this->m_natural_boundary_for_closed_spline = 1;
    if(remake_face) this->m_remake_face = remake_face;
    if(adaptive_flag) this->m_adaptive_flag = adaptive_flag;
}

standard_offsetter::~standard_offsetter() {
    offset_options* m_off_opts;  // [rsp+20h] [rbp-18h]

    // this->__vftable = (standard_offsetter_vtbl*)standard_offsetter::`vftable';
    if(this->m_off_opts) {
        m_off_opts = this->m_off_opts;
        if(m_off_opts) {
            // offset_options::`scalar deleting destructor'(m_off_opts, 1u);

            ACIS_DELETE m_off_opts;
        }
    }
}

surface* standard_offsetter::offset_surface_single(FACE* original_face, FACE* face_copy, SPApar_box* face_pb, SPAbox& face_box, double offset_distance, error_info*& err) {
    SURFACE* original_surf = face_copy->geometry();
    int did_adaptive = 0;
    acis_exception error_info_holder(0, (error_info_base*)0);
    error_info_base* error_info_base_ptr = nullptr;
    exception_save exception_save_mark;
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    int part_inv = 0;
    int make_buff_face = 0;
    error_info* err_info = nullptr;
    offset_options* pOffsetOptions = this->m_off_opts;
    offset_options dummy;
    if(!pOffsetOptions) {
        pOffsetOptions = &dummy;
    }
    pOffsetOptions->verify_version();
    offset_surface_options off_sur_opts;
    off_sur_opts.simplify = pOffsetOptions->get_simplify();
    off_sur_opts.do_adaptive = 1;
    off_sur_opts.replace_edges_with_parintcurve = 1;
    off_sur_opts.trim_bad_geometry = pOffsetOptions->get_trim_bad_geom();
    SPApar_box* in_par_box = face_pb;
    surface* original_surface = (surface*)&(original_surf->equation());
    surface* offset_geom = offset_surface(original_surface, face_box, offset_distance, part_inv, &off_sur_opts, err_info, &make_buff_face, *in_par_box, face_copy, &did_adaptive, original_face);
    if(this->m_adaptive_flag) {
        *this->m_adaptive_flag = did_adaptive;
    }
    if(did_adaptive) {
        sg_get_face_par_box(face_copy, *face_pb);
        face_copy->geometry();
    }
    if(err_info) {
        err_info->remove();
        err_info = nullptr;
    }
    // dummy.~offset_options();
    if(!offset_geom) {
        ofst_error(NO_SPLINE_APPROX, 1, original_face);
    }
    // exception_save_mark.~exception_save();
    if(acis_interrupted()) {
        sys_error(0, error_info_base_ptr);
    }
    // error_info_holder.~acis_exception();
    return offset_geom;
}

void standard_offsetter::make_face_with_natural_boundaries(FACE*& face_copy, FACE* original_face, surface* offset_geom, SPApar_box& face_pb, int failed_full_surface) {
    outcome oc = api_delent(face_copy);
    if(!oc.ok()) {
        ofst_error(NO_SPLINE_APPROX, 1, original_face);
    }
    if(failed_full_surface) {
        AcisVersion vt2(12, 0, 0);
        AcisVersion vt1 = GET_ALGORITHMIC_VERSION();

        if(careful_option.on() && (vt1 >= vt2)) {
            ofst_error(OFFSET_PARTIAL, 1, original_face);
        } else {
            sys_warning(OFFSET_PARTIAL);
        }
    }
    SPApar_box offsurf_pb = offset_geom->param_range();
    if(offset_geom->periodic_u()) {
        SPAinterval vb_new = offsurf_pb.v_range();
        SPAinterval ub_new(interval_infinite);
        offsurf_pb = SPApar_box(ub_new, vb_new);
    }
    if(offset_geom->periodic_v()) {
        offsurf_pb = SPApar_box(offsurf_pb.u_range(), SPAinterval(interval_infinite));
    }
    // operator&(&new_pb, &offsurf_pb, face_pb);
    SPApar_box new_pb = offsurf_pb & face_pb;
    if((GET_ALGORITHMIC_VERSION() > AcisVersion(23, 0, 1)) && (offsurf_pb >> new_pb)) {
        surface* offsurf = offset_geom->subset(new_pb);
        outcome face_spline = api_make_face_spline(offsurf, face_copy);
        check_outcome(face_spline);
        // face_spline.~outcome();
        //  if(offsurf) {
        //      // v38 = offsurf->~surface;
        //      // v39 = ((__int64(__fastcall*)(surface*, __int64))v38)(offsurf, 1i64);
        //      offsurf->~surface();
        //  } else {
        //      // v39 = 0i64;
        //  }
    } else {
        check_outcome(api_make_face_spline(offset_geom, face_copy));
    }
    if(!face_copy) {
        // v15 = message_module::message_code(&spaacisds_sgapi_errmod, 22);
        ofst_error(DEGENERATE_FACE, 1, original_face);
    }
    SPApar_box copy_face_pb;
    sg_get_face_par_box(face_copy, copy_face_pb);
    if(!(face_pb >> copy_face_pb)) {
        // v16 = message_module::message_code(&spaacisds_splsur_errmod, 9);
        ofst_error(NO_SPLINE_APPROX, 1, original_face);
    }
    // oc.~outcome();
}

int standard_offsetter::did_adaptive() {
    if(this->m_adaptive_flag)
        return *(unsigned int*)this->m_adaptive_flag;
    else
        return NULL;
}

update_face_geom_options::update_face_geom_options(int use_lops_settings) {
    this->replace_surf = 1;
    this->replace_pcurves = 1;
    this->allow_extension = 1;
    this->g1_disc = 0;
    this->fixup_vertex = 1;
    if(use_lops_settings) {
        this->tolerize = 1;
        this->replace_edges_with_parintcurve = 0;
        this->replace_all_edges = 1;
        this->prefer_c2_pcur = 1;
    } else {
        this->tolerize = 0;
        this->replace_edges_with_parintcurve = 1;
        this->replace_all_edges = 0;
        this->prefer_c2_pcur = 0;
    }
}

int get_pcurve_edge_overlap_interval(COEDGE* coedge, double& start, double& end) {
    bool v8 = FALSE;
    bool v9 = TRUE;
    pcurve result;
    if(coedge->geometry()) {
        if(coedge->edge()) {
            result = coedge->geometry()->equation(0);
            v8 = TRUE;
            if(result.cur()) {
                v9 = FALSE;
            }
        }
    }
    if((v8 & 1) != 0) {
        // result.~pcurve();
    }
    if(v9) {
        return 0;
    }
    pcurve pc = coedge->geometry()->equation(1);
    SPAinterval r_range = pc.param_range();
    SPAinterval edge_range = coedge->edge()->param_range();
    if(coedge->sense() == 1) {
        edge_range.negate();
    }
    start = -1.0;
    end = -1.0;
    if(fabs(pc.param_period()) <= SPAresnor) {
        double started = edge_range.start_pt();
        if(started < r_range)
            start = r_range.start_pt();
        else
            start = edge_range.start_pt();
        if(edge_range.end_pt() > r_range)
            end = r_range.end_pt();
        else
            end = edge_range.end_pt();
    } else {
        start = edge_range.start_pt();
        end = edge_range.end_pt();
    }
    // pc.~pcurve();
    return 1;
}

// pcurve* generate_c2_pcurve(COEDGE* ce, double tol, SPApar_box* suggested_pb, SPA_pcurve_fit_options* opts) {
//     typedef pcurve* (*generate_c2_pcurve_type)(COEDGE* ce, double tol, SPApar_box* suggested_pb, SPA_pcurve_fit_options* opts);
//     generate_c2_pcurve_type f = (generate_c2_pcurve_type)apiFinderACIS.GetAddress("?generate_c2_pcurve@@YAPEAVpcurve@@PEAVCOEDGE@@NPEAVSPApar_box@@PEAVSPA_pcurve_fit_options@@@Z", prostate);
//     if(f) {
//         return f(ce, tol, suggested_pb, opts);
//     } else {
//         return nullptr;
//     }
// }

curve* make_offset_curve_internal(surface* base_sf1, curve* base_cu, const double& offset, pcurve* pcur1, surface* base_sf2, const double& sf2_offset, pcurve* pcur2, surface* offs_sf1, surface* offs_sf2, COEDGE* coed, int& pcurve_invalid, int thicken,
                                  int& approx_offset, int approx_offset_sf1, int approx_offset_sf2) {
    typedef curve* (*make_offset_curve_internal_type)(surface* base_sf1, curve* base_cu, const double& offset, pcurve* pcur1, surface* base_sf2, const double& sf2_offset, pcurve* pcur2, surface* offs_sf1, surface* offs_sf2, COEDGE* coed,
                                                      int& pcurve_invalid, int thicken, int& approx_offset, int approx_offset_sf1, int approx_offset_sf2);
    make_offset_curve_internal_type f = (make_offset_curve_internal_type)apiFinderACIS.GetAddress("?make_offset_curve_internal@@YAPEAVcurve@@PEBVsurface@@PEBV1@AEBNPEBVpcurve@@02300PEAVCOEDGE@@AEAHH5HH@Z", prostate);
    if(f) {
        return f(base_sf1, base_cu, offset, pcur1, base_sf2, sf2_offset, pcur2, offs_sf1, offs_sf2, coed, pcurve_invalid, thicken, approx_offset, approx_offset_sf1, approx_offset_sf2);
    } else {
        return nullptr;
    }
}

// int add_c2_pcurves(COEDGE* ce, double tol, SPApar_box* suggested_pb, SPA_pcurve_fit_options* opts) {
//     if(!ce) {
//         return NULL;
//     }
//     if(!ce->edge()->geometry()) {
//         return NULL;
//     }
//     if(!ce->loop()) {
//         return 0i64;
//         ;
//     }
//     SPA_pcurve_fit_options default_opts;
//     if(!opts) {
//         opts = &default_opts;
//     }
//     SPApar_box tempPbox;
//     if(suggested_pb) {
//         SPApar_box temp;
//         memcpy(suggested_pb, &temp, sizeof(SPApar_box));
//     } else if(opts->fail_if_off_surf() || opts->extend_if_off_surf()) {
//         suggested_pb = &tempPbox;
//     }
//     int gotPcu = 0;
//     LOOP* loop = ce->loop();
//     if(loop && loop->face()) {
//         acis_exception error_info_holder(0);
//         exception_save exception_save_mark;
//         exception_save_mark.begin();
//         get_error_mark().buffer_init = 1;
//         pcurve* pcu = generate_c2_pcurve(ce, tol, suggested_pb, opts);
//         if(pcu) {
//             if(ce->sense() != ce->edge()->sense()) {
//                 pcu->negate();
//             }
//             ce->set_geometry(ACIS_NEW PCURVE(*pcu));
//             if(is_TCOEDGE(ce)) {
//                 ((TCOEDGE*)ce)->set_3D_curve(0i64);
//                 double start = -1.0;
//                 double end = -1.0;
//                 get_pcurve_edge_overlap_interval(ce, start, end);
//                 ((TCOEDGE*)ce)->set_param_range(SPAinterval(start, end));
//                 update_coedge_tolerance(ce, 1);
//             }
//             gotPcu = 1;
//
//             if(ce->loop()->face()) {
//                 ce->loop()->face()->set_uv_bound(NULL);
//                 ce->loop()->set_classification(loop_unknown);
//             }
//         }
//         if(pcu) {
//             ACIS_DELETE pcu;
//         }
//         exception_save_mark.~exception_save();
//         if(acis_interrupted()) {
//             sys_error(0, (error_info_base*)nullptr);
//         }
//         error_info_holder.~acis_exception();
//     }
//     default_opts.~SPA_pcurve_fit_options();
//     return gotPcu;
// }

// void get_parintcur_edges_wSameSurf(ENTITY_LIST& el, FACE* face) {
//     typedef void (*get_parintcur_edges_wSameSurf_type)(ENTITY_LIST& el, FACE* face);
//     get_parintcur_edges_wSameSurf_type f = (get_parintcur_edges_wSameSurf_type)apiFinderACIS.GetAddress("?get_parintcur_edges_wSameSurf@@YAXAEAVENTITY_LIST@@PEBVFACE@@@Z", prostate);
//     if(f) {
//         f(el, face);
//     }
// }

// void get_parintcur_edges_wSameSurf(ENTITY_LIST& el, FACE* face) {
//     spline* v3;    // rax
//     spline* v5;    // rax
//     spline* v6;    // rax
//     SURFACE* v18;  // [rsp+80h] [rbp-48h]
//     el.clear();
//     if(face && is_spline_face(face)) {
//         ENTITY_LIST edge_list;
//         outcome out = api_get_edges(face, edge_list, PAT_CAN_CREATE);
//         check_outcome(out);
//         edge_list.init();
//         EDGE* ent = nullptr;
//         for(ent = (EDGE*)edge_list.next(); ent; ent = (EDGE*)edge_list.next()) {
//             if(is_EDGE(ent)) {
//                 EDGE* ed = ent;
//                 if(is_intcurve_edge(ent)) {
//                     intcurve* old_ic = (intcurve*)&(ed->geometry()->equation());
//                     const int_cur& int_cur = old_ic->get_int_cur();
//                     if(int_cur.type() == par_int_cur::id() || (old_ic->get_int_cur().type() == surf_int_cur::id()) /*|| (old_ic->get_int_cur().type() == int_int_cur::id())*/) {
//                         const spl_sur* ss1 = nullptr;
//                         if(!SpaAcis::NullObj::check_surface(old_ic->surf1(0))) {
//                             v3 = (spline*)&(old_ic->surf1(0));
//                             ss1 = &(v3->get_spl_sur());
//                         }
//                         /*if(old_ic->surf1(0))
//                         {
//                             if(!SpaAcis::NullObj::check_surface(old_ic->surf1(0)))
//                             {
//                                 v3 = (spline*)&(old_ic->surf1(0));
//                                 ss1 = &(v3->get_spl_sur());
//                             }
//                         }*/
//                         const spl_sur* ss2 = nullptr;
//                         if(!SpaAcis::NullObj::check_surface(old_ic->surf2(0))) {
//                             v5 = (spline*)&(old_ic->surf2(0));
//                             ss2 = &(v5->get_spl_sur());
//                         }
//                         /*if(old_ic->surf2(0))
//                         {
//                             if(!SpaAcis::NullObj::check_surface(old_ic->surf2(0)))
//                             {
//                                 v5 = (spline*)&(old_ic->surf2(0));
//                                 ss2 = &(v5->get_spl_sur());
//                             }
//                         }*/
//                         v18 = face->geometry();
//                         v6 = (spline*)&(v18->equation());
//                         const spl_sur* ss0 = &(v6->get_spl_sur());
//                         if(*ss0 == *ss1 || *ss0 == *ss2) {
//                             el.add(ent, 1);
//                         }
//                     }
//                 }
//             }
//         }
//         out.~outcome();
//         edge_list.~ENTITY_LIST();
//     }
// }

// int par_int_to_edge(COEDGE* coed, int fixup_vertex) {
//     typedef int (*par_int_to_edge_type)(COEDGE* coed, int fixup_vertex);
//     par_int_to_edge_type f = (par_int_to_edge_type)apiFinderACIS.GetAddress("?par_int_to_edge@@YAHPEAVCOEDGE@@H@Z", prostate);
//     if(f) {
//         return f(coed, fixup_vertex);
//     } else {
//         return NULL;
//     }
// }

void sg_update_face_geometry(FACE* ref_face, surface* surf, update_face_geom_options* ufg_opts) {
    typedef void (*sg_update_face_geometry_type)(FACE* ref_face, surface* surf, update_face_geom_options* ufg_opts);
    sg_update_face_geometry_type f = (sg_update_face_geometry_type)apiFinderACIS.GetAddress("?sg_update_face_geometry@@YAXPEAVFACE@@PEAVsurface@@PEAVupdate_face_geom_options@@@Z", prostate);
    if(f) {
        f(ref_face, surf, ufg_opts);
    }
}

int add_pcurve_to_coedge_using_intcurve_pcurve(COEDGE* coedge, int exact, bndy_type bndy_info, int reset_pattern, surface* surf) {
    typedef int (*add_pcurve_to_coedge_using_intcurve_pcurve_type)(COEDGE* coedge, int exact, bndy_type bndy_info, int reset_pattern, surface* surf);
    add_pcurve_to_coedge_using_intcurve_pcurve_type f = (add_pcurve_to_coedge_using_intcurve_pcurve_type)apiFinderACIS.GetAddress("?add_pcurve_to_coedge_using_intcurve_pcurve@@YAHPEAVCOEDGE@@HW4bndy_type@@HPEBVsurface@@@Z", prostate);
    if(f) {
        return f(coedge, exact, bndy_info, reset_pattern, surf);
    } else {
        return NULL;
    }
}

// void sg_update_face_geometry(FACE* ref_face, surface* surf, update_face_geom_options* ufg_opts) {
//     if(ref_face) {
//         update_face_geom_options def_opts(0);
//         if(!ufg_opts) {
//             ufg_opts = &def_opts;
//         }
//         if(ufg_opts->replace_surf) {
//             SURFACE* SURF = make_surface(*surf);
//             ref_face->set_geometry(SURF, 1);
//         }
//         ENTITY_LIST clist;
//         get_coedges(ref_face, clist, PAT_CAN_CREATE);
//         if(ufg_opts->replace_pcurves) {
//             if(ufg_opts->g1_disc || !ufg_opts->prefer_c2_pcur) {
//                 sg_rm_pcurves_from_entity(ref_face, 0, 1, 1);
//                 sg_add_pcurves_to_entity(ref_face, 1);
//             } else {
//                 clist.init();
//                 ENTITY* ent = clist.next();
//                 while(ent) {
//                     SPA_pcurve_fit_options C2opts;
//                     C2opts.set_check_ctrl_pts(1);
//                     C2opts.set_fail_if_off_surf(1);
//                     C2opts.set_fit_to_xyz_errors(1);
//                     C2opts.set_extend_if_off_surf(ufg_opts->allow_extension);
//                     COEDGE* ce = (COEDGE*)ent;
//                     int replace_pcur;
//                     if(ufg_opts->replace_surf) {
//                         replace_pcur = 1;
//                     } else {
//                         replace_pcur = (ce->geometry() != NULL);
//                     }
//                     if(replace_pcur) {
//                         // v3 = safe_function_type<double>::operator double(&SPAresfit);
//                         if(!add_c2_pcurves(ce, SPAresfit, NULL, &C2opts)) {
//                             sg_rm_pcurves_from_entity(ce, 0, 1, 1);
//                             sg_add_pcurves_to_entity(ce, 1);
//                         }
//                     }
//                     ent = clist.next();
//                     C2opts.~SPA_pcurve_fit_options();
//                 }
//             }
//         } else {
//             clist.init();
//             COEDGE* i;
//             for(i = (COEDGE*)clist.next(); i; i = (COEDGE*)clist.next()) {
//                 if(i->geometry()) {
//                     PCURVE* PC = i->geometry();
//                     pcurve pc = PC->equation();
//                     pc.set_surface(*surf);
//                     PCURVE* NEW_PC = ACIS_NEW PCURVE(pc);
//                     i->set_geometry(NEW_PC, 1);
//                     if(is_TEDGE(i->edge())) {
//                         i->edge()->set_bound(NULL);
//                         ((TEDGE*)i->edge())->mark_to_update();
//                     }
//                     if(is_TVERTEX(i->start())) {
//                         ((TVERTEX*)i->start())->mark_to_update();
//                     }
//                     if(is_TVERTEX(i->end())) {
//                         ((TVERTEX*)i->end())->mark_to_update();
//                     }
//                     pc.~pcurve();
//                 }
//             }
//         }
//         AcisVersion vt2 = AcisVersion(12, 0, 0);
//         AcisVersion vt1 = GET_ALGORITHMIC_VERSION();
//         if((vt1 >= vt2) && ufg_opts->replace_edges_with_parintcurve) {
//             ENTITY_LIST edge_list;
//             if(ufg_opts->replace_all_edges)
//                 get_edges(ref_face, edge_list, PAT_CAN_CREATE);
//             else
//                 get_parintcur_edges_wSameSurf(edge_list, ref_face);
//             edge_list.init();
//             ENTITY* e;
//             for(e = edge_list.next(); e; e = edge_list.next()) {
//                 if(is_EDGE(e)) {
//                     EDGE* ed = (EDGE*)e;
//                     int fixup_vertex = ufg_opts->fixup_vertex;
//                     COEDGE* coed = ed->coedge(ref_face);
//                     par_int_to_edge(coed, fixup_vertex);
//                 }
//             }
//             edge_list.~ENTITY_LIST();
//         }
//         if(ufg_opts->tolerize) {
//             outcome result(0);
//             ENTITY_LIST bad_list;
//             ENTITY_LIST edges;
//             result = api_get_edges(ref_face, edges, PAT_CAN_CREATE);
//             check_outcome(result);
//             ENTITY* worst_entity = nullptr;
//             ENTITY_LIST bad_list2;
//             double tol = SPAresabs;
//             bad_list.clear();
//             double worst_error;
//             result = api_check_edge_errors(edges, bad_list, worst_entity, worst_error, tol, 0, bad_list2);
//             check_outcome(result);
//             ENTITY_LIST vertex_list;
//             result = api_get_vertices(ref_face, vertex_list, PAT_CAN_CREATE);
//             check_outcome(result);
//             bad_list.clear();
//             bad_list2.clear();
//             result = api_check_vertex_errors(vertex_list, bad_list, worst_entity, worst_error, tol, 0, bad_list2, 0i64);
//             check_outcome(result);
//             vertex_list.~ENTITY_LIST();
//             bad_list2.~ENTITY_LIST();
//             edges.~ENTITY_LIST();
//             bad_list.~ENTITY_LIST();
//             result.~outcome();
//         }
//         clist.~ENTITY_LIST();
//     }
// }

void standard_offsetter::make_face_with_offset_boundaries(FACE* face_copy, FACE* original_face, SURFACE* offset_surf, double offset_distance, SPApar_box& face_pb, int did_adaptive) {
    standard_offsetter::offset_loops(original_face, face_copy, offset_surf, offset_distance, face_pb);
    face_copy->set_geometry(offset_surf, 1);
    if(offset_surf->identity(2) == SPLINE_TYPE) {
        if(did_adaptive) {
            update_face_geom_options ufg_opts(1);
            sg_update_face_geometry(face_copy, &(offset_surf->equation_for_update()), &ufg_opts);
        } else {
            sg_add_pcurves_to_entity(face_copy, 1);
        }
    }
}

void standard_offsetter::offset_loops(FACE* orig_face, FACE* this_face, SURFACE* offset_surf, double offset_distance, SPApar_box& pb) {
    int resignal_no = 0;
    acis_exception error_info_holder(0, NULL, NULL, 0);
    error_info_base* error_info_base_ptr = nullptr;
    exception_save exception_save_mark;
    ENTITY_LIST list;
    ENTITY_LIST coedge_list;
    ENTITY_LIST vertex_list;
    LOP_PROTECTED_LIST* edge_processed = ACIS_NEW LOP_PROTECTED_LIST();
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    int error_no = 0;
    const surface& offset_surface = offset_surf->equation();
    const surface& orig_surf = this_face->geometry()->equation();
    VERTEX* sing_vert = nullptr;
    int u_sing = 0;
    int v_sing = 0;
    SPAbox face_box = get_face_box(this_face);
    double u_start = orig_surf.param_range_u(face_box).start_pt();
    double u_end = orig_surf.param_range_u(face_box).end_pt();
    double v_start = orig_surf.param_range_v(face_box).start_pt();
    double v_end = orig_surf.param_range_v(face_box).end_pt();

    int u_sing_diff_par = 0;
    AcisVersion vt2 = AcisVersion(33, 0, 0);
    AcisVersion vt1 = GET_ALGORITHMIC_VERSION();
    bool postR33 = vt1 > vt2;
    bool check_offset_cone_new_sing = is_cone(&orig_surf) && offset_distance < 0.0;
    SPApar_pos uv;
    SPAposition sing_pos;
    if(orig_surf.singular_u(u_start)) {
        if(is_torus(&orig_surf) || check_offset_cone_new_sing) {
            double u_off_start = offset_surface.param_range_u().start_pt();
            uv = SPApar_pos(u_start, orig_surf.param_range_v(face_box).mid_pt());
            if(fabs(u_off_start - u_start) > SPAresabs) {
                u_sing_diff_par = 1;
                sing_pos = orig_surf.eval_position(uv);
                uv = SPApar_pos(u_off_start, orig_surf.param_range_v(face_box).mid_pt());
            }
            if(!offset_surface.singular_u(u_off_start)) {
                u_sing = 1;
            }
        } else {
            if(!offset_surface.singular_u(u_start)) {
                uv = SPApar_pos(u_start, orig_surf.param_range_v(face_box).mid_pt());
                u_sing = 1;
            }
        }
    } else {
        if(orig_surf.singular_u(u_end)) {
            if(is_torus(&orig_surf) || check_offset_cone_new_sing) {
                double u_off_end = offset_surface.param_range_u().end_pt();
                uv = SPApar_pos(u_end, orig_surf.param_range_v(face_box).mid_pt());
                if(fabs(u_off_end - u_end) > SPAresabs) {
                    u_sing_diff_par = 1;
                    sing_pos = orig_surf.eval_position(uv);
                    uv = SPApar_pos(u_off_end, orig_surf.param_range_v(face_box).mid_pt());
                }
                if(!offset_surface.singular_u(u_off_end)) {
                    u_sing = 1;
                }
            } else {
                if(!offset_surface.singular_u(u_end)) {
                    uv = SPApar_pos(u_end, orig_surf.param_range_v(face_box).mid_pt());
                    u_sing = 1;
                }
            }
        } else {
            if(!orig_surf.singular_v(v_start) || offset_surface.singular_v(v_start)) {
                if(orig_surf.singular_v(v_end)) {
                    if(!offset_surface.singular_v(v_end)) {
                        uv = SPApar_pos(orig_surf.param_range_u(face_box).mid_pt(), v_end);
                        v_sing = 1;
                    }
                }
            } else {
                uv = SPApar_pos(orig_surf.param_range_v(face_box).mid_pt(), v_start);
                v_sing = 1;
            }
        }
    }
    int add_edge = u_sing || v_sing;
    if(add_edge) {
        sing_pos = orig_surf.eval_position(uv);
        LOOP* loop = nullptr;
        for(loop = this_face->loop(); loop && !sing_vert; loop = loop->next()) {
            COEDGE* s_coedge = loop->start();
            COEDGE* coedge = s_coedge;
            while(coedge && !sing_vert) {
                SPAposition curr_pos(coedge->start()->geometry()->coords());
                if(same_point(sing_pos, curr_pos)) {
                    sing_vert = coedge->start();
                    if(coedge->start() == coedge->end()) {
                        add_edge = 0;
                    }
                }
                coedge = coedge->next();
                if(coedge == s_coedge) {
                    coedge = nullptr;
                }
            }
        }
        if(!sing_vert) {
            add_edge = 0;
        }
    }
    EDGE* new_edge = nullptr;
    if(add_edge) {
        if(GET_ALGORITHMIC_VERSION() > AcisVersion(25, 0, 0)) {
            COEDGE* change_coed = nullptr;
            for(LOOP* j = this_face->loop(); j; j = j->next()) {
                for(COEDGE* tmpcoed = j->start(); tmpcoed; tmpcoed = tmpcoed->next()) {
                    if(tmpcoed->end() == sing_vert) {
                        change_coed = tmpcoed;
                        break;
                    }
                }
                if(change_coed) {
                    break;
                }
            }
            COEDGE* new_coed = lopt_adev(change_coed, (CURVE*)0, 0);
            new_edge = new_coed->edge();
        } else {
            // new_edge = add_edge_at_singularity(this_face, sing_vert);
        }
    }
    get_coedges(this_face, coedge_list, PAT_CAN_CREATE);
    get_vertices(this_face, vertex_list, PAT_CAN_CREATE);
    SURFACE* original_surf = this_face->geometry();
    int num_entities = vertex_list.count();
    for(int i = 0; i < num_entities; ++i) {
        VERTEX* vert = (VERTEX*)vertex_list[i];
        APOINT* this_point = vert->geometry();
        SPAposition offset_pos;
        SPAposition original_pos(this_point->coords());
        standard_offsetter::offset_vertex_pos(vert, sing_pos, offset_distance, offset_pos, new_edge, original_surf, offset_surf, pb, uv, add_edge, u_sing_diff_par);
        this_point->set_coords(offset_pos);
        if(is_TVERTEX(vert)) {
            ((TVERTEX*)vert)->mark_to_update();
        }
    }

    int remove_pcus = offset_surf && ((offset_surf->identity(2) == SPLINE_TYPE) || (this_face->geometry()->identity(2) == SPLINE_TYPE));
    num_entities = coedge_list.count();
    coedge_list.init();
    while(1) {
        TCOEDGE* this_coedge = (TCOEDGE*)(coedge_list.next());
        if(!this_coedge) {
            break;
        }
        int err_num = 0;
        acis_exception v362(0);
        error_info_base* e_info = nullptr;
        exception_save v236;
        pcurve pcu_geom;
        curve* offset_geom = nullptr;
        CURVE* off_curve = nullptr;
        v236.begin();
        get_error_mark().buffer_init = 1;
        TEDGE* this_edge = (TEDGE*)(this_coedge->edge());
        if(edge_processed->lookup(this_edge) <= -1) {
            edge_processed->add_ent(this_edge);
            bool degen_edge = this_edge->start() == this_edge->end() && (!this_edge->geometry() || (CUR_is_degenerate(this_edge->geometry()->equation())));
            bool verR24SP1x = GET_ALGORITHMIC_VERSION() > AcisVersion(24, 0, 1);
            int adjust_coedge_dir = 0;
            int bMakeTolerant = 0;
            this->offset_curve(this_coedge, pcu_geom, offset_geom, offset_distance, orig_face, original_surf, offset_surf, add_edge, new_edge, u_sing, v_sing, adjust_coedge_dir, uv, &bMakeTolerant);
            if(!offset_geom) {
                if(this_edge->start() != this_edge->end() || verR24SP1x && !degen_edge) {
                    EDGE* tmpedge = nullptr;
                    COPY_ANNOTATION* ann = (COPY_ANNOTATION*)find_annotation(this_edge, is_COPY_ANNOTATION);
                    if(ann) {
                        if(is_EE_LIST(ann->source())) {
                            tmpedge = nullptr;
                        } else {
                            if(is_EE_LIST(ann->copy())) {
                                tmpedge = nullptr;
                            } else {
                                ENTITY* source = get_actual_live_entity(ann->source());
                                if(is_EDGE(source))
                                    tmpedge = (EDGE*)source;
                                else
                                    tmpedge = nullptr;
                            }
                        }
                    } else {
                        tmpedge = nullptr;
                    }
                    ofst_error(DEGENERATE_EDGE, 1, tmpedge);
                }
            }
            CURVE* Curve = nullptr;
            if(offset_geom)
                Curve = make_curve(*offset_geom);
            else
                Curve = nullptr;
            off_curve = Curve;
            if(Curve) {
                off_curve->add();
            }
            this_edge->set_geometry(off_curve);
            if(is_TVERTEX(this_edge->start()) || (is_TVERTEX(this_edge->end()))) {
                this_edge->set_param_range(NULL);
            }
            double start_p;
            if(this_edge->geometry()) {
                if(!is_TEDGE(this_edge)) {
                    if(GET_ALGORITHMIC_VERSION() >= AcisVersion(10, 0, 4)) {
                        SPAinterval ed_range = this_edge->param_range();
                        if(this_edge->start()) {
                            if(!is_TVERTEX(this_edge->start())) {
                                if(this_edge->sense())
                                    start_p = -ed_range.end_pt();
                                else
                                    start_p = ed_range.start_pt();
                                const SPAposition& p2 = this_edge->start()->geometry()->coords();
                                SPAposition p1 = this_edge->geometry()->equation().eval_position(start_p);
                                if(((p1 - p2).len_sq()) > (SPAresabs * SPAresabs)) {
                                    TVERTEX* temp = nullptr;
                                    replace_vertex_with_tvertex(this_edge->start(), temp);
                                }
                            }
                        }
                        if(this_edge->end()) {
                            if(!is_TVERTEX(this_edge->end())) {
                                if(this_edge->end() != this_edge->start()) {
                                    double end_p = 0.0;
                                    if(this_edge->sense())
                                        end_p = -ed_range.start_pt();
                                    else
                                        end_p = ed_range.end_pt();
                                    const SPAposition& v312 = this_edge->end()->geometry()->coords();
                                    SPAposition v313 = this_edge->geometry()->equation().eval_position(end_p);
                                    if(((v313 - v312).len_sq()) > (SPAresabs * SPAresabs)) {
                                        TVERTEX* this_tvertex = nullptr;
                                        replace_vertex_with_tvertex(this_edge->end(), this_tvertex);
                                    }
                                }
                            }
                        }
                        if(offset_surf && off_curve && !is_TEDGE(this_edge)) {
                            if(!bMakeTolerant) {
                                int nSamplePoints = 7;
                                double fDelta = ed_range.length() / (double)nSamplePoints;
                                for(int t = 0; t < nSamplePoints - 1; ++t) {
                                    double started = ed_range.start_pt();
                                    double fParam = started + (double)t * fDelta;
                                    if(this_edge->sense()) {
                                        fParam = -fParam;
                                    }
                                    SPAposition curvePos = off_curve->equation().eval_position(fParam);
                                    SPAposition surfPos;
                                    offset_surf->equation().point_perp(curvePos, surfPos);
                                    if(((curvePos - surfPos).len()) > (SPAresabs)) {
                                        bMakeTolerant = 1;
                                        break;
                                    }
                                }
                            }
                            if(bMakeTolerant) {
                                TEDGE* this_tedge = nullptr;
                                int bTangent = this_edge->get_convexity() == EDGE_cvty_tangent;
                                COEDGE* part_coed = this_coedge->partner();
                                int replace_partner_also = 0;
                                if((GET_ALGORITHMIC_VERSION() >= AcisVersion(20, 0, 0)) && part_coed && coedge_list.lookup(part_coed) > -1) {
                                    coedge_list.remove(part_coed);
                                    replace_partner_also = 1;
                                }
                                if(replace_edge_with_tedge(this_edge, 1, bTangent, this_tedge)) {
                                    this_edge = this_tedge;
                                    this_coedge = (TCOEDGE*)(this_tedge->coedge());
                                    if(replace_partner_also) {
                                        part_coed = this_coedge->partner();
                                        coedge_list.add(part_coed, 1);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if(remove_pcus && !is_TCOEDGE(this_coedge)) {
                this_coedge->set_geometry(nullptr, 1);
            }
            if(is_TCOEDGE(this_coedge)) {
                SPAinterval old_range = this_edge->param_range();
                this_coedge->edge()->set_param_range(nullptr);
                SPAinterval new_range = this_edge->param_range();

                if((GET_ALGORITHMIC_VERSION() < AcisVersion(21, 0, 0)) || !add_pcurve_to_coedge_using_intcurve_pcurve((COEDGE*)this_coedge, 0, bndy_unknown, 1, (surface*)&offset_surface)) {
                    if(this_edge->sense()) {
                        new_range = (old_range - new_range);
                    }
                    this_coedge->set_3D_curve(nullptr);
                    CURVE* blank_crv = this_edge->geometry();
                    curve* the_curve = nullptr;
                    the_curve = blank_crv->trans_curve();
                    the_curve->limit(new_range);
                    pcurve* pc = ACIS_NEW pcurve(*the_curve, offset_surface, SPAresfit);
                    if(the_curve) {
                        ACIS_DELETE the_curve;
                    } else {
                        the_curve = nullptr;
                    }
                    if(this_coedge->sense() != this_edge->sense()) {
                        pc->negate();
                    }
                    PCURVE* new_pcurve = ACIS_NEW PCURVE(*pc);
                    SPAinterval new_pc_range = pc->param_range();
                    if(pc) {
                        ACIS_DELETE pc;
                    } else {
                        pc = nullptr;
                    }
                    this_coedge->set_geometry(new_pcurve);
                    this_coedge->set_param_range(new_pc_range);
                }
            }
            if(add_edge && this_edge == new_edge || adjust_coedge_dir) {
                int reverse_coed = 0;
                if(this_edge->start() == this_edge->end()) {
                    SPAvector coed_end_dir(coedge_end_dir(this_coedge));
                    SPAposition pos = this_edge->start_pos();
                    SPAvector norm = offset_surface.point_normal(pos);
                    if(this_face->sense() == 1) {
                        norm = (coed_end_dir - norm);
                    }
                    if(this_coedge == this_coedge->next()) {
                        if(adjust_coedge_dir) {
                            if(offset_surface.type() == cone_type) {
                                // v345 = offset_surface->__vftable[1].~surface;
                                // v106 = (const SPAposition*)((__int64(__fastcall*)(const surface*, char*))v345)(offset_surface, v404);
                                //((cone*)&offset_surface)->get_apex();  //yhy add
                                // operator-(&from_apex, &pos, v106);
                                SPAvector from_apex = pos - ((cone*)&offset_surface)->get_apex();
                                SPAvector v2 = coed_end_dir * from_apex;
                                if(antiparallel(norm, v2, SPAresabs)) {
                                    reverse_coed = 1;
                                }
                            } else {
                                offset_surface.type();
                            }
                        }
                    } else {
                        SPAvector cross = coed_end_dir * coedge_start_dir(this_coedge->next());
                        if(antiparallel(norm, cross, SPAresabs)) {
                            reverse_coed = 1;
                        }
                    }
                    if(reverse_coed) {
                        if(this_coedge->sense())
                            this_coedge->set_sense(0, 1);
                        else
                            this_coedge->set_sense(1, 1);
                    }
                }
            }
            if(is_TEDGE(this_edge)) {
                this_edge->mark_to_update();
            }
            if(offset_geom) {
                /*v191 = offset_geom;
                v346 = offset_geom->~curve;
                v347 = ((__int64(__fastcall*)(curve*, __int64))v346)(offset_geom, 1i64);*/
                ACIS_DELETE offset_geom;
            }
            if(off_curve) {
                off_curve->remove();
            }
            // pcu_geom.~pcurve();
            // v236.~exception_save();
            if(err_num || acis_interrupted()) {
                sys_error(err_num, e_info);
            }
            // 362.~acis_exception();
        } else {
            // pcu_geom.~pcurve();
            // v236.~exception_save();
            // v362.~acis_exception();
        }
    }
    ENTITY_LIST tolerantList;
    get_tedges(this_face, tolerantList, PAT_CAN_CREATE);
    get_tvertices(this_face, tolerantList, PAT_CAN_CREATE);
    tolerantList.init();
    while(1) {
        ENTITY* pEnt = tolerantList.next();
        if(!pEnt) {
            break;
        }
        if(is_TVERTEX(pEnt)) {
            ((TVERTEX*)pEnt)->mark_to_update();
        } else if(is_TEDGE(pEnt)) {
            ((TEDGE*)pEnt)->mark_to_update();
        }
    }
    ENTITY_LIST loopsList;
    ENTITY_LIST edgesToReplace;
    get_loops(this_face, loopsList, PAT_CAN_CREATE);
    loopsList.init();
    while(1) {
        LOOP* pThisLoop = (LOOP*)loopsList.next();
        if(!pThisLoop) {
            break;
        }
        COEDGE* pStartCoed = pThisLoop->start();
        COEDGE* pThisCoed = pStartCoed;
        do {
            if(!pThisCoed) {
                break;
            }
            EDGE* pThisEdge = pThisCoed->edge();
            VERTEX* pStart = pThisEdge->start();
            VERTEX* pEnd = pThisEdge->end();
            if(pStart != pEnd) {
                SPAposition startPos(pStart->geometry()->coords());
                double startTol = SPAresabs;
                if(pStart->get_tolerance() > SPAresabs) {
                    startTol = pStart->get_tolerance();
                }
                SPAposition endPos(pEnd->geometry()->coords());
                double endTol = SPAresabs;
                if(pEnd->get_tolerance() > SPAresabs) {
                    endTol = pEnd->get_tolerance();
                }
                double maxTol;
                if(startTol <= endTol)
                    maxTol = endTol;
                else
                    maxTol = startTol;
                double fDist = (startPos - endPos).len();
                if(SPAresabs > fDist || (is_TVERTEX(pStart) || is_TVERTEX(pEnd)) && (maxTol + SPAresmch > fDist)) {
                    edgesToReplace.add(pThisEdge, 1);
                }
            }
            pThisCoed = pThisCoed->next();
        } while(pThisCoed != pStartCoed);
    }
    ENTITY_LIST tvertexList;
    api_replace_edge_with_tvertex(edgesToReplace, tvertexList);
    // tvertexList.~ENTITY_LIST();
    // edgesToReplace.~ENTITY_LIST();
    // loopsList.~ENTITY_LIST();
    // tolerantList.~ENTITY_LIST();
    if(edge_processed) {
        edge_processed->lose();
    }
    // vertex_list.~ENTITY_LIST();
    // coedge_list.~ENTITY_LIST();
    // list.~ENTITY_LIST();
    // exception_save_mark.~exception_save();
    if(resignal_no || acis_interrupted()) {
        sys_error(resignal_no, error_info_base_ptr);
    }
    // error_info_holder.~acis_exception();
}

void standard_offsetter::make_face(FACE*& face_copy, FACE* original_face, SURFACE* original_surf, surface* offset_geom, SPAbox& face_box, int& failed_full_surface, SPApar_box& face_pb, int did_adaptive, double offset_distance) {
    SURFACE* offset_surf = nullptr;
    failed_full_surface = check_failed_full_surface(face_copy, offset_geom, face_box, face_pb);
    int closed_not_periodic_spline_case = 0;
    if(offset_geom->type() == spline_type) {
        /*const surface& v30 = original_surf->equation();
        v30.closed_u();
        const surface& v31 = original_surf->equation();*/
        if((original_surf->equation().closed_u()) && !offset_geom->closed_u() || (original_surf->equation().closed_v()) && !offset_geom->closed_v()) {
            closed_not_periodic_spline_case = 1;
        }
    }
    AcisVersion vt2 = AcisVersion(24, 0, 1);
    AcisVersion vt1 = GET_ALGORITHMIC_VERSION();
    // bool verR24SP1x = operator>(vt1, vt2);
    bool verR24SP1x = vt1 > vt2;
    option_header* iop_os = find_option("iop_adaptive_ofst_surface");
    // v22 = iop_os && option_header::on(iop_os);
    // bool v22 = iop_os && iop_os->on();
    bool face_is_entire_sf = iop_os && iop_os->on();
    if(!face_is_entire_sf) {
        face_is_entire_sf = check_if_face_is_full_surface(original_face, original_surf, offset_geom);
    }
    bool is_face_within_offset_surf_bounds = offset_geom->type() != spline_type;
    SPApar_box off_sur_par_box = offset_geom->param_range();
    if((GET_ALGORITHMIC_VERSION() > AcisVersion(27, 0, 2)) && !is_face_within_offset_surf_bounds) {
        is_face_within_offset_surf_bounds = (off_sur_par_box >> face_pb) != 0;
        if(!is_face_within_offset_surf_bounds) {
            if(failed_full_surface) {
                ENTITY_LIST face_vert_list;
                get_vertices(original_face, face_vert_list, PAT_CAN_CREATE);
                face_vert_list.init();
                bool offset_ok = TRUE;
                while(1) {
                    VERTEX* temp_vert = (VERTEX*)face_vert_list.next();
                    if(!temp_vert) {
                        break;
                    }

                    // APOINT* v12 = temp_vert->geometry();
                    // const SPAposition& v13 = temp_vert->geometry()->coords();

                    SPAposition pos(temp_vert->geometry()->coords());

                    SPAposition foot;
                    // param_actual = SpaAcis::NullObj::get_par_pos();
                    // param_guess = SpaAcis::NullObj::get_par_pos();
                    // surface::point_perp(offset_geom, &pos, &foot, param_guess, param_actual, 0);
                    offset_geom->point_perp(pos, foot);
                    // v14 = operator-(&v54, &pos, &foot);
                    // SPAvector v54 = pos - foot;
                    // dist = SPAvector::len(v14);
                    double dist = (pos - foot).len();
                    // v15 = offset_distance + 10.0 * safe_function_type<double>::operator double(&SPAresfit);
                    // double v15 = offset_distance + 10.0 * SPAresfit;
                    if(dist > (offset_distance + 10.0 * SPAresfit)) {
                        offset_ok = FALSE;
                        break;
                    }
                    temp_vert = (VERTEX*)face_vert_list.next();
                }
                if(offset_ok) {
                    is_face_within_offset_surf_bounds = TRUE;
                }
                // face_vert_list.~ENTITY_LIST();
            }
        }
    }
    if(verR24SP1x && face_is_entire_sf && offset_geom->type() == spline_type || failed_full_surface && !is_face_within_offset_surf_bounds || closed_not_periodic_spline_case && this->m_natural_boundary_for_closed_spline) {
        standard_offsetter::make_face_with_natural_boundaries(face_copy, original_face, offset_geom, face_pb, failed_full_surface);
    } else {
        offset_surf = make_surface(*offset_geom);
    }
    if(offset_surf) {
        // make_face_with_offset_boundaries = this->make_face_with_offset_boundaries;
        //((void(__fastcall*)(standard_offsetter*, _QWORD, const FACE*, SURFACE*, _QWORD, const SPApar_box*, int))make_face_with_offset_boundaries)(this, *face_copy, original_face, offset_surf, *(_QWORD*)&offset_distance, face_pb, did_adaptive);
        make_face_with_offset_boundaries(face_copy, original_face, offset_surf, offset_distance, face_pb, did_adaptive);
    }
    if(face_copy) {
        ENTITY_LIST tol_ents;
        get_tedges(original_face, tol_ents, PAT_CAN_CREATE);
        get_tvertices(original_face, tol_ents, PAT_CAN_CREATE);
        double origface_tol = SPAresabs;
        ENTITY* tmpent = nullptr;
        for(tmpent = tol_ents.first(); tmpent; tmpent = tol_ents.next()) {
            double curtol = 0.0;
            if(is_EDGE(tmpent)) {
                curtol = ((TEDGE*)tmpent)->get_tolerance();
                // curtol = ((double(__fastcall*)(ENTITY*))    tmpent->__vftable[1].identity    )(tmpent);
            } else if(is_VERTEX(tmpent)) {
                curtol = ((TVERTEX*)tmpent)->get_tolerance();
                // curtol = ((double(__fastcall*)(ENTITY*))tmpent->__vftable[1].make_copy)(tmpent);
            }
            origface_tol = D3_max(origface_tol, curtol);
        }
        ENTITY_LIST tiny_edges;
        double tiny_tol = D3_min(SPAresfit, origface_tol);
        // v49 = ipi_detect_short_edges(&v53, *face_copy, &tiny_edges, tiny_tol, 1, 0i64);
        // v50 = v49;
        // check_outcome(v49);
        check_outcome(api_detect_short_edges(face_copy, tiny_edges, tiny_tol, TRUE, NULL));
        // outcome::~outcome(&v53);
        // tiny_edges.~ENTITY_LIST();
        // tol_ents.~ENTITY_LIST();
    }
    reset_boxes_downward(face_copy);
}

void standard_offsetter::offset_vertex_pos(VERTEX* vert, SPAposition& sing_pos, double offset_distance, SPAposition& off_pos, EDGE* new_edge, SURFACE* original_surf, SURFACE* offset_surf, SPApar_box& pb, SPApar_pos& uv, int& add_edge,
                                           const int& u_sing_diff_par) {
    APOINT* this_point = vert->geometry();
    const surface& orig_surf = original_surf->equation();
    if(add_edge && (same_point(this_point->coords(), sing_pos))) {
        off_pos = offset_pos_at_sing(vert, new_edge, offset_distance, (surface*)&orig_surf, pb);
    } else if(u_sing_diff_par && (same_point(this_point->coords(), sing_pos))) {
        off_pos = offset_surf->equation().eval_position(uv);
    } else {
        COEDGE* coed = vert->edge()->coedge();
        if(SUR_is_spline(offset_surf->equation()) && (coed->start() == vert && coed->starts_at_singularity() || coed->end() == vert && coed->ends_at_singularity())) {
            off_pos = offset_pos_at_sing(vert, new_edge, offset_distance, (surface*)&orig_surf, pb);
        } else {
            SPApar_pos uv1;
            uv1 = original_surf->equation().param(this_point->coords());
            off_pos = offset_pos(*(SPAposition*)&this_point->coords(), (surface*)&original_surf->equation(), (surface*)&offset_surf->equation(), uv1, offset_distance);
        }
        const surface& surf = original_surf->equation();
        const SPAposition& test_pos = this_point->coords();
        // sg_at_apex这个函数后续需自己写，该函数是本模块的
        if(sg_at_apex(test_pos, &surf)) {
            if(vert->edge()->coedge()->loop()->face()->sense() == 1) {
                EDGE* ap_edge = vert->edge();
                if(ap_edge->sense() == 1)
                    ap_edge->set_sense(0, 1);
                else
                    ap_edge->set_sense(1, 1);
            }
        }
    }
}

SPAposition standard_offsetter::offset_pos_at_sing(VERTEX* vert, EDGE* edge, double offset_distance, surface* orig_surf, SPApar_box& pb) {
    int bTryBoundedSurface = 1;
    SPAposition Result;
    EDGE* adjacent_edge;
    if(vert->edge(0) == edge) {
        COEDGE* coed = vert->edge(0)->coedge();
        if(coed->end() == vert)
            adjacent_edge = coed->next()->edge();
        else
            adjacent_edge = coed->previous()->edge();
    } else {
        adjacent_edge = vert->edge(0);
    }
    const curve& adj_curve = adjacent_edge->geometry()->equation();
    SPAbox con_fa_box = orig_surf->bound(pb);
    double t = adj_curve.param(vert->geometry()->coords());
    SPAunit_vector vecNormal;
    if(is_straight(&adj_curve) && is_cone(orig_surf)) {
        bTryBoundedSurface = 0;
        if(vert == adjacent_edge->start()) {
            SPAposition otherEnd(adjacent_edge->end()->geometry()->coords());
            SPApar_pos otherUV;
            vecNormal = orig_surf->point_normal(otherEnd, otherUV);
        } else {
            SPAposition otherEnd(adjacent_edge->start()->geometry()->coords());
            SPApar_pos otherUV;
            vecNormal = orig_surf->point_normal(otherEnd, otherUV);
        }
    }
    if(bTryBoundedSurface) {
        acis_exception error_info_holder(0);
        exception_save exception_save_mark;
        exception_save_mark.begin();
        get_error_mark().buffer_init = 1;
        SPAinterval range = adj_curve.param_range(con_fa_box);
        CVEC cvec(ACIS_NEW BOUNDED_CURVE(&adj_curve, range), t, 0);
        cvec.prepare_data(1);
        SPApar_pos uv = orig_surf->param(cvec.P());
        BOUNDED_SURFACE* bsf = BSF_make_bounded_surface(orig_surf, pb);
        SVEC svc(bsf, 1.0e37, 1.0e37, 99, 99);
        svc.overwrite(uv, 99, 99);
        if(!svc.regular()) {
            SPAvector v2 = svc.singular_direction();
            SPAunit_vector v1 = cvec.T();
            double val = v1 % v2;
            int forward = 1;
            if(fabs(val) <= SPAresmch) {
                if(adjacent_edge->start() == vert && adjacent_edge->sense() == 1 || adjacent_edge->end() == vert && !adjacent_edge->sense()) {
                    forward = 0;
                }
                svc.parametrise_singular(cvec, forward);
            } else {
                svc.parametrise_singular(cvec, val < 0.0);
            }
        }
        svc.prepare_normals(1, -1);
        vecNormal = svc.N();
        // svc.~SVEC();
        // cvec.~CVEC();
        // exception_save_mark.~exception_save();
        if(bsf) {
            ACIS_DELETE bsf;
        }
        if(acis_interrupted()) {
            sys_error(0, (error_info_base*)nullptr);
        }
        // error_info_holder.~acis_exception();
    }
    SPAvector v = offset_distance * vecNormal;
    const SPAposition& p = vert->geometry()->coords();
    Result = p + v;
    return Result;
}

SPAposition standard_offsetter::offset_pos(SPAposition& original_pos, surface* surf, surface* offset_surface, SPApar_pos& uv, double offset_distance) {
    SPAposition Result = sg_offset_pos(original_pos, surf, offset_distance);  // 后续需要自己写，这个函数是OFST模块的
    return Result;
}

void standard_offsetter::offset_curve(COEDGE* this_coedge, pcurve& pcu_geom, curve*& offset_geom, double offset_distance, FACE* orig_face, SURFACE* original_surf, SURFACE* offset_surf, int add_edge, EDGE* new_edge, int u_sing, int v_sing,
                                      int& adjust_coedge_dir, SPApar_pos& uv, int* approx_offset) {
    EDGE* this_edge = this_coedge->edge();
    const surface& offset_surface = offset_surf->equation();
    if(add_edge && this_edge == new_edge || !this_edge->geometry()) {
        if(u_sing) {
            offset_geom = offset_surface.v_param_line(uv.u);
        } else if(v_sing) {
            offset_geom = offset_surface.u_param_line(uv.v);
        }
        SPAposition s_pos = this_coedge->start_pos();
        SPAposition e_pos = this_coedge->end_pos();
        if(!add_edge && !this_edge->geometry()) {
            if(offset_geom) {
                if(same_point(s_pos, e_pos, SPAresabs)) {
                    adjust_coedge_dir = 1;
                }
            }
        }
        if(add_edge) {
            if(!same_point(s_pos, e_pos, SPAresabs)) {
                double s_param = offset_geom->param(s_pos);
                double e_param = offset_geom->param(e_pos);
                if(offset_geom->periodic()) {
                    double m_param = (s_param + e_param) / 2.0;
                    if(!this_edge->sense() && s_param > m_param && m_param > e_param || this_edge->sense() == 1 && e_param > m_param && m_param > s_param) {
                        offset_geom->param_period();
                    }
                    SPAposition cur_pos = offset_geom->eval_position(m_param);
                    SPAposition o_pos = this_coedge->next()->end_pos();
                    SPApar_pos o_uv = offset_surface.param(o_pos);
                    double test_u;
                    double test_v;
                    if(u_sing) {
                        test_u = o_uv.u;
                        test_v = (offset_surface.param(cur_pos)).v;
                    } else {
                        test_u = (offset_surface.param(cur_pos)).u;
                        test_v = o_uv.v;
                    }
                    SPApar_pos test_uv(test_u, test_v);
                    SPAposition test_pos = original_surf->equation().eval_position(test_uv);
                    AcisVersion vt2(31, 0, 1);
                    AcisVersion vt1 = GET_ALGORITHMIC_VERSION();
                    point_face_containment pt1_in_face;
                    if((vt1 > vt2)) {
                        pt1_in_face = sg_point_in_face(test_pos, orig_face, SPAtransf(), test_uv, 0, 10);
                    } else {
                        // pt1_in_face = point_in_face(&test_pos, orig_face, 0i64, &test_uv, 0, 10);
                    }
                    if(pt1_in_face == point_outside_face) {
                        if(this_edge->sense() == 1) {
                            this_edge->set_sense(0, 1);
                        } else {
                            this_edge->set_sense(1, 1);
                        }
                    }
                } else if(this_edge->sense() || s_param <= e_param) {
                    if(this_edge->sense() == 1 && e_param > s_param) {
                        this_edge->set_sense(0, 1);
                    }
                } else {
                    this_edge->set_sense(1, 1);
                }
            }
        }
    } else {
        PCURVE* this_pcurve = this_coedge->geometry();
        if(this_pcurve) {
            pcu_geom = this_pcurve->equation();
            if(this_coedge->sense() != this_edge->sense()) {
                pcu_geom.negate();
            }
        }
        int v22;
        if(offset_surf) {
            surface* offsurf = (surface*)&(offset_surf->equation());
            v22 = use_pcurve_for_curve_offset(this_coedge, offsurf);
        } else {
            v22 = use_pcurve_for_curve_offset(this_coedge, nullptr);
        }
        if(v22) {
            acis_exception error_info_holder(0, (error_info_base*)0);
            exception_save exception_save_mark;
            exception_save_mark.begin();
            get_error_mark().buffer_init = 1;
            offset_geom = make_offset_curve_from_pcurve(this_coedge, (surface&)(offset_surf->equation()));
            // exception_save_mark.~exception_save();
            if(acis_interrupted()) {
                sys_error(0, (error_info_base*)0);
            }
            // error_info_holder.~acis_exception();
        }
        if(!offset_geom) {
            acis_exception v146(0, (error_info_base*)0);
            exception_save v137;
            curve* old_crv = nullptr;
            v137.begin();
            get_error_mark().buffer_init = 1;
            if(this_edge->geometry()) {
                if(GET_ALGORITHMIC_VERSION() >= AcisVersion(10, 0, 4)) {
                    bool v39 = FALSE;
                    if(this_edge->geometry()->equation().periodic() == 1) {
                        if(this_edge->start() != this_edge->end()) {
                            v39 = TRUE;
                        }
                    }
                    if(!this_edge->geometry()->equation().periodic() || v39) {
                        SPAinterval ed_range;
                        if(this_edge->sense()) {
                            ed_range = -(this_edge->param_range());
                        } else {
                            ed_range = this_edge->param_range();
                        }
                        old_crv = this_edge->geometry()->trans_curve();
                        old_crv->limit(ed_range);
                        if(this_pcurve) {
                            if(!is_TCOEDGE(this_coedge)) {
                                pcu_geom.trim(ed_range);
                            }
                        }
                    }
                }
            }
            if(old_crv) {
                if(GET_ALGORITHMIC_VERSION() < AcisVersion(21, 0, 0)) {
                    const surface* offs_sf1 = nullptr;
                    if(offset_surf) {
                        offs_sf1 = (const surface*)&(offset_surf->equation());
                    }
                    double sf2_offset = 0.0;
                    pcurve* pcur1 = nullptr;
                    if(this_pcurve) {
                        pcur1 = &pcu_geom;
                    }
                    const surface* base_sf1 = (const surface*)&(original_surf->equation());
                    offset_geom = make_offset_curve(base_sf1, old_crv, offset_distance, pcur1, NULL, sf2_offset, NULL, offs_sf1, NULL, NULL);
                } else {
                    surface* offs_sf1 = nullptr;
                    if(offset_surf) {
                        offs_sf1 = (surface*)&(offset_surf->equation());
                    }
                    double sf2_offset = 0.0;
                    pcurve* pcur1 = nullptr;
                    if(this_pcurve) {
                        pcur1 = &pcu_geom;
                    }
                    surface* base_sf1 = (surface*)&(original_surf->equation());
                    offset_geom = make_offset_curve_internal(base_sf1, old_crv, offset_distance, pcur1, NULL, sf2_offset, NULL, offs_sf1, NULL, this_coedge, SpaAcis::NullObj::get_logical(), 0, *approx_offset, 0, 0);
                }
                if(!offset_geom) {
                    double end_param = old_crv->param_range().end_pt();
                    double start_param = old_crv->param_range().start_pt();
                    if(this_pcurve)
                        offset_geom = sg_offset_surface_curve(old_crv, &pcu_geom, (const surface*)&(original_surf->equation()), (const surface*)&(offset_surf->equation()), start_param, end_param, offset_distance);
                    else
                        offset_geom = sg_offset_surface_curve(old_crv, NULL, (const surface*)&(original_surf->equation()), (const surface*)&(offset_surf->equation()), start_param, end_param, offset_distance);
                }
                if(offset_geom) {
                    offset_geom->unlimit();
                }
            } else {
                if(GET_ALGORITHMIC_VERSION() < AcisVersion(21, 0, 0)) {
                    const surface* offs_sf1 = nullptr;
                    if(offset_surf) {
                        offs_sf1 = (const surface*)&(offset_surf->equation());
                    }
                    double sf2_offset = 0.0;
                    pcurve* pcur1 = nullptr;
                    if(this_pcurve) {
                        pcur1 = &pcu_geom;
                    }
                    const curve* base_cu = nullptr;
                    if(this_edge->geometry()) {
                        base_cu = (const curve*)&(this_edge->geometry()->equation());
                    }
                    const surface* base_sf1 = (const surface*)&(original_surf->equation());
                    offset_geom = make_offset_curve(base_sf1, base_cu, offset_distance, pcur1, NULL, sf2_offset, NULL, offs_sf1, NULL, NULL);
                } else {
                    surface* offs_sf1 = nullptr;
                    if(offset_surf) {
                        offs_sf1 = (surface*)&(offset_surf->equation());
                    }
                    double sf2_offset = 0.0;
                    pcurve* pcur1 = nullptr;
                    if(this_pcurve) {
                        pcur1 = &pcu_geom;
                    }
                    curve* base_cu = nullptr;
                    if(this_edge->geometry()) {
                        base_cu = (curve*)&(this_edge->geometry()->equation());
                    }
                    surface* base_sf1 = (surface*)&(original_surf->equation());
                    offset_geom = make_offset_curve_internal(base_sf1, base_cu, offset_distance, pcur1, NULL, sf2_offset, NULL, offs_sf1, NULL, NULL, SpaAcis::NullObj::get_logical(), 0, *approx_offset, 0, 0);
                }
            }
            if(old_crv) {
                ACIS_DELETE old_crv;
            }
            // v137.~exception_save();
            if(acis_interrupted()) {
                sys_error(0, (error_info_base*)0);
            }
            // v146.~acis_exception();
        }
    }
}

surface* offset_plane(plane* original_plane, double offset_distance) {
    plane* offs_plane = nullptr;
    SPAunit_vector plane_normal(original_plane->normal);
    SPAposition offset_root = original_plane->root_point + (plane_normal * offset_distance);
    offs_plane = ACIS_NEW plane(offset_root, plane_normal);
    offs_plane->u_deriv = original_plane->u_deriv;
    return offs_plane;
}

surface* offset_sphere(sphere* original_sphere, double offset_distance, error_info*& err) {
    sphere* offs_sphere = nullptr;
    double new_radius = original_sphere->radius + offset_distance;
    if(fabs(new_radius) <= SPAresabs || new_radius * original_sphere->radius <= 0.0) {
        SPApar_pos uv_fail(0.0, 0.0);
        if(err) {
            err = ACIS_NEW curvature_error_info(&uv_fail, 1, offset_distance, (ENTITY*)0, original_sphere->radius);
        }
    } else {
        offs_sphere = ACIS_NEW sphere(original_sphere->centre, new_radius);
        offs_sphere->uv_oridir = original_sphere->uv_oridir;
        offs_sphere->pole_dir = original_sphere->pole_dir;
    }
    return offs_sphere;
}

surface* offset_surface(surface* original_surface, SPAbox& region_of_interest, double offset_distance, int& part_inv, offset_surface_options* off_sur_opts, error_info*& err, int* remake_face, SPApar_box& in_par_box, FACE* in_face, int* did_adaptive,
                        FACE* orig_face) {
    const surface* v12;          // rax
    const AcisVersion* v13;      // rax
    base_surface_law_data* v14;  // rax
    law* v15;                    // rax
    law* v16;                    // rax
    law* v17;                    // rax
    long double v18;             // xmm0_8
    long double v19;             // xmm0_8
    law* v20;                    // rax
    long double v21;             // xmm0_8
    long double v22;             // xmm0_8
    law* v23;                    // rax
    law* v24;                    // rax
    error_info* v25;             // rax
    const AcisVersion* v26;      // rax
    long double v27;             // xmm0_8
    error_info_base* v28;        // rax
    int v29;                     // eax
    __int64 v30;                 // [rsp+58h] [rbp-4D0h]
    int i;                       // [rsp+68h] [rbp-4C0h]

    int compute_curvature;  // [rsp+7Ch] [rbp-4ACh]
    FACE* err_ent;          // [rsp+80h] [rbp-4A8h]
    long double rad_crv;    // [rsp+88h] [rbp-4A0h] BYREF
    error_info* err_info;   // [rsp+90h] [rbp-498h]
    AcisVersion cav;        // [rsp+98h] [rbp-490h] BYREF
    // aux_data_set* data_set;                                                           // [rsp+A0h] [rbp-488h] BYREF
    // aux_data_manager data_mgr;                                                        // [rsp+A8h] [rbp-480h] BYREF
    long double curvature;               // [rsp+B0h] [rbp-478h] BYREF
    base_surface_law_data* ld_law_data;  // [rsp+B8h] [rbp-470h]
    law* cur_law1;                       // [rsp+C0h] [rbp-468h]
    law* cur_law;                        // [rsp+C8h] [rbp-460h]
    long double min_curvature;           // [rsp+D0h] [rbp-458h]

    int v46;  // [rsp+DCh] [rbp-44Ch]
    // error_collator::complexity_source src;                                            // [rsp+E0h] [rbp-448h] BYREF
    law* dom_law10;                                                                   // [rsp+E8h] [rbp-440h]
    law* dom_law;                                                                     // [rsp+F0h] [rbp-438h]
    ENTITY_LIST err_ents;                                                             // [rsp+F8h] [rbp-430h] BYREF
    error_info_base* error_info_base_ptr;                                             // [rsp+100h] [rbp-428h]
    law* dom_law22;                                                                   // [rsp+108h] [rbp-420h]
    law* dom_law21;                                                                   // [rsp+110h] [rbp-418h]
    law* dom_law12;                                                                   // [rsp+118h] [rbp-410h]
    law* dom_law11;                                                                   // [rsp+120h] [rbp-408h]
    law* dom_law20;                                                                   // [rsp+128h] [rbp-400h]
    curvature_error_info* v57;                                                        // [rsp+130h] [rbp-3F8h]
    error_info* v58;                                                                  // [rsp+138h] [rbp-3F0h]
    SPApar_pos min_uv;                                                                // [rsp+140h] [rbp-3E8h] BYREF
    SURFACE* v60;                                                                     // [rsp+150h] [rbp-3D8h]
    SURFACE* v61;                                                                     // [rsp+158h] [rbp-3D0h]
    SURFACE* v62;                                                                     // [rsp+160h] [rbp-3C8h]
    const surface* v63;                                                               // [rsp+168h] [rbp-3C0h]
    SURFACE* v64;                                                                     // [rsp+170h] [rbp-3B8h]
    const surface* v65;                                                               // [rsp+178h] [rbp-3B0h]
    SURFACE* v66;                                                                     // [rsp+180h] [rbp-3A8h]
    surface_law_data* v67;                                                            // [rsp+188h] [rbp-3A0h]
    base_surface_law_data* v68;                                                       // [rsp+190h] [rbp-398h]
    min_curvature_law* v69;                                                           // [rsp+198h] [rbp-390h]
    law* v70;                                                                         // [rsp+1A0h] [rbp-388h]
    max_curvature_law* v71;                                                           // [rsp+1A8h] [rbp-380h]
    law* v72;                                                                         // [rsp+1B0h] [rbp-378h]
    negate_law* v73;                                                                  // [rsp+1B8h] [rbp-370h]
    law* v74;                                                                         // [rsp+1C0h] [rbp-368h]
    and_law* v75;                                                                     // [rsp+1C8h] [rbp-360h]
    law* v76;                                                                         // [rsp+1D0h] [rbp-358h]
    and_law* v77;                                                                     // [rsp+1D8h] [rbp-350h]
    law* v78;                                                                         // [rsp+1E0h] [rbp-348h]
    and_law* v79;                                                                     // [rsp+1E8h] [rbp-340h]
    law* v80;                                                                         // [rsp+1F0h] [rbp-338h]
    curvature_error_info* curvature_err;                                              // [rsp+1F8h] [rbp-330h]
    long double stopping_value;                                                       // [rsp+200h] [rbp-328h]
    curvature_error_info* v83;                                                        // [rsp+208h] [rbp-320h]
    error_info* v84;                                                                  // [rsp+210h] [rbp-318h]
    COPY_ANNOTATION* ann;                                                             // [rsp+218h] [rbp-310h]
    FACE* temp_fc;                                                                    // [rsp+220h] [rbp-308h]
    SPAnvector res;                                                                   // [rsp+228h] [rbp-300h] BYREF
    AcisVersion v88;                                                                  // [rsp+238h] [rbp-2F0h] BYREF
    AcisVersion v89;                                                                  // [rsp+23Ch] [rbp-2ECh] BYREF
    void(__fastcall * v90)(__int64, SPAinterval*, SPAbox*);                           // [rsp+240h] [rbp-2E8h]
    SPAbox* v91;                                                                      // [rsp+248h] [rbp-2E0h]
    void(__fastcall * v92)(__int64, SPAinterval*, SPAbox*);                           // [rsp+250h] [rbp-2D8h]
    long double started;                                                              // [rsp+258h] [rbp-2D0h]
    unsigned int(__fastcall * v94)(__int64);                                          // [rsp+260h] [rbp-2C8h]
    long double v95;                                                                  // [rsp+268h] [rbp-2C0h]
    unsigned int(__fastcall * v96)(__int64);                                          // [rsp+270h] [rbp-2B8h]
    long double v97;                                                                  // [rsp+278h] [rbp-2B0h]
    unsigned int(__fastcall * v98)(__int64);                                          // [rsp+280h] [rbp-2A8h]
    long double v99;                                                                  // [rsp+288h] [rbp-2A0h]
    unsigned int(__fastcall * v100)(__int64);                                         // [rsp+290h] [rbp-298h]
    SPAbox* v101;                                                                     // [rsp+298h] [rbp-290h]
    SPAinterval*(__fastcall * param_range_u)(surface*, SPAinterval*, const SPAbox*);  // [rsp+2A0h] [rbp-288h]
    SPAbox* v103;                                                                     // [rsp+2A8h] [rbp-280h]
    SPAinterval*(__fastcall * param_range_v)(surface*, SPAinterval*, const SPAbox*);  // [rsp+2B0h] [rbp-278h]
    surface* in_acis_surface;                                                         // [rsp+2B8h] [rbp-270h]
    base_surface_law_data* v106;                                                      // [rsp+2C0h] [rbp-268h]
    law* v107;                                                                        // [rsp+2C8h] [rbp-260h]
    law* v108;                                                                        // [rsp+2D0h] [rbp-258h]
    law* v109;                                                                        // [rsp+2D8h] [rbp-250h]
    long double vval;                                                                 // [rsp+2E0h] [rbp-248h]
    long double uval;                                                                 // [rsp+2E8h] [rbp-240h]
    law* v112;                                                                        // [rsp+2F0h] [rbp-238h]
    law* v113;                                                                        // [rsp+2F8h] [rbp-230h]
    law* v114;                                                                        // [rsp+300h] [rbp-228h]
    const SPApar_pos* err_uv_param;                                                   // [rsp+308h] [rbp-220h]
    SPAnvector* v116;                                                                 // [rsp+310h] [rbp-218h]
    SPAnvector* nv;                                                                   // [rsp+318h] [rbp-210h]
    SPAparameter v118;                                                                // [rsp+320h] [rbp-208h] BYREF
    SPAparameter v119;                                                                // [rsp+328h] [rbp-200h] BYREF
    error_info* v120;                                                                 // [rsp+330h] [rbp-1F8h]
    double v121;                                                                      // [rsp+338h] [rbp-1F0h]
    ENTITY* ent;                                                                      // [rsp+340h] [rbp-1E8h]
    ATTRIB_TAG* oet;                                                                  // [rsp+348h] [rbp-1E0h]
    error_info* v124;                                                                 // [rsp+350h] [rbp-1D8h]
    curvature_error_info* v125;                                                       // [rsp+358h] [rbp-1D0h]
    long double elem;                                                                 // [rsp+360h] [rbp-1C8h] BYREF
    // error_collator* v127;                                                             // [rsp+368h] [rbp-1C0h]
    exception_save exception_save_mark;  // [rsp+370h] [rbp-1B8h] BYREF
    SPAbox* box;                         // [rsp+380h] [rbp-1A8h]

    SPAnvector dxy;        // [rsp+390h] [rbp-198h] BYREF
    SPAinterval v_range;   // [rsp+3A0h] [rbp-188h] BYREF
    SPAinterval u_range;   // [rsp+3B8h] [rbp-170h] BYREF
    SPAnvector nuv;        // [rsp+3D0h] [rbp-158h] BYREF
    SPAnvector nuv_step;   // [rsp+3E0h] [rbp-148h] BYREF
    error_info_list v136;  // [rsp+3F0h] [rbp-138h] BYREF
    SPApar_pos uv_step;    // [rsp+400h] [rbp-128h] BYREF
    SPApar_pos v138;       // [rsp+410h] [rbp-118h] BYREF
    SPApar_pos v139;       // [rsp+420h] [rbp-108h] BYREF
    SPAnvector v140;       // [rsp+430h] [rbp-F8h] BYREF
    SPAnvector result;     // [rsp+440h] [rbp-E8h] BYREF
    SPAinterval range_u;   // [rsp+450h] [rbp-D8h] BYREF
    SPAinterval range_v;   // [rsp+468h] [rbp-C0h] BYREF
    // acis_exception error_info_holder;                                                 // [rsp+480h] [rbp-A8h] BYREF

    offset_surface_options def_opts;
    if(!off_sur_opts) {
        off_sur_opts = &def_opts;
    }
    if(!original_surface) {
        return nullptr;
    }
    if(offset_distance == 0.0) {
        return original_surface->copy_surf();
    }
    int surf_type = original_surface->type();
    int force_error = 0;
    surface* offset_surfacea = nullptr;
    switch(surf_type) {
        case plane_type:
            offset_surfacea = offset_plane((plane*)original_surface, offset_distance);
            break;
        case cone_type:
            // offset_surfacea = offset_cone((const cone*)original_surface, region_of_interest, offset_distance, err, in_face, 1, did_adaptive);
            break;
        case sphere_type:
            offset_surfacea = offset_sphere((sphere*)original_surface, offset_distance, err);
            break;
        case torus_type:
            break;
        case spline_type:
            break;
        default:
            offset_surfacea = nullptr;
            break;
    }
    return offset_surfacea;
}

surface* standard_offsetter::offset_surface_tweak(FACE* fa, SPAbox& region_of_interest, double offset_distance, error_info*& err) {
    return nullptr;
}

int standard_offsetter::check_collapsing_face(FACE* face, double face_offset_dist, ENTITY_LIST& collapsedFaceList, int dummy) {
    return NULL;
}

int standard_offsetter::is_reparameterized() {
    return NULL;
}

int standard_offsetter::did_remake_face() {
    return NULL;
}

int standard_offsetter::is_part_inv() {
    return NULL;
}

int standard_offsetter::need_par_int_curs() {
    return NULL;
}
