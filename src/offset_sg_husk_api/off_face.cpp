#include "PrivateInterfaces/ofst_jour.hxx"
#include "ProtectedInterfaces/ofst_api_prot.hxx"
#include "ProtectedInterfaces/sgoffrtn_prot.hxx"
#include "PublicInterfaces/gme_ofstapi.hxx"
#include "acis/DSYWarningManager.h"
#include "acis/SPA_approx_options.hxx"
#include "acis/SPA_edge_line_arc_options.hxx"
#include "acis/acistype.hxx"
#include "acis/add_pcu.hxx"
#include "acis/api.err"
#include "acis/box.hxx"
#include "acis/check.hxx"
#include "acis/ckoutcom.hxx"
#include "acis/cucuint.hxx"
#include "acis/err_info.hxx"
#include "acis/errmsg.hxx"
#include "acis/exct_int.hxx"
#include "acis/faceutil.hxx"
#include "acis/importexport.h"
#include "acis/intdef.hxx"
#include "acis/interval.hxx"
#include "acis/kernopts.hxx"
#include "acis/law.hxx"
#include "acis/lists_iterator.hxx"
#include "acis/method.hxx"
#include "acis/module.hxx"
#include "acis/rm_pcu.hxx"
#include "acis/rot_spl.hxx"
#include "acis/sp2crtn.hxx"
#include "acis/spa_null_base.hxx"
#include "acis/spldef.hxx"
#include "acis/sur.hxx"
#include "acis/sur_off.err"
#include "acis/transfrm.hxx"
#include "acis/unitvec.hxx"
#include "acis/vlists.hxx"


option_header iop_adaptive_ofst_surface("iop_adaptive_ofst_surface", 0);


// outcome api_offset_face(FACE* given_face, double offset_distance, FACE*& offset_face, AcisOptions* ao) {
//     return api_offset_face(given_face, offset_distance, offset_face, NULL, ao);
// }

void lop_scale_offset(BODY*& body, double& offset) {
    TRANSFORM* BodyT = body->transform();
    if(BodyT) offset = offset / (BodyT->transform().scaling());
}

// outcome ipi_offset_surface(surface* given_surface, double offset_distance, surface*& offset_surface, offset_options* pOffOpts, AcisOptions* ao) {
//     __int64 v6;                            // rax
//     FACE* progenitor_face;                 // [rsp+40h] [rbp-138h] BYREF
//     int v9;                                // [rsp+48h] [rbp-130h]
//     error_info_base* error_info_base_ptr;  // [rsp+50h] [rbp-128h]
//     int call_update_from_bb;               // [rsp+58h] [rbp-120h]
//     acis_version_span _avs;                // [rsp+5Ch] [rbp-11Ch] BYREF
//     AcisVersion* av;                       // [rsp+60h] [rbp-118h]
//     outcome* v14;                          // [rsp+68h] [rbp-110h]
//     outcome* o;                            // [rsp+70h] [rbp-108h]
//     exception_save exception_save_mark;    // [rsp+88h] [rbp-F0h] BYREF
//     problems_list_prop problems_prop;      // [rsp+98h] [rbp-E0h] BYREF
//     __int64 v18;                           // [rsp+A8h] [rbp-D0h]
//     acis_exception error_info_holder;      // [rsp+B0h] [rbp-C8h] BYREF
//     api_bb_save make_bulletin_board;       // [rsp+110h] [rbp-68h] BYREF
//     outcome v21;                           // [rsp+128h] [rbp-50h] BYREF
//
//     set_global_error_info(0i64);
//     outcome::outcome(result, 0, 0i64);
//     v18 = v6;
//     v9 = 1;
//     problems_list_prop::problems_list_prop(&problems_prop);
//     acis_exception::acis_exception(&error_info_holder, 0, 0i64, 0i64, 0);
//     error_info_base_ptr = 0i64;
//     exception_save::exception_save(&exception_save_mark);
//     api_bb_save::api_bb_save(&make_bulletin_board, result, normal);
//     exception_save::begin(&exception_save_mark);
//     get_error_mark()->buffer_init = 1;
//     ACISExceptionCheck("API");
//     call_update_from_bb = 1;
//     if(ao)
//         av = ao->get_version(ao);
//     else
//         av = 0i64;
//     acis_version_span::acis_version_span(&_avs, av);
//     progenitor_face = 0i64;
//     sg_make_face_from_surface(given_surface, &progenitor_face, 0);
//     if(ao && AcisOptions::journal_on(ao)) J_ipi_offset_surface(progenitor_face, offset_distance, pOffOpts, ao);
//     v14 = offset_surface_internal(&v21, given_surface, offset_distance, offset_surface, pOffOpts, progenitor_face);
//     o = v14;
//     outcome::operator=(result, v14);
//     outcome::~outcome(&v21);
//     if(progenitor_face) {
//         delete_entity(progenitor_face);
//         progenitor_face = 0i64;
//     }
//     if(outcome::ok(result) && call_update_from_bb) update_from_bb();
//     acis_version_span::~acis_version_span(&_avs);
//     api_bb_save::~api_bb_save(&make_bulletin_board);
//     exception_save::~exception_save(&exception_save_mark);
//     if(acis_interrupted()) sys_error(0, error_info_base_ptr);
//     acis_exception::~acis_exception(&error_info_holder);
//     outcome::error_number(result);
//     problems_list_prop::process_result(&problems_prop, result, PROBLEMS_LIST_PROP_ONLY, 0);
//     problems_list_prop::~problems_list_prop(&problems_prop);
//     return result;
// }

