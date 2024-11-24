#include "PublicInterfaces/gme_ofstapi.hxx"
#include "acis/api.err"
#include "acis/check.hxx"
#include "acis/module.hxx"
#include "acis/unitvec.hxx"
#include "PublicInterfaces/gme_sgcofrtn.hxx"

#define MODULE() api
extern DECL_BASE module_debug api_module_header;

outcome gme_api_offset_planar_wire(WIRE* given_wire, TRANSFORM const* trans, double offset_distance, const SPAunit_vector& wire_normal, BODY*& offset_wire, AcisOptions* ao) {
    DEBUG_LEVEL(DEBUG_CALLS) {
        acis_fprintf(debug_file_ptr, "calling api_offset_planar_wire\n");
    }
    API_BEGIN
    if(api_check_on()) {
        check_wire(given_wire);
        check_transform(trans);
        check_pos_length(wire_normal.len(), "normal");
    }
    if(ao && ao->journal_on()) {
        // J_api_offset_planar_wire(given_wire, trans, offset_distance, wire_normal, ao);
    }
    offset_wire = gme_sg_offset_planar_wire(given_wire, (TRANSFORM*)trans, offset_distance, (SPAunit_vector&)wire_normal, arc, 0, 1, 0, 0);
    if(offset_wire) {
        result = outcome(0, (error_info*)0);
    } else {
        result = outcome(API_FAILED, (error_info*)0);
    }
    API_END
    DEBUG_LEVEL(DEBUG_FLOW) {
        acis_fprintf(debug_file_ptr, "leaving api_offset_planar_wire: %s\n", find_err_ident(result.error_number()));
    }
    return result;
}




  //void __fastcall trim_edge_geom(EDGE* this_edge) {
