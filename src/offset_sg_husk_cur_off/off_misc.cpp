#include "PublicInterfaces/off_misc.hxx"
#include "acis/ckoutcom.hxx"
#include "acis/kernapi.hxx"
#include "acis/wire.hxx"
#include "ProtectedInterfaces/ofsttools.hxx"
#include "acis/discinfo.hxx"
#include"acis/ellipse.hxx"
#include "acis/sps3crtn.hxx"
#include "acis/curve_approx.hxx"
#include "acis/sgapi.err"
#include "acis/acismath.h"
#include "acis/fit.hxx"
#include "acis/degenerate.hxx"




void order_wire_coedges(WIRE*& wire) 
{
    ENTITY_LIST coedge_list;
    outcome coedges = api_get_coedges((ENTITY*)wire, coedge_list, PAT_CAN_CREATE);
    check_outcome(coedges);
    if(coedge_list.count() > 2) {
        COEDGE* start_coedge = wire->coedge();
        COEDGE* current_coedge = start_coedge;
        for(COEDGE* next_coedge = start_coedge->next(); next_coedge && next_coedge != current_coedge && next_coedge != start_coedge; next_coedge = next_coedge->next()) {
            if(next_coedge->previous() != current_coedge) {
                COEDGE* prev = next_coedge->previous();
                next_coedge->set_previous(current_coedge, 0, 1);
                next_coedge->set_next(prev, 0, 1);
                if(next_coedge->sense())
                    next_coedge->set_sense(0, 1);
                else
                    next_coedge->set_sense(1, 1);
            }
            current_coedge = next_coedge;
        }
        COEDGE* current_coedgea = start_coedge;
        for(COEDGE* previous_coedge = start_coedge->previous(); previous_coedge && previous_coedge != current_coedgea && previous_coedge != start_coedge; previous_coedge = previous_coedge->previous()) {
            if(previous_coedge->next() != current_coedgea) {
                COEDGE* next = previous_coedge->next();
                previous_coedge->set_next(current_coedgea, 0, 1);
                previous_coedge->set_previous(next, 0, 1);
                if(previous_coedge->sense())
                    previous_coedge->set_sense(0, 1);
                else
                    previous_coedge->set_sense(1, 1);
            }
            current_coedgea = previous_coedge;
        }
    }

}
void order_wire_coedges(ENTITY* wire_body) {
    bool postR22Sp1 = GET_ALGORITHMIC_VERSION() > AcisVersion(22, 0, 1);
    BODY* wirebody = (BODY*)wire_body;
    WIRE* wire = ((BODY*)wire_body)->wire();  
    if(wire) {
        do {
            order_wire_coedges(wire);
            wire = wire->next(PAT_CAN_CREATE);
        } while(postR22Sp1 && wire);
    } 
    else 
    {
        ENTITY_LIST wire_shells;
        outcome shells = api_get_shells((BODY*)wire_body, wire_shells, PAT_CAN_CREATE);
        check_outcome(shells);
        wire_shells.init();
        SHELL* wire_shell = (SHELL*)wire_shells.next();
        if(wire_shell) {
            do {
                wire = wire_shell->wire();
                if(wire) {
                    do {
                        order_wire_coedges(wire);
                        wire = wire->next(PAT_CAN_CREATE);
                    } 
                    while(postR22Sp1 && wire);
                }
                wire_shell = (SHELL*)wire_shells.next();
            } while(postR22Sp1 && wire_shell);
        }
    }
}
curve* gme_sg_offset_planar_curve(curve& geom, SPAinterval& range, double fit_data, law* dist_law, law* twist_law, SPAunit_vector& off_nor, int all_spline, double tol)
{
    if(woffset_module_header.debug_level >= 20) acis_fprintf(debug_file_ptr, "Inside sg_offset_planar_curve.\n");
    if(SPAresmch > tol) tol = SPAresmch;
    curve*the_curve = nullptr;
    
    if(dist_law->constant() &&  tol > fabs(dist_law->eval(1.0))) 
    {
        if(geom.type() != 11 && all_spline) 
        {
            double end_param = range.end_pt();
            double start_param = range.start_pt();
            bs3_curve_def *result = bs3_curve_make_cur(geom, start_param, end_param, 0.0);
            intcurve* v32 = ACIS_NEW intcurve(result, 0.0, SpaAcis::NullObj::get_surface(), SpaAcis::NullObj::get_surface());
            the_curve = (curve*)v32;
        } 
        else {
            the_curve = geom.make_copy();
        }
    } 
    else if(geom.type() == 1 && dist_law->constant() &&  twist_law->zero(SPAresabs))
    {
        double off_dist = dist_law->eval(1.0);
        int discount;
        SPAunit_vector v12 = normalise(((straight&)geom).direction * off_nor);
        const SPAvector v13 = off_dist * v12;
        SPAposition root = ((straight&)geom).root_point + v13;
        straight* v34 = ACIS_NEW straight(root,((straight&)geom).direction);
        curve* res = (curve*)v34;
        if(all_spline) 
        {
            double v58 = range.end_pt();
            double started = range.start_pt();
            bs3_curve_def* f = bs3_curve_make_cur(*res, started, v58);
            if(v34) 
            {
                v34 = nullptr;
            } 
            intcurve* v36 = ACIS_NEW intcurve(f, 0.0, SpaAcis::NullObj::get_surface(), SpaAcis::NullObj::get_surface());
            res = (curve*)v36;
        }
        the_curve = res;
    } 
    else if(geom.type() == 2) 
    {
        double v67 = fabs(((ellipse&)geom).radius_ratio- 1.0);  // 不懂
        if(SPAresmch > v67) {
            if(dist_law->constant()) {
                if(twist_law->zero(SPAresabs)) {
                    double d = dist_law->eval(1.0);
                    double dotProduct = ((ellipse&)geom).normal % off_nor;//不确定
                    if(GET_ALGORITHMIC_VERSION() < AcisVersion(31, 0, 0) || fabs(dotProduct) > 1.0 - 10.0 * SPAresmch) 
                    {
                        if(dotProduct < 0.0) d = -d;
                        SPAunit_vector v19 = normalise(((ellipse&)geom).major_axis);
                        const SPAvector v20 = d * v19;
                        SPAvector new_major = (((ellipse&)geom).major_axis + v20);
                        double v73 = new_major.len();
                        if(v73 <= SPAresabs) 
                        {
                            the_curve = nullptr;
                        } 
                        else 
                        {
                            ellipse* v30 = ACIS_NEW ellipse(((ellipse&)geom).centre, ((ellipse&)geom).normal, new_major, 1.0, 0.0);
                            curve* cur = (curve*)v30;
                            if(all_spline) 
                            {
                                bs3_curve_def* v83 = bs3_curve_make_cur(*cur, range.start_pt(), range.end_pt(), 0.0);
                                if(v30) {
                                    v30 = nullptr;
                                } 
                                intcurve* v39 = ACIS_NEW intcurve(v83, 0.0, SpaAcis::NullObj::get_surface(), SpaAcis::NullObj::get_surface());
                                cur = (curve*)v39;
                            }
                            the_curve = cur;
                        }
                    }
                }
            }
        }
    }
    
    if(!the_curve) return sg_offset_planar_curve_internal(geom, range, fit_data, dist_law, twist_law, off_nor, tol);
    //if(woffset_module_header.debug_level >= 0x1E) {
    //    acis_fprintf(debug_file_ptr, "The offset curve is: \n");
    //    debug = the_curve->debug;
    //    debug(the_curve, "\t", debug_file_ptr);
    //    acis_fprintf(debug_file_ptr, "\n");
    //}
    return the_curve;
}