// outcome ipi_offset_surface_map_bs2curves(surface* given_surface, surface* offset_surface, double offset_distance, int num_curves, bs2_curve_def** given_bs2_curves, bs2_curve_def**& offset_bs2_curves, AcisOptions* ao) {
//     __int64 v8;                                                                             // rax
//     intcurve* v9;                                                                           // rax
//     const SPAinterval* v10;                                                                 // rax
//     long double v11;                                                                        // xmm0_8
//     long double v12;                                                                        // xmm0_8
//     const SPAinterval* v13;                                                                 // rax
//     int v14;                                                                                // eax
//     outcome* v15;                                                                           // rax
//     bool adaptive_ofst;                                                                     // [rsp+50h] [rbp-418h]
//     int jj;                                                                                 // [rsp+54h] [rbp-414h]
//     curve** curves_on_surface;                                                              // [rsp+60h] [rbp-408h] BYREF
//     int err_num;                                                                            // [rsp+68h] [rbp-400h]
//     int v21;                                                                                // [rsp+6Ch] [rbp-3FCh]
//     int ii;                                                                                 // [rsp+70h] [rbp-3F8h]
//     int v23;                                                                                // [rsp+74h] [rbp-3F4h]
//     int v24;                                                                                // [rsp+78h] [rbp-3F0h]
//     int resignal_no;                                                                        // [rsp+7Ch] [rbp-3ECh]
//     intcurve* in_curve;                                                                     // [rsp+80h] [rbp-3E8h]
//     int num_proj_curves;                                                                    // [rsp+88h] [rbp-3E0h] BYREF
//     int error_num;                                                                          // [rsp+8Ch] [rbp-3DCh]
//     int v29;                                                                                // [rsp+90h] [rbp-3D8h]
//     BOOL v30;                                                                               // [rsp+94h] [rbp-3D4h]
//     BOOL v31;                                                                               // [rsp+98h] [rbp-3D0h]
//     BOOL v32;                                                                               // [rsp+9Ch] [rbp-3CCh]
//     surface* out_surf;                                                                      // [rsp+A0h] [rbp-3C8h] BYREF
//     error_info_base* error_info_base_ptr;                                                   // [rsp+A8h] [rbp-3C0h]
//     surface* extended_surf;                                                                 // [rsp+B0h] [rbp-3B8h] BYREF
//     int call_update_from_bb;                                                                // [rsp+B8h] [rbp-3B0h]
//     acis_version_span _avs;                                                                 // [rsp+BCh] [rbp-3ACh] BYREF
//     surface* v38;                                                                           // [rsp+C0h] [rbp-3A8h]
//     error_info_base* e_info;                                                                // [rsp+C8h] [rbp-3A0h]
//     surface* v40;                                                                           // [rsp+D0h] [rbp-398h]
//     error_info_base* v41;                                                                   // [rsp+D8h] [rbp-390h]
//     curve* v42;                                                                             // [rsp+E0h] [rbp-388h]
//     intcurve* v43;                                                                          // [rsp+E8h] [rbp-380h]
//     AcisVersion* av;                                                                        // [rsp+F0h] [rbp-378h]
//     const off_spl_sur* off_spl;                                                             // [rsp+F8h] [rbp-370h]
//     bs2_curve_def* this_bs2;                                                                // [rsp+100h] [rbp-368h]
//     intcurve* v47;                                                                          // [rsp+108h] [rbp-360h]
//     intcurve* v48;                                                                          // [rsp+110h] [rbp-358h]
//     SPA_internal_approx_options* internal_approxOpts;                                       // [rsp+118h] [rbp-350h]
//     bs2_curve_def* new_bs2;                                                                 // [rsp+120h] [rbp-348h] BYREF
//     SPA_internal_approx_options* approxOpts;                                                // [rsp+128h] [rbp-340h]
//     bs2_curve_def* coedge_bs2_approx_result;                                                // [rsp+130h] [rbp-338h] BYREF
//     SPA_COEDGE_approx_options coed_approxOpts;                                              // [rsp+138h] [rbp-330h] BYREF
//     SPA_COEDGE_approx_options v54;                                                          // [rsp+148h] [rbp-320h] BYREF
//     const spl_sur* offset_prog_spl;                                                         // [rsp+158h] [rbp-310h]
//     SPAinterval* safe_range;                                                                // [rsp+160h] [rbp-308h]
//     surface* s2;                                                                            // [rsp+168h] [rbp-300h]
//     intcurve* v58;                                                                          // [rsp+170h] [rbp-2F8h]
//     SPAbox* box;                                                                            // [rsp+178h] [rbp-2F0h]
//     SPAinterval*(__fastcall * param_range)(struct intcurve*, SPAinterval*, const SPAbox*);  // [rsp+180h] [rbp-2E8h]
//     intcurve* v61;                                                                          // [rsp+188h] [rbp-2E0h]
//     bs2_curve_def* other_bs2;                                                               // [rsp+190h] [rbp-2D8h]
//     bs2_curve_def* v63;                                                                     // [rsp+198h] [rbp-2D0h]
//     SPAbox* v64;                                                                            // [rsp+1A0h] [rbp-2C8h]
//     curve* v65;                                                                             // [rsp+1A8h] [rbp-2C0h]
//     SPAinterval*(__fastcall * v66)(curve*, SPAinterval*, const SPAbox*);                    // [rsp+1B0h] [rbp-2B8h]
//     SPAinterval* in_domain;                                                                 // [rsp+1B8h] [rbp-2B0h]
//     curve* v68;                                                                             // [rsp+1C0h] [rbp-2A8h]
//     void(__fastcall * v69)(surface*);                                                       // [rsp+1C8h] [rbp-2A0h]
//     __int64 v70;                                                                            // [rsp+1D0h] [rbp-298h]
//     SPAbox* v71;                                                                            // [rsp+1D8h] [rbp-290h]
//     SPAinterval*(__fastcall * v72)(struct intcurve*, SPAinterval*, const SPAbox*);          // [rsp+1E0h] [rbp-288h]
//     void(__fastcall * v73)(surface*);                                                       // [rsp+1E8h] [rbp-280h]
//     __int64 v74;                                                                            // [rsp+1F0h] [rbp-278h]
//     outcome* v75;                                                                           // [rsp+1F8h] [rbp-270h]
//     outcome* o;                                                                             // [rsp+200h] [rbp-268h]
//     void(__fastcall * v77)(curve*);                                                         // [rsp+208h] [rbp-260h]
//     __int64 v78;                                                                            // [rsp+210h] [rbp-258h]
//     void* alloc_ptr;                                                                        // [rsp+218h] [rbp-250h]
//     void(__fastcall * v80)(struct intcurve*);                                               // [rsp+220h] [rbp-248h]
//     __int64 v81;                                                                            // [rsp+228h] [rbp-240h]
//     exception_save v82;                                                                     // [rsp+240h] [rbp-228h] BYREF
//     exception_save v83;                                                                     // [rsp+250h] [rbp-218h] BYREF
//     exception_save exception_save_mark;                                                     // [rsp+260h] [rbp-208h] BYREF
//     const surface* progenitor_sur;                                                          // [rsp+270h] [rbp-1F8h]
//     const spl_sur* original_prog_spl;                                                       // [rsp+278h] [rbp-1F0h]
//     problems_list_prop problems_prop;                                                       // [rsp+280h] [rbp-1E8h] BYREF
//     __int64 v88;                                                                            // [rsp+290h] [rbp-1D8h]
//     acis_exception v89;                                                                     // [rsp+298h] [rbp-1D0h] BYREF
//     acis_exception v90;                                                                     // [rsp+2B8h] [rbp-1B0h] BYREF
//     acis_exception error_info_holder;                                                       // [rsp+2D8h] [rbp-190h] BYREF
//     api_bb_save make_bulletin_board;                                                        // [rsp+3B8h] [rbp-B0h] BYREF
//     outcome v93;                                                                            // [rsp+3F0h] [rbp-78h] BYREF
//     SPAinterval v94;                                                                        // [rsp+410h] [rbp-58h] BYREF
//     SPAinterval v95;                                                                        // [rsp+428h] [rbp-40h] BYREF
//     char v96;                                                                               // [rsp+440h] [rbp-28h] BYREF
//
//     set_global_error_info(0i64);
//     outcome::outcome(result, 0, 0i64);
//     v88 = v8;
//     v29 = 1;
//     error_num = 0;
//     problems_list_prop::problems_list_prop(&problems_prop);
//     resignal_no = 0;
//     acis_exception::acis_exception(&error_info_holder, 0, 0i64, 0i64, 0);
//     error_info_base_ptr = 0i64;
//     exception_save::exception_save(&exception_save_mark);
//     api_bb_save::api_bb_save(&make_bulletin_board, result, normal);
//     exception_save::begin(&exception_save_mark);
//     get_error_mark()->buffer_init = 1;
//     ACISExceptionCheck("API");
//     call_update_from_bb = 1;
//     if(ao)
//         av = ao->get_version(ao);
//     else
//         av = 0i64;
//     acis_version_span::acis_version_span(&_avs, av);
//     if(ao && AcisOptions::journal_on(ao)) J_ipi_offset_surface_map_bs2curves(given_surface, offset_surface, offset_distance, num_curves, given_bs2_curves, ao);
//     adaptive_ofst = 0;
//     if(SUR_is_offset(offset_surface)) {
//         off_spl = (const off_spl_sur*)spline::get_spl_sur(offset_surface);
//         v30 = off_spl_sur::get_did_adaptive_offset((off_spl_sur*)off_spl) == 1;
//         adaptive_ofst = v30;
//         if(!v30) {
//             progenitor_sur = off_spl_sur::get_progenitor((off_spl_sur*)off_spl);
//             offset_prog_spl = spline::get_spl_sur((spline*)progenitor_sur);
//             original_prog_spl = spline::get_spl_sur(given_surface);
//             v31 = subtype_object::operator!=(&offset_prog_spl->subtrans_object, original_prog_spl);
//             adaptive_ofst = v31;
//         }
//     }
//     for(jj = 0; jj < num_curves; ++jj) {
//         this_bs2 = bs2_curve_copy(given_bs2_curves[jj]);
//         if(adaptive_ofst) {
//             v47 = (intcurve*)ACIS_OBJECT::operator new(0x68ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_api.m\\src\\off_face.cpp", 363, &alloc_file_index_3483);
//             if(v47) {
//                 safe_range = SpaAcis::NullObj::get_interval();
//                 s2 = SpaAcis::NullObj::get_surface();
//                 intcurve::intcurve(v47, 0i64, -1.0, given_surface, s2, this_bs2, 0i64, safe_range, 1, 0);
//                 v48 = v9;
//             } else {
//                 v48 = 0i64;
//             }
//             v58 = v48;
//             in_curve = v48;
//             if(v48) {
//                 curves_on_surface = 0i64;
//                 num_proj_curves = 0;
//                 param_range = in_curve->param_range;
//                 box = SpaAcis::NullObj::get_box();
//                 v10 = param_range(in_curve, &v94, box);
//                 v32 = project_curve_to_surface(in_curve, v10, offset_surface, &curves_on_surface, &num_proj_curves, 0, 0, 0i64, 0) == 1;
//                 if(v32 && num_proj_curves == 1 && !CUR_is_degenerate(*curves_on_surface)) {
//                     if(CUR_is_proj_intcurve(*curves_on_surface) || CUR_is_par_intcurve(*curves_on_surface)) {
//                         v61 = (intcurve*)*curves_on_surface;
//                         other_bs2 = intcurve::pcur1(v61, 0);
//                         v63 = bs2_curve_copy(other_bs2);
//                         (*offset_bs2_curves)[jj] = v63;
//                     } else {
//                         err_num = 0;
//                         acis_exception::acis_exception(&v89, 0, 0i64, 0i64, 0);
//                         e_info = 0i64;
//                         exception_save::exception_save(&v82);
//                         extended_surf = 0i64;
//                         SPA_COEDGE_approx_options::SPA_COEDGE_approx_options(&coed_approxOpts);
//                         SPA_COEDGE_approx_options::set_extend_surface(&coed_approxOpts, 0);
//                         internal_approxOpts = SPA_approx_options::get_impl(&coed_approxOpts);
//                         SPA_internal_approx_options::set_uDegree(internal_approxOpts, SPA_approx_degree_QUINTIC);
//                         v11 = safe_function_type<double>::operator double(&SPAresfit);
//                         SPA_approx_options::set_requested_tol(&coed_approxOpts, v11);
//                         new_bs2 = 0i64;
//                         exception_save::begin(&v82);
//                         get_error_mark()->buffer_init = 1;
//                         v23 = 0;
//                         v65 = *curves_on_surface;
//                         v66 = (*curves_on_surface)->param_range;
//                         v64 = SpaAcis::NullObj::get_box();
//                         in_domain = v66(v65, &v95, v64);
//                         v68 = *curves_on_surface;
//                         sg_approx(v68, offset_surface, in_domain, &new_bs2, &extended_surf, internal_approxOpts);
//                         (*offset_bs2_curves)[jj] = new_bs2;
//                         if(extended_surf) {
//                             v38 = extended_surf;
//                             v69 = extended_surf->~surface;
//                             v70 = ((__int64(__fastcall*)(surface*, __int64))v69)(extended_surf, 1i64);
//                             extended_surf = 0i64;
//                         }
//                         SPA_COEDGE_approx_options::~SPA_COEDGE_approx_options(&coed_approxOpts);
//                         exception_save::~exception_save(&v82);
//                         if(err_num || acis_interrupted()) sys_error(err_num, e_info);
//                         acis_exception::~acis_exception(&v89);
//                     }
//                 } else {
//                     v21 = 0;
//                     acis_exception::acis_exception(&v90, 0, 0i64, 0i64, 0);
//                     v41 = 0i64;
//                     exception_save::exception_save(&v83);
//                     out_surf = 0i64;
//                     SPA_COEDGE_approx_options::SPA_COEDGE_approx_options(&v54);
//                     SPA_COEDGE_approx_options::set_extend_surface(&v54, 0);
//                     approxOpts = SPA_approx_options::get_impl(&v54);
//                     SPA_internal_approx_options::set_uDegree(approxOpts, SPA_approx_degree_QUINTIC);
//                     v12 = safe_function_type<double>::operator double(&SPAresfit);
//                     SPA_approx_options::set_requested_tol(&v54, v12);
//                     coedge_bs2_approx_result = 0i64;
//                     exception_save::begin(&v83);
//                     get_error_mark()->buffer_init = 1;
//                     v24 = 0;
//                     v72 = in_curve->param_range;
//                     v71 = SpaAcis::NullObj::get_box();
//                     v13 = v72(in_curve, (SPAinterval*)&v96, v71);
//                     sg_approx(in_curve, offset_surface, v13, &coedge_bs2_approx_result, &out_surf, approxOpts);
//                     (*offset_bs2_curves)[jj] = coedge_bs2_approx_result;
//                     if(out_surf) {
//                         v40 = out_surf;
//                         v73 = out_surf->~surface;
//                         v74 = ((__int64(__fastcall*)(surface*, __int64))v73)(out_surf, 1i64);
//                         out_surf = 0i64;
//                     }
//                     SPA_COEDGE_approx_options::~SPA_COEDGE_approx_options(&v54);
//                     exception_save::~exception_save(&v83);
//                     if(v21 || acis_interrupted()) sys_error(v21, v41);
//                     acis_exception::~acis_exception(&v90);
//                     if(!(*offset_bs2_curves)[jj]) {
//                         v14 = message_module::message_code(&spaacisds_api_errmod, 0);
//                         outcome::outcome(&v93, v14, 0i64);
//                         v75 = v15;
//                         o = v15;
//                         outcome::operator=(result, v15);
//                         outcome::~outcome(&v93);
//                     }
//                 }
//                 if(curves_on_surface) {
//                     for(ii = 0; ii < num_proj_curves; ++ii) {
//                         v42 = curves_on_surface[ii];
//                         if(v42) {
//                             v77 = v42->~curve;
//                             v78 = ((__int64(__fastcall*)(curve*, __int64))v77)(v42, 1i64);
//                         } else {
//                             v78 = 0i64;
//                         }
//                         curves_on_surface[ii] = 0i64;
//                     }
//                     alloc_ptr = curves_on_surface;
//                     ACIS_STD_TYPE_OBJECT::operator delete[](curves_on_surface);
//                     curves_on_surface = 0i64;
//                 }
//                 v43 = in_curve;
//                 if(in_curve) {
//                     v80 = v43->~intcurve;
//                     v81 = ((__int64(__fastcall*)(intcurve*, __int64))v80)(v43, 1i64);
//                 } else {
//                     v81 = 0i64;
//                 }
//                 in_curve = 0i64;
//             }
//         } else {
//             (*offset_bs2_curves)[jj] = this_bs2;
//         }
//     }
//     if(outcome::ok(result) && call_update_from_bb) update_from_bb();
//     acis_version_span::~acis_version_span(&_avs);
//     api_bb_save::~api_bb_save(&make_bulletin_board);
//     exception_save::~exception_save(&exception_save_mark);
//     if(acis_interrupted()) sys_error(resignal_no, error_info_base_ptr);
//     acis_exception::~acis_exception(&error_info_holder);
//     error_num = outcome::error_number(result);
//     problems_list_prop::process_result(&problems_prop, result, PROBLEMS_LIST_PROP_ONLY, 0);
//     problems_list_prop::~problems_list_prop(&problems_prop);
//     return result;
// }