//    SPAparameter* started;                                                        // rax
//    SPAparameter* v2;                                                             // rax
//    APOINT* v3;                                                                   // rax
//    APOINT* v4;                                                                   // rax
//    APOINT* v5;                                                                   // rax
//    SPAvector* v6;                                                                // rax
//    double v7;                                                                    // xmm0_8
//    SPAinterval* v8;                                                              // rax
//    SPAinterval* v9;                                                              // rax
//    long double v10;                                                              // xmm0_8
//    __int64 v11;                                                                  // rdx
//    const SPAinterval* v12;                                                       // rax
//    __int64 v13;                                                                  // rdx
//    __int64 v14;                                                                  // rdx
//    long double v15;                                                              // xmm0_8
//    __int64 v16;                                                                  // rdx
//    int v17;                                                                      // eax
//    long double v18;                                                              // xmm0_8
//    pcurve* v19;                                                                  // rax
//    double v20;                                                                   // xmm0_8
//    long double v21;                                                              // xmm0_8
//    const SPApar_vec* par_vec;                                                    // rax
//    long double v23;                                                              // xmm0_8
//    const SPApar_vec* v24;                                                        // rax
//    __int64 v25;                                                                  // rax
//    __int64 v26;                                                                  // rax
//    bool pc_periodic;                                                             // [rsp+30h] [rbp-438h]
//    curve* new_crv;                                                               // [rsp+38h] [rbp-430h]
//    curve* new_crva;                                                              // [rsp+38h] [rbp-430h]
//    PCURVE* this_pcurve;                                                          // [rsp+40h] [rbp-428h]
//    pcurve* new_pcur;                                                             // [rsp+48h] [rbp-420h]
//    curve* old_crv;                                                               // [rsp+50h] [rbp-418h]
//    CURVE* new_cur;                                                               // [rsp+58h] [rbp-410h]
//    BOOL v34;                                                                     // [rsp+64h] [rbp-404h]
//    int old_crv_periodic;                                                         // [rsp+68h] [rbp-400h]
//    COEDGE* this_coedge;                                                          // [rsp+70h] [rbp-3F8h]
//    COEDGE* next_coedge;                                                          // [rsp+78h] [rbp-3F0h]
//    double end_param;                                                             // [rsp+80h] [rbp-3E8h]
//    double start_param;                                                           // [rsp+88h] [rbp-3E0h]
//    long double trim_start;                                                       // [rsp+90h] [rbp-3D8h]
//    pcurve* old_pcur;                                                             // [rsp+98h] [rbp-3D0h]
//    long double startp_param;                                                     // [rsp+A0h] [rbp-3C8h]
//    long double endp_param;                                                       // [rsp+A8h] [rbp-3C0h]
//    CURVE* old_cur;                                                               // [rsp+B0h] [rbp-3B8h]
//    int index;                                                                    // [rsp+B8h] [rbp-3B0h]
//    unsigned int v46;                                                             // [rsp+BCh] [rbp-3ACh]
//    long double respar;                                                           // [rsp+E0h] [rbp-388h]
//    long double respara;                                                          // [rsp+E0h] [rbp-388h]
//    const char* v49;                                                              // [rsp+108h] [rbp-360h]
//    VERTEX* end_vertex;                                                           // [rsp+118h] [rbp-350h]
//    VERTEX* start_vertex;                                                         // [rsp+120h] [rbp-348h]
//    double old_crv_period;                                                        // [rsp+128h] [rbp-340h]
//    curve* first_piece;                                                           // [rsp+130h] [rbp-338h]
//    pcurve* v54;                                                                  // [rsp+138h] [rbp-330h]
//    pcurve* v55;                                                                  // [rsp+140h] [rbp-328h]
//    PCURVE* v56;                                                                  // [rsp+150h] [rbp-318h]
//    __int64 v57;                                                                  // [rsp+158h] [rbp-310h]
//    PCURVE* v58;                                                                  // [rsp+160h] [rbp-308h]
//    __int64 v59;                                                                  // [rsp+168h] [rbp-300h]
//    COEDGE* first_coedge;                                                         // [rsp+170h] [rbp-2F8h]
//    SPAposition end;                                                              // [rsp+178h] [rbp-2F0h] BYREF
//    SPAposition start;                                                            // [rsp+190h] [rbp-2D8h] BYREF
//    APOINT* v63;                                                                  // [rsp+1A8h] [rbp-2C0h]
//    void(__fastcall * debug)(curve*, const char*, _iobuf*);                       // [rsp+1B0h] [rbp-2B8h]
//    SPAposition* p2;                                                              // [rsp+1B8h] [rbp-2B0h]
//    SPAposition* p1;                                                              // [rsp+1C0h] [rbp-2A8h]
//    double v67;                                                                   // [rsp+1C8h] [rbp-2A0h]
//    SPAbox* box;                                                                  // [rsp+1D0h] [rbp-298h]
//    SPAinterval*(__fastcall * param_range)(curve*, SPAinterval*, const SPAbox*);  // [rsp+1D8h] [rbp-290h]
//    double v70;                                                                   // [rsp+1E0h] [rbp-288h]
//    double v71;                                                                   // [rsp+1E8h] [rbp-280h]
//    curve*(__fastcall * split)(curve*, long double, const SPAposition*);          // [rsp+1F0h] [rbp-278h]
//    __int64 v73;                                                                  // [rsp+200h] [rbp-268h]
//    void(__fastcall * v74)(curve*);                                               // [rsp+208h] [rbp-260h]
//    __int64 v75;                                                                  // [rsp+210h] [rbp-258h]
//    curve*(__fastcall * v76)(curve*, long double, const SPAposition*);            // [rsp+218h] [rbp-250h]
//    void(__fastcall * v77)(curve*);                                               // [rsp+220h] [rbp-248h]
//    __int64 v78;                                                                  // [rsp+228h] [rbp-240h]
//    curve* middle_piece;                                                          // [rsp+230h] [rbp-238h]
//    void(__fastcall * set_geometry)(EDGE*, CURVE*, int);                          // [rsp+238h] [rbp-230h]
//    _iobuf* fp;                                                                   // [rsp+240h] [rbp-228h]
//    pcurve* v82;                                                                  // [rsp+248h] [rbp-220h]
//    _iobuf* v83;                                                                  // [rsp+250h] [rbp-218h]
//    _iobuf* v84;                                                                  // [rsp+258h] [rbp-210h]
//    void* SPAptr;                                                                 // [rsp+260h] [rbp-208h]
//    long double(__fastcall * eval_deriv_len)(curve*, long double, int, int);      // [rsp+268h] [rbp-200h]
//    double v87;                                                                   // [rsp+270h] [rbp-1F8h]
//    long double(__fastcall * v88)(curve*, long double, int, int);                 // [rsp+278h] [rbp-1F0h]
//    double v89;                                                                   // [rsp+280h] [rbp-1E8h]
//    pcurve* v90;                                                                  // [rsp+288h] [rbp-1E0h]
//    double v91;                                                                   // [rsp+290h] [rbp-1D8h]
//    double v92;                                                                   // [rsp+298h] [rbp-1D0h]
//    pcurve* v93;                                                                  // [rsp+2A0h] [rbp-1C8h]
//    void(__fastcall * v94)(pcurve*);                                              // [rsp+2A8h] [rbp-1C0h]
//    __int64 v95;                                                                  // [rsp+2B0h] [rbp-1B8h]
//    double v96;                                                                   // [rsp+2B8h] [rbp-1B0h]
//    void(__fastcall * v97)(pcurve*);                                              // [rsp+2C0h] [rbp-1A8h]
//    __int64 v98;                                                                  // [rsp+2C8h] [rbp-1A0h]
//    pcurve* v99;                                                                  // [rsp+2D0h] [rbp-198h]
//    __int64 v100;                                                                 // [rsp+2D8h] [rbp-190h]
//    void(__fastcall * v101)(pcurve*);                                             // [rsp+2E0h] [rbp-188h]
//    __int64 v102;                                                                 // [rsp+2E8h] [rbp-180h]
//    SPApar_vec* shift;                                                            // [rsp+2F0h] [rbp-178h]
//    __int64 v104;                                                                 // [rsp+2F8h] [rbp-170h]
//    void(__fastcall * v105)(COEDGE*, PCURVE*, int);                               // [rsp+300h] [rbp-168h]
//    void(__fastcall * remove)(struct CURVE*, int);                                // [rsp+308h] [rbp-160h]
//    SPAposition*(__fastcall * start_pos)(EDGE*, SPAposition*);                    // [rsp+320h] [rbp-148h]
//    SPAposition*(__fastcall * end_pos)(EDGE*, SPAposition*);                      // [rsp+328h] [rbp-140h]
//    SPAposition startp;                                                           // [rsp+330h] [rbp-138h] BYREF
//    SPAposition endp;                                                             // [rsp+348h] [rbp-120h] BYREF
//    SPApar_pos uv_start;                                                          // [rsp+360h] [rbp-108h] BYREF
//    SPApar_pos uv_end;                                                            // [rsp+370h] [rbp-F8h] BYREF
//    SPAparameter result;                                                          // [rsp+380h] [rbp-E8h] BYREF
//    SPAparameter v114;                                                            // [rsp+388h] [rbp-E0h] BYREF
//    SPAinterval par_range;                                                        // [rsp+390h] [rbp-D8h] BYREF
//    SPAposition tmp;                                                              // [rsp+3A8h] [rbp-C0h] BYREF
//    SPAposition v117;                                                             // [rsp+3C0h] [rbp-A8h] BYREF
//    SPAinterval v118;                                                             // [rsp+3D8h] [rbp-90h] BYREF
//    SPAvector v119;                                                               // [rsp+3F0h] [rbp-78h] BYREF
//    SPAinterval v120;                                                             // [rsp+408h] [rbp-60h] BYREF
//    SPAinterval v121;                                                             // [rsp+420h] [rbp-48h] BYREF
//    SPAinterval v122;                                                             // [rsp+438h] [rbp-30h] BYREF
//
//    if(euler_module_header.debug_level >= 0x14) {
//        acis_fprintf(debug_file_ptr, "\ntrim_edge_geom for edge ");
//        debug_pointer(this_edge, debug_file_ptr);
//        debug_newline(debug_file_ptr);
//    }
//    old_cur = EDGE::geometry(this_edge);
//    if(old_cur) {
//        old_crv = (curve*)old_cur->equation(old_cur);
//        old_crv_periodic = old_crv->periodic(old_crv);
//        old_crv_period = old_crv->param_period(old_crv);
//        start_vertex = EDGE::start(this_edge);
//        end_vertex = EDGE::end(this_edge);
//        start_pos = this_edge->start_pos;
//        start_pos(this_edge, &start);
//        end_pos = this_edge->end_pos;
//        end_pos(this_edge, &end);
//        started = EDGE::start_param(this_edge, &result);
//        start_param = SPAparameter::operator double(started);
//        v2 = EDGE::end_param(this_edge, &v114);
//        end_param = SPAparameter::operator double(v2);
//        v63 = VERTEX::geometry(start_vertex);
//        v3 = VERTEX::geometry(end_vertex);
//        v34 = v63 == v3;
//        if(euler_module_header.debug_level >= 0x28) {
//            acis_fprintf(debug_file_ptr, "new start param %g end param %g\n", start_param, end_param);
//            if(EDGE::sense(this_edge))
//                v49 = "REVERSED";
//            else
//                v49 = "forward";
//            if(v34)
//                acis_fprintf(debug_file_ptr, "\tedge is %s and %s on curve\n", "closed", v49);
//            else
//                acis_fprintf(debug_file_ptr, "\tedge is %s and %s on curve\n", "open", v49);
//            acis_fprintf(debug_file_ptr, "\n\t curve is ");
//            debug_pointer(old_cur, debug_file_ptr);
//            debug_newline(debug_file_ptr);
//            debug = old_crv->debug;
//            debug(old_crv, "\t", debug_file_ptr);
//            debug_newline(debug_file_ptr);
//        }
//        if(v34 || (v4 = VERTEX::geometry(end_vertex), p2 = (SPAposition*)APOINT::coords(v4), v5 = VERTEX::geometry(start_vertex), p1 = (SPAposition*)APOINT::coords(v5), v67 = safe_function_type<double>::operator double(&SPAresabs),
//                   v6 = operator-(&v119, p1, p2), v7 = SPAvector::len(v6), v67 <= v7)) {
//            if(!v34) {
//                param_range = old_crv->param_range;
//                box = SpaAcis::NullObj::get_box();
//                v8 = param_range(old_crv, &v120, box);
//                v71 = SPAinterval::length(v8);
//                v70 = safe_function_type<double>::operator double(&SPAresnor);
//                v9 = EDGE::param_range(this_edge, &v121);
//                v10 = SPAinterval::length(v9);
//                if(v10 + v70 <= v71) {
//                    if(EDGE::sense(this_edge) == 1) {
//                        *(_QWORD*)&trim_start = *(_QWORD*)&end_param ^ _xmm;
//                        SPAposition::SPAposition(&tmp, &start);
//                        qmemcpy(&start, &end, sizeof(start));
//                        qmemcpy(&end, &tmp, sizeof(end));
//                    } else {
//                        trim_start = start_param;
//                    }
//                    new_crv = curve::copy_curve(old_crv);
//                    if(!curve::subsetted(new_crv) && old_crv_periodic && old_crv_period > 0.0) {
//                        SPAinterval::SPAinterval(&v122, trim_start, trim_start + old_crv_period);
//                        curve::limit(new_crv, v12);
//                    }
//                    split = new_crv->split;
//                    first_piece = (curve*)((__int64(__fastcall*)(curve*, __int64, SPAposition*))split)(new_crv, v11, &start);
//                    if(!first_piece) {
//                        if(euler_module_header.debug_level >= 0x28) acis_fprintf(debug_file_ptr, "\tcurve not splittable\n");
//                        if(new_crv)
//                            v73 = ((__int64(__fastcall*)(curve*, __int64))new_crv->~curve)(new_crv, 1i64);
//                        else
//                            v73 = 0i64;
//                        return;
//                    }
//                    v74 = first_piece->~curve;
//                    v75 = ((__int64(__fastcall*)(curve*, __int64))v74)(first_piece, 1i64);
//                    v76 = new_crv->split;
//                    middle_piece = (curve*)((__int64(__fastcall*)(curve*, __int64, SPAposition*))v76)(new_crv, v13, &end);
//                    if(new_crv) {
//                        v77 = new_crv->~curve;
//                        v78 = ((__int64(__fastcall*)(curve*, __int64))v77)(new_crv, 1i64);
//                    } else {
//                        v78 = 0i64;
//                    }
//                    new_crva = middle_piece;
//                    new_cur = make_curve(middle_piece);
//                    if(new_cur) new_cur->add(new_cur);
//                    set_geometry = this_edge->set_geometry;
//                    set_geometry(this_edge, new_cur, 1);
//                    if(EDGE::sense(this_edge) == 1) {
//                        SPAposition::SPAposition(&v117, &start);
//                        qmemcpy(&start, &end, sizeof(start));
//                        qmemcpy(&end, &v117, sizeof(end));
//                    }
//                    first_coedge = EDGE::coedge(this_edge);
//                    next_coedge = first_coedge;
//                    while(1) {
//                        this_coedge = next_coedge;
//                        next_coedge = COEDGE::partner(next_coedge);
//                        this_pcurve = COEDGE::geometry(this_coedge);
//                        if(euler_module_header.debug_level >= 0x28) {
//                            acis_fprintf(debug_file_ptr, "\tcoedge ");
//                            debug_pointer(this_coedge, debug_file_ptr);
//                            acis_fprintf(debug_file_ptr, ", pcurve ");
//                            if(this_pcurve) {
//                                if(PCURVE::index(this_pcurve)) {
//                                    v46 = PCURVE::index(this_pcurve);
//                                    v83 = debug_file_ptr;
//                                    acis_fprintf(debug_file_ptr, "pcurve %d of curve ", v46);
//                                    v84 = debug_file_ptr;
//                                    SPAptr = PCURVE::ref_curve(this_pcurve);
//                                    debug_pointer(SPAptr, v84);
//                                } else {
//                                    v82 = (pcurve*)PCURVE::def_pcur(this_pcurve);
//                                    fp = debug_file_ptr;
//                                    pcurve::debug(v82, "\t", debug_file_ptr);
//                                }
//                            } else {
//                                acis_fprintf(debug_file_ptr, "null");
//                            }
//                            debug_newline(debug_file_ptr);
//                        }
//                        if(!this_pcurve) goto LABEL_80;
//                        if(PCURVE::index(this_pcurve)) {
//                            if(PCURVE::ref_curve(this_pcurve) == old_cur) {
//                                v58 = (PCURVE*)ACIS_OBJECT::operator new(0x90ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAeulr\\euler_kerndata_euler.m\\src\\trim.cpp", 350, &alloc_file_index_1230);
//                                if(v58) {
//                                    shift = SpaAcis::NullObj::get_par_vec();
//                                    index = PCURVE::index(this_pcurve);
//                                    PCURVE::PCURVE(v58, new_cur, index, 0, shift);
//                                    v59 = v26;
//                                } else {
//                                    v59 = 0i64;
//                                }
//                                v104 = v59;
//                                this_pcurve = (PCURVE*)v59;
//                            }
//                        } else {
//                            old_pcur = (pcurve*)PCURVE::def_pcur(this_pcurve);
//                            SPAposition::SPAposition(&startp);
//                            SPAposition::SPAposition(&endp);
//                            if((unsigned int)COEDGE::sense(this_coedge)) {
//                                *(_QWORD*)&startp_param = *(_QWORD*)&end_param ^ _xmm;
//                                *(_QWORD*)&endp_param = *(_QWORD*)&start_param ^ _xmm;
//                                qmemcpy(&startp, &end, sizeof(startp));
//                                qmemcpy(&endp, &start, sizeof(endp));
//                            } else {
//                                startp_param = start_param;
//                                endp_param = end_param;
//                                qmemcpy(&startp, &start, sizeof(startp));
//                                qmemcpy(&endp, &end, sizeof(endp));
//                            }
//                            if(old_crv_periodic) {
//                                if(pcurve::param_period(old_pcur) == 0.0) {
//                                    pcurve::param_range(old_pcur, &par_range);
//                                    eval_deriv_len = new_crva->eval_deriv_len;
//                                    v87 = ((double(__fastcall*)(curve*, __int64, _QWORD, __int64))eval_deriv_len)(new_crva, v14, 0i64, 1i64);
//                                    v15 = safe_function_type<double>::operator double(&SPAresabs);
//                                    respar = v15 / v87;
//                                    if(SPAinterval::start_pt(&par_range) - respar > startp_param || (v88 = new_crva->eval_deriv_len, v89 = ((double(__fastcall*)(curve*, __int64, _QWORD, __int64))v88)(new_crva, v16, 0i64, 1i64),
//                                                                                                     v18 = safe_function_type<double>::operator double(&SPAresabs), respara = v18 / v89, endp_param > SPAinterval::end_pt(&par_range) + respara)) {
//                                        v17 = message_module::message_code(&spaacisds_insanity_errmod, 136);
//                                        sys_error(v17);
//                                        goto LABEL_80;
//                                    }
//                                }
//                            }
//                            v54 = (pcurve*)ACIS_OBJECT::operator new(0x28ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAeulr\\euler_kerndata_euler.m\\src\\trim.cpp", 303, &alloc_file_index_1230);
//                            if(v54) {
//                                pcurve::pcurve(v54, old_pcur);
//                                v55 = v19;
//                            } else {
//                                v55 = 0i64;
//                            }
//                            v90 = v55;
//                            new_pcur = v55;
//                            pcurve::param_range(old_pcur, &v118);
//                            v91 = pcurve::param_period(old_pcur);
//                            v20 = safe_function_type<double>::operator double(&SPAresnor);
//                            pc_periodic = v91 > v20;
//                            if(v91 > v20 || (v92 = SPAinterval::start_pt(&v118), v21 = safe_function_type<double>::operator double(&SPAresnor), startp_param > v92 + v21)) {
//                                pcurve::eval_position(v55, &uv_start, &startp, startp_param, 0);
//                                par_vec = SpaAcis::NullObj::get_par_vec();
//                                v93 = pcurve::split(v55, startp_param, &uv_start, par_vec);
//                                if(v93) {
//                                    v94 = v93->~pcurve;
//                                    v95 = ((__int64(__fastcall*)(pcurve*, __int64))v94)(v93, 1i64);
//                                } else {
//                                    v95 = 0i64;
//                                }
//                            }
//                            if(pc_periodic || (v96 = SPAinterval::end_pt(&v118), v23 = safe_function_type<double>::operator double(&SPAresnor), v96 - v23 > endp_param)) {
//                                pcurve::eval_position(v55, &uv_end, &endp, endp_param, 0);
//                                v24 = SpaAcis::NullObj::get_par_vec();
//                                v99 = pcurve::split(v55, endp_param, &uv_end, v24);
//                                if(v55) {
//                                    v97 = v55->~pcurve;
//                                    v98 = ((__int64(__fastcall*)(pcurve*, __int64))v97)(v55, 1i64);
//                                } else {
//                                    v98 = 0i64;
//                                }
//                                new_pcur = v99;
//                            }
//                            if(new_pcur) {
//                                v56 = (PCURVE*)ACIS_OBJECT::operator new(0x90ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAeulr\\euler_kerndata_euler.m\\src\\trim.cpp", 332, &alloc_file_index_1230);
//                                if(v56) {
//                                    PCURVE::PCURVE(v56, new_pcur);
//                                    v57 = v25;
//                                } else {
//                                    v57 = 0i64;
//                                }
//                                v100 = v57;
//                                this_pcurve = (PCURVE*)v57;
//                                v101 = new_pcur->~pcurve;
//                                v102 = ((__int64(__fastcall*)(pcurve*, __int64))v101)(new_pcur, 1i64);
//                            } else {
//                                this_pcurve = 0i64;
//                            }
//                        }
//                        v105 = this_coedge->set_geometry;
//                        v105(this_coedge, this_pcurve, 1);
//                    LABEL_80:
//                        if(next_coedge == first_coedge || !next_coedge) {
//                            if(new_cur) {
//                                remove = new_cur->remove;
//                                remove(new_cur, 1);
//                            }
//                            if(new_crva) ((void(__fastcall*)(curve*, __int64))new_crva->~curve)(new_crva, 1i64);
//                            return;
//                        }
//                    }
//                }
//            }
//        }
//    }
//}