curve* sg_offset_planar_curve_internal(curve& geom, SPAinterval& range, double fit_data, law* dist_law, law* twist_law, SPAunit_vector& off_nor, double tol) 
{
   //  v7;                                                                                     // rax
   // AcisVersion* v8;                                                                                       // rax
   //  v9;                                                                                     // rax
   // bs3_curve_def* v10;                                                                                    // rax
   // __int64 v11;                                                                                           // rdx
   // __int64 v12;                                                                                           // rdx
   // long double v13;                                                                                       // xmm0_8
   // unsigned __int64 v15;                                                                                  // rax
   // unsigned __int64 v16;                                                                                  // rax
   // curve* v18;                                                                                            // rax
   // curve* v19;                                                                                            // rax
   // curve* v20;                                                                                            // rax
   // long double v21;                                                                                       // xmm0_8
   // long double v22;                                                                                       // xmm0_8
   // bs3_curve_def* v23;                                                                                    // rax
   //  num_pts;                                                                                           // [rsp+60h] [rbp-538h] BYREF
   // int n;                                                                                                 // [rsp+64h] [rbp-534h]
   // int i;                                                                                                 // [rsp+68h] [rbp-530h]
   // 
   // int ii;                                                                                                // [rsp+78h] [rbp-520h]
   // int repeats;                                                                                           // [rsp+7Ch] [rbp-51Ch]
   // int jj;                                                                                                // [rsp+80h] [rbp-518h]
   // int kk;                                                                                                // [rsp+84h] [rbp-514h]
   //  pos_array;                                                                                // [rsp+88h] [rbp-510h]
   // int j;                                                                                                 // [rsp+90h] [rbp-508h]
   // int k;                                                                                                 // [rsp+94h] [rbp-504h]
   // int m;                                                                                                 // [rsp+98h] [rbp-500h]
   // ;                                                                                               // [rsp+9Ch] [rbp-4FCh]
   // int err_num;                                                                                           // [rsp+A0h] [rbp-4F8h]
   // int resignal_no;                                                                                       // [rsp+A4h] [rbp-4F4h]
   //  offset_intcurve;                                                                                // [rsp+A8h] [rbp-4F0h]
   // int error_no;                                                                                          // [rsp+B0h] [rbp-4E8h]
   //  tan_array;                                                                                  // [rsp+B8h] [rbp-4E0h]
   //                                                                              // [rsp+C0h] [rbp-4D8h]
   // int num_ctrlpts;                                                                                       // [rsp+C4h] [rbp-4D4h] BYREF
   // SPAposition* ctrlpts;                                                                                  // [rsp+C8h] [rbp-4D0h] BYREF
   //;                                                                                      // [rsp+D0h] [rbp-4C8h]
   // int use_bs3_knots;                                                                                     // [rsp+D8h] [rbp-4C0h]
   // int coin_ctlpts;                                                                                       // [rsp+DCh] [rbp-4BCh]
   //;                                                                                   // [rsp+E0h] [rbp-4B8h] BYREF
   // int degree;                                                                                            // [rsp+E8h] [rbp-4B0h]
   // int v51;                                                                                               // [rsp+ECh] [rbp-4ACh]
   // void* __t;                                                                                             // [rsp+F0h] [rbp-4A8h]
   // void* v53;                                                                                             // [rsp+F8h] [rbp-4A0h]
   // long ;                                                                               // [rsp+100h] [rbp-498h]
   // curve* v55;                                                                                            // [rsp+108h] [rbp-490h]
   //  e_info;                                                                               // [rsp+110h] [rbp-488h]
   //  error_info_base_ptr;                                                                  // [rsp+118h] [rbp-480h]
   // curve* v58;                                                                                            // [rsp+120h] [rbp-478h]
   // bs3_curve_def* bsc;                                                                                    // [rsp+128h] [rbp-470h] BYREF
   // unsigned __int64 __n;                                                                                  // [rsp+130h] [rbp-468h]
   // SPAposition* v61;                                                                                      // [rsp+138h] [rbp-460h]
   // unsigned __int64 v62;                                                                                  // [rsp+140h] [rbp-458h]
   // SPAvector* v63;                                                                                        // [rsp+148h] [rbp-450h]
   // long double end_param;                                                                                 // [rsp+150h] [rbp-448h]
   // intcurve* v65;                                                                                         // [rsp+158h] [rbp-440h]
   // curve* v66;                                                                                            // [rsp+160h] [rbp-438h]
   // degenerate_curve* v67;                                                                                 // [rsp+168h] [rbp-430h]
   // curve* v68;                                                                                            // [rsp+170h] [rbp-428h]
   // intcurve* v69;                                                                                         // [rsp+178h] [rbp-420h]
   // AcisVersion v70;                                                                                       // [rsp+180h] [rbp-418h] BYREF
   // AcisVersion result;                                                                                    // [rsp+184h] [rbp-414h] BYREF
   // AcisVersion* vt2;                                                                                      // [rsp+188h] [rbp-410h]
   // AcisVersion* vt1;                                                                                      // [rsp+190h] [rbp-408h]
   // ;                                                                                      // [rsp+198h] [rbp-400h] BYREF
   // int(__fastcall * evaluate)(curve*, long double, SPAposition*, SPAvector**, int, evaluate_curve_side);  // [rsp+1A0h] [rbp-3F8h]
   //                                                                                       // [rsp+1A8h] [rbp-3F0h] BYREF
   // int(__fastcall * v77)(curve*, long double, SPAposition*, SPAvector**, int, evaluate_curve_side);       // [rsp+1B0h] [rbp-3E8h]
   // double v78;                                                                                            // [rsp+1B8h] [rbp-3E0h]
   //  v79;                                                                                            // [rsp+1C0h] [rbp-3D8h]
   // long double* v81;                                                                                      // [rsp+1D0h] [rbp-3C8h]
  
   // SPAposition* v83;                                                                                      // [rsp+1E0h] [rbp-3B8h]
   // SPAvector* v84;                                                                                        // [rsp+1E8h] [rbp-3B0h]
   // long double fit_tol;                                                                                   // [rsp+1F0h] [rbp-3A8h]
   //  domain;                                                                                   // [rsp+1F8h] [rbp-3A0h]
   // SPAinterval* safe_range;                                                                               // [rsp+200h] [rbp-398h]
   // surface* s2;                                                                                           // [rsp+208h] [rbp-390h]
   // surface* s1;                                                                                           // [rsp+210h] [rbp-388h]
   // bs3_curve_def* f;                                                                                      // [rsp+218h] [rbp-380h]
   // curve* v91;                                                                                            // [rsp+220h] [rbp-378h]
   // SPAvector* ddpos;                                                                                      // [rsp+228h] [rbp-370h]
   // long ;                                                                                     // [rsp+230h] [rbp-368h]
   // SPAposition* v94;                                                                                      // [rsp+238h] [rbp-360h]
   // SPAvector* v95;                                                                                        // [rsp+240h] [rbp-358h]
   //                                                                                        // [rsp+248h] [rbp-350h]
 
   // void(__fastcall * v99)(curve*);                                                                        // [rsp+260h] [rbp-338h]
   // __int64 v100;                                                                                          // [rsp+268h] [rbp-330h]
   // curve* v101;                                                                                           // [rsp+270h] [rbp-328h]
   // SPAbox* region_of_interest;                                                                            // [rsp+278h] [rbp-320h]
   // curve* v103;                                                                                           // [rsp+280h] [rbp-318h]
   // bs3_curve_def* bs;                                                                                     // [rsp+288h] [rbp-310h]
   // double v105;                                                                                           // [rsp+290h] [rbp-308h]
   // void* alloc_ptr;                                                                                       // [rsp+298h] [rbp-300h]
   // check_fix* fixed;                                                                                      // [rsp+2A0h] [rbp-2F8h]
   // check_fix* fix;                                                                                        // [rsp+2A8h] [rbp-2F0h]
   // void* v109;                                                                                            // [rsp+2B0h] [rbp-2E8h]
   // void* v110;                                                                                            // [rsp+2B8h] [rbp-2E0h]
   // void* v111;                                                                                            // [rsp+2C0h] [rbp-2D8h]
   // void(__fastcall * debug)(curve*, const char*, _iobuf*);                                                // [rsp+2C8h] [rbp-2D0h]
   // exception_save v113;                                                                                   // [rsp+2D0h] [rbp-2C8h] BYREF
   // exception_save exception_save_mark;                                                                    // [rsp+2E0h] [rbp-2B8h] BYREF
   //                                                                                 // [rsp+2F0h] [rbp-2A8h]
   //
   // long double resabs_sq;                                                                                 // [rsp+330h] [rbp-268h]
   // SPAvector d1;                                                                                          // [rsp+338h] [rbp-260h] BYREF
   // SPAvector d0;                                                                                          // [rsp+350h] [rbp-248h] BYREF
   // ;                                                                                        // [rsp+368h] [rbp-230h] BYREF
   // acis_exception v121;                                                                                   // [rsp+380h] [rbp-218h] BYREF
   // acis_exception error_info_holder;                                                                      // [rsp+3A0h] [rbp-1F8h] BYREF
   //  bs3_range;                                                                                 // [rsp+440h] [rbp-158h] BYREF
   // SPAposition p0;                                                                                        // [rsp+458h] [rbp-140h] BYREF
   // SPAposition p1;                                                                                        // [rsp+470h] [rbp-128h] BYREF
   // SPAposition second_pos;                                                                                // [rsp+488h] [rbp-110h] BYREF
   // SPAposition first_pos;                                                                                 // [rsp+4A0h] [rbp-F8h] BYREF
   // SPAinterval v128;                                                                                      // [rsp+4B8h] [rbp-E0h] BYREF
    

  /*  if(curve_module_header.debug_level >= 0x14) acis_fprintf(debug_file_ptr, "Inside sg_offset_planar_curve_internal.\n");*/
   
    //if(SPAresmch > tol) tol = SPAresmch;
    //int num_pts = 0;
    //curve*  offset_intcurve = nullptr;
    //int resignal_no = 0;
    //acis_exception error_info_holder(0);
    //error_info_base* error_info_base_ptr = nullptr;
    //exception_save exception_save_mark;
    //SPAposition* pos_array = nullptr;
    //SPAvector* tan_array = nullptr;
    //double* par_array = nullptr;
    //exception_save_mark.begin();
    //get_error_mark().buffer_init = 1;
    //int error_no = 0;
    //double start_param = range.start_pt();
    //double end_param = range.end_pt();
    //double delta = end_param - start_param;
    //int use_bs3_knots = 0;
    //SPAinterval bs3_range;
    //if(geom.type() == 11 && ((intcurve&)geom).cur_present()) 
    //{
    //    bs3_curve_def* v7 = ((intcurve&)geom).cur(-1.0, 0);
    //    bs3_range = bs3_curve_range(v7);
    //    use_bs3_knots = bs3_range == range;
    //}
    //if(use_bs3_knots && GET_ALGORITHMIC_VERSION() >= AcisVersion(12, 0, 0)) 
    //{
    //    bs3_curve_def* v9 = ((intcurve&)geom).cur(-1.0, 0);
    //    int degree = bs3_curve_degree(v9);
    //    bs3_curve_def* v10 = ((intcurve&)geom).cur(-1.0, 0);
    //    int repeats;
    //    bs3_curve_knots(v10, num_pts, par_array, 0);
    //    for(int i = 1; i < num_pts; ++i) {
    //        
    //        for(repeats = 0; repeats + i < num_pts && par_array[i - 1] == par_array[repeats + i]; ++repeats)
    //            ;
    //        if(repeats) 
    //        {
    //            if(repeats + i == num_pts) {
    //                par_array[i - 1] = par_array[num_pts - 1];
    //            } 
    //            else 
    //            {
    //                for(int j = i; j < num_pts - repeats; ++j) par_array[j] = par_array[repeats + j];
    //            }
    //            num_pts -= repeats;
    //            if(i != 1 && i != num_pts && degree - repeats <= 1) 
    //            {
    //                SPAposition p0;
    //                SPAposition p1;
    //                SPAvector d0;
    //                SPAvector d1;
    //                SPAvector* v0[1];
    //                SPAvector* v1[1];
    //                v0[0] = &d0;
    //                v1[0] = &d1;
    //                if(geom.evaluate(par_array[i - 1], p0, v0, 1, evaluate_curve_below) >= 1) 
    //                {
    //                    if(geom.evaluate(par_array[i], p1, v1, 1, evaluate_curve_above) >= 1) 
    //                    {
    //                        SPAvector diff = d0 - d1;
    //                        double v79 = diff % diff;
    //                        if(v79 > SPAresabs * SPAresabs) {
    //                            for(int k = num_pts; k > i; --k) par_array[k] = par_array[k - 1];
    //                            par_array[i] = par_array[i - 1];
    //                            ++num_pts;
    //                            ++i;
    //                        }
    //                    }
    //                }
    //            }
    //        }
    //    }
    //} 
    //else {
    //    num_pts = 7;
    //    double rands[7];  
    //    rands[0] = 0.0;
    //    rands[1] = 0.171642; //不确定
    //    rands[2] = 0.343284382; 
    //    rands[3] = 0.521247843;
    //    rands[4] = 0.798645987;
    //    rands[5] = 0.878743434;
    //    rands[6] = 1.0;
    //    double* v81 = ACIS_NEW double[num_pts];
    //    par_array = v81;
    //    for(int m = 0; m < num_pts; ++m) par_array[m] = start_param + rands[m] * delta;
    //}
    //SPAposition* v61 = ACIS_NEW SPAposition[num_pts];
    //pos_array = v61;
    //SPAvector* v63 = ACIS_NEW SPAvector[num_pts];
    //tan_array = v63;    
    //int err_num = 0;
    //acis_exception v121(0);
    //error_info_base* e_info = nullptr;
    //exception_save v113;
    //curve* temp_geom = nullptr;
    //bs3_curve_def* copy = nullptr;
    //v113.begin();
    //get_error_mark().buffer_init = 1;
    //int v37 = 0;
    //if(geom.type() == 11) {
    //    temp_geom = geom.make_copy();
    //} 
    //else {
    //    double fit_tol = SPAresabs / 10.0;
    //    SPAinterval v128(start_param, end_param);
    //    SPAinterval domain = v128;
    //    copy = curve_to_bs3_curve(&geom, v128, fit_tol, fit_tol, knot_removal_smart);
    //    intcurve* v66 = ACIS_NEW intcurve(copy, 0.0, SpaAcis::NullObj::get_surface(), SpaAcis::NullObj::get_surface());
    //    temp_geom = v66;
    //}
    //evaluate_curve_side side;
    //for(int n = 0; n < num_pts; ++n) {
    //    side = evaluate_curve_unknown;
    //    if(n < num_pts - 1 && par_array[n] == par_array[n + 1]) {
    //        side = evaluate_curve_below;
    //    } else if(n > 0 && par_array[n] == par_array[n - 1]) {
    //        side = evaluate_curve_above;
    //    }
    //    double param = par_array[n];
    //    sg_point_on_offset(*temp_geom, range, off_nor, param, pos_array[n], tan_array[n], SpaAcis::NullObj::get_vector(), dist_law, twist_law, side);
    //    //if(curve_module_header.debug_level >= 0x1E) {
    //    //    acis_fprintf(debug_file_ptr, "par = %g,\n\tpos = ", (double)par_array[(__int64)n]);
    //    //    v94 = &pos_array[n];
    //    //    SPAposition::debug(v94, debug_file_ptr);
    //    //    acis_fprintf(debug_file_ptr, "\n\ttan = ");
    //    //    v95 = &tan_array[n];
    //    //    SPAvector::debug(v95, debug_file_ptr);
    //    //    acis_fprintf(debug_file_ptr, "\n");
    //    //}
    //}
    //int ii;
    //for(int n = 1; n < num_pts; ++n) 
    //{
    //    for(ii = 0; ii + n < num_pts && par_array[n - 1] != par_array[ii + n] && same_point(pos_array[n - 1], pos_array[ii + n], fit_data / 10.0); ++ii) {
    //        ;
    //    }
    //    if(ii) {
    //        if(ii + n == num_pts) {
    //            pos_array[n - 1] = pos_array[num_pts - 1];
    //            par_array[n - 1] = par_array[num_pts - 1];
    //            tan_array[n - 1]=tan_array[num_pts - 1];
    //        } 
    //        else {
    //            for(int jj = n; jj < num_pts - ii; ++jj) {
    //                pos_array[jj]=pos_array[ii + jj];
    //                par_array[jj] = par_array[ii + jj];
    //                tan_array[jj] = tan_array[ii + jj];
    //            }
    //        }
    //        num_pts -= ii;
    //    }
    //}
    //int v37 = 0;//不懂
    //if(v37)
    //{
    //    if(copy) bs3_curve_delete(copy);
    //    copy = nullptr;
    //}
    //if(temp_geom) 
    //{
    //    temp_geom = nullptr;
    //}
    //if(err_num || acis_interrupted()) sys_error(err_num, e_info);
    //if(num_pts == 1) 
    //{
    //    degenerate_curve* v68 = ACIS_NEW degenerate_curve(*pos_array);
    //    offset_intcurve = v68;
    //} 
    //else 
    //{
    //    offset_int_interp offset_interp(geom, range, off_nor, num_pts, pos_array, tan_array, par_array, fit_data, dist_law, twist_law);
    //    intcurve* v58 = ACIS_NEW intcurve(offset_interp, SpaAcis::NullObj::get_box(), 0);
    //    v69 = (intcurve*)ACIS_OBJECT::operator new(0x68ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAkern\\kernel_sg_husk_intcur.m\\src\\ofintcur.cpp", 2184, &alloc_file_index_2857);
    //    if(v69) {
    //        region_of_interest = SpaAcis::NullObj::get_box();
    //        intcurve::intcurve(v69, &offset_interp, region_of_interest, 0);
    //        v58 = v20;
    //    } else {
    //        v58 = 0i64;
    //    }
    //    v103 = v58;
    //    offset_intcurve = v58;
    //    coin_ctlpts = 0;
    //    if(v103->type() == 11) 
    //    {
    //        bs = intcurve::cur((intcurve*)offset_intcurve, -1.0, 0);
    //        num_ctrlpts = 0;
    //        ctrlpts = 0i64;
    //        bs3_curve_control_points(bs, num_ctrlpts, ctrlpts, 0);
    //        v105 = safe_function_type<double>::operator double(&SPAresabs);
    //        v21 = safe_function_type<double>::operator double(&SPAresabs);
    //        resabs_sq = v105 * v21;
    //        for(kk = 1; kk < num_ctrlpts; ++kk) {
    //            SPAposition::SPAposition(&first_pos, &ctrlpts[kk]);
    //            v51 = kk - 1;
    //            SPAposition::SPAposition(&second_pos, &ctrlpts[kk - 1]);
    //            if(kk == 1 || kk == num_ctrlpts - 1) {
    //                v22 = safe_function_type<double>::operator double(&SPAresabs);
    //                if(same_point(&first_pos, &second_pos, v22)) {
    //                    coin_ctlpts = 1;
    //                    break;
    //                }
    //            }
    //        }
    //        if(ctrlpts) {
    //            alloc_ptr = ctrlpts;
    //            SPAposition::operator delete[](ctrlpts);
    //        }
    //        ctrlpts = 0i64;
    //    }
    //    if(coin_ctlpts) {
    //        v23 = intcurve::cur((intcurve*)offset_intcurve, -1.0, 0);
    //        bsc = bs3_curve_copy(v23);
    //        fixed = SpaAcis::NullObj::get_check_fix();
    //        fix = SpaAcis::NullObj::get_check_fix();
    //        bs3_curve_fix_coi_verts(&bsc, fix, fixed);
    //        ((intcurve*)offset_intcurve)->set_cur(bsc, -1.0, 1, 0);
    //    }
    //}
    //v109 = par_array;
    //ACIS_DELETE[] STD_CAST par_array;
    //ACIS_DELETE[] pos_array;
    //ACIS_DELETE[] tan_array;
    //if(resignal_no || acis_interrupted()) sys_error(resignal_no, error_info_base_ptr);
    ////if(curve_module_header.debug_level >= 0x1E) {
    ////    acis_fprintf(debug_file_ptr, "The offset curve is: \n");
    ////    debug = offset_intcurve->debug;
    ////    debug(offset_intcurve, "\t", debug_file_ptr);
    ////    acis_fprintf(debug_file_ptr, "\n");
    ////}
    //return offset_intcurve;
  return NULL;
}