// outcome ipi_initialize_offset_surface(surface* base_sf, SPApar_box& pbx, double offset_d, surface*& offset_sf, offset_options* ofst_opts, AcisOptions* ao) {
//     __int64 v7;                            // rax
//     AcisVersion* v8;                       // rax
//     off_spl_sur* v9;                       // rax
//     surface* v10;                          // rax
//     int use_adaptive_offset;               // [rsp+60h] [rbp-278h]
//     int failure_ignored;                   // [rsp+6Ch] [rbp-26Ch] BYREF
//     off_spl_sur* off_spl;                  // [rsp+70h] [rbp-268h]
//     error_info_base* error_info_base_ptr;  // [rsp+78h] [rbp-260h]
//     int proceed_on_failure;                // [rsp+84h] [rbp-254h]
//     int abort_on_illegal;                  // [rsp+88h] [rbp-250h]
//     int illegal_bispan_check;              // [rsp+8Ch] [rbp-24Ch]
//     int keep_approx;                       // [rsp+90h] [rbp-248h]
//     int full_surf;                         // [rsp+94h] [rbp-244h]
//     int call_update_from_bb;               // [rsp+98h] [rbp-240h]
//     acis_version_span _avs;                // [rsp+9Ch] [rbp-23Ch] BYREF
//     error_info_base* e_info;               // [rsp+A8h] [rbp-230h]
//     spline* v25;                           // [rsp+B0h] [rbp-228h]
//     surface* v26;                          // [rsp+B8h] [rbp-220h]
//     AcisVersion* av;                       // [rsp+C0h] [rbp-218h]
//     const SPApar_box* v28;                 // [rsp+C8h] [rbp-210h]
//     const SPApar_box* p_pbx;               // [rsp+D0h] [rbp-208h]
//     off_spl_sur* v30;                      // [rsp+D8h] [rbp-200h]
//     off_spl_sur* v31;                      // [rsp+E0h] [rbp-1F8h]
//     AcisVersion v32;                       // [rsp+E8h] [rbp-1F0h] BYREF
//     AcisVersion v33;                       // [rsp+ECh] [rbp-1ECh] BYREF
//     AcisVersion* vt2;                      // [rsp+F0h] [rbp-1E8h]
//     AcisVersion* vt1;                      // [rsp+F8h] [rbp-1E0h]
//     off_spl_sur* v36;                      // [rsp+100h] [rbp-1D8h]
//     surface* v37;                          // [rsp+108h] [rbp-1D0h]
//     spline* v38;                           // [rsp+110h] [rbp-1C8h]
//     long double reqd_fit;                  // [rsp+118h] [rbp-1C0h]
//     outcome* v40;                          // [rsp+130h] [rbp-1A8h]
//     outcome* o;                            // [rsp+138h] [rbp-1A0h]
//     exception_save v42;                    // [rsp+150h] [rbp-188h] BYREF
//     exception_save exception_save_mark;    // [rsp+160h] [rbp-178h] BYREF
//     problems_list_prop problems_prop;      // [rsp+170h] [rbp-168h] BYREF
//     __int64 v45;                           // [rsp+180h] [rbp-158h]
//     acis_exception v46;                    // [rsp+188h] [rbp-150h] BYREF
//     acis_exception error_info_holder;      // [rsp+1A8h] [rbp-130h] BYREF
//     api_bb_save make_bulletin_board;       // [rsp+248h] [rbp-90h] BYREF
//     outcome v49;                           // [rsp+260h] [rbp-78h] BYREF
//     SPApar_box trimed_box;                 // [rsp+2A0h] [rbp-38h] BYREF
//
//     set_global_error_info(0i64);
//     outcome::outcome(result, 0, 0i64);
//     v45 = v7;
//     problems_list_prop::problems_list_prop(&problems_prop);
//     acis_exception::acis_exception(&error_info_holder, 0, 0i64, 0i64, 0);
//     error_info_base_ptr = 0i64;
//     exception_save::exception_save(&exception_save_mark);
//     api_bb_save::api_bb_save(&make_bulletin_board, result, normal);
//     exception_save::begin(&exception_save_mark);
//     get_error_mark()->buffer_init = 1;
//     ACISExceptionCheck("API");
//     call_update_from_bb = 1;
//     if(ao)
//         av = ao->get_version(ao);
//     else
//         av = 0i64;
//     acis_version_span::acis_version_span(&_avs, av);
//     use_adaptive_offset = 0;
//     acis_exception::acis_exception(&v46, 0, 0i64, 0i64, 0);
//     e_info = 0i64;
//     exception_save::exception_save(&v42);
//     exception_save::begin(&v42);
//     get_error_mark()->buffer_init = 1;
//     if(SUR_is_spline(base_sf)) {
//         if(SPApar_box::empty(pbx))
//             v28 = 0i64;
//         else
//             v28 = pbx;
//         p_pbx = v28;
//         SPApar_box::SPApar_box(&trimed_box);
//         if(SUR_is_rot_surface(base_sf)) {
//             AcisVersion::AcisVersion(&v32, 32, 0, 0);
//             vt2 = v8;
//             vt1 = GET_ALGORITHMIC_VERSION(&v33);
//             if(operator>=(vt1, vt2)) {
//                 if(trim_rot_sur_to_singularity(base_sf, offset_d, &trimed_box)) p_pbx = &trimed_box;
//             }
//         }
//         v30 = (off_spl_sur*)ACIS_OBJECT::operator new(0x188ui64);
//         if(v30) {
//             off_spl_sur::off_spl_sur(v30, base_sf, offset_d, p_pbx, 0, 0, 0, 0);
//             v31 = v9;
//         } else {
//             v31 = 0i64;
//         }
//         v36 = v31;
//         off_spl = v31;
//         v25 = (spline*)ACIS_OBJECT::operator new(0xB8ui64);
//         if(v25) {
//             spline::spline(v25, off_spl);
//             v26 = v10;
//         } else {
//             v26 = 0i64;
//         }
//         v37 = v26;
//         *offset_sf = (spline*)v26;
//         full_surf = 0;
//         keep_approx = 1;
//         illegal_bispan_check = 0;
//         abort_on_illegal = 0;
//         proceed_on_failure = 1;
//         failure_ignored = 0;
//         off_spl->abort_on_failure = 0;
//         v38 = *offset_sf;
//         reqd_fit = option_header::value(&surface_fitol);
//         off_spl_sur::make_approx_for_testing(off_spl, reqd_fit, v38, full_surf, keep_approx, illegal_bispan_check, abort_on_illegal, proceed_on_failure, &failure_ignored);
//         off_spl->abort_on_failure = 0;
//         if(failure_ignored) off_spl->illegal_eval_found = 1;
//     } else {
//         use_adaptive_offset = 1;
//     }
//     exception_save::~exception_save(&v42);
//     if(acis_interrupted()) sys_error(0, e_info);
//     acis_exception::~acis_exception(&v46);
//     if(use_adaptive_offset) {
//         v40 = ipi_offset_surface(&v49, base_sf, offset_d, offset_sf, ofst_opts, ao);
//         o = v40;
//         outcome::operator=(result, v40);
//         outcome::~outcome(&v49);
//     }
//     if(outcome::ok(result) && call_update_from_bb) update_from_bb();
//     acis_version_span::~acis_version_span(&_avs);
//     api_bb_save::~api_bb_save(&make_bulletin_board);
//     exception_save::~exception_save(&exception_save_mark);
//     if(acis_interrupted()) sys_error(0, error_info_base_ptr);
//     acis_exception::~acis_exception(&error_info_holder);
//     outcome::error_number(result);
//     problems_list_prop::process_result(&problems_prop, result, PROBLEMS_LIST_PROP_ONLY, 0);
//     problems_list_prop::~problems_list_prop(&problems_prop);
//     return result;
// }

// outcome ipi_finalize_offset_face(FACE* offset_f, offset_options* ofst_opts, AcisOptions* ao) {
//     __int64 v4;                            // rax
//     error_info_base* error_info_base_ptr;  // [rsp+40h] [rbp-128h]
//     int call_update_from_bb;               // [rsp+48h] [rbp-120h]
//     acis_version_span _avs;                // [rsp+4Ch] [rbp-11Ch] BYREF
//     AcisVersion* av;                       // [rsp+50h] [rbp-118h]
//     outcome* v10;                          // [rsp+58h] [rbp-110h]
//     outcome* o;                            // [rsp+60h] [rbp-108h]
//     exception_save exception_save_mark;    // [rsp+78h] [rbp-F0h] BYREF
//     problems_list_prop problems_prop;      // [rsp+88h] [rbp-E0h] BYREF
//     __int64 v14;                           // [rsp+98h] [rbp-D0h]
//     acis_exception error_info_holder;      // [rsp+A0h] [rbp-C8h] BYREF
//     api_bb_save make_bulletin_board;       // [rsp+100h] [rbp-68h] BYREF
//     outcome v17;                           // [rsp+118h] [rbp-50h] BYREF
//
//     set_global_error_info(0i64);
//     outcome::outcome(result, 0, 0i64);
//     v14 = v4;
//     problems_list_prop::problems_list_prop(&problems_prop);
//     acis_exception::acis_exception(&error_info_holder, 0, 0i64, 0i64, 0);
//     error_info_base_ptr = 0i64;
//     exception_save::exception_save(&exception_save_mark);
//     api_bb_save::api_bb_save(&make_bulletin_board, result, normal);
//     exception_save::begin(&exception_save_mark);
//     get_error_mark()->buffer_init = 1;
//     ACISExceptionCheck("API");
//     call_update_from_bb = 1;
//     if(ao)
//         av = ao->get_version(ao);
//     else
//         av = 0i64;
//     acis_version_span::acis_version_span(&_avs, av);
//     if(api_check_on()) {
//         check_face(offset_f, 0, 0);
//         check_face_geom(offset_f, 0, 0);
//     }
//     v10 = finalize_offset_face(&v17, offset_f, ofst_opts);
//     o = v10;
//     outcome::operator=(result, v10);
//     outcome::~outcome(&v17);
//     if(outcome::ok(result)) update_from_bb();
//     acis_version_span::~acis_version_span(&_avs);
//     api_bb_save::~api_bb_save(&make_bulletin_board);
//     exception_save::~exception_save(&exception_save_mark);
//     if(acis_interrupted()) sys_error(0, error_info_base_ptr);
//     acis_exception::~acis_exception(&error_info_holder);
//     outcome::error_number(result);
//     problems_list_prop::process_result(&problems_prop, result, PROBLEMS_LIST_PROP_ONLY, 0);
//     problems_list_prop::~problems_list_prop(&problems_prop);
//     return result;
// }