void sg_point_on_offset(curve& orig_curve, SPAinterval& __formal, SPAunit_vector& n, double param, SPAposition& pos, SPAvector& dpos, SPAvector& ddpos, law* dist_law, law* twist_law, evaluate_curve_side side)
{
 /*   if(curve_module_header.debug_level >= 0x1E) 
    {
        acis_fprintf(debug_file_ptr, "sg_point_on_offset input: \n");
        acis_fprintf(debug_file_ptr, "\tparam: %g\n", (double)param);
        acis_fprintf(debug_file_ptr, "\tthe original curve: \n");
        debug = orig_curve->debug;
        debug(orig_curve, (const char*)&unk_183FE634F, debug_file_ptr);
        debug_newline(debug_file_ptr);
        acis_fprintf(debug_file_ptr, "\tthe dist law: ");
        if(dist_law) {
            string = dist_law->string;
            law_data_node_ptr = SpaAcis::NullObj::get_law_data_node_ptr();
            v152 = SpaAcis::NullObj::get_int();
            str = string(dist_law, DEFAULT, v152, law_data_node_ptr);
            acis_fprintf(debug_file_ptr, "%s\n", str);
            alloc_ptr = str;
            ACIS_STD_TYPE_OBJECT::operator delete[](str);
        } else {
            acis_fprintf(debug_file_ptr, "NULL\n");
        }
        acis_fprintf(debug_file_ptr, "\tthe twist law: \n");
        if(twist_law) {
            v62 = twist_law->string;
            v60 = SpaAcis::NullObj::get_law_data_node_ptr();
            v61 = SpaAcis::NullObj::get_int();
            v51 = v62(twist_law, DEFAULT, v61, v60);
            acis_fprintf(debug_file_ptr, "%s\n", v51);
            v63 = v51;
            ACIS_STD_TYPE_OBJECT::operator delete[](v51);
        } else {
            acis_fprintf(debug_file_ptr, "NULL\n");
        }
    }*/
    SPAvector* deriv[3];
    SPAposition p_gx(0.0, 0.0, 0.0);
    SPAvector gpx(0.0, 0.0, 0.0);
    SPAvector gppx(0.0, 0.0, 0.0);
    SPAvector gpppx(0.0, 0.0, 0.0);
    deriv[0] = &gpx;
    deriv[1] = &gppx;
    deriv[2] = &gpppx;
    int nd = 1;
    if(&ddpos && !SpaAcis::NullObj::check_vector(ddpos) || &dpos && !SpaAcis::NullObj::check_vector(dpos)) nd = 3;//不确定
    int ngot = orig_curve.evaluate(param, p_gx, deriv, nd, side);
    double z = p_gx.z();
    double y = p_gx.y();
    double x = p_gx.x();
    SPAvector gx(x, y, z);
    if(&ddpos && !SpaAcis::NullObj::check_vector(ddpos) && ngot < 3) {
        SPAvector junk1;
        SPAvector junk2;
        SPAposition p_gppa;
        SPAposition p_gppb;
        orig_curve.eval(param - 0.0001, p_gppa, junk1, junk2, 0, 0);
        orig_curve.eval(param + 0.0001, p_gppb, junk1, junk2, 0, 0);//不确定
        double v70 = p_gppa.z();
        double v71 = p_gppa.y();
        double v72 = p_gppa.x();
        SPAvector gppa(v72, v71, v70);
        double v73 = p_gppb.z();
        double v74 = p_gppb.y();
        double v75 = p_gppb.x();
        SPAvector gppb(v75, v74, v73);
        const SPAvector v13 = gppb - gppa;
        gpppx = v13 * 1.0 / (2.0 * 0.0001);
    }
    SPAvector rx;
    SPAvector px;
    SPAvector v;
    SPAvector fx;



    double d  =  dist_law->eval(param);
    double tx = -twist_law->eval(param);
    rx=n;
    double deriv_length = gpx.len();
    if(SPAresnor > deriv_length) {
   
        sys_error(BS3_COI_VERTS);
    }
    double sx = 1.0 / deriv_length;
    px = gpx * 1.0 / deriv_length;//单位切矢
    v = px*rx;
    double costx = acis_cos(tx);
    double sintx = acis_sin(tx);
    SPAvector v2 = sintx * rx;
    SPAvector v1 = costx * v;
    const SPAvector v16 = v1 + v2;
    const SPAvector v17 = d * v16;
    fx = gx + v17;
    if(&pos && !SpaAcis::NullObj::check_position(pos)) {
        double zi = fx.z();
        double yi = fx.y();
        double xi = fx.x();
        SPAposition p_fx(xi, yi, zi);
        pos = p_fx;
    }







    double gpx_d_gppx = 0.0;
    SPAvector zero(0.0, 0.0, 0.0);
    double dpx = 0.0;
    double tpx = 0.0;
    double spx = 0.0;
    SPAvector rpx;
    SPAvector ppx;
    SPAvector bpx;
    SPAvector fpx;
    double sx3 = sx * sx * sx;
    if(&dpos && !SpaAcis::NullObj::check_vector(dpos) || &ddpos && !SpaAcis::NullObj::check_vector(ddpos)) {
        dpx = dist_law->evaluateDR_R(param,1);
        tpx = -twist_law->evaluateDR_R(param, 1) ;
        rpx=zero;
        double gpx_d_gppx = gpx % gppx;
        spx = - gpx_d_gppx  * sx3;
        SPAvector v91 = sx * gppx;
        SPAvector v92 = gpx * spx;
        SPAvector ppx = v92 + v91;
        SPAvector v93 = ppx * rx;
        SPAvector v94 = px * rpx;
        SPAvector bpx = v94 + v93;
        const SPAvector v18 = v * sintx;
        SPAvector v99 = v18 * tpx;
        SPAvector v19 = costx * rx;
        SPAvector v97 = v19 * tpx;
        SPAvector v95 = sintx * rpx;
        SPAvector v96 = costx * bpx;
        SPAvector v98 = v96 + v95;
        SPAvector v100 = v98 + v97;
        const SPAvector v20 = v100 - v99;
        SPAvector v103 = d * v20;
        SPAvector v101 = rx * sintx;
        SPAvector v102 = v * costx;
        const SPAvector v21 = v102 + v101;
        const SPAvector v22 = v21 * dpx;
        SPAvector v104 = v22 + gpx;
        fpx=v104+v103;
        if(&dpos) {
            if(!SpaAcis::NullObj::check_vector(dpos)) dpos=fpx;
        }
    }
    SPAvector fppx;
    if(&ddpos) {
        if(!SpaAcis::NullObj::check_vector(ddpos)) {
            double sx5 = sx3 * sx * sx;
            SPAvector rppx;
            SPAvector pppx;
            SPAvector bppx;
            double dppx = dist_law->evaluateDR_R(param, 2);
            double tppx = -twist_law->evaluateDR_R(param, 2) ;
            rppx=zero;
            double sx2 = sx * sx;
            double v112 = sx * sx * sx;
            double v113 = 3.0 * gpx_d_gppx * gpx_d_gppx * sx5;
            double v111 = gppx % gppx;
            double v23 = gpx % gpppx;
            double sppx = v113 - (v111 + v23) * v112;
            SPAvector v117 = sx * gpppx;
            SPAvector v115 = gpx * sppx;
            SPAvector v116 = (2.0 * spx) * gppx;
            SPAvector v118 = v116 + v115;
            pppx =v118+v117;
            SPAvector v121 = pppx * rx;
            const SPAvector v24 = 2.0 * ppx;
            SPAvector v119 = v24 * rpx;
            SPAvector v120 = px * rppx;
            SPAvector v122 = v120 + v119;
            bppx = v122+v121;
            const SPAvector v25 = v * sintx;
            SPAvector v135 = v25 * tppx;
            const SPAvector v26 = costx * rx; 
            SPAvector v133 =  v26 * tppx;
            SPAvector v131 =  sintx*rppx;
            SPAvector v129 =  costx*bppx;
            const SPAvector v27 =  rx* sintx;
            const SPAvector v28 =  v27* tpx;
            SPAvector v127 = v28 * tpx;
            const SPAvector v29 = v * costx;
            const SPAvector v30 = v29 * tpx;
            SPAvector v125 = v30 * tpx;
            const SPAvector v31 = (2.0 * costx) * rpx;
            SPAvector v123 = v31 * tpx;
            const SPAvector v32 = (-2.0 * sintx) * bpx;
            SPAvector v124 = v32 * tpx;
            SPAvector v126 = v124 + v123;
            SPAvector v128 = v126 - v125;
            SPAvector v130 = v128 - v127;
            SPAvector v132 = v130 + v129;
            SPAvector v134 = v132 + v131;
            SPAvector v136 = v134 + v133;
            const SPAvector v33 = v136 - v135;
            SPAvector v148 =  d * v33;
            SPAvector v137 = rx * sintx;
            SPAvector v138 = v * costx;
            const SPAvector v34 = v138 + v137;
            SPAvector v146 = v34 * dppx;
            const SPAvector v35 = v * sintx;
            SPAvector v144 = v35 * tpx;
            const SPAvector v36 = costx * rx;
            SPAvector v142 = v36 * tpx;
            SPAvector v140 = sintx* rpx;
            SPAvector v141 = costx* bpx;
            SPAvector v143 = v141 + v140;
            SPAvector v145 = v143 + v142;
            const SPAvector v37 = v145 - v144;
            SPAvector v147 = (2.0 * dpx) * v37;
            const SPAvector v38 = v147 + v146;
            SPAvector v149 = v38 + gppx;
            fppx = v149+ v148;
            ddpos = fppx;
        }
    }


}

//
//offset_int_interp::offset_int_interp(curve& orig_spline, SPAinterval& range, SPAunit_vector& curve_normal, int npt, SPAposition* pos_arr, SPAvector* tan_arr, double* par_arr, double fit_data, law* law_dist, law* law_twist) {
//
//
//    this->offset_plane = SPAunit_vector();
//    this->curve_range =SPAinterval();
//    this->param = par_arr;
//    this->dist_law = law_dist;
//    this->dist_law->add();
//    this->twist_law = law_twist;
//    this->twist_law->add();
//    this->offset_plane = curve_normal;
//    this->curve_range = range;
//    this->orig_curve = orig_spline.make_copy();
//}