// outcome finalize_offset_face(FACE* offset_f, offset_options* ofst_opts) {
//     __int64 v3;                                                                   // rax
//     SURFACE* surface;                                                             // rax
//     SPAinterval* v5;                                                              // rax
//     const curve* v6;                                                              // rax
//     const curve* v7;                                                              // rax
//     int_cur* v8;                                                                  // rax
//     bs2_curve_def* v9;                                                            // rax
//     SPApar_vec* v10;                                                              // rax
//     double v11;                                                                   // xmm0_8
//     PCURVE* v12;                                                                  // rax
//     COEDGE* v13;                                                                  // rax
//     int adaptive_ofst;                                                            // [rsp+54h] [rbp-4A4h]
//     COEDGE* c;                                                                    // [rsp+58h] [rbp-4A0h]
//     EDGE* ed;                                                                     // [rsp+68h] [rbp-490h]
//     int regen_surf;                                                               // [rsp+70h] [rbp-488h]
//     surface* new_offset_sf;                                                       // [rsp+78h] [rbp-480h] BYREF
//     PCURVE* pPC;                                                                  // [rsp+80h] [rbp-478h]
//     int v21;                                                                      // [rsp+88h] [rbp-470h]
//     int v22;                                                                      // [rsp+8Ch] [rbp-46Ch]
//     BOOL v23;                                                                     // [rsp+90h] [rbp-468h]
//     LOOP* l;                                                                      // [rsp+98h] [rbp-460h]
//     const surface* curr_base_sf;                                                  // [rsp+A0h] [rbp-458h]
//     offset_options local_opts;                                                    // [rsp+A8h] [rbp-450h] BYREF
//     int got_pbox;                                                                 // [rsp+B0h] [rbp-448h]
//     int subset_surf;                                                              // [rsp+B4h] [rbp-444h]
//     off_spl_sur* off_spl;                                                         // [rsp+B8h] [rbp-440h]
//     error_info_base* error_info_base_ptr;                                         // [rsp+C0h] [rbp-438h]
//     const surface* eqn;                                                           // [rsp+C8h] [rbp-430h]
//     surface* v32;                                                                 // [rsp+D0h] [rbp-428h]
//     surface* v33;                                                                 // [rsp+D8h] [rbp-420h]
//     intcurve* v34;                                                                // [rsp+E0h] [rbp-418h]
//     bs2_curve_def* bs;                                                            // [rsp+E8h] [rbp-410h]
//     SURFACE* v36;                                                                 // [rsp+F0h] [rbp-408h]
//     off_spl_sur* spl_sur;                                                         // [rsp+F8h] [rbp-400h]
//     surface* v38;                                                                 // [rsp+100h] [rbp-3F8h]
//     surface* base_sf;                                                             // [rsp+108h] [rbp-3F0h]
//     const off_spl_sur* new_off_spl;                                               // [rsp+110h] [rbp-3E8h]
//     CURVE* v41;                                                                   // [rsp+118h] [rbp-3E0h]
//     CURVE* v42;                                                                   // [rsp+120h] [rbp-3D8h]
//     CURVE* v43;                                                                   // [rsp+128h] [rbp-3D0h]
//     bs3_curve_def* tight_bs3;                                                     // [rsp+130h] [rbp-3C8h]
//     bs3_curve_def* cpy;                                                           // [rsp+138h] [rbp-3C0h]
//     exact_int_cur* v46;                                                           // [rsp+140h] [rbp-3B8h]
//     int_cur* v47;                                                                 // [rsp+148h] [rbp-3B0h]
//     PCURVE* v48;                                                                  // [rsp+150h] [rbp-3A8h]
//     PCURVE* v49;                                                                  // [rsp+158h] [rbp-3A0h]
//     SURFACE* pSF;                                                                 // [rsp+160h] [rbp-398h]
//     spline* v51;                                                                  // [rsp+168h] [rbp-390h]
//     spline* v52;                                                                  // [rsp+170h] [rbp-388h]
//     SPAbox* box;                                                                  // [rsp+178h] [rbp-380h]
//     SPApar_box*(__fastcall * param_range)(surface*, SPApar_box*, const SPAbox*);  // [rsp+180h] [rbp-378h]
//     SPAinterval* i2;                                                              // [rsp+188h] [rbp-370h]
//     SPAinterval* i1;                                                              // [rsp+190h] [rbp-368h]
//     SPAinterval* v57;                                                             // [rsp+198h] [rbp-360h]
//     SPAinterval* v58;                                                             // [rsp+1A0h] [rbp-358h]
//     long double offset_d;                                                         // [rsp+1A8h] [rbp-350h]
//     outcome* v60;                                                                 // [rsp+1B0h] [rbp-348h]
//     outcome* o;                                                                   // [rsp+1B8h] [rbp-340h]
//     void(__fastcall * v62)(surface*);                                             // [rsp+1C0h] [rbp-338h]
//     __int64 v63;                                                                  // [rsp+1C8h] [rbp-330h]
//     const spline* new_spl;                                                        // [rsp+1D0h] [rbp-328h]
//     const surface* prog;                                                          // [rsp+1D8h] [rbp-320h]
//     const spl_sur* original_prog_spl;                                             // [rsp+1E0h] [rbp-318h]
//     const spl_sur* offset_prog_spl;                                               // [rsp+1E8h] [rbp-310h]
//     void(__fastcall * v68)(surface*);                                             // [rsp+1F0h] [rbp-308h]
//     __int64 v69;                                                                  // [rsp+1F8h] [rbp-300h]
//     surface* s2;                                                                  // [rsp+200h] [rbp-2F8h]
//     surface* s1;                                                                  // [rsp+208h] [rbp-2F0h]
//     int_cur* cur;                                                                 // [rsp+210h] [rbp-2E8h]
//     CURVE* CU;                                                                    // [rsp+218h] [rbp-2E0h]
//     void(__fastcall * set_geometry)(EDGE*, CURVE*, int);                          // [rsp+220h] [rbp-2D8h]
//     double v75;                                                                   // [rsp+228h] [rbp-2D0h]
//     long double v;                                                                // [rsp+230h] [rbp-2C8h]
//     long double u;                                                                // [rsp+238h] [rbp-2C0h]
//     PCURVE* v78;                                                                  // [rsp+240h] [rbp-2B8h]
//     void(__fastcall * v79)(COEDGE*, PCURVE*, int);                                // [rsp+248h] [rbp-2B0h]
//     surface* s;                                                                   // [rsp+250h] [rbp-2A8h]
//     long double t;                                                                // [rsp+258h] [rbp-2A0h]
//     COEDGE* v82;                                                                  // [rsp+260h] [rbp-298h]
//     exception_save exception_save_mark;                                           // [rsp+268h] [rbp-290h] BYREF
//     SPApar_vec off;                                                               // [rsp+278h] [rbp-280h] BYREF
//     pcurve pcur;                                                                  // [rsp+288h] [rbp-270h] BYREF
//     __int64 v86;                                                                  // [rsp+2B0h] [rbp-248h]
//     acis_exception error_info_holder;                                             // [rsp+2B8h] [rbp-240h] BYREF
//     SPApar_box pbox;                                                              // [rsp+2D8h] [rbp-220h] BYREF
//     pcurve new_pc;                                                                // [rsp+348h] [rbp-1B0h] BYREF
//     SPApar_vec v90;                                                               // [rsp+370h] [rbp-188h] BYREF
//     outcome v91;                                                                  // [rsp+380h] [rbp-178h] BYREF
//     SPApar_box sf_pbox;                                                           // [rsp+3A0h] [rbp-158h] BYREF
//     SPAinterval v93;                                                              // [rsp+3D0h] [rbp-128h] BYREF
//     SPAinterval v94;                                                              // [rsp+3E8h] [rbp-110h] BYREF
//     SPAinterval v95;                                                              // [rsp+400h] [rbp-F8h] BYREF
//     SPAinterval v96;                                                              // [rsp+418h] [rbp-E0h] BYREF
//     SPAinterval v97;                                                              // [rsp+430h] [rbp-C8h] BYREF
//     SPApar_transf pt;                                                             // [rsp+448h] [rbp-B0h] BYREF
//     intcurve new_cu;                                                              // [rsp+480h] [rbp-78h] BYREF
//
//     outcome::outcome(result, 0, 0i64);
//     v86 = v3;
//     v21 = 1;
//     regen_surf = 0;
//     v36 = FACE::geometry(offset_f);
//     eqn = v36->equation(v36);
//     acis_exception::acis_exception(&error_info_holder, 0, 0i64, 0i64, 0);
//     error_info_base_ptr = 0i64;
//     exception_save::exception_save(&exception_save_mark);
//     exception_save::begin(&exception_save_mark);
//     get_error_mark()->buffer_init = 1;
//     if(SUR_is_offset(eqn)) {
//         v51 = (spline*)eqn;
//         off_spl = (off_spl_sur*)spline::get_spl_sur((spline*)eqn);
//         off_spl->abort_on_failure = 1;
//         if(off_spl->illegal_eval_found)
//             regen_surf = 1;
//         else
//             off_spl_sur::check_surface(off_spl, 1);
//     }
//     exception_save::~exception_save(&exception_save_mark);
//     if(acis_interrupted()) sys_error(0, error_info_base_ptr);
//     acis_exception::~acis_exception(&error_info_holder);
//     if(regen_surf) {
//         v52 = (spline*)eqn;
//         spl_sur = (off_spl_sur*)spline::get_spl_sur((spline*)eqn);
//         curr_base_sf = off_spl_sur::get_progenitor(spl_sur);
//         offset_d = off_spl_sur::get_offset_distance(spl_sur);
//         SPApar_box::SPApar_box(&pbox);
//         got_pbox = sg_get_face_par_box(offset_f, &pbox);
//         param_range = curr_base_sf->param_range;
//         box = SpaAcis::NullObj::get_box();
//         param_range((surface*)curr_base_sf, &sf_pbox, box);
//         v22 = got_pbox && ((i2 = SPApar_box::u_range(&pbox, &v93), i1 = SPApar_box::u_range(&sf_pbox, &v94), operator!=(i1, i2)) || (v57 = SPApar_box::v_range(&pbox, &v95), v58 = SPApar_box::v_range(&sf_pbox, &v96), operator!=(v58, v57)));
//         subset_surf = v22;
//         if(v22)
//             v38 = surface::subset((surface*)curr_base_sf, &pbox);
//         else
//             v38 = surface::copy_surf((surface*)curr_base_sf);
//         base_sf = v38;
//         offset_options::offset_options(&local_opts);
//         offset_options::set_trim_bad_geom(&local_opts, 0);
//         if(ofst_opts) local_opts = *ofst_opts;
//         new_offset_sf = 0i64;
//         v60 = offset_surface_internal(&v91, base_sf, offset_d, &new_offset_sf, &local_opts, 0i64);
//         o = v60;
//         outcome::operator=(result, v60);
//         outcome::~outcome(&v91);
//         v32 = base_sf;
//         if(base_sf) {
//             v62 = v32->~surface;
//             v63 = ((__int64(__fastcall*)(surface*, __int64))v62)(v32, 1i64);
//         } else {
//             v63 = 0i64;
//         }
//         check_outcome(result);
//         if(SUR_is_offset(new_offset_sf)) {
//             new_spl = (const spline*)new_offset_sf;
//             new_off_spl = (const off_spl_sur*)spline::get_spl_sur((spline*)new_offset_sf);
//             v23 = off_spl_sur::get_did_adaptive_offset((off_spl_sur*)new_off_spl) == 1;
//             adaptive_ofst = v23;
//             if(!v23) {
//                 prog = off_spl_sur::get_progenitor((off_spl_sur*)new_off_spl);
//                 offset_prog_spl = spline::get_spl_sur((spline*)prog);
//                 original_prog_spl = spline::get_spl_sur((spline*)curr_base_sf);
//                 adaptive_ofst = subtype_object::operator!=(&offset_prog_spl->subtrans_object, original_prog_spl);
//             }
//         } else {
//             adaptive_ofst = SUR_is_spline(new_offset_sf) != 0;
//         }
//         surface = make_surface(new_offset_sf);
//         FACE::set_geometry(offset_f, surface, 1);
//         v33 = new_offset_sf;
//         if(new_offset_sf) {
//             v68 = v33->~surface;
//             v69 = ((__int64(__fastcall*)(surface*, __int64))v68)(v33, 1i64);
//         } else {
//             v69 = 0i64;
//         }
//         pSF = FACE::geometry(offset_f);
//         for(l = FACE::loop(offset_f); l; l = LOOP::next(l, PAT_CAN_CREATE)) {
//             for(c = LOOP::start(l); c; c = COEDGE::next(c)) {
//                 ed = COEDGE::edge(c);
//                 v5 = EDGE::param_range(ed, &v97);
//                 if(SPAinterval::length(v5) > 0.0) trim_edge_geom(ed);
//                 if(EDGE::geometry(ed)) {
//                     v41 = EDGE::geometry(ed);
//                     v6 = v41->equation(v41);
//                     if(CUR_is_intcurve(v6)) {
//                         v42 = EDGE::geometry(ed);
//                         v7 = v42->equation(v42);
//                         if(!CUR_is_exact_intcurve(v7)) {
//                             v43 = EDGE::geometry(ed);
//                             v34 = (intcurve*)v43->equation_for_update(v43);
//                             intcurve::make_tight_approx(v34);
//                             tight_bs3 = intcurve::tight_bs3(v34);
//                             if(tight_bs3) {
//                                 cpy = bs3_curve_copy(tight_bs3);
//                                 if(intcurve::reversed(v34)) bs3_curve_reverse(cpy);
//                                 v46 = (exact_int_cur*)ACIS_OBJECT::operator new(0x130ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_api.m\\src\\off_face.cpp", 766, &alloc_file_index_3483);
//                                 if(v46) {
//                                     s2 = SpaAcis::NullObj::get_surface();
//                                     s1 = SpaAcis::NullObj::get_surface();
//                                     exact_int_cur::exact_int_cur(v46, cpy, s1, s2, 0i64, 0i64);
//                                     v47 = v8;
//                                 } else {
//                                     v47 = 0i64;
//                                 }
//                                 cur = v47;
//                                 intcurve::intcurve(&new_cu, v47, 0);
//                                 CU = make_curve(&new_cu);
//                                 set_geometry = ed->set_geometry;
//                                 set_geometry(ed, CU, 1);
//                                 intcurve::~intcurve(&new_cu);
//                             }
//                         }
//                     }
//                 }
//                 if(adaptive_ofst) {
//                     sg_rm_pcurves_from_entity(c, 0, 0, 1);
//                     sg_add_pcurve_to_coedge(c, 0, bndy_unknown, 0, 1);
//                 } else if(COEDGE::geometry(c)) {
//                     pPC = COEDGE::geometry(c);
//                     PCURVE::equation(pPC, &pcur, 1);
//                     v9 = pcurve::cur(&pcur);
//                     bs = bs2_curve_copy(v9);
//                     v10 = pcurve::offset(&pcur, &v90);
//                     v75 = SPApar_vec::len_sq(v10);
//                     v11 = safe_function_type<double>::operator double(&SPAresabs);
//                     if(v75 > v11) {
//                         pcurve::offset(&pcur, &off);
//                         v = SPAparameter::operator double(&off.dv);
//                         u = SPAparameter::operator double(&off.du);
//                         SPApar_transf::SPApar_transf(&pt, 1.0, 1.0, u, v);
//                         bs2_curve_par_trans(bs, &pt);
//                     }
//                     if(pcurve::reversed(&pcur)) bs2_curve_reverse(bs);
//                     if(pPC->use_count(pPC) > 1) {
//                         v48 = (PCURVE*)ACIS_OBJECT::operator new(0x90ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_api.m\\src\\off_face.cpp", 798, &alloc_file_index_3483);
//                         if(v48) {
//                             PCURVE::PCURVE(v48);
//                             v49 = v12;
//                         } else {
//                             v49 = 0i64;
//                         }
//                         v78 = v49;
//                         pPC = v49;
//                         v79 = c->set_geometry;
//                         v79(c, v49, 1);
//                     }
//                     s = (surface*)pSF->equation(pSF);
//                     t = safe_function_type<double>::operator double(&SPAresfit);
//                     pcurve::pcurve(&new_pc, bs, t, s, -1, -1, -1, -1, 1, -1.0);
//                     PCURVE::set_def(pPC, &new_pc);
//                     pcurve::~pcurve(&new_pc);
//                     pcurve::~pcurve(&pcur);
//                 }
//                 v82 = COEDGE::next(c);
//                 v13 = LOOP::start(l);
//                 if(v82 == v13) break;
//             }
//         }
//         offset_options::~offset_options(&local_opts);
//     }
//     return result;
// }

// outcome api_offset_surface(surface* sf, double off_d, surface*& off_sf, offset_options* ofst_opts, AcisOptions* acis_opts) {
//     return ipi_offset_surface(sf, off_d, off_sf, ofst_opts, acis_opts);
// }

// outcome api_offset_surface_map_bs2curves(surface* sf, surface* off_sf, double off_d, int num_c, bs2_curve_def** pcu_arr, bs2_curve_def**& off_pcu_arr, AcisOptions* acis_opts) {
//     return ipi_offset_surface_map_bs2curves(sf, off_sf, off_d, num_c, pcu_arr, off_pcu_arr, acis_opts);
// }

outcome offset_face_internal(FACE* given_face, double offset_distance, FACE*& offset_face, offset_options* pOffOpts) {
    outcome result(0, (error_info*)0);
    int failed_full_surface = 0;
    option_header* option = find_option("offset_approx");
    int use_approx_offset = option->on();
    offset_face = sg_offset_face(given_face, offset_distance, failed_full_surface, pOffOpts);
    if(!offset_face) {
        result = outcome(API_FAILED, (error_info*)0);
    }
    if(failed_full_surface) {
        error_info* ei = ACIS_NEW error_info(OFFSET_PARTIAL, SPA_OUTCOME_ERROR, given_face);
        result.add_problem(ei);
    }
    return result;
}

// outcome offset_surface_internal(surface* given_surface, double offset_distance, surface*& offset_surface, offset_options* pOffOpts, FACE* progenitor_face) {
//     __int64 v6;           // rax
//     surface* v7;          // rax
//     bool bLocalProgFace;  // [rsp+30h] [rbp-58h]
//     FACE* offset_face;    // [rsp+38h] [rbp-50h] BYREF
//     SURFACE* v11;         // [rsp+40h] [rbp-48h]
//     outcome* v12;         // [rsp+48h] [rbp-40h]
//     outcome* o;           // [rsp+50h] [rbp-38h]
//     __int64 v14;          // [rsp+58h] [rbp-30h]
//     outcome v15;          // [rsp+60h] [rbp-28h] BYREF
//
//     outcome::outcome(result, 0, 0i64);
//     v14 = v6;
//     bLocalProgFace = 0;
//     if(!progenitor_face) {
//         sg_make_face_from_surface(given_surface, &progenitor_face, 0);
//         bLocalProgFace = 1;
//     }
//     option_header::push(&iop_adaptive_ofst_surface, 1);
//     offset_face = 0i64;
//     v12 = offset_face_internal(&v15, progenitor_face, offset_distance, &offset_face, pOffOpts);
//     o = v12;
//     outcome::operator=(result, v12);
//     outcome::~outcome(&v15);
//     option_header::pop(&iop_adaptive_ofst_surface);
//     if(outcome::ok(result)) {
//         v11 = FACE::geometry(offset_face);
//         v7 = (surface*)v11->equation(v11);
//         *offset_surface = surface::copy_surf(v7);
//         if(surface::subsetted(*offset_surface)) surface::unlimit(*offset_surface);
//         delete_entity(offset_face);
//         offset_face = 0i64;
//     }
//     if(bLocalProgFace && progenitor_face) {
//         delete_entity(progenitor_face);
//         progenitor_face = 0i64;
//     }
//     return result;
// }

// int trim_rot_sur_to_singularity(surface* base_sf, double offset_d, SPApar_box& trim_box) {
//     long double started;                                                                                    // xmm0_8
//     long double v4;                                                                                         // xmm0_8
//     SPAinterval* v5;                                                                                        // rax
//     __int64 v6;                                                                                             // r8
//     SPAinterval* v7;                                                                                        // rax
//     const SPAvector* v8;                                                                                    // rax
//     SPAinterval* v9;                                                                                        // rax
//     long double v10;                                                                                        // xmm0_8
//     SPAunit_vector* v11;                                                                                    // rax
//     law* v12;                                                                                               // rax
//     law* v13;                                                                                               // rax
//     straight* v14;                                                                                          // rax
//     const void* v15;                                                                                        // rax
//     const void* v16;                                                                                        // rax
//     const void* v17;                                                                                        // rax
//     const SPAinterval* v18;                                                                                 // rax
//     const void* v19;                                                                                        // rax
//     curve_curve_int* cci;                                                                                   // [rsp+40h] [rbp-448h]
//     int singular_at_start;                                                                                  // [rsp+48h] [rbp-440h]
//     rot_spl_sur* this_rot;                                                                                  // [rsp+50h] [rbp-438h]
//     curve* profile;                                                                                         // [rsp+58h] [rbp-430h]
//     int singular_at_end;                                                                                    // [rsp+60h] [rbp-428h]
//     int v26;                                                                                                // [rsp+64h] [rbp-424h]
//     unsigned int result;                                                                                    // [rsp+68h] [rbp-420h]
//     int pl_data_ok;                                                                                         // [rsp+6Ch] [rbp-41Ch] BYREF
//     int singular_at_both;                                                                                   // [rsp+70h] [rbp-418h]
//     long double offset_distance;                                                                            // [rsp+78h] [rbp-410h]
//     curve* v31;                                                                                             // [rsp+80h] [rbp-408h]
//     straight* v32;                                                                                          // [rsp+88h] [rbp-400h]
//     long double v33;                                                                                        // [rsp+90h] [rbp-3F8h]
//     long double param;                                                                                      // [rsp+98h] [rbp-3F0h]
//     constant_law* v35;                                                                                      // [rsp+A0h] [rbp-3E8h]
//     law* v36;                                                                                               // [rsp+A8h] [rbp-3E0h]
//     constant_law* v37;                                                                                      // [rsp+B0h] [rbp-3D8h]
//     law* v38;                                                                                               // [rsp+B8h] [rbp-3D0h]
//     straight* v39;                                                                                          // [rsp+C0h] [rbp-3C8h]
//     straight* v40;                                                                                          // [rsp+C8h] [rbp-3C0h]
//     curve_curve_int* v41;                                                                                   // [rsp+D0h] [rbp-3B8h]
//     curve_curve_int* temp;                                                                                  // [rsp+D8h] [rbp-3B0h]
//     law* dist_law;                                                                                          // [rsp+E0h] [rbp-3A8h]
//     law* twist_law;                                                                                         // [rsp+E8h] [rbp-3A0h]
//     curve* offset_curve;                                                                                    // [rsp+F0h] [rbp-398h]
//     straight* axis;                                                                                         // [rsp+F8h] [rbp-390h]
//     SPAinterval u_range;                                                                                    // [rsp+100h] [rbp-388h] BYREF
//     SPAinterval*(__fastcall * param_range_u)(surface*, SPAinterval*, const SPAbox*);                        // [rsp+118h] [rbp-370h]
//     SPAbox* v49;                                                                                            // [rsp+120h] [rbp-368h]
//     SPAinterval*(__fastcall * param_range)(curve*, SPAinterval*, const SPAbox*);                            // [rsp+128h] [rbp-360h]
//     SPAbox* v51;                                                                                            // [rsp+130h] [rbp-358h]
//     SPAinterval*(__fastcall * v52)(curve*, SPAinterval*, const SPAbox*);                                    // [rsp+138h] [rbp-350h]
//     SPAunit_vector*(__fastcall * eval_direction)(curve*, SPAunit_vector*, long double, int, int);           // [rsp+140h] [rbp-348h]
//     SPAbox* v54;                                                                                            // [rsp+148h] [rbp-340h]
//     SPAinterval*(__fastcall * param_range_v)(struct rot_spl_sur*, SPAinterval*, const SPAbox*);             // [rsp+150h] [rbp-338h]
//     void(__fastcall * eval)(struct rot_spl_sur*, const SPApar_pos*, SPAposition*, SPAvector*, SPAvector*);  // [rsp+158h] [rbp-330h]
//     long double test;                                                                                       // [rsp+160h] [rbp-328h]
//     law* v58;                                                                                               // [rsp+168h] [rbp-320h]
//     law* v59;                                                                                               // [rsp+170h] [rbp-318h]
//     long double tol;                                                                                        // [rsp+178h] [rbp-310h]
//     long double fit_data;                                                                                   // [rsp+180h] [rbp-308h]
//     straight* v62;                                                                                          // [rsp+188h] [rbp-300h]
//     long double v63;                                                                                        // [rsp+190h] [rbp-2F8h]
//     SPAbox* region_of_interest;                                                                             // [rsp+198h] [rbp-2F0h]
//     long double d2;                                                                                         // [rsp+1A0h] [rbp-2E8h]
//     long double d1;                                                                                         // [rsp+1A8h] [rbp-2E0h]
//     long double param2;                                                                                     // [rsp+1B0h] [rbp-2D8h]
//     long double v68;                                                                                        // [rsp+1B8h] [rbp-2D0h]
//     SPAbox* v69;                                                                                            // [rsp+1C0h] [rbp-2C8h]
//     SPAinterval*(__fastcall * v70)(surface*, SPAinterval*, const SPAbox*);                                  // [rsp+1C8h] [rbp-2C0h]
//     void* v71;                                                                                              // [rsp+1D0h] [rbp-2B8h]
//     void(__fastcall * v72)(curve*);                                                                         // [rsp+1D8h] [rbp-2B0h]
//     __int64 v73;                                                                                            // [rsp+1E0h] [rbp-2A8h]
//     void(__fastcall * v74)(struct straight*);                                                               // [rsp+1E8h] [rbp-2A0h]
//     __int64 v75;                                                                                            // [rsp+1F0h] [rbp-298h]
//     SPAbox* box;                                                                                            // [rsp+1F8h] [rbp-290h]
//     SPAinterval new_range;                                                                                  // [rsp+200h] [rbp-288h] BYREF
//     SPAvector tangent;                                                                                      // [rsp+218h] [rbp-270h] BYREF
//     SPAvector dir;                                                                                          // [rsp+230h] [rbp-258h] BYREF
//     SPAunit_vector normal;                                                                                  // [rsp+248h] [rbp-240h] BYREF
//     SPApar_pos uv;                                                                                          // [rsp+260h] [rbp-228h] BYREF
//     SPAposition root;                                                                                       // [rsp+270h] [rbp-218h] BYREF
//     SPAposition pos;                                                                                        // [rsp+288h] [rbp-200h] BYREF
//     SPAvector sur_normal;                                                                                   // [rsp+2A0h] [rbp-1E8h] BYREF
//     SPAvector derivs[2];                                                                                    // [rsp+2B8h] [rbp-1D0h] BYREF
//     SPAinterval v86;                                                                                        // [rsp+2E8h] [rbp-1A0h] BYREF
//     SPAinterval v87;                                                                                        // [rsp+300h] [rbp-188h] BYREF
//     char v88[24];                                                                                           // [rsp+318h] [rbp-170h] BYREF
//     SPAvector v89;                                                                                          // [rsp+330h] [rbp-158h] BYREF
//     SPAinterval v90;                                                                                        // [rsp+348h] [rbp-140h] BYREF
//     SPAunit_vector v91;                                                                                     // [rsp+360h] [rbp-128h] BYREF
//     SPAunit_vector v92;                                                                                     // [rsp+378h] [rbp-110h] BYREF
//     SPAinterval v93;                                                                                        // [rsp+390h] [rbp-F8h] BYREF
//     SPAinterval v94;                                                                                        // [rsp+3A8h] [rbp-E0h] BYREF
//     SPAinterval v95;                                                                                        // [rsp+3C0h] [rbp-C8h] BYREF
//     SPAinterval v96;                                                                                        // [rsp+3D8h] [rbp-B0h] BYREF
//     SPAvector derivs2[3];                                                                                   // [rsp+3F0h] [rbp-98h] BYREF
//     SPApar_box v98;                                                                                         // [rsp+440h] [rbp-48h] BYREF
//
//     result = 0;
//     offset_distance = offset_d;
//     this_rot = (rot_spl_sur*)spline::get_spl_sur(base_sf);
//     param_range_u = base_sf->param_range_u;
//     box = SpaAcis::NullObj::get_box();
//     param_range_u(base_sf, &u_range, box);
//     profile = this_rot->cur;
//     SPAposition::SPAposition(&root);
//     SPAunit_vector::SPAunit_vector(&normal);
//     started = SPAinterval::start_pt(&u_range);
//     singular_at_start = spl_sur::singular_u(this_rot, started);
//     v4 = SPAinterval::end_pt(&u_range);
//     singular_at_end = spl_sur::singular_u(this_rot, v4);
//     v26 = singular_at_start && singular_at_end;
//     singular_at_both = v26;
//     if((singular_at_start || singular_at_end) && is_planar(profile, &u_range, &pl_data_ok, &root, &normal) && pl_data_ok) {
//         if(singular_at_start) {
//             param_range = profile->param_range;
//             v49 = SpaAcis::NullObj::get_box();
//             v5 = param_range(profile, &v86, v49);
//             v33 = SPAinterval::start_pt(v5);
//         } else {
//             v52 = profile->param_range;
//             v51 = SpaAcis::NullObj::get_box();
//             v7 = v52(profile, &v87, v51);
//             v33 = SPAinterval::end_pt(v7);
//         }
//         param = v33;
//         eval_direction = profile->eval_direction;
//         v8 = (const SPAvector*)((__int64(__fastcall*)(curve*, char*, __int64, _QWORD, _DWORD))eval_direction)(profile, v88, v6, 0i64, 0);
//         SPAvector::SPAvector(&tangent, v8);
//         if(!singular_at_start) qmemcpy(&tangent, operator-(&v89, &tangent), sizeof(tangent));
//         param_range_v = this_rot->param_range_v;
//         v54 = SpaAcis::NullObj::get_box();
//         v9 = param_range_v(this_rot, &v90, v54);
//         v10 = SPAinterval::start_pt(v9);
//         SPApar_pos::SPApar_pos(&uv, param, v10);
//         SPAposition::SPAposition(&pos);
//         `vector constructor iterator'(derivs, 0x18ui64, 2ui64, (void *(__fastcall *)(void *))SPAvector::SPAvector);
//			`vector constructor iterator'(derivs2, 0x18ui64, 3ui64, (void *(__fastcall *)(void *))SPAvector::SPAvector); eval = this_rot->eval; eval(this_rot, &uv, &pos, derivs, derivs2); operator*(&sur_normal, derivs, &derivs2[1]); operator*(&dir,
//&sur_normal,
//                                                                                                                                                                                                                                            &tangent);
//         qmemcpy(&dir, normalise(&v91, &dir), sizeof(dir));
//         v11 = normalise(&v92, &dir);
//         test = operator%(&normal, v11);
//         if(test < 0.0) *(_QWORD*)&offset_distance ^= _xmm;
//         v35 = (constant_law*)ACIS_OBJECT::operator new(0x50ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_api.m\\src\\off_face.cpp", 497, &alloc_file_index_3483);
//         if(v35) {
//             constant_law::constant_law(v35, offset_distance);
//             v36 = v12;
//         } else {
//             v36 = 0i64;
//         }
//         v58 = v36;
//         dist_law = v36;
//         v37 = (constant_law*)ACIS_OBJECT::operator new(0x50ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_api.m\\src\\off_face.cpp", 498, &alloc_file_index_3483);
//         if(v37) {
//             constant_law::constant_law(v37, 0.0);
//             v38 = v13;
//         } else {
//             v38 = 0i64;
//         }
//         v59 = v38;
//         twist_law = v38;
//         tol = safe_function_type<double>::operator double(&SPAresabs);
//         fit_data = safe_function_type<double>::operator double(&SPAresfit);
//         offset_curve = sg_offset_planar_curve(profile, &u_range, fit_data, dist_law, twist_law, &normal, 1, tol);
//         v39 = (straight*)ACIS_OBJECT::operator new(0x58ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_api.m\\src\\off_face.cpp", 507, &alloc_file_index_3483);
//         if(v39) {
//             straight::straight(v39, &this_rot->axis_root, &this_rot->axis_dir, 1.0);
//             v40 = v14;
//         } else {
//             v40 = 0i64;
//         }
//         v62 = v40;
//         axis = v40;
//         SPAinterval::SPAinterval(&new_range);
//         v63 = safe_function_type<double>::operator double(&SPAresabs);
//         region_of_interest = SpaAcis::NullObj::get_box();
//         cci = int_cur_cur(axis, offset_curve, region_of_interest, v63);
//         if(cci) {
//             if(singular_at_both && cci->next) {
//                 SPAinterval::SPAinterval(&v93, cci->param2, cci->next->param2);
//                 qmemcpy(&new_range, v15, sizeof(new_range));
//             } else if(singular_at_start) {
//                 d2 = SPAinterval::end_pt(&u_range);
//                 d1 = cci->param2;
//                 SPAinterval::SPAinterval(&v94, d1, d2);
//                 qmemcpy(&new_range, v16, sizeof(new_range));
//             } else if(singular_at_end) {
//                 param2 = cci->param2;
//                 v68 = SPAinterval::start_pt(&u_range);
//                 SPAinterval::SPAinterval(&v95, v68, param2);
//                 qmemcpy(&new_range, v17, sizeof(new_range));
//             }
//             result = 1;
//             v70 = base_sf->param_range_v;
//             v69 = SpaAcis::NullObj::get_box();
//             v18 = v70(base_sf, &v96, v69);
//             SPApar_box::SPApar_box(&v98, &new_range, v18);
//             qmemcpy(trim_box, v19, sizeof(SPApar_box));
//         }
//         temp = cci;
//         while(cci) {
//             cci = cci->next;
//             v41 = temp;
//             if(temp) v71 = curve_curve_int::`scalar deleting destructor'(v41, 1u); else v71 = 0i64; temp = cci;
//         }
//         law::remove(dist_law);
//         law::remove(twist_law);
//         v31 = offset_curve;
//         if(offset_curve) {
//             v72 = v31->~curve;
//             v73 = ((__int64(__fastcall*)(curve*, __int64))v72)(v31, 1i64);
//         } else {
//             v73 = 0i64;
//         }
//         v32 = axis;
//         if(axis) {
//             v74 = v32->~straight;
//             v75 = ((__int64(__fastcall*)(straight*, __int64))v74)(v32, 1i64);
//         } else {
//             v75 = 0i64;
//         }
//     }
//     return result;
// }

// outcome api_offset_face(FACE* given_face, double offset_distance, FACE*& offset_face, offset_options* pOffOpts = NULL, AcisOptions* ao = NULL) {
//     // int v6; // eax
//     // int v8; // eax
//     // error_info_base* error_info_base_ptr; // [rsp+40h] [rbp-148h]
//     // int call_update_from_bb; // [rsp+48h] [rbp-140h]
//     // acis_version_span _avs; // [rsp+4Ch] [rbp-13Ch] BYREF
//     // AcisVersion* av; // [rsp+50h] [rbp-138h]
//     AcisVersion* av;
//     // outcome resulta; // [rsp+58h] [rbp-130h] BYREF
//     // outcome* v14; // [rsp+78h] [rbp-110h]
//     // outcome* o; // [rsp+80h] [rbp-108h]
//     const char* err_ident;  // [rsp+98h] [rbp-F0h]
//     _iobuf* fp;             // [rsp+A0h] [rbp-E8h]
//     // exception_save exception_save_mark; // [rsp+A8h] [rbp-E0h] BYREF
//     // problems_list_prop problems_prop; // [rsp+B8h] [rbp-D0h] BYREF
//     // acis_exception error_info_holder; // [rsp+C8h] [rbp-C0h] BYREF
//     // api_bb_save make_bulletin_board; // [rsp+128h] [rbp-60h] BYREF
//     // outcome v22; // [rsp+140h] [rbp-48h] BYREF
//
//     // 目前暂时不清楚spa_is_unlocked的含义，所以暂时临时不走if中的语句，后续明白含义后可以再调整
//     /*if (spa_is_unlocked("ACIS_KERNEL"))
//     {
//         v6 = message_module::message_code(&spaacisds_comp_lock_errmod, 0);
//         outcome::outcome(result, v6, 0i64);
//         return result;
//     }*/
//
//     int hy = 0;  // hy这个变量是临时设置的，设置的目的是跳过if中的语句
//     if(hy != 0) {
//     } else {
//         /*if (api_module_header.debug_level >= 0xA)
//             acis_fprintf(debug_file_ptr, "calling api_offset_face\n");*/
//         // set_global_error_info(0i64);
//         DEBUG_LEVEL(DEBUG_CALLS)
//         acis_fprintf(debug_file_ptr, "calling api_offset_face\n");  // yhy add
//         set_global_error_info();                                    // yhy add
//         // outcome::outcome(&resulta, 0, 0i64);
//         outcome resulta(0);           // yhy add
//         err_mess_type error_num = 0;  // yhy add
//         // problems_list_prop::problems_list_prop(&problems_prop);
//         problems_list_prop problems_prop;  // yhy add
//         err_mess_type resignal_no = 0;     // yhy add
//         // acis_exception::acis_exception(&error_info_holder, 0, 0i64, 0i64, 0);
//         acis_exception error_info_holder(0);  // yhy add
//         // error_info_base_ptr = 0i64;
//         error_info_base* error_info_base_ptr = NULL;  // yhy add
//         // exception_save::exception_save(&exception_save_mark);
//         exception_save exception_save_mark;  // yhy add
//         // api_bb_save::api_bb_save(&make_bulletin_board, &resulta, normal);
//         api_bb_save make_bulletin_board(resulta, api_bb_save::normal);  // yhy add
//         // exception_save::begin(&exception_save_mark);
//         exception_save_mark.begin();  // yhy add
//         // get_error_mark()->buffer_init = 1;
//         get_error_mark().buffer_init = TRUE;  // yhy add
//         ACISExceptionCheck("API");
//         // call_update_from_bb = 1;
//         logical call_update_from_bb = TRUE;  // yhy add
//         if(ao)
//             // av = ao->get_version(ao);
//             *av = ao->get_version();
//         else
//             // av = 0i64;
//             av = NULL;
//         // acis_version_span::acis_version_span(&_avs, av);
//         acis_version_span _avs(av);  // yhy add
//         if(api_check_on()) check_face(given_face, 0, 0);
//         /*if (ao && AcisOptions::journal_on(ao))
//             J_api_offset_face(given_face, offset_distance, pOffOpts, ao);*/
//         if(ao && ao->journal_on()) J_api_offset_face(given_face, offset_distance, pOffOpts, ao);  // yhy add
//
//         // v14 = offset_face_internal(&v22, given_face, offset_distance, offset_face, pOffOpts);
//         // o = v14;
//         // outcome::operator=(&resulta, v14);
//         // outcome::~outcome(&v22);
//         resulta = offset_face_internal(given_face, offset_distance, offset_face, pOffOpts);  // yhy add
//         /*if (outcome::ok(&resulta))
//             update_from_bb();*/
//         if(resulta.ok()) update_from_bb();  // yhy add
//         // acis_version_span::~acis_version_span(&_avs);
//         _avs.~acis_version_span();  // yhy add
//         // api_bb_save::~api_bb_save(&make_bulletin_board);
//         make_bulletin_board.~api_bb_save();  // yhy add
//         // exception_save::~exception_save(&exception_save_mark);
//         exception_save_mark.~exception_save();  // yhy add
//         if(acis_interrupted()) sys_error(0, error_info_base_ptr);
//         // acis_exception::~acis_exception(&error_info_holder);
//         error_info_holder.~acis_exception();  // yhy add
//         // outcome::error_number(&resulta);
//         resulta.error_number();  // yhy add
//         // problems_list_prop::process_result(&problems_prop, &resulta, PROBLEMS_LIST_PROP_ONLY, 0);
//         problems_prop.process_result(resulta, PROBLEMS_LIST_PROP_ONLY, 0);
//         /*if (api_module_header.debug_level >= 0x14)
//         {
//             v8 = outcome::error_number(&resulta);
//             err_ident = find_err_ident(v8);
//             fp = debug_file_ptr;
//             acis_fprintf(debug_file_ptr, "leaving api_offset_face: %s\n", err_ident);
//         }*/
//         DEBUG_LEVEL(DEBUG_FLOW)
//         acis_fprintf(debug_file_ptr, "leaving api_offset_face: %s\n", find_err_ident(resulta.error_number()));  // yhy add
//         // outcome::outcome(result, &resulta);
//         outcome result(resulta);  // yhy add
//         // problems_list_prop::~problems_list_prop(&problems_prop);
//         problems_prop.~problems_list_prop();  // yhy add
//         // outcome::~outcome(&resulta);
//         resulta.~outcome();  // yhy add
//         return result;
//     }
// }

outcome gme_api_offset_face(FACE* given_face, double offset_distance, FACE*& offset_face, offset_options* pOffOpts, AcisOptions* ao) {
    DEBUG_LEVEL(DEBUG_CALLS) {
        acis_fprintf(debug_file_ptr, "calling api_offset_face\n");
    }
    API_BEGIN
    AcisVersion* av;
    if(ao) {
        av = &(ao->get_version());
    } else {
        av = NULL;
    }
    acis_version_span _avs(av);
    if(api_check_on()) {
        check_face(given_face, 0, 0);
    }
    if(ao && ao->journal_on()) {
        J_api_offset_face(given_face, offset_distance, pOffOpts, ao);
    }
    result = offset_face_internal(given_face, offset_distance, offset_face, pOffOpts);
    API_END
    DEBUG_LEVEL(DEBUG_FLOW) {
        acis_fprintf(debug_file_ptr, "leaving api_offset_face: %s\n", find_err_ident(result.error_number()));
    }
    return result;
}
