
#include "PublicInterfaces/gme_sgcofrtn.hxx"
#include "acis/law_base.hxx"
#include "acis/main_law.hxx"
#include "acis/dmexcept.hxx"
#include "PrivateInterfaces/ofst_edge_smooth_manager.hxx"
#include "acis/get_top.hxx"
#include "acis/err_info_base.hxx"
#include "acis/list_header.hxx"
#include "acis/deltop.hxx"
#include "acis/point.hxx"
#include "ProtectedInterfaces/ofsttools.hxx"


BODY* sg_offset_planar_wire(WIRE* wire, TRANSFORM* wire_transf, double offset_dist, SPAunit_vector& wire_normal, sg_gap_type close_type, int add_attribs, int trim, int zero_length, int overlap) {
    acis_exception error_info_holder(0, (error_info_base*)0);
    exception_save exception_save_mark;
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    constant_law* v13 = ACIS_NEW constant_law(offset_dist);
    constant_law* v15 = ACIS_NEW constant_law(0.0);
    BODY* result = sg_offset_planar_wire(wire, wire_transf, v13, v15, wire_normal, close_type, add_attribs, trim, zero_length, overlap, 0);
    if (acis_interrupted())
    {
        sys_error(0,(error_info_base*)0);
    }
    v13->remove();
    v15->remove();
    return result;
}

BODY* sg_offset_planar_wire(WIRE* wire, TRANSFORM* wire_transf, law* dist_law, law* twist_law, SPAunit_vector& wire_normal, sg_gap_type close_type, int add_attribs, int trim, int zero_length, int overlap, int iKeepMiniTopo) 
{
    AcisVersion vt2 = AcisVersion(20, 0, 0);
    AcisVersion vt1 = GET_ALGORITHMIC_VERSION();
    int useSmoothMgr = (vt1 >= vt2) && (!dist_law || dist_law->constant());
    option_header* no_smooth = find_option("ofst_planar_wire_no_smooth");
    if(no_smooth->on()) 
    {
        useSmoothMgr = 0;
    }
    ofst_edge_smooth_manager edgeSmoothMgr;
    if(useSmoothMgr) {
        ENTITY_LIST edges;
        get_edges(wire, edges);
        edgeSmoothMgr.init(&edges, 0, 5);
    }
    BODY* offset_wire_body = nullptr;
    ofst_edge_smooth_manager* iEdgeSmoothMgr=nullptr;
    if(useSmoothMgr) {
        iEdgeSmoothMgr = &edgeSmoothMgr;
    }
    offset_wire_body = sg_offset_planar_wire_internal(wire, wire_transf, dist_law, twist_law, wire_normal, close_type, add_attribs, trim, zero_length, overlap, iKeepMiniTopo, iEdgeSmoothMgr);
    if(useSmoothMgr) {
        //int max_iteration = 10;
        for(int numIter = 0; !offset_success(offset_wire_body) && edgeSmoothMgr.apply_more_smooth(numIter) && numIter < 10; ++numIter) {
            if(offset_wire_body)
            {
                delete_entity(offset_wire_body);
            }
            offset_wire_body = sg_offset_planar_wire_internal(wire, wire_transf, dist_law, twist_law, wire_normal, close_type, add_attribs, trim, zero_length, overlap, iKeepMiniTopo, &edgeSmoothMgr);
        }
    }
    return offset_wire_body;
}


int offset_success(BODY*& iOffsetBody) {
    int retOk=0;
    if(iOffsetBody) 
    {
        ENTITY_LIST wireList;
        get_wires(iOffsetBody,wireList);
        retOk = wireList.count()> 0;
      
    }
    return retOk;
}


void display_wire_offset_io(WIRE* in_wire, law* dist_law, BODY* out_wire) {
    char brkpt_str[60];                                                        // [rsp+A8h] [rbp-150h] BYREF
    if(out_wire)
        sprintf(brkpt_str, "%s", "wire offset - output");
    else
        sprintf(brkpt_str, "%s", "wire offset - inputs");
}

int is_open_wire(WIRE* w) {
    int bAnswer = 1;
    COEDGE* first_coedge = start_of_wire_chain(w);
    COEDGE* last_coedge = first_coedge;
    for(COEDGE* next_coedge = first_coedge->next(); next_coedge && next_coedge != last_coedge && next_coedge != first_coedge; next_coedge = next_coedge->next()) {
        last_coedge = next_coedge;
    }
    SPAposition start_pos(first_coedge->start()->geometry()->coords());
    SPAposition end_pos(last_coedge->end()->geometry()->coords());
    if(start_pos==end_pos)
    {
        return 0;
    }
    return bAnswer;
}

BODY* sg_offset_planar_wire_internal(WIRE* wire, TRANSFORM* wire_transf, law* dist_law, law* twist_law, SPAunit_vector& wire_normal, sg_gap_type close_type, int add_attribs, int trim, int zero_length, int overlap, int iKeepMiniTopo,
                                     ofst_edge_smooth_manager* iEdgeSmoothMgr) {
    // int v12;                                                                                               // eax
    // const AcisVersion* v13;                                                                                // rax
    // int v14;                                                                                               // eax
    // long double v15;                                                                                       // xmm0_8
    // long double v16;                                                                                       // xmm0_8
    // long double v17;                                                                                       // xmm0_8
    // long double v18;                                                                                       // xmm0_8
    // const AcisVersion* v19;                                                                                // rax
    // EDGE* v20;                                                                                             // rax
    // const curve* v21;                                                                                      // rax
    // EDGE* v22;                                                                                             // rax
    // const AcisVersion* v23;                                                                                // rax
    // EDGE* v24;                                                                                             // rax
    // const curve* v25;                                                                                      // rax
    // EDGE* v26;                                                                                             // rax
    // const curve* v27;                                                                                      // rax
    // EDGE* v28;                                                                                             // rax
    // EDGE* v29;                                                                                             // rax
    // SPAvector* v30;                                                                                        // rax
    // SPAvector* v31;                                                                                        // rax
    // double v32;                                                                                            // xmm0_8
    // double v33;                                                                                            // xmm0_8
    // double v34;                                                                                            // xmm0_8
    // double v35;                                                                                            // xmm0_8
    // SPAvector* v36;                                                                                        // rax
    // double v37;                                                                                            // xmm0_8
    // VERTEX* v38;                                                                                           // rax
    // APOINT* v39;                                                                                           // rax
    // VERTEX* v40;                                                                                           // rax
    // APOINT* v41;                                                                                           // rax
    // const SPAtransf* v42;                                                                                  // rax
    // SPAunit_vector* started;                                                                               // rax
    // const SPAtransf* v44;                                                                                  // rax
    // SPAunit_vector* v45;                                                                                   // rax
    // const AcisVersion* v46;                                                                                // rax
    // EDGE* v47;                                                                                             // rax
    // WIRE* v48;                                                                                             // rax
    // wire_law_data* v49;                                                                                    // rax
    // EDGE* v50;                                                                                             // rax
    // EDGE* v51;                                                                                             // rax
    // const curve* v52;                                                                                      // rax
    // COEDGE* v53;                                                                                           // rax
    // COEDGE* v54;                                                                                           // rax
    // EDGE* v55;                                                                                             // rax
    // EDGE* v56;                                                                                             // rax
    // EDGE* v57;                                                                                             // rax
    // VERTEX* v58;                                                                                           // rax
    // offset_segment* v59;                                                                                   // rax
    // offset_segment* v60;                                                                                   // rax
    // COEDGE* v61;                                                                                           // rax
    // COEDGE* v62;                                                                                           // rax
    // ANNOTATION* v63;                                                                                       // rax
    // AcisVersion* v64;                                                                                      // rax
    // COEDGE* v65;                                                                                           // rax
    // EDGE* v66;                                                                                             // rax
    // ANNOTATION* v67;                                                                                       // rax
    // EDGE* v68;                                                                                             // rax
    // EDGE* v69;                                                                                             // rax
    // VERTEX* v70;                                                                                           // rax
    // APOINT* v71;                                                                                           // rax
    // const SPAposition* v72;                                                                                // rax
    // VERTEX* v73;                                                                                           // rax
    // APOINT* v74;                                                                                           // rax
    // const SPAposition* v75;                                                                                // rax
    // SPAvector* v76;                                                                                        // rax
    // double v77;                                                                                            // xmm0_8
    // EDGE* v78;                                                                                             // rax
    // EDGE* v79;                                                                                             // rax
    // VERTEX* v80;                                                                                           // rax
    // APOINT* v81;                                                                                           // rax
    // const SPAposition* v82;                                                                                // rax
    // COEDGE* v83;                                                                                           // rax
    // VERTEX* v84;                                                                                           // rax
    // APOINT* v85;                                                                                           // rax
    // const SPAposition* v86;                                                                                // rax
    // SPAvector* v87;                                                                                        // rax
    // long double v88;                                                                                       // xmm0_8
    // EDGE* v89;                                                                                             // rax
    // VERTEX* v90;                                                                                           // rax
    // VERTEX* v91;                                                                                           // rax
    // VERTEX* v92;                                                                                           // rax
    // APOINT* v93;                                                                                           // rax
    // const SPAposition* v94;                                                                                // rax
    // VERTEX* v95;                                                                                           // rax
    // APOINT* v96;                                                                                           // rax
    // const SPAposition* v97;                                                                                // rax
    // const SPAvector* v98;                                                                                  // rax
    // VERTEX* v99;                                                                                           // rax
    // VERTEX* v100;                                                                                          // rax
    // AcisVersion* v101;                                                                                     // rax
    // long double v102;                                                                                      // xmm0_8
    // AcisVersion* v103;                                                                                     // rax
    // SPAparameter* v104;                                                                                    // rax
    // SPAparameter* v105;                                                                                    // rax
    // AcisVersion* v106;                                                                                     // rax
    // AcisVersion* v107;                                                                                     // rax
    // EDGE* v108;                                                                                            // rax
    // EDGE* v109;                                                                                            // rax
    // const curve* v110;                                                                                     // rax
    // EDGE* v111;                                                                                            // rax
    // EDGE* v112;                                                                                            // rax
    // EDGE* v113;                                                                                            // rax
    // const curve* v114;                                                                                     // rax
    // EDGE* v115;                                                                                            // rax
    // EDGE* v116;                                                                                            // rax
    // offset_segment* v117;                                                                                  // rax
    // COEDGE* v118;                                                                                          // rax
    // VERTEX* v119;                                                                                          // rax
    // VERTEX* v120;                                                                                          // rax
    // VERTEX* v121;                                                                                          // rax
    // VERTEX* v122;                                                                                          // rax
    // APOINT* v123;                                                                                          // rax
    // const SPAposition* v124;                                                                               // rax
    // VERTEX* v125;                                                                                          // rax
    // APOINT* v126;                                                                                          // rax
    // const SPAposition* v127;                                                                               // rax
    // const SPAvector* v128;                                                                                 // rax
    // VERTEX* v129;                                                                                          // rax
    // VERTEX* v130;                                                                                          // rax
    // AcisVersion* v131;                                                                                     // rax
    // long double v132;                                                                                      // xmm0_8
    // SPAparameter* v133;                                                                                    // rax
    // __int64 v134;                                                                                          // rax
    // TRANSFORM* v135;                                                                                       // rax
    // long double v136;                                                                                      // xmm0_8
    // offset_segment* v138;                                                                                  // rax
    // EDGE* v139;                                                                                            // rax
    // EDGE* v140;                                                                                            // rax
    // EDGE* v141;                                                                                            // rax
    // EDGE* v142;                                                                                            // rax
    // bs3_curve_def* v143;                                                                                   // rax
    // bs3_curve_def* v144;                                                                                   // rax
    // EDGE* v145;                                                                                            // rax
    // long double v146;                                                                                      // xmm0_8
    // LUMP* v147;                                                                                            // rax
    // LUMP* v148;                                                                                            // rax
    // SHELL* v149;                                                                                           // rax
    // LUMP* v150;                                                                                            // rax
    // int v151;                                                                                              // eax
    // COEDGE* v152;                                                                                          // rax
    // COEDGE* v153;                                                                                          // rax
    // COEDGE* v154;                                                                                          // rax
    // COEDGE* v155;                                                                                          // rax
    // VERTEX* v156;                                                                                          // rax
    // VERTEX* v157;                                                                                          // rax
    // AcisVersion* v158;                                                                                     // rax
    // bool v159;                                                                                             // al
    // LUMP* v160;                                                                                            // rax
    // COEDGE* v161;                                                                                          // rax
    // COEDGE* v162;                                                                                          // rax
    // VERTEX* v163;                                                                                          // rax
    // VERTEX* v164;                                                                                          // rax
    // WIRE* v165;                                                                                            // rax
    // SHELL* v166;                                                                                           // rax
    // LUMP* v167;                                                                                            // rax
    // WIRE* v168;                                                                                            // rax
    // COEDGE* v169;                                                                                          // rax
    // WIRE* v170;                                                                                            // rax
    // COEDGE* v171;                                                                                          // rax
    // VERTEX* v172;                                                                                          // rax
    // APOINT* v173;                                                                                          // rax
    // WIRE* v174;                                                                                            // rax
    // COEDGE* v175;                                                                                          // rax
    // SPAtransf* v176;                                                                                       // rax
    // SPAvector* v177;                                                                                       // rax
    // double v178;                                                                                           // xmm0_8
    // WIRE* v179;                                                                                            // rax
    // const SPAvector* v180;                                                                                 // rax
    // SPAunit_vector* v181;                                                                                  // rax
    // const curve* v182;                                                                                     // rax
    // int v183;                                                                                              // eax
    // LUMP* v184;                                                                                            // rax
    // SPAparameter* v185;                                                                                    // rax
    // double v186;                                                                                           // xmm0_8
    // __int64 v187;                                                                                          // rdx
    // long double v188;                                                                                      // xmm0_8
    // double v189;                                                                                           // xmm0_8
    // long double v190;                                                                                      // xmm0_8
    // SPAparameter* v191;                                                                                    // rax
    // __int64 v192;                                                                                          // rdx
    // long double v193;                                                                                      // xmm0_8
    // double v194;                                                                                           // xmm0_8
    // long double v195;                                                                                      // xmm0_8
    // SPAparameter* v196;                                                                                    // rax
    // SPAparameter* v197;                                                                                    // rax
    // VERTEX* v198;                                                                                          // rax
    // VERTEX* v199;                                                                                          // rax
    // VERTEX* v200;                                                                                          // rax
    // VERTEX* v201;                                                                                          // rax
    // long double v202;                                                                                      // xmm0_8
    // VERTEX* v203;                                                                                          // rax
    // APOINT* v204;                                                                                          // rax
    // __int64 v205;                                                                                          // rdx
    // SPAvector* v206;                                                                                       // rax
    // long double v207;                                                                                      // xmm1_8
    // long double v208;                                                                                      // xmm0_8
    // __int64 v209;                                                                                          // rdx
    // SPAvector* v210;                                                                                       // rax
    // long double v211;                                                                                      // xmm0_8
    // double v212;                                                                                           // xmm0_8
    // long double v213;                                                                                      // xmm0_8
    // long double v214;                                                                                      // xmm0_8
    // VERTEX* v215;                                                                                          // rax
    // APOINT* v216;                                                                                          // rax
    // __int64 v217;                                                                                          // rdx
    // SPAvector* v218;                                                                                       // rax
    // long double v219;                                                                                      // xmm1_8
    // long double v220;                                                                                      // xmm0_8
    // __int64 v221;                                                                                          // rdx
    // SPAvector* v222;                                                                                       // rax
    // long double v223;                                                                                      // xmm0_8
    // long double v224;                                                                                      // xmm0_8
    // SPAinterval* v225;                                                                                     // rax
    // __int64 v226;                                                                                          // r8
    // APOINT* v227;                                                                                          // rax
    // VERTEX* v228;                                                                                          // rax
    // VERTEX* v229;                                                                                          // rax
    // APOINT* v230;                                                                                          // rax
    // APOINT* v231;                                                                                          // rax
    // VERTEX* v232;                                                                                          // rax
    // SPAinterval* v233;                                                                                     // rax
    // __int64 v234;                                                                                          // r8
    // APOINT* v235;                                                                                          // rax
    // VERTEX* v236;                                                                                          // rax
    // VERTEX* v237;                                                                                          // rax
    // APOINT* v238;                                                                                          // rax
    // APOINT* v239;                                                                                          // rax
    // VERTEX* v240;                                                                                          // rax
    // int v241;                                                                                              // eax
    // bool degen_off_coed_deleted;                                                                           // [rsp+60h] [rbp-1658h]
    // int i;                                                                                                 // [rsp+64h] [rbp-1654h]
    // EDGE* edge;                                                                                            // [rsp+68h] [rbp-1650h]
    // COEDGE* this_coedge;                                                                                   // [rsp+70h] [rbp-1648h]
    // COEDGE* this_coedgea;                                                                                  // [rsp+70h] [rbp-1648h]
    // BODY* offset_wire_body;                                                                                // [rsp+78h] [rbp-1640h]
    // COEDGE* this_off_coedge;                                                                               // [rsp+80h] [rbp-1638h]
    // COEDGE* prev_off_coedge;                                                                               // [rsp+88h] [rbp-1630h]
    // SHELL* a_shell;                                                                                        // [rsp+90h] [rbp-1628h]
    // SHELL* a_shella;                                                                                       // [rsp+90h] [rbp-1628h]
    // SHELL* a_shellb;                                                                                       // [rsp+90h] [rbp-1628h]
    // SHELL* a_shellc;                                                                                       // [rsp+90h] [rbp-1628h]
    // int coedge_count;                                                                                      // [rsp+98h] [rbp-1620h]
    // int n;                                                                                                 // [rsp+9Ch] [rbp-161Ch]
    // int kk;                                                                                                // [rsp+9Ch] [rbp-161Ch]
    // const curve* edge_cur;                                                                                 // [rsp+A0h] [rbp-1618h]
    // COEDGE* prev_coedge;                                                                                   // [rsp+A8h] [rbp-1610h]
    // COEDGE* prev_coedgea;                                                                                  // [rsp+A8h] [rbp-1610h]
    // COEDGE* prev_coedgeb;                                                                                  // [rsp+A8h] [rbp-1610h]
    // int ii;                                                                                                // [rsp+B0h] [rbp-1608h]
    // WIRE* its_wire;                                                                                        // [rsp+B8h] [rbp-1600h]
    // WIRE* its_wirea;                                                                                       // [rsp+B8h] [rbp-1600h]
    // WIRE* its_wireb;                                                                                       // [rsp+B8h] [rbp-1600h]
    // ENTITY_LIST off_coedge_list;                                                                           // [rsp+C0h] [rbp-15F8h] BYREF
    // COEDGE* start_coedge;                                                                                  // [rsp+C8h] [rbp-15F0h]
    // COEDGE* previous;                                                                                      // [rsp+D0h] [rbp-15E8h]
    // COEDGE* offset_coedge;                                                                                 // [rsp+D8h] [rbp-15E0h]
    // int j;                                                                                                 // [rsp+E0h] [rbp-15D8h]
    // COEDGE* prev_orig_coedge;                                                                              // [rsp+E8h] [rbp-15D0h]
    // SHELL* shell;                                                                                          // [rsp+F0h] [rbp-15C8h]
    // COEDGE* coed;                                                                                          // [rsp+F8h] [rbp-15C0h]
    // int skipbackup;                                                                                        // [rsp+100h] [rbp-15B8h]
    // int jj;                                                                                                // [rsp+104h] [rbp-15B4h]
    // int lawi;                                                                                              // [rsp+108h] [rbp-15B0h]
    // int resignal_no;                                                                                       // [rsp+10Ch] [rbp-15ACh]
    // int err_num;                                                                                           // [rsp+110h] [rbp-15A8h]
    // COEDGE* last_off_coedge;                                                                               // [rsp+118h] [rbp-15A0h]
    // COEDGE* coed_next;                                                                                     // [rsp+120h] [rbp-1598h]
    // COEDGE* first_off_coedge;                                                                              // [rsp+128h] [rbp-1590h]
    // LUMP* current_lump;                                                                                    // [rsp+130h] [rbp-1588h]
    //
    // int is_offset_by_radius;                                                                               // [rsp+13Ch] [rbp-157Ch]
    // int error_no;                                                                                          // [rsp+140h] [rbp-1578h]
    // int v286;                                                                                              // [rsp+144h] [rbp-1574h]
    // int index;                                                                                             // [rsp+148h] [rbp-1570h]
    // int simple_wire;                                                                                       // [rsp+14Ch] [rbp-156Ch]
    // COEDGE* start_coed;                                                                                    // [rsp+150h] [rbp-1568h]
    // COEDGE* this_orig_coedge;                                                                              // [rsp+158h] [rbp-1560h]
    // law** dist_laws;                                                                                       // [rsp+160h] [rbp-1558h]
    // bs3_curve_def* new_bs3;                                                                                // [rsp+168h] [rbp-1550h] BYREF
    // long double start_par;                                                                                 // [rsp+170h] [rbp-1548h]
    // long double end_par;                                                                                   // [rsp+178h] [rbp-1540h]
    // long double dir;                                                                                       // [rsp+180h] [rbp-1538h]
    // long double end_t_step;                                                                                // [rsp+188h] [rbp-1530h]
    // offset_segment* curr_seg;                                                                              // [rsp+190h] [rbp-1528h] BYREF
    // ENTITY_LIST orig_coedge_list;                                                                          // [rsp+198h] [rbp-1520h] BYREF
    // RenderingObject* v299;                                                                                 // [rsp+1A0h] [rbp-1518h]
    // long double start_t_step;                                                                              // [rsp+1A8h] [rbp-1510h]
    // long double max_t_step;                                                                                // [rsp+1B0h] [rbp-1508h]
    // int nni;                                                                                               // [rsp+1B8h] [rbp-1500h]
    // int reset_period;                                                                                      // [rsp+1BCh] [rbp-14FCh]
    // SPAvector* _deriv_ptr[3];                                                                              // [rsp+1C0h] [rbp-14F8h] BYREF
    // int dist_law_size;                                                                                     // [rsp+1D8h] [rbp-14E0h] BYREF
    // int did_close;                                                                                         // [rsp+1DCh] [rbp-14DCh]
    // int dist_law_index;                                                                                    // [rsp+1E0h] [rbp-14D8h]
    // int wire_periodic;                                                                                     // [rsp+1E4h] [rbp-14D4h]
    // int check_status;                                                                                      // [rsp+1E8h] [rbp-14D0h]
    // int skipStartDegOffsetCoedges;                                                                         // [rsp+1ECh] [rbp-14CCh]
    // double v311;                                                                                           // [rsp+1F0h] [rbp-14C8h]
    // long double div;                                                                                       // [rsp+1F8h] [rbp-14C0h]
    // WIRE* offset_wire;                                                                                     // [rsp+200h] [rbp-14B8h]
    // int v314;                                                                                              // [rsp+208h] [rbp-14B0h]
    // int v315;                                                                                              // [rsp+20Ch] [rbp-14ACh]
    // int v316;                                                                                              // [rsp+210h] [rbp-14A8h]
    // int sign1;                                                                                             // [rsp+214h] [rbp-14A4h]
    // int openWire;                                                                                          // [rsp+218h] [rbp-14A0h]
    // int start;                                                                                             // [rsp+21Ch] [rbp-149Ch]
    // int v320;                                                                                              // [rsp+220h] [rbp-1498h]
    // int check_offset;                                                                                      // [rsp+224h] [rbp-1494h]
    // int v322;                                                                                              // [rsp+228h] [rbp-1490h]
    // int old_i;                                                                                             // [rsp+22Ch] [rbp-148Ch]
    // int v324;                                                                                              // [rsp+230h] [rbp-1488h]
    // int inside_wire_trimmed;                                                                               // [rsp+234h] [rbp-1484h]
    // int intersection_found;                                                                                // [rsp+238h] [rbp-1480h]
    // int cutout;                                                                                            // [rsp+23Ch] [rbp-147Ch]
    // int v328;                                                                                              // [rsp+240h] [rbp-1478h]
    // int adjust_edge_start;                                                                                 // [rsp+244h] [rbp-1474h]
    // int open_start_vert;                                                                                   // [rsp+248h] [rbp-1470h]
    // int adjust_edge_end;                                                                                   // [rsp+24Ch] [rbp-146Ch]
    // int open_end_vert;                                                                                     // [rsp+250h] [rbp-1468h]
    // ENTITY_LIST all_lumps;                                                                                 // [rsp+258h] [rbp-1460h] BYREF
    // ENTITY_LIST all_shells;                                                                                // [rsp+260h] [rbp-1458h] BYREF
    // offset_segment* new_seg;                                                                               // [rsp+268h] [rbp-1450h]
    // offset_segment* this_seg;                                                                              // [rsp+270h] [rbp-1448h]
    // ENTITY_LIST new_wires;                                                                                 // [rsp+278h] [rbp-1440h] BYREF
    // ENTITY_LIST coeds;                                                                                     // [rsp+280h] [rbp-1438h] BYREF
    // ENTITY_LIST edges;                                                                                     // [rsp+288h] [rbp-1430h] BYREF
    // intcurve* ofintc_upd;                                                                                  // [rsp+290h] [rbp-1428h]
    // LUMP* a_lump;                                                                                          // [rsp+298h] [rbp-1420h]
    // long double rem_tol;                                                                                   // [rsp+2A0h] [rbp-1418h]
    // law** twist_laws;                                                                                      // [rsp+2A8h] [rbp-1410h]
    // long double t_adjust;                                                                                  // [rsp+2B0h] [rbp-1408h]
    // COEDGE* ce;                                                                                            // [rsp+2B8h] [rbp-1400h]
    // RenderingObject* v346;                                                                                 // [rsp+2C0h] [rbp-13F8h]
    // RenderingObject* pRO;                                                                                  // [rsp+2C8h] [rbp-13F0h]
    // ELLIPSE* ell2;                                                                                         // [rsp+2D0h] [rbp-13E8h]
    // ELLIPSE* ell1;                                                                                         // [rsp+2D8h] [rbp-13E0h]
    // double v350;                                                                                           // [rsp+2E0h] [rbp-13D8h]
    // long double t;                                                                                         // [rsp+2E8h] [rbp-13D0h]
    // long double max_error;                                                                                 // [rsp+2F0h] [rbp-13C8h] BYREF
    // WIRE* m;                                                                                               // [rsp+2F8h] [rbp-13C0h]
    // SPAposition pt;                                                                                        // [rsp+300h] [rbp-13B8h] BYREF
    // int bClosedWire;                                                                                       // [rsp+318h] [rbp-13A0h]
    // int sign2;                                                                                             // [rsp+31Ch] [rbp-139Ch]
    // int in_R10;                                                                                            // [rsp+320h] [rbp-1398h]
    // int v358;                                                                                              // [rsp+324h] [rbp-1394h]
    // ENTITY_LIST elist;                                                                                     // [rsp+328h] [rbp-1390h] BYREF
    // COEDGE* ofc;                                                                                           // [rsp+330h] [rbp-1388h]
    // error_info_base* e_info;                                                                               // [rsp+338h] [rbp-1380h]
    // SHELL* last_shell;                                                                                     // [rsp+340h] [rbp-1378h]
    // TVERTEX* tvert;                                                                                        // [rsp+348h] [rbp-1370h] BYREF
    // double v364;                                                                                           // [rsp+350h] [rbp-1368h]
    // double v365;                                                                                           // [rsp+358h] [rbp-1360h]
    // double v366;                                                                                           // [rsp+360h] [rbp-1358h]
    // TVERTEX* this_tvertex;                                                                                 // [rsp+368h] [rbp-1350h] BYREF
    // double v368;                                                                                           // [rsp+370h] [rbp-1348h]
    // ENTITY_LIST end_vert_edges;                                                                            // [rsp+380h] [rbp-1338h] BYREF
    // long double t_val;                                                                                     // [rsp+388h] [rbp-1330h]
    // LUMP* head_lump;                                                                                       // [rsp+390h] [rbp-1328h]
    // SHELL* current_shell;                                                                                  // [rsp+398h] [rbp-1320h]
    // ellipse* v374;                                                                                         // [rsp+3A0h] [rbp-1318h]
    // ENTITY_LIST start_vert_edges;                                                                          // [rsp+3A8h] [rbp-1310h] BYREF
    // SHELL* v376;                                                                                           // [rsp+3B0h] [rbp-1308h]
    // wire_law_data* wld;                                                                                    // [rsp+3B8h] [rbp-1300h]
    // error_info_base* error_info_base_ptr;                                                                  // [rsp+3C0h] [rbp-12F8h]
    // EDGE* this_edge;                                                                                       // [rsp+3C8h] [rbp-12F0h]
    // long double offset_dist;                                                                               // [rsp+3D0h] [rbp-12E8h]
    // COEDGE* next_orig_coedge;                                                                              // [rsp+3D8h] [rbp-12E0h]
    // COEDGE* next_coedge;                                                                                   // [rsp+3E0h] [rbp-12D8h]
    // COEDGE* k;                                                                                             // [rsp+3E8h] [rbp-12D0h]
    // ellipse* v384;                                                                                         // [rsp+3F0h] [rbp-12C8h]
    // ellipse* elli;                                                                                         // [rsp+3F8h] [rbp-12C0h]
    // breakpoint_callback* v386;                                                                             // [rsp+400h] [rbp-12B8h]
    // LUMP* v387;                                                                                            // [rsp+408h] [rbp-12B0h]
    // LUMP* v388;                                                                                            // [rsp+410h] [rbp-12A8h]
    // LUMP* prev_lump;                                                                                       // [rsp+418h] [rbp-12A0h]
    // CURVE* v390;                                                                                           // [rsp+420h] [rbp-1298h]
    // double v391;                                                                                           // [rsp+428h] [rbp-1290h]
    // CURVE* v392;                                                                                           // [rsp+430h] [rbp-1288h]
    // APOINT* v393;                                                                                          // [rsp+438h] [rbp-1280h]
    // APOINT* v394;                                                                                          // [rsp+440h] [rbp-1278h]
    // APOINT* v395;                                                                                          // [rsp+448h] [rbp-1270h]
    // CURVE* v396;                                                                                           // [rsp+450h] [rbp-1268h]
    // APOINT* v397;                                                                                          // [rsp+458h] [rbp-1260h]
    // APOINT* v398;                                                                                          // [rsp+460h] [rbp-1258h]
    // APOINT* v399;                                                                                          // [rsp+468h] [rbp-1250h]
    // APOINT* v400;                                                                                          // [rsp+470h] [rbp-1248h]
    // CURVE* v401;                                                                                           // [rsp+478h] [rbp-1240h]
    // ENTITY_LIST new_edges;                                                                                 // [rsp+480h] [rbp-1238h] BYREF
    // CURVE* v403;                                                                                           // [rsp+488h] [rbp-1230h]
    // CURVE* v404;                                                                                           // [rsp+490h] [rbp-1228h]
    // long double radius;                                                                                    // [rsp+498h] [rbp-1220h]
    // WIRE* v406;                                                                                            // [rsp+4A0h] [rbp-1218h]
    // WIRE* v407;                                                                                            // [rsp+4A8h] [rbp-1210h]
    // wire_law_data* v408;                                                                                   // [rsp+4B0h] [rbp-1208h]
    // wire_law_data* v409;                                                                                   // [rsp+4B8h] [rbp-1200h]
    // CURVE* v410;                                                                                           // [rsp+4C0h] [rbp-11F8h]
    // breakpoint_callback* breakpoint_callback;                                                              // [rsp+4C8h] [rbp-11F0h]
    // ellipse* v412;                                                                                         // [rsp+4D0h] [rbp-11E8h]
    // breakpoint_callback* v413;                                                                             // [rsp+4D8h] [rbp-11E0h]
    // breakpoint_callback* v414;                                                                             // [rsp+4E0h] [rbp-11D8h]
    // RenderingObject* v415;                                                                                 // [rsp+4E8h] [rbp-11D0h]
    // breakpoint_callback* v416;                                                                             // [rsp+4F0h] [rbp-11C8h]
    // offset_segment* v417;                                                                                  // [rsp+4F8h] [rbp-11C0h]
    // offset_segment* v418;                                                                                  // [rsp+500h] [rbp-11B8h]
    // WIRE_OFFSET_ANNO* v419;                                                                                // [rsp+508h] [rbp-11B0h]
    // ANNOTATION* v420;                                                                                      // [rsp+510h] [rbp-11A8h]
    // WIRE_OFFSET_ANNO* v421;                                                                                // [rsp+518h] [rbp-11A0h]
    // ANNOTATION* v422;                                                                                      // [rsp+520h] [rbp-1198h]
    // VERTEX* v423;                                                                                          // [rsp+528h] [rbp-1190h]
    // EDGE* v424;                                                                                            // [rsp+530h] [rbp-1188h]
    // VERTEX* v425;                                                                                          // [rsp+538h] [rbp-1180h]
    // EDGE* v426;                                                                                            // [rsp+540h] [rbp-1178h]
    // long double tol;                                                                                       // [rsp+548h] [rbp-1170h]
    // CURVE* v428;                                                                                           // [rsp+550h] [rbp-1168h]
    // law* local_law;                                                                                        // [rsp+558h] [rbp-1160h]
    // CURVE* v430;                                                                                           // [rsp+560h] [rbp-1158h]
    // law* v431;                                                                                             // [rsp+568h] [rbp-1150h]
    // COEDGE* v432;                                                                                          // [rsp+570h] [rbp-1148h]
    // VERTEX* v433;                                                                                          // [rsp+578h] [rbp-1140h]
    // EDGE* v434;                                                                                            // [rsp+580h] [rbp-1138h]
    // offset_segment* v435;                                                                                  // [rsp+588h] [rbp-1130h]
    // offset_segment* v436;                                                                                  // [rsp+590h] [rbp-1128h]
    // long double in_tol;                                                                                    // [rsp+598h] [rbp-1120h]
    // RenderingObject* v438;                                                                                 // [rsp+5A0h] [rbp-1118h]
    // BODY* v439;                                                                                            // [rsp+5A8h] [rbp-1110h]
    // TRANSFORM* v440;                                                                                       // [rsp+5B0h] [rbp-1108h]
    // TRANSFORM* v441;                                                                                       // [rsp+5B8h] [rbp-1100h]
    // TRANSFORM* v442;                                                                                       // [rsp+5C0h] [rbp-10F8h]
    // _iobuf* fptr;                                                                                          // [rsp+5C8h] [rbp-10F0h]
    // CURVE* v444;                                                                                           // [rsp+5D0h] [rbp-10E8h]
    // COEDGE* pgc;                                                                                           // [rsp+5D8h] [rbp-10E0h]
    // CURVE* v446;                                                                                           // [rsp+5E0h] [rbp-10D8h]
    // const intcurve* ofintc;                                                                                // [rsp+5E8h] [rbp-10D0h]
    // CURVE* v448;                                                                                           // [rsp+5F0h] [rbp-10C8h]
    // long double actual_tol;                                                                                // [rsp+5F8h] [rbp-10C0h] BYREF
    // long double t1;                                                                                        // [rsp+600h] [rbp-10B8h]
    // LUMP* v451;                                                                                            // [rsp+608h] [rbp-10B0h]
    // LUMP* v452;                                                                                            // [rsp+610h] [rbp-10A8h]
    // SHELL* v453;                                                                                           // [rsp+618h] [rbp-10A0h]
    // SHELL* v454;                                                                                           // [rsp+620h] [rbp-1098h]
    // WIRE* v455;                                                                                            // [rsp+628h] [rbp-1090h]
    // WIRE* v456;                                                                                            // [rsp+630h] [rbp-1088h]
    // SHELL* v457;                                                                                           // [rsp+638h] [rbp-1080h]
    // SHELL* v458;                                                                                           // [rsp+640h] [rbp-1078h]
    // SPAposition* v459;                                                                                     // [rsp+648h] [rbp-1070h]
    // breakpoint_callback* v460;                                                                             // [rsp+650h] [rbp-1068h]
    // RenderingObject* v461;                                                                                 // [rsp+658h] [rbp-1060h]
    // double X;                                                                                              // [rsp+660h] [rbp-1058h]
    // APOINT* v463;                                                                                          // [rsp+668h] [rbp-1050h]
    // BODY* v464;                                                                                            // [rsp+670h] [rbp-1048h]
    // AcisVersion v465;                                                                                      // [rsp+678h] [rbp-1040h] BYREF
    // AcisVersion v466;                                                                                      // [rsp+67Ch] [rbp-103Ch] BYREF
    // int twist_law_size;                                                                                    // [rsp+680h] [rbp-1038h] BYREF
    // AcisVersion v468;                                                                                      // [rsp+684h] [rbp-1034h] BYREF
    // AcisVersion v469;                                                                                      // [rsp+688h] [rbp-1030h] BYREF
    // AcisVersion v470;                                                                                      // [rsp+68Ch] [rbp-102Ch] BYREF
    // AcisVersion v471;                                                                                      // [rsp+690h] [rbp-1028h] BYREF
    // AcisVersion v472;                                                                                      // [rsp+694h] [rbp-1024h] BYREF
    // AcisVersion v473;                                                                                      // [rsp+698h] [rbp-1020h] BYREF
    // AcisVersion v474;                                                                                      // [rsp+69Ch] [rbp-101Ch] BYREF
    // AcisVersion v475;                                                                                      // [rsp+6A0h] [rbp-1018h] BYREF
    // AcisVersion v476;                                                                                      // [rsp+6A4h] [rbp-1014h] BYREF
    // AcisVersion v477;                                                                                      // [rsp+6A8h] [rbp-1010h] BYREF
    // AcisVersion v478;                                                                                      // [rsp+6ACh] [rbp-100Ch] BYREF
    // AcisVersion v479;                                                                                      // [rsp+6B0h] [rbp-1008h] BYREF
    // AcisVersion v480;                                                                                      // [rsp+6B4h] [rbp-1004h] BYREF
    // AcisVersion v481;                                                                                      // [rsp+6B8h] [rbp-1000h] BYREF
    // long double* guess;                                                                                    // [rsp+6C0h] [rbp-FF8h]
    // int* side;                                                                                             // [rsp+6C8h] [rbp-FF0h]
    // long double* v484;                                                                                     // [rsp+6D0h] [rbp-FE8h]
    // int* v485;                                                                                             // [rsp+6D8h] [rbp-FE0h]
    // long double* v486;                                                                                     // [rsp+6E0h] [rbp-FD8h]
    // int* v487;                                                                                             // [rsp+6E8h] [rbp-FD0h]
    // double v488;                                                                                           // [rsp+6F0h] [rbp-FC8h]
    // long double radius2;                                                                                   // [rsp+6F8h] [rbp-FC0h]
    // double v490;                                                                                           // [rsp+700h] [rbp-FB8h]
    // SPAposition* p2;                                                                                       // [rsp+708h] [rbp-FB0h]
    // SPAposition* p1;                                                                                       // [rsp+710h] [rbp-FA8h]
    // double v493;                                                                                           // [rsp+718h] [rbp-FA0h]
    // SPAposition* v494;                                                                                     // [rsp+720h] [rbp-F98h]
    // SPAposition* v495;                                                                                     // [rsp+728h] [rbp-F90h]
    // SPAposition* v496;                                                                                     // [rsp+730h] [rbp-F88h]
    // SPAposition* v497;                                                                                     // [rsp+738h] [rbp-F80h]
    // VERTEX* merge_vt;                                                                                      // [rsp+740h] [rbp-F78h]
    // WIRE* v499;                                                                                            // [rsp+748h] [rbp-F70h]
    // wire_law_data* v500;                                                                                   // [rsp+750h] [rbp-F68h]
    // RenderingObject*(__fastcall * new_render_object)(breakpoint_callback*, int);                           // [rsp+758h] [rbp-F60h]
    // void(__fastcall * delete_render_object)(breakpoint_callback*, RenderingObject*);                       // [rsp+760h] [rbp-F58h]
    // RenderingObject*(__fastcall * v503)(breakpoint_callback*, int);                                        // [rsp+768h] [rbp-F50h]
    // void(__fastcall * v504)(breakpoint_callback*, RenderingObject*);                                       // [rsp+770h] [rbp-F48h]
    // COEDGE* c_original;                                                                                    // [rsp+778h] [rbp-F40h]
    // COEDGE* c_offset;                                                                                      // [rsp+780h] [rbp-F38h]
    // offset_segment* v507;                                                                                  // [rsp+788h] [rbp-F30h]
    // ENTITY* offset_ent;                                                                                    // [rsp+790h] [rbp-F28h]
    // ENTITY* original_ent;                                                                                  // [rsp+798h] [rbp-F20h]
    // ANNOTATION* a;                                                                                         // [rsp+7A0h] [rbp-F18h]
    // ANNOTATION* v511;                                                                                      // [rsp+7A8h] [rbp-F10h]
    // AcisVersion* vt2;                                                                                      // [rsp+7B0h] [rbp-F08h]
    // AcisVersion* vt1;                                                                                      // [rsp+7B8h] [rbp-F00h]
    // ENTITY* v514;                                                                                          // [rsp+7C0h] [rbp-EF8h]
    // ENTITY* v515;                                                                                          // [rsp+7C8h] [rbp-EF0h]
    // ANNOTATION* v516;                                                                                      // [rsp+7D0h] [rbp-EE8h]
    // ANNOTATION* v517;                                                                                      // [rsp+7D8h] [rbp-EE0h]
    // double v518;                                                                                           // [rsp+7E0h] [rbp-ED8h]
    // double v519;                                                                                           // [rsp+7E8h] [rbp-ED0h]
    // double v520;                                                                                           // [rsp+7F0h] [rbp-EC8h]
    // APOINT* v521;                                                                                          // [rsp+7F8h] [rbp-EC0h]
    // APOINT* v522;                                                                                          // [rsp+800h] [rbp-EB8h]
    // TVERTEX* v523;                                                                                         // [rsp+808h] [rbp-EB0h]
    // TVERTEX* v524;                                                                                         // [rsp+810h] [rbp-EA8h]
    // AcisVersion* v525;                                                                                     // [rsp+818h] [rbp-EA0h]
    // AcisVersion* v526;                                                                                     // [rsp+820h] [rbp-E98h]
    // VERTEX* vert2;                                                                                         // [rsp+828h] [rbp-E90h]
    // VERTEX* vert1;                                                                                         // [rsp+830h] [rbp-E88h]
    // AcisVersion* v529;                                                                                     // [rsp+838h] [rbp-E80h]
    // AcisVersion* v530;                                                                                     // [rsp+840h] [rbp-E78h]
    // EDGE* v531;                                                                                            // [rsp+848h] [rbp-E70h]
    // long double* v532;                                                                                     // [rsp+850h] [rbp-E68h]
    // int* v533;                                                                                             // [rsp+858h] [rbp-E60h]
    // long double x;                                                                                         // [rsp+860h] [rbp-E58h]
    // EDGE* v535;                                                                                            // [rsp+868h] [rbp-E50h]
    // long double* v536;                                                                                     // [rsp+870h] [rbp-E48h]
    // int* v537;                                                                                             // [rsp+878h] [rbp-E40h]
    // long double v538;                                                                                      // [rsp+880h] [rbp-E38h]
    // law* v539;                                                                                             // [rsp+888h] [rbp-E30h]
    // AcisVersion* v540;                                                                                     // [rsp+890h] [rbp-E28h]
    // AcisVersion* v541;                                                                                     // [rsp+898h] [rbp-E20h]
    // AcisVersion* v542;                                                                                     // [rsp+8A0h] [rbp-E18h]
    // AcisVersion* v543;                                                                                     // [rsp+8A8h] [rbp-E10h]
    // COEDGE* next_off_coedge;                                                                               // [rsp+8B0h] [rbp-E08h]
    // law* v545;                                                                                             // [rsp+8B8h] [rbp-E00h]
    // VERTEX* common_vertex;                                                                                 // [rsp+8C0h] [rbp-DF8h]
    // law* v547;                                                                                             // [rsp+8C8h] [rbp-DF0h]
    // VERTEX* v548;                                                                                          // [rsp+8D0h] [rbp-DE8h]
    // COEDGE* v549;                                                                                          // [rsp+8D8h] [rbp-DE0h]
    // offset_segment* v550;                                                                                  // [rsp+8E0h] [rbp-DD8h]
    // VERTEX* v551;                                                                                          // [rsp+8E8h] [rbp-DD0h]
    // APOINT* v552;                                                                                          // [rsp+8F0h] [rbp-DC8h]
    // APOINT* v553;                                                                                          // [rsp+8F8h] [rbp-DC0h]
    // TVERTEX* v554;                                                                                         // [rsp+900h] [rbp-DB8h]
    // TVERTEX* v555;                                                                                         // [rsp+908h] [rbp-DB0h]
    // AcisVersion* v556;                                                                                     // [rsp+910h] [rbp-DA8h]
    // AcisVersion* v557;                                                                                     // [rsp+918h] [rbp-DA0h]
    // VERTEX* v558;                                                                                          // [rsp+920h] [rbp-D98h]
    // VERTEX* v559;                                                                                          // [rsp+928h] [rbp-D90h]
    // EDGE* v560;                                                                                            // [rsp+930h] [rbp-D88h]
    // long double* v561;                                                                                     // [rsp+938h] [rbp-D80h]
    // int* v562;                                                                                             // [rsp+940h] [rbp-D78h]
    // long double v563;                                                                                      // [rsp+948h] [rbp-D70h]
    // long double v564;                                                                                      // [rsp+950h] [rbp-D68h]
    // law* v565;                                                                                             // [rsp+958h] [rbp-D60h]
    // VERTEX* v566;                                                                                          // [rsp+960h] [rbp-D58h]
    // law* v567;                                                                                             // [rsp+968h] [rbp-D50h]
    // VERTEX* v568;                                                                                          // [rsp+970h] [rbp-D48h]
    // void* alloc_ptr;                                                                                       // [rsp+978h] [rbp-D40h]
    // void* v570;                                                                                            // [rsp+980h] [rbp-D38h]
    // BODY* v571;                                                                                            // [rsp+988h] [rbp-D30h]
    // SPAtransf* v572;                                                                                       // [rsp+990h] [rbp-D28h]
    // TRANSFORM* v573;                                                                                       // [rsp+998h] [rbp-D20h]
    // TRANSFORM* off_wire_transf;                                                                            // [rsp+9A0h] [rbp-D18h]
    // __int64 v575;                                                                                          // [rsp+9A8h] [rbp-D10h]
    // offset_segment* last_seg;                                                                              // [rsp+9B0h] [rbp-D08h]
    // const intcurve* pgintc;                                                                                // [rsp+9B8h] [rbp-D00h]
    // long double t2;                                                                                        // [rsp+9C0h] [rbp-CF8h]
    // SPAbox* box;                                                                                           // [rsp+9C8h] [rbp-CF0h]
    // SPAinterval*(__fastcall * param_range)(struct intcurve*, SPAinterval*, const SPAbox*);                 // [rsp+9D0h] [rbp-CE8h]
    // long double v581;                                                                                      // [rsp+9D8h] [rbp-CE0h]
    // long double t3;                                                                                        // [rsp+9E0h] [rbp-CD8h]
    // offset_segment* next_seg;                                                                              // [rsp+9E8h] [rbp-CD0h]
    // LUMP* v584;                                                                                            // [rsp+9F0h] [rbp-CC8h]
    // SHELL* v585;                                                                                           // [rsp+9F8h] [rbp-CC0h]
    // WIRE* next_wire;                                                                                       // [rsp+A00h] [rbp-CB8h]
    // AcisVersion* v587;                                                                                     // [rsp+A08h] [rbp-CB0h]
    // AcisVersion* v588;                                                                                     // [rsp+A10h] [rbp-CA8h]
    // VERTEX* v589;                                                                                          // [rsp+A18h] [rbp-CA0h]
    // COEDGE* next;                                                                                          // [rsp+A20h] [rbp-C98h]
    // COEDGE* v591;                                                                                          // [rsp+A28h] [rbp-C90h]
    // VERTEX* v592;                                                                                          // [rsp+A30h] [rbp-C88h]
    // WIRE* v593;                                                                                            // [rsp+A38h] [rbp-C80h]
    // SHELL* v594;                                                                                           // [rsp+A40h] [rbp-C78h]
    // long double* v595;                                                                                     // [rsp+A48h] [rbp-C70h]
    // int* v596;                                                                                             // [rsp+A50h] [rbp-C68h]
    // SPAposition* p;                                                                                        // [rsp+A58h] [rbp-C60h]
    // SPAtransf* face_trans;                                                                                 // [rsp+A60h] [rbp-C58h]
    // EDGE* v599;                                                                                            // [rsp+A68h] [rbp-C50h]
    // long double testdist;                                                                                  // [rsp+A70h] [rbp-C48h]
    // double v601;                                                                                           // [rsp+A78h] [rbp-C40h]
    // RenderingObject*(__fastcall * v602)(breakpoint_callback*, int);                                        // [rsp+A80h] [rbp-C38h]
    // __int64(__fastcall * insert_point)(RenderingObject*, const SPAposition*);                              // [rsp+A88h] [rbp-C30h]
    // __int64(__fastcall * v604)(RenderingObject*, const SPAposition*);                                      // [rsp+A90h] [rbp-C28h]
    // ellipse* v605;                                                                                         // [rsp+A98h] [rbp-C20h]
    // void(__fastcall * v606)(struct ellipse*);                                                              // [rsp+AA0h] [rbp-C18h]
    // __int64 v607;                                                                                          // [rsp+AA8h] [rbp-C10h]
    // void(__fastcall * v608)(breakpoint_callback*, RenderingObject*);                                       // [rsp+AB0h] [rbp-C08h]
    // SHELL* nshell;                                                                                         // [rsp+AB8h] [rbp-C00h]
    // outcome* shells;                                                                                       // [rsp+AC0h] [rbp-BF8h]
    // outcome* v611;                                                                                         // [rsp+AC8h] [rbp-BF0h]
    // outcome* lumps;                                                                                        // [rsp+AD0h] [rbp-BE8h]
    // outcome* v613;                                                                                         // [rsp+AD8h] [rbp-BE0h]
    // LUMP* v614;                                                                                            // [rsp+AE0h] [rbp-BD8h]
    // outcome* v615;                                                                                         // [rsp+AE8h] [rbp-BD0h]
    // outcome* v616;                                                                                         // [rsp+AF0h] [rbp-BC8h]
    // EDGE* first_edge;                                                                                      // [rsp+AF8h] [rbp-BC0h]
    // SPAparameter* v618;                                                                                    // [rsp+B00h] [rbp-BB8h]
    // SPAparameter* v619;                                                                                    // [rsp+B08h] [rbp-BB0h]
    // SPAparameter* v620;                                                                                    // [rsp+B10h] [rbp-BA8h]
    // SPAparameter* v621;                                                                                    // [rsp+B18h] [rbp-BA0h]
    // int(__fastcall * evaluate)(curve*, long double, SPAposition*, SPAvector**, int, evaluate_curve_side);  // [rsp+B20h] [rbp-B98h]
    // double v623;                                                                                           // [rsp+B28h] [rbp-B90h]
    // double v624;                                                                                           // [rsp+B30h] [rbp-B88h]
    // double v625;                                                                                           // [rsp+B38h] [rbp-B80h]
    // int(__fastcall * v626)(curve*, long double, SPAposition*, SPAvector**, int, evaluate_curve_side);      // [rsp+B40h] [rbp-B78h]
    // double v627;                                                                                           // [rsp+B48h] [rbp-B70h]
    // double v628;                                                                                           // [rsp+B50h] [rbp-B68h]
    // double v629;                                                                                           // [rsp+B58h] [rbp-B60h]
    // VERTEX* v630;                                                                                          // [rsp+B60h] [rbp-B58h]
    // VERTEX* v631;                                                                                          // [rsp+B68h] [rbp-B50h]
    // double v632;                                                                                           // [rsp+B70h] [rbp-B48h]
    // int(__fastcall * v633)(curve*, long double, SPAposition*, SPAvector**, int, evaluate_curve_side);      // [rsp+B78h] [rbp-B40h]
    // double v634;                                                                                           // [rsp+B80h] [rbp-B38h]
    // int(__fastcall * v635)(curve*, long double, SPAposition*, SPAvector**, int, evaluate_curve_side);      // [rsp+B88h] [rbp-B30h]
    // double v636;                                                                                           // [rsp+B90h] [rbp-B28h]
    // double v637;                                                                                           // [rsp+B98h] [rbp-B20h]
    // double v638;                                                                                           // [rsp+BA0h] [rbp-B18h]
    // int(__fastcall * v639)(curve*, long double, SPAposition*, SPAvector**, int, evaluate_curve_side);      // [rsp+BA8h] [rbp-B10h]
    // double v640;                                                                                           // [rsp+BB0h] [rbp-B08h]
    // int(__fastcall * v641)(curve*, long double, SPAposition*, SPAvector**, int, evaluate_curve_side);      // [rsp+BB8h] [rbp-B00h]
    // double v642;                                                                                           // [rsp+BC0h] [rbp-AF8h]
    // SPAinterval* given_range;                                                                              // [rsp+BC8h] [rbp-AF0h]
    // curve* v644;                                                                                           // [rsp+BD0h] [rbp-AE8h]
    // SPAposition*(__fastcall * eval_position)(curve*, SPAposition*, long double, int, int);                 // [rsp+BD8h] [rbp-AE0h]
    // APOINT* v646;                                                                                          // [rsp+BE0h] [rbp-AD8h]
    // APOINT* start_point;                                                                                   // [rsp+BE8h] [rbp-AD0h]
    // VERTEX* v648;                                                                                          // [rsp+BF0h] [rbp-AC8h]
    // SPAposition* here;                                                                                     // [rsp+BF8h] [rbp-AC0h]
    // APOINT* v650;                                                                                          // [rsp+C00h] [rbp-AB8h]
    // APOINT* ap;                                                                                            // [rsp+C08h] [rbp-AB0h]
    // SPAinterval* v652;                                                                                     // [rsp+C10h] [rbp-AA8h]
    // curve* v653;                                                                                           // [rsp+C18h] [rbp-AA0h]
    // SPAposition*(__fastcall * v654)(curve*, SPAposition*, long double, int, int);                          // [rsp+C20h] [rbp-A98h]
    // APOINT* v655;                                                                                          // [rsp+C28h] [rbp-A90h]
    // APOINT* end_point;                                                                                     // [rsp+C30h] [rbp-A88h]
    // VERTEX* v657;                                                                                          // [rsp+C38h] [rbp-A80h]
    // SPAposition* v658;                                                                                     // [rsp+C40h] [rbp-A78h]
    // APOINT* v659;                                                                                          // [rsp+C48h] [rbp-A70h]
    // APOINT* point;                                                                                         // [rsp+C50h] [rbp-A68h]
    // BODY* v661;                                                                                            // [rsp+C58h] [rbp-A60h]
    // exception_save exception_save_mark;                                                                    // [rsp+C60h] [rbp-A58h] BYREF
    // exception_save v663;                                                                                   // [rsp+C70h] [rbp-A48h] BYREF
    // AcisVersion v664;                                                                                      // [rsp+C80h] [rbp-A38h] BYREF
    // AcisVersion v665;                                                                                      // [rsp+C84h] [rbp-A34h] BYREF
    // SPAposition cpoint;                                                                                    // [rsp+C88h] [rbp-A30h] BYREF
    // SPAposition prev_pt;                                                                                   // [rsp+CA0h] [rbp-A18h] BYREF
    // SPAposition v668;                                                                                      // [rsp+CB8h] [rbp-A00h] BYREF
    // SPAposition average_pos;                                                                               // [rsp+CD0h] [rbp-9E8h] BYREF
    // SPAposition new_coords;                                                                                // [rsp+CE8h] [rbp-9D0h] BYREF
    // SPAposition testpos;                                                                                   // [rsp+D00h] [rbp-9B8h] BYREF
    // SPAunit_vector offsetDir;                                                                              // [rsp+D18h] [rbp-9A0h] BYREF
    // SPAunit_vector endOfstDir;                                                                             // [rsp+D30h] [rbp-988h] BYREF
    // SPAunit_vector startOfstDir;                                                                           // [rsp+D48h] [rbp-970h] BYREF
    // SPAunit_vector iEndOfstDir;                                                                            // [rsp+D60h] [rbp-958h] BYREF
    // SPAunit_vector iStartOfstDir;                                                                          // [rsp+D78h] [rbp-940h] BYREF
    // SPAunit_vector oTangent;                                                                               // [rsp+D90h] [rbp-928h] BYREF
    //
    // SPAparameter v679;                                                                                     // [rsp+DC0h] [rbp-8F8h] BYREF
    // SPAparameter v680;                                                                                     // [rsp+DC8h] [rbp-8F0h] BYREF
    // SPAparameter v681;                                                                                     // [rsp+DD0h] [rbp-8E8h] BYREF
    // SPAparameter v682;                                                                                     // [rsp+DD8h] [rbp-8E0h] BYREF
    // SPAparameter v683;                                                                                     // [rsp+DE0h] [rbp-8D8h] BYREF
    // long double curvature;                                                                                 // [rsp+DE8h] [rbp-8D0h]
    // SPAparameter v685;                                                                                     // [rsp+DF0h] [rbp-8C8h] BYREF
    // SPAparameter v686;                                                                                     // [rsp+DF8h] [rbp-8C0h] BYREF
    // SPAparameter v687;                                                                                     // [rsp+E00h] [rbp-8B8h] BYREF
    // SPAparameter v688;                                                                                     // [rsp+E08h] [rbp-8B0h] BYREF
    // SPAparameter v689;                                                                                     // [rsp+E10h] [rbp-8A8h] BYREF
    // SPAparameter v690;                                                                                     // [rsp+E18h] [rbp-8A0h] BYREF
    // SPAvector diff;                                                                                        // [rsp+E20h] [rbp-898h] BYREF
    // SPAvector v;                                                                                           // [rsp+E38h] [rbp-880h] BYREF
    // SPAvector start_curvature;                                                                             // [rsp+E50h] [rbp-868h] BYREF
    // SPAvector end_curvature;                                                                               // [rsp+E68h] [rbp-850h] BYREF
    // acis_exception error_info_holder;                                                                      // [rsp+E80h] [rbp-838h] BYREF
    // acis_exception v696;                                                                                   // [rsp+EA0h] [rbp-818h] BYREF
    // SPAvector vec2;                                                                                        // [rsp+F40h] [rbp-778h] BYREF
    // SPAposition this_pos;                                                                                  // [rsp+F58h] [rbp-760h] BYREF
    // SPAposition prev_pos;                                                                                  // [rsp+F70h] [rbp-748h] BYREF
    // SPAposition next_start;                                                                                // [rsp+F88h] [rbp-730h] BYREF
    // SPAposition prev_end;                                                                                  // [rsp+FA0h] [rbp-718h] BYREF
    // SPAposition temp;                                                                                      // [rsp+FB8h] [rbp-700h] BYREF
    // SPAposition v703;                                                                                      // [rsp+FD0h] [rbp-6E8h] BYREF
    // SPAinterval the_int;                                                                                   // [rsp+FE8h] [rbp-6D0h] BYREF
    // SPAvector direction;                                                                                   // [rsp+1000h] [rbp-6B8h] BYREF
    // SPAvector dpdt;                                                                                        // [rsp+1018h] [rbp-6A0h] BYREF
    // SPAvector dp2dt2;                                                                                      // [rsp+1030h] [rbp-688h] BYREF
    // SPAposition start_pos;                                                                                 // [rsp+1048h] [rbp-670h] BYREF
    // SPAposition end_pos;                                                                                   // [rsp+1060h] [rbp-658h] BYREF
    //
    // SPAvector start_tang1;                                                                                 // [rsp+1090h] [rbp-628h] BYREF
    // SPAvector curvature_vec1;                                                                              // [rsp+10A8h] [rbp-610h] BYREF
    // SPAvector start_tang2;                                                                                 // [rsp+10C0h] [rbp-5F8h] BYREF
    // SPAvector curvature_vec2;                                                                              // [rsp+10D8h] [rbp-5E0h] BYREF
    // SPAvector vec1;                                                                                        // [rsp+10F0h] [rbp-5C8h] BYREF
    // outcome result;                                                                                        // [rsp+1108h] [rbp-5B0h] BYREF
    // outcome v717;                                                                                          // [rsp+1128h] [rbp-590h] BYREF
    // outcome v718;                                                                                          // [rsp+1148h] [rbp-570h] BYREF
    // outcome v719;                                                                                          // [rsp+1168h] [rbp-550h] BYREF
    // outcome v720;                                                                                          // [rsp+1188h] [rbp-530h] BYREF
    // outcome v721;                                                                                          // [rsp+11A8h] [rbp-510h] BYREF
    // outcome v722;                                                                                          // [rsp+11C8h] [rbp-4F0h] BYREF
    // offset_segment_list seg_list;                                                                          // [rsp+11F0h] [rbp-4C8h] BYREF
    // SPAvector v724;                                                                                        // [rsp+13D0h] [rbp-2E8h] BYREF
    // SPAvector v725;                                                                                        // [rsp+13E8h] [rbp-2D0h] BYREF
    // SPAvector v726;                                                                                        // [rsp+1400h] [rbp-2B8h] BYREF
    // SPAunit_vector v727;                                                                                   // [rsp+1418h] [rbp-2A0h] BYREF
    // SPAvector v728;                                                                                        // [rsp+1430h] [rbp-288h] BYREF
    // SPAvector v729;                                                                                        // [rsp+1448h] [rbp-270h] BYREF
    // SPAvector v730;                                                                                        // [rsp+1460h] [rbp-258h] BYREF
    // SPAinterval v731;                                                                                      // [rsp+1478h] [rbp-240h] BYREF
    // SPAinterval v732;                                                                                      // [rsp+1490h] [rbp-228h] BYREF
    // SPAvector v733;                                                                                        // [rsp+14A8h] [rbp-210h] BYREF
    // SPAunit_vector v734;                                                                                   // [rsp+14C0h] [rbp-1F8h] BYREF
    // SPAunit_vector v735;                                                                                   // [rsp+14D8h] [rbp-1E0h] BYREF
    // SPAvector v736;                                                                                        // [rsp+14F0h] [rbp-1C8h] BYREF
    // SPAvector v737;                                                                                        // [rsp+1508h] [rbp-1B0h] BYREF
    // SPAvector v738;                                                                                        // [rsp+1520h] [rbp-198h] BYREF
    // SPAvector v739;                                                                                        // [rsp+1538h] [rbp-180h] BYREF
    // SPAtransf v740;                                                                                        // [rsp+1550h] [rbp-168h] BYREF
    // SPAtransf v741;                                                                                        // [rsp+15C0h] [rbp-F8h] BYREF
    // SPAtransf v742;                                                                                        // [rsp+1630h] [rbp-88h] BYREF
    //
    // /*if(woffset_module_header.debug_level >= 0xA) {
    //     acis_fprintf(debug_file_ptr, "    Entering sg_offset_planar_wire()\n");
    //     acis_fprintf(debug_file_ptr, "    wire = ");
    //     debug_pointer(wire, debug_file_ptr);
    //     acis_fprintf(debug_file_ptr, " )\n");
    // }*/
    // display_wire_offset_io(wire, dist_law, 0i64);
    // if(branched_wire(wire)) {
    // /*       v12 = message_module::message_code(&spaacisds_cur_off_errmod, 0);*/
    //     ofst_error(OFFSET_BRANCHED_WIRE, 1, wire);
    // }
    // int bOutwards= 0;
    // if(GET_ALGORITHMIC_VERSION() >= AcisVersion(14, 0, 3)) {
    //     SPAunit_vector wireNormal;
    //     SPAposition centroid;
    //     if(!is_planar_wire(wire, centroid, wireNormal, 1, 1)) {
    //         sys_error(OFFSET_INPUT_NOT_PLANAR);
    //     }
    //     v314 = is_open_wire(wire) == 0;
    //     bClosedWire = v314;
    //     if(v314) {
    //         v15 = safe_function_type<double>::operator double(&SPAresnor);
    //         if(parallel(&wireNormal, wire_normal, v15)) {
    //             if(dist_law) {
    //                 if(law::constant(dist_law)) {
    //                     guess = SpaAcis::NullObj::get_double();
    //                     side = SpaAcis::NullObj::get_int();
    //                     if(law::eval(dist_law, 1.0, side, guess) > 0.0) {
    //                         if(twist_law) {
    //                             v16 = safe_function_type<double>::operator double(&SPAresabs);
    //                             if(law::zero(twist_law, v16)) bOutwards = 1;
    //                         }
    //                     }
    //                 }
    //             }
    //         } else {
    //             v17 = safe_function_type<double>::operator double(&SPAresnor);
    //             if(biparallel(&wireNormal, wire_normal, v17)) {
    //                 if(dist_law) {
    //                     if(law::constant(dist_law)) {
    //                         v484 = SpaAcis::NullObj::get_double();
    //                         v485 = SpaAcis::NullObj::get_int();
    //                         if(law::eval(dist_law, 1.0, v485, v484) < 0.0) {
    //                             if(twist_law) {
    //                                 v18 = safe_function_type<double>::operator double(&SPAresabs);
    //                                 if(law::zero(twist_law, v18)) bOutwards = 1;
    //                             }
    //                         }
    //                     }
    //                 }
    //             } else {
    //                 bOutwards = 0;
    //             }
    //         }
    //     }
    // }
    // offset_segment_list::offset_segment_list(&seg_list);
    // offset_segment_list::set_distance(&seg_list, dist_law);
    // offset_segment_list::set_normal(&seg_list, wire_normal);
    // offset_segment_list::set_add_attribs(&seg_list, add_attribs);
    // offset_segment_list::set_gap_type(&seg_list, close_type);
    // start_coedge = start_of_wire_chain(wire);
    // this_coedge = start_coedge;
    // AcisVersion::AcisVersion(&v665, 10, 0, 4);
    // if(operator>=(&cav, v19)) {
    //     do {
    //         prev_coedge = this_coedge;
    //         this_coedge = COEDGE::next(this_coedge);
    //         v20 = COEDGE::edge(prev_coedge);
    //         v401 = EDGE::geometry(v20);
    //         v21 = v401->equation(v401);
    //         if(CUR_is_intcurve(v21)) {
    //             ENTITY_LIST::ENTITY_LIST(&new_edges);
    //             v22 = COEDGE::edge(prev_coedge);
    //             sg_split_edge_at_disc(&result, v22, &new_edges, 1);
    //             outcome::~outcome(&result);
    //             ENTITY_LIST::~ENTITY_LIST(&new_edges);
    //         } else {
    //             AcisVersion::AcisVersion(&v465, 16, 0, 1);
    //             if(operator>=(&cav, v23)) {
    //                 if(law::constant(dist_law)) {
    //                     if(this_coedge) {
    //                         if(this_coedge != prev_coedge) {
    //                             v24 = COEDGE::edge(prev_coedge);
    //                             v403 = EDGE::geometry(v24);
    //                             v25 = v403->equation(v403);
    //                             if(CUR_is_ellipse(v25)) {
    //                                 v26 = COEDGE::edge(this_coedge);
    //                                 v404 = EDGE::geometry(v26);
    //                                 v27 = v404->equation(v404);
    //                                 if(CUR_is_ellipse(v27)) {
    //                                     v28 = COEDGE::edge(prev_coedge);
    //                                     ell1 = (ELLIPSE*)EDGE::geometry(v28);
    //                                     v29 = COEDGE::edge(this_coedge);
    //                                     ell2 = (ELLIPSE*)EDGE::geometry(v29);
    //                                     if(ELLIPSE::radius_ratio(ell1) == 1.0 && ELLIPSE::radius_ratio(ell2) == 1.0) {
    //                                         v30 = (SPAvector*)ELLIPSE::major_axis(ell1);
    //                                         radius = SPAvector::len(v30);
    //                                         v31 = (SPAvector*)ELLIPSE::major_axis(ell2);
    //                                         radius2 = SPAvector::len(v31);
    //                                         v486 = SpaAcis::NullObj::get_double();
    //                                         v487 = SpaAcis::NullObj::get_int();
    //                                         v488 = safe_function_type<double>::operator double(&SPAresabs);
    //                                         v32 = law::eval(dist_law, 1.0, v487, v486);
    //                                         v33 = fabs_0(v32);
    //                                         v34 = fabs_0(radius - v33);
    //                                         if(v488 > v34) {
    //                                             v490 = fabs_0(radius - radius2);
    //                                             v35 = safe_function_type<double>::operator double(&SPAresabs);
    //                                             if(v35 > v490) {
    //                                                 p2 = (SPAposition*)ELLIPSE::centre(ell2);
    //                                                 p1 = (SPAposition*)ELLIPSE::centre(ell1);
    //                                                 v493 = safe_function_type<double>::operator double(&SPAresabs);
    //                                                 v36 = operator-(&v733, p1, p2);
    //                                                 v37 = SPAvector::len(v36);
    //                                                 if(v493 > v37) {
    //                                                     v38 = COEDGE::start(prev_coedge);
    //                                                     v39 = VERTEX::geometry(v38);
    //                                                     v494 = (SPAposition*)APOINT::coords(v39);
    //                                                     v495 = (SPAposition*)ELLIPSE::centre(ell1);
    //                                                     operator-(&curvature_vec1, v495, v494);
    //                                                     v40 = COEDGE::start(this_coedge);
    //                                                     v41 = VERTEX::geometry(v40);
    //                                                     v496 = (SPAposition*)APOINT::coords(v41);
    //                                                     v497 = (SPAposition*)ELLIPSE::centre(ell2);
    //                                                     operator-(&curvature_vec2, v497, v496);
    //                                                     SPAtransf::SPAtransf(&v740);
    //                                                     started = coedge_start_dir(&v734, prev_coedge, v42, 1);
    //                                                     SPAvector::SPAvector(&start_tang1, started);
    //                                                     SPAtransf::SPAtransf(&v741);
    //                                                     v45 = coedge_start_dir(&v735, this_coedge, v44, 1);
    //                                                     SPAvector::SPAvector(&start_tang2, v45);
    //                                                     operator*(&vec1, &curvature_vec1, &start_tang1);
    //                                                     operator*(&vec2, &curvature_vec2, &start_tang2);
    //                                                     v315 = operator%(&vec1, wire_normal) > 0.0;
    //                                                     sign1 = v315;
    //                                                     v316 = operator%(&vec2, wire_normal) > 0.0;
    //                                                     sign2 = v316;
    //                                                     if(sign1 == v316 && bOutwards == sign1) {
    //                                                         AcisVersion::AcisVersion(&v466, 24, 0, 0);
    //                                                         if(operator>(&cav, v46)) {
    //                                                             merge_vt = COEDGE::end(prev_coedge);
    //                                                             merge_vertex(merge_vt, sel_ed);
    //                                                         } else {
    //                                                             v47 = COEDGE::edge(prev_coedge);
    //                                                             api_clean_entity(&v717, v47, 0i64);
    //                                                             outcome::~outcome(&v717);
    //                                                         }
    //                                                         if(COEDGE::next(prev_coedge) != this_coedge) this_coedge = COEDGE::next(prev_coedge);
    //                                                     }
    //                                                 }
    //                                             }
    //                                         }
    //                                     }
    //                                 }
    //                             }
    //                         }
    //                     }
    //                 }
    //             }
    //         }
    //     } while(this_coedge != prev_coedge && this_coedge != start_coedge);
    //     this_coedge = start_coedge;
    // }
    // v406 = (WIRE*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 997, &alloc_file_index_3517);
    // if(v406) {
    //     WIRE::WIRE(v406, 0i64, 0i64);
    //     v407 = v48;
    // } else {
    //     v407 = 0i64;
    // }
    // v499 = v407;
    // offset_wire = v407;
    // v408 = (wire_law_data*)ACIS_OBJECT::operator new(0x78ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 999, &alloc_file_index_3517);
    // if(v408) {
    //     wire_law_data::wire_law_data(v408, wire);
    //     v409 = v49;
    // } else {
    //     v409 = 0i64;
    // }
    // v500 = v409;
    // wld = v409;
    // dist_laws = wire_law_data::map_laws(v409, dist_law, &dist_law_size, 0);
    // twist_laws = wire_law_data::map_laws(wld, twist_law, &twist_law_size, 0);
    // law_data::remove(wld);
    // coedge_count = 0;
    // resignal_no = 0;
    // acis_exception::acis_exception(&error_info_holder, 0, 0i64, 0i64, 0);
    // error_info_base_ptr = 0i64;
    // exception_save::exception_save(&exception_save_mark);
    // ENTITY_LIST::ENTITY_LIST(&off_coedge_list);
    // ENTITY_LIST::ENTITY_LIST(&orig_coedge_list);
    // exception_save::begin(&exception_save_mark);
    // get_error_mark()->buffer_init = 1;
    // error_no = 0;
    // openWire = is_open_wire(wire);
    // do {
    //     offset_coedge = sg_offset_pl_coedge(this_coedge, dist_laws[coedge_count], twist_laws[coedge_count], wire_normal);
    //     if(offset_coedge) {
    //         v50 = COEDGE::edge(offset_coedge);
    //         if(!EDGE::geometry(v50)) {
    //             if(openWire) {
    //                 v51 = COEDGE::edge(this_coedge);
    //                 v410 = EDGE::geometry(v51);
    //                 v52 = v410->equation(v410);
    //                 if(!is_circular(v52)) {
    //                     v53 = COEDGE::next(offset_coedge);
    //                     if(offset_coedge == v53 || !COEDGE::next(offset_coedge) || (v54 = COEDGE::previous(offset_coedge), offset_coedge == v54) || !COEDGE::previous(offset_coedge)) {
    //                         api_delent(&v718, offset_coedge, 0i64);
    //                         outcome::~outcome(&v718);
    //                         offset_coedge = 0i64;
    //                     }
    //                 }
    //             }
    //         }
    //     }
    //     if(Debug_Break_Active("planar wire offset: segment-wise", "WIRE-OFFSET")) {
    //         if(get_breakpoint_callback()) {
    //             breakpoint_callback = get_breakpoint_callback();
    //             new_render_object = breakpoint_callback->new_render_object;
    //             v438 = new_render_object(breakpoint_callback, 1);
    //         } else {
    //             v438 = 0i64;
    //         }
    //         pRO = v438;
    //         if(v438) {
    //             v55 = COEDGE::edge(this_coedge);
    //             show_entity(v55, ORANGE, pRO);
    //             if(offset_coedge && (v56 = COEDGE::edge(offset_coedge), EDGE::geometry(v56))) {
    //                 acis_fprintf(debug_file_ptr, "sg_offset_planar_wire - Displaying offset edge\n");
    //                 v57 = COEDGE::edge(offset_coedge);
    //                 show_entity(v57, RED, pRO);
    //             } else {
    //                 acis_fprintf(debug_file_ptr, "sg_offset_planar_wire - Edge has collapsed\n");
    //             }
    //             Debug_Break("planar wire offset: segment-wise", "WIRE-OFFSET", "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 1055);
    //             if(get_breakpoint_callback()) {
    //                 v413 = get_breakpoint_callback();
    //                 delete_render_object = v413->delete_render_object;
    //                 delete_render_object(v413, pRO);
    //             }
    //         }
    //     }
    //     ++coedge_count;
    //     ENTITY_LIST::add(&off_coedge_list, offset_coedge, 1);
    //     ENTITY_LIST::add(&orig_coedge_list, this_coedge, 1);
    //     prev_coedgea = this_coedge;
    //     this_coedge = COEDGE::next(this_coedge);
    // } while(this_coedge != prev_coedgea && this_coedge != start_coedge);
    // if(Debug_Break_Active("planar wire offset: all segments", "WIRE-OFFSET")) {
    //     if(get_breakpoint_callback()) {
    //         v414 = get_breakpoint_callback();
    //         v503 = v414->new_render_object;
    //         v415 = v503(v414, 1);
    //     } else {
    //         v415 = 0i64;
    //     }
    //     v346 = v415;
    //     if(v415) {
    //         for(k = (COEDGE*)ENTITY_LIST::first(&off_coedge_list); k; k = (COEDGE*)ENTITY_LIST::next(&off_coedge_list)) {
    //             this_edge = COEDGE::edge(k);
    //             if(EDGE::geometry(this_edge)) {
    //                 show_entity(this_edge, RED, v346);
    //             } else {
    //                 v58 = EDGE::start(this_edge);
    //                 show_entity(v58, MAGENTA, v346);
    //             }
    //         }
    //         acis_fprintf(debug_file_ptr, "sg_offset_planar_wire - Displaying all offsets\n");
    //         Debug_Break("planar wire offset: all segments", "WIRE-OFFSET", "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 1085);
    //         if(get_breakpoint_callback()) {
    //             v416 = get_breakpoint_callback();
    //             v504 = v416->delete_render_object;
    //             v504(v416, v346);
    //         }
    //     }
    // }
    // ENTITY_LIST::init(&off_coedge_list);
    // ENTITY_LIST::init(&orig_coedge_list);
    // if(!ENTITY_LIST::operator[](&off_coedge_list, 0) || ENTITY_LIST::count(&off_coedge_list) > 1 && all_offset_degenerated(&off_coedge_list)) {
    //     delete_entity(offset_wire);
    //     offset_wire = 0i64;
    // } else {
    //     v417 = (offset_segment*)ACIS_OBJECT::operator new(0xB8ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 1096, &alloc_file_index_3517);
    //     if(v417) {
    //         c_original = (COEDGE*)ENTITY_LIST::operator[](&orig_coedge_list, 0);
    //         c_offset = (COEDGE*)ENTITY_LIST::operator[](&off_coedge_list, 0);
    //         offset_segment::offset_segment(v417, c_offset, c_original);
    //         v418 = v59;
    //     } else {
    //         v418 = 0i64;
    //     }
    //     v507 = v418;
    //     new_seg = v418;
    //     v60 = offset_segment_list::first_segment(&seg_list);
    //     curr_seg = offset_segment::previous(v60);
    //     offset_segment_list::insert_segment(&seg_list, new_seg, curr_seg);
    //     curr_seg = new_seg;
    //     coedge_count = ENTITY_LIST::count(&off_coedge_list);
    //     prev_off_coedge = (COEDGE*)ENTITY_LIST::operator[](&off_coedge_list, 0);
    //     prev_orig_coedge = (COEDGE*)ENTITY_LIST::operator[](&orig_coedge_list, 0);
    //     COEDGE::set_wire(prev_off_coedge, offset_wire, 1);
    //     first_off_coedge = prev_off_coedge;
    //     if(option_header::on(&annotations)) {
    //         if(option_header::on(&annotations)) {
    //             v419 = (WIRE_OFFSET_ANNO*)ACIS_OBJECT::operator new(0x58ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 1168, &alloc_file_index_3517);
    //             if(v419) {
    //                 v61 = (COEDGE*)ENTITY_LIST::operator[](&off_coedge_list, 0);
    //                 offset_ent = COEDGE::edge(v61);
    //                 v62 = (COEDGE*)ENTITY_LIST::operator[](&orig_coedge_list, 0);
    //                 original_ent = COEDGE::edge(v62);
    //                 WIRE_OFFSET_ANNO::WIRE_OFFSET_ANNO(v419, original_ent, offset_ent, 0i64);
    //                 v420 = v63;
    //             } else {
    //                 v420 = 0i64;
    //             }
    //             a = v420;
    //             v511 = __hookup__(v420);
    //         } else {
    //             v511 = 0i64;
    //         }
    //     }
    //     start = 1;
    //     i = 1;
    // LABEL_106:
    //     if(i < coedge_count) {
    //         degen_off_coed_deleted = 0;
    //         this_orig_coedge = 0i64;
    //         nni = -1;
    //         ii = 1;
    //         old_i = i;
    //         if(close_type == natural) {
    //             AcisVersion::AcisVersion(&v468, 20, 0, 0);
    //             vt2 = v64;
    //             vt1 = GET_ALGORITHMIC_VERSION(&v469);
    //             if(operator>=(vt1, vt2)) {
    //                 for(j = i;; ++j) {
    //                     if(j >= coedge_count) goto LABEL_114;
    //                     v65 = (COEDGE*)ENTITY_LIST::operator[](&off_coedge_list, j);
    //                     v66 = COEDGE::edge(v65);
    //                     if(EDGE::geometry(v66)) break;
    //                 }
    //                 nni = j;
    //             LABEL_114:
    //                 if(nni > i) {
    //                     ii = 0;
    //                     i = nni;
    //                 }
    //             }
    //         }
    //         skipbackup = 0;
    //         while(1) {
    //             skipbackup = ii == 0;
    //             this_off_coedge = (COEDGE*)ENTITY_LIST::operator[](&off_coedge_list, i);
    //             this_orig_coedge = (COEDGE*)ENTITY_LIST::operator[](&orig_coedge_list, i);
    //             if(option_header::on(&annotations)) {
    //                 if(option_header::on(&annotations)) {
    //                     v421 = (WIRE_OFFSET_ANNO*)ACIS_OBJECT::operator new(0x58ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 1217, &alloc_file_index_3517);
    //                     if(v421) {
    //                         v514 = COEDGE::edge(this_off_coedge);
    //                         v515 = COEDGE::edge(this_orig_coedge);
    //                         WIRE_OFFSET_ANNO::WIRE_OFFSET_ANNO(v421, v515, v514, 0i64);
    //                         v422 = v67;
    //                     } else {
    //                         v422 = 0i64;
    //                     }
    //                     v516 = v422;
    //                     v517 = __hookup__(v422);
    //                 } else {
    //                     v517 = 0i64;
    //                 }
    //             }
    //             v68 = COEDGE::edge(prev_off_coedge);
    //             if(EDGE::geometry(v68) || (v69 = COEDGE::edge(this_off_coedge), EDGE::geometry(v69)) || !option_header::on(&rem_null_edge)) {
    //                 v79 = COEDGE::edge(this_off_coedge);
    //                 if(EDGE::geometry(v79)) goto LABEL_134;
    //                 if(!option_header::on(&rem_null_edge)) goto LABEL_134;
    //                 v80 = COEDGE::end(prev_off_coedge);
    //                 v81 = VERTEX::geometry(v80);
    //                 v82 = APOINT::coords(v81);
    //                 SPAposition::SPAposition(&prev_end, v82);
    //                 v83 = COEDGE::next(this_off_coedge);
    //                 v84 = COEDGE::start(v83);
    //                 v85 = VERTEX::geometry(v84);
    //                 v86 = APOINT::coords(v85);
    //                 SPAposition::SPAposition(&next_start, v86);
    //                 v87 = operator-(&v737, &prev_end, &next_start);
    //                 v520 = SPAvector::len_sq(v87);
    //                 v519 = safe_function_type<double>::operator double(&SPAresfit);
    //                 v88 = safe_function_type<double>::operator double(&SPAresfit);
    //                 if(v519 * v88 <= v520) {
    //                 LABEL_134:
    //                     COEDGE::set_wire(this_off_coedge, offset_wire, 1);
    //                     COEDGE::set_previous(this_off_coedge, prev_off_coedge, 0, 1);
    //                     COEDGE::set_next(prev_off_coedge, this_off_coedge, 0, 1);
    //                     v90 = COEDGE::end(prev_off_coedge);
    //                     if(is_TVERTEX(v90) && (v91 = COEDGE::start(this_off_coedge), is_TVERTEX(v91))) {
    //                         v92 = COEDGE::end(prev_off_coedge);
    //                         v93 = VERTEX::geometry(v92);
    //                         v94 = APOINT::coords(v93);
    //                         SPAposition::SPAposition(&average_pos, v94);
    //                         v95 = COEDGE::start(this_off_coedge);
    //                         v96 = VERTEX::geometry(v95);
    //                         v97 = APOINT::coords(v96);
    //                         SPAposition::SPAposition(&temp, v97);
    //                         v98 = operator-(&v738, &temp, &average_pos);
    //                         operator*(&diff, 0.5, v98);
    //                         SPAposition::operator+=(&average_pos, &diff);
    //                         v99 = COEDGE::end(prev_off_coedge);
    //                         v521 = VERTEX::geometry(v99);
    //                         APOINT::set_coords(v521, &average_pos);
    //                         v100 = COEDGE::start(this_off_coedge);
    //                         v522 = VERTEX::geometry(v100);
    //                         APOINT::set_coords(v522, &average_pos);
    //                         tol = 1.001 * SPAvector::len(&diff);
    //                         v523 = (TVERTEX*)COEDGE::end(prev_off_coedge);
    //                         TVERTEX::set_tolerance(v523, tol, 0);
    //                         v524 = (TVERTEX*)COEDGE::start(this_off_coedge);
    //                         TVERTEX::set_tolerance(v524, tol, 0);
    //                         AcisVersion::AcisVersion(&v470, 20, 0, 0);
    //                         v525 = v101;
    //                         v526 = GET_ALGORITHMIC_VERSION(&v471);
    //                         if(operator>=(v526, v525)) {
    //                             vert2 = COEDGE::start(this_off_coedge);
    //                             vert1 = COEDGE::end(prev_off_coedge);
    //                             make_same_vertex_pointers(vert1, vert2, prev_off_coedge, this_off_coedge, 1);
    //                         }
    //                     } else {
    //                         v102 = safe_function_type<double>::operator double(&SPAresabs);
    //                         if(law::zero(twist_law, v102)) {
    //                             dist_law_index = i - 1;
    //                             offset_dist = 0.0;
    //                             AcisVersion::AcisVersion(&v472, 10, 0, 4);
    //                             v529 = v103;
    //                             v530 = GET_ALGORITHMIC_VERSION(&v473);
    //                             if(operator<(v530, v529)) {
    //                                 dist_law_index = i;
    //                                 v532 = SpaAcis::NullObj::get_double();
    //                                 v533 = SpaAcis::NullObj::get_int();
    //                                 v531 = COEDGE::edge(this_orig_coedge);
    //                                 v104 = EDGE::start_param(v531, &v685);
    //                                 x = SPAparameter::operator double(v104);
    //                                 offset_dist = law::eval(dist_law, x, v533, v532);
    //                             } else {
    //                                 v539 = dist_laws[i];
    //                                 v536 = SpaAcis::NullObj::get_double();
    //                                 v537 = SpaAcis::NullObj::get_int();
    //                                 v535 = COEDGE::edge(this_orig_coedge);
    //                                 v105 = EDGE::start_param(v535, &v686);
    //                                 v538 = SPAparameter::operator double(v105);
    //                                 offset_dist = law::eval(v539, v538, v537, v536);
    //                             }
    //                             if(ii == 1) {
    //                                 check_status = check_gap_type(this_orig_coedge, prev_orig_coedge, wire_normal, offset_dist);
    //                             } else {
    //                                 check_status = 2;
    //                                 degen_off_coed_deleted = 0;
    //                             }
    //                             skipStartDegOffsetCoedges = 0;
    //                             if(openWire) {
    //                                 AcisVersion::AcisVersion(&v474, 18, 0, 0);
    //                                 v540 = v106;
    //                                 v541 = GET_ALGORITHMIC_VERSION(&v481);
    //                                 if(operator>=(v541, v540)) {
    //                                     AcisVersion::AcisVersion(&v475, 20, 0, 2);
    //                                     v542 = v107;
    //                                     v543 = GET_ALGORITHMIC_VERSION(&v476);
    //                                     check_offset = operator>=(v543, v542);
    //                                     is_offset_by_radius = 0;
    //                                     if(i == start) {
    //                                         if(check_offset) {
    //                                             v108 = COEDGE::edge(prev_orig_coedge);
    //                                             local_law = edge_dist_law_to_coedge_dist_law(prev_orig_coedge, v108, dist_law);
    //                                             v109 = COEDGE::edge(prev_orig_coedge);
    //                                             v428 = EDGE::geometry(v109);
    //                                             v110 = v428->equation(v428);
    //                                             is_offset_by_radius = is_circle_offset_by_radius(v110, wire_normal, local_law, twist_law);
    //                                             law::remove(local_law);
    //                                         }
    //                                         v320 = !prev_off_coedge || (v111 = COEDGE::edge(prev_off_coedge), !EDGE::geometry(v111)) && !is_offset_by_radius;
    //                                         skipStartDegOffsetCoedges = v320;
    //                                     } else if(i + 1 == coedge_count && prev_off_coedge) {
    //                                         if(check_offset) {
    //                                             next_orig_coedge = COEDGE::next(prev_orig_coedge);
    //                                             v112 = COEDGE::edge(next_orig_coedge);
    //                                             v431 = edge_dist_law_to_coedge_dist_law(next_orig_coedge, v112, dist_law);
    //                                             v113 = COEDGE::edge(next_orig_coedge);
    //                                             v430 = EDGE::geometry(v113);
    //                                             v114 = v430->equation(v430);
    //                                             is_offset_by_radius = is_circle_offset_by_radius(v114, wire_normal, v431, twist_law);
    //                                             law::remove(v431);
    //                                         }
    //                                         next_coedge = COEDGE::next(prev_off_coedge);
    //                                         v322 = !next_coedge || (v115 = COEDGE::edge(next_coedge), !EDGE::geometry(v115)) && !is_offset_by_radius;
    //                                         skipStartDegOffsetCoedges = v322;
    //                                         next_coedge = 0i64;
    //                                     }
    //                                 }
    //                             }
    //                             did_close = 1;
    //                             if(!skipStartDegOffsetCoedges) {
    //                                 SPAunit_vector::SPAunit_vector(&offsetDir, 0.0, 0.0, 0.0);
    //                                 SPAunit_vector::SPAunit_vector(&startOfstDir, 0.0, 0.0, 0.0);
    //                                 SPAunit_vector::SPAunit_vector(&endOfstDir, 0.0, 0.0, 0.0);
    //                                 if(prev_off_coedge)
    //                                     v432 = COEDGE::next(prev_off_coedge);
    //                                 else
    //                                     v432 = 0i64;
    //                                 next_off_coedge = v432;
    //                                 comp_tangent_at_coedge_commom_vertex(prev_orig_coedge, this_orig_coedge, prev_off_coedge, v432, &offsetDir, &startOfstDir, &endOfstDir);
    //                                 if(check_status == 1 && trim) {
    //                                     v545 = dist_laws[dist_law_index];
    //                                     common_vertex = COEDGE::end(prev_orig_coedge);
    //                                     did_close = sg_close_offset_gap(&seg_list, &curr_seg, prev_off_coedge, common_vertex, &offsetDir, &startOfstDir, &endOfstDir, v545, arc, iKeepMiniTopo, skipbackup);
    //                                 } else if(check_status == 2) {
    //                                     v547 = dist_laws[dist_law_index];
    //                                     v548 = COEDGE::end(prev_orig_coedge);
    //                                     did_close = sg_close_offset_gap(&seg_list, &curr_seg, prev_off_coedge, v548, &offsetDir, &startOfstDir, &endOfstDir, v547, close_type, iKeepMiniTopo, skipbackup);
    //                                 }
    //                                 if(skipbackup) {
    //                                     if(did_close) {
    //                                         ii = 1;
    //                                         for(jj = old_i; jj < i; ++jj) {
    //                                             ce = (COEDGE*)ENTITY_LIST::operator[](&off_coedge_list, jj);
    //                                             v116 = COEDGE::edge(ce);
    //                                             v433 = EDGE::start(v116);
    //                                             v433->lose(v433);
    //                                             v434 = COEDGE::edge(ce);
    //                                             v434->lose(v434);
    //                                             ce->lose(ce);
    //                                         }
    //                                     } else {
    //                                         i = old_i;
    //                                     }
    //                                 }
    //                             }
    //                         }
    //                     }
    //                     ++ii;
    //                     goto LABEL_183;
    //                 }
    //                 display_null_offset_coedge_to_be_deleted(this_off_coedge, this_orig_coedge);
    //                 v89 = COEDGE::edge(this_off_coedge);
    //                 v425 = EDGE::start(v89);
    //                 v425->lose(v425);
    //                 v426 = COEDGE::edge(this_off_coedge);
    //                 v426->lose(v426);
    //                 this_off_coedge->lose(this_off_coedge);
    //                 degen_off_coed_deleted = 1;
    //             } else {
    //                 v70 = COEDGE::start(prev_off_coedge);
    //                 v71 = VERTEX::geometry(v70);
    //                 v72 = APOINT::coords(v71);
    //                 SPAposition::SPAposition(&prev_pos, v72);
    //                 v73 = COEDGE::start(this_off_coedge);
    //                 v74 = VERTEX::geometry(v73);
    //                 v75 = APOINT::coords(v74);
    //                 SPAposition::SPAposition(&this_pos, v75);
    //                 v76 = operator-(&v736, &prev_pos, &this_pos);
    //                 v518 = SPAvector::len(v76);
    //                 v77 = safe_function_type<double>::operator double(&SPAresabs);
    //                 if(v77 <= v518) goto LABEL_134;
    //                 display_null_offset_coedge_to_be_deleted(this_off_coedge, this_orig_coedge);
    //                 v78 = COEDGE::edge(this_off_coedge);
    //                 v423 = EDGE::start(v78);
    //                 v423->lose(v423);
    //                 v424 = COEDGE::edge(this_off_coedge);
    //                 v424->lose(v424);
    //                 this_off_coedge->lose(this_off_coedge);
    //                 degen_off_coed_deleted = 1;
    //             }
    //         LABEL_183:
    //             if(ii >= 2 || degen_off_coed_deleted) {
    //                 if(!degen_off_coed_deleted) {
    //                     v435 = (offset_segment*)ACIS_OBJECT::operator new(0xB8ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 1398, &alloc_file_index_3517);
    //                     if(v435) {
    //                         v549 = (COEDGE*)ENTITY_LIST::operator[](&orig_coedge_list, i);
    //                         offset_segment::offset_segment(v435, this_off_coedge, v549);
    //                         v436 = v117;
    //                     } else {
    //                         v436 = 0i64;
    //                     }
    //                     v550 = v436;
    //                     new_seg = v436;
    //                     offset_segment_list::insert_segment(&seg_list, v436, curr_seg);
    //                     curr_seg = new_seg;
    //                     prev_off_coedge = this_off_coedge;
    //                 }
    //                 prev_orig_coedge = this_orig_coedge;
    //                 ++i;
    //                 goto LABEL_106;
    //             }
    //         }
    //     }
    //     v118 = COEDGE::previous(start_coedge);
    //     if(v118 != start_coedge || (v551 = COEDGE::start(start_coedge), v119 = COEDGE::end(start_coedge), v551 == v119)) {
    //         last_off_coedge = prev_off_coedge;
    //         COEDGE::set_previous(first_off_coedge, prev_off_coedge, 0, 1);
    //         COEDGE::set_next(last_off_coedge, first_off_coedge, 0, 1);
    //         v120 = COEDGE::end(last_off_coedge);
    //         if(is_TVERTEX(v120) && (v121 = COEDGE::start(first_off_coedge), is_TVERTEX(v121))) {
    //             v122 = COEDGE::end(last_off_coedge);
    //             v123 = VERTEX::geometry(v122);
    //             v124 = APOINT::coords(v123);
    //             SPAposition::SPAposition(&new_coords, v124);
    //             v125 = COEDGE::start(first_off_coedge);
    //             v126 = VERTEX::geometry(v125);
    //             v127 = APOINT::coords(v126);
    //             SPAposition::SPAposition(&v703, v127);
    //             v128 = operator-(&v724, &v703, &new_coords);
    //             operator*(&v, 0.5, v128);
    //             SPAposition::operator+=(&new_coords, &v);
    //             v129 = COEDGE::end(last_off_coedge);
    //             v552 = VERTEX::geometry(v129);
    //             APOINT::set_coords(v552, &new_coords);
    //             v130 = COEDGE::start(first_off_coedge);
    //             v553 = VERTEX::geometry(v130);
    //             APOINT::set_coords(v553, &new_coords);
    //             in_tol = 1.001 * SPAvector::len(&v);
    //             v554 = (TVERTEX*)COEDGE::end(last_off_coedge);
    //             TVERTEX::set_tolerance(v554, in_tol, 0);
    //             v555 = (TVERTEX*)COEDGE::start(first_off_coedge);
    //             TVERTEX::set_tolerance(v555, in_tol, 0);
    //             AcisVersion::AcisVersion(&v477, 20, 0, 0);
    //             v556 = v131;
    //             v557 = GET_ALGORITHMIC_VERSION(&v478);
    //             if(operator>=(v557, v556)) {
    //                 v558 = COEDGE::start(first_off_coedge);
    //                 v559 = COEDGE::end(last_off_coedge);
    //                 make_same_vertex_pointers(v559, v558, last_off_coedge, first_off_coedge, 1);
    //             }
    //         } else {
    //             v132 = safe_function_type<double>::operator double(&SPAresabs);
    //             if(law::zero(twist_law, v132)) {
    //                 SPAunit_vector::SPAunit_vector(&oTangent, 0.0, 0.0, 0.0);
    //                 SPAunit_vector::SPAunit_vector(&iStartOfstDir, 0.0, 0.0, 0.0);
    //                 SPAunit_vector::SPAunit_vector(&iEndOfstDir, 0.0, 0.0, 0.0);
    //                 comp_tangent_at_coedge_commom_vertex(prev_orig_coedge, start_coedge, last_off_coedge, first_off_coedge, &oTangent, &iStartOfstDir, &iEndOfstDir);
    //                 v561 = SpaAcis::NullObj::get_double();
    //                 v562 = SpaAcis::NullObj::get_int();
    //                 v560 = COEDGE::edge(start_coedge);
    //                 v133 = EDGE::start_param(v560, &v687);
    //                 v563 = SPAparameter::operator double(v133);
    //                 v564 = law::eval(dist_law, v563, v562, v561);
    //                 v324 = check_gap_type(start_coedge, prev_orig_coedge, wire_normal, v564);
    //                 if(v324 == 1 && trim) {
    //                     v565 = dist_laws[dist_law_size - 1];
    //                     v566 = COEDGE::end(prev_orig_coedge);
    //                     sg_close_offset_gap(&seg_list, &curr_seg, last_off_coedge, v566, &oTangent, &iStartOfstDir, &iEndOfstDir, v565, arc, iKeepMiniTopo, 0);
    //                 } else if(v324 == 2) {
    //                     v567 = dist_laws[dist_law_size - 1];
    //                     v568 = COEDGE::end(prev_orig_coedge);
    //                     sg_close_offset_gap(&seg_list, &curr_seg, last_off_coedge, v568, &oTangent, &iStartOfstDir, &iEndOfstDir, v567, close_type, iKeepMiniTopo, 0);
    //                 }
    //             }
    //         }
    //     }
    //     WIRE::set_coedge(offset_wire, first_off_coedge);
    // }
    // for(lawi = 0; lawi < dist_law_size; ++lawi) {
    //     law::remove(dist_laws[lawi]);
    //     law::remove(twist_laws[lawi]);
    // }
    // alloc_ptr = dist_laws;
    // ACIS_STD_TYPE_OBJECT::operator delete[](dist_laws);
    // dist_laws = 0i64;
    // v570 = twist_laws;
    // ACIS_STD_TYPE_OBJECT::operator delete[](twist_laws);
    // twist_laws = 0i64;
    // ENTITY_LIST::~ENTITY_LIST(&orig_coedge_list);
    // ENTITY_LIST::~ENTITY_LIST(&off_coedge_list);
    // exception_save::~exception_save(&exception_save_mark);
    // if(resignal_no || acis_interrupted()) sys_error(resignal_no, error_info_base_ptr);
    // acis_exception::~acis_exception(&error_info_holder);
    // v464 = (BODY*)ACIS_OBJECT::operator new(0x58ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 1478, &alloc_file_index_3517);
    // if(v464) {
    //     BODY::BODY(v464, offset_wire);
    //     v439 = (BODY*)v134;
    // } else {
    //     v439 = 0i64;
    // }
    // v571 = v439;
    // offset_wire_body = v439;
    // if(wire_transf) {
    //     v440 = (TRANSFORM*)ACIS_OBJECT::operator new(0xA8ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 1480, &alloc_file_index_3517);
    //     if(v440) {
    //         v572 = (SPAtransf*)TRANSFORM::transform(wire_transf);
    //         TRANSFORM::TRANSFORM(v440, v572);
    //         v441 = v135;
    //     } else {
    //         v441 = 0i64;
    //     }
    //     v573 = v441;
    //     v442 = v441;
    // } else {
    //     v442 = 0i64;
    // }
    // off_wire_transf = v442;
    // BODY::set_transform(offset_wire_body, v442, 1);
    // if(woffset_module_header.debug_level >= 0x1E) {
    //     acis_fprintf(debug_file_ptr, "!!! seg_list before sg_trim_offset_wire()\n");
    //     offset_segment_list::print(&seg_list, debug_file_ptr);
    //     acis_fprintf(debug_file_ptr, "\n");
    // }
    // if(woffset_module_header.debug_level >= 0x23) {
    //     acis_fprintf(debug_file_ptr, "!!! offset_wire_body before sg_trim_offset_wire()\n");
    //     debug_entity(offset_wire_body, debug_file_ptr);
    //     acis_fprintf(debug_file_ptr, "\n");
    // }
    // if(woffset_module_header.debug_level >= 0x28) {
    //     acis_fprintf(debug_file_ptr, "!!! Creating wb.sat file\n\n");
    //     fptr = fopen("wb.sat", "w");
    //     ENTITY_LIST::ENTITY_LIST(&elist);
    //     ENTITY_LIST::add(&elist, offset_wire_body, 1);
    //     api_save_entity_list(&v719, fptr, 1, &elist, 0i64);
    //     outcome::~outcome(&v719);
    //     fclose(fptr);
    //     ENTITY_LIST::~ENTITY_LIST(&elist);
    // }
    // intersection_found = 0;
    // inside_wire_trimmed = 0;
    // if(trim && (v136 = safe_function_type<double>::operator double(&SPAresabs), law::zero(twist_law, v136))) {
    //     offset_segment_list::set_base_wire(&seg_list, wire, coedge_count);
    //     offset_segment_list::set_edge_smooth_manager(&seg_list, iEdgeSmoothMgr);
    //     intersection_found = sg_trim_offset_wire(&seg_list, zero_length, overlap);
    //     inside_wire_trimmed = offset_segment_list::trim_inside_wire_done(&seg_list);
    //     if(seg_list.mSelfIntesectError) {
    //         delete_entity(offset_wire_body);
    //         v575 = 0i64;
    //         offset_segment_list::~offset_segment_list(&seg_list);
    //         return (BODY*)v575;
    //     }
    // } else {
    //     this_seg = offset_segment_list::first_segment(&seg_list);
    //     v138 = offset_segment_list::last_segment(&seg_list);
    //     last_seg = offset_segment::next(v138);
    //     while(this_seg != last_seg) {
    //         next_seg = offset_segment::next(this_seg);
    //         ofc = offset_segment::coedge(this_seg);
    //         pgc = offset_segment::original_coedge(this_seg);
    //         v139 = COEDGE::edge(ofc);
    //         if(is_intcurve_edge(v139)) {
    //             v140 = COEDGE::edge(pgc);
    //             if(is_intcurve_edge(v140)) {
    //                 v141 = COEDGE::edge(ofc);
    //                 v444 = EDGE::geometry(v141);
    //                 ofintc = (const intcurve*)v444->equation(v444);
    //                 v142 = COEDGE::edge(pgc);
    //                 v446 = EDGE::geometry(v142);
    //                 pgintc = (const intcurve*)v446->equation(v446);
    //                 v143 = intcurve::cur((intcurve*)pgintc, -1.0, 0);
    //                 t1 = bs3_curve_angle(v143);
    //                 v144 = intcurve::cur((intcurve*)ofintc, -1.0, 0);
    //                 t2 = bs3_curve_angle(v144);
    //                 if(t2 > t1 + 3.0) {
    //                     v581 = intcurve::fitol((intcurve*)ofintc);
    //                     actual_tol = 0.0;
    //                     v145 = COEDGE::edge(ofc);
    //                     v448 = EDGE::geometry(v145);
    //                     ofintc_upd = (intcurve*)v448->equation_for_update(v448);
    //                     param_range = ofintc_upd->param_range;
    //                     box = SpaAcis::NullObj::get_box();
    //                     param_range(ofintc_upd, &the_int, box);
    //                     new_bs3 = 0i64;
    //                     err_num = 0;
    //                     acis_exception::acis_exception(&v696, 0, 0i64, 0i64, 0);
    //                     e_info = 0i64;
    //                     exception_save::exception_save(&v663);
    //                     exception_save::begin(&v663);
    //                     get_error_mark()->buffer_init = 1;
    //                     v286 = 0;
    //                     new_bs3 = bs3_curve_make_approx(ofintc_upd, &the_int, v581, &actual_tol, 0, 0i64, 0, 0, 0);
    //                     if(new_bs3) {
    //                         rem_tol = actual_tol;
    //                         v146 = safe_function_type<double>::operator double(&SPAresabs);
    //                         if(v146 > rem_tol) rem_tol = safe_function_type<double>::operator double(&SPAresabs);
    //                         bs3_curve_rem_extra_knots(new_bs3, rem_tol);
    //                         t3 = bs3_curve_angle(new_bs3);
    //                         if(t1 + 3.0 > t3) {
    //                             intcurve::set_cur(ofintc_upd, new_bs3, rem_tol, 1, 0);
    //                             new_bs3 = 0i64;
    //                         }
    //                     }
    //                     if(new_bs3) {
    //                         bs3_curve_delete(&new_bs3);
    //                         new_bs3 = 0i64;
    //                     }
    //                     exception_save::~exception_save(&v663);
    //                     if(err_num || acis_interrupted()) sys_error(err_num, e_info);
    //                     acis_exception::~acis_exception(&v696);
    //                 }
    //             }
    //         }
    //         this_seg = next_seg;
    //     }
    // }
    // shell = 0i64;
    // if(BODY::lump(offset_wire_body)) {
    // LABEL_250:
    //     v147 = BODY::lump(offset_wire_body);
    //     shell = LUMP::shell(v147);
    //     goto LABEL_319;
    // }
    // if(BODY::wire(offset_wire_body)) {
    //     its_wire = BODY::wire(offset_wire_body);
    //     BODY::set_wire(offset_wire_body, 0i64, 1);
    //     v451 = (LUMP*)ACIS_OBJECT::operator new(0x50ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 1601, &alloc_file_index_3517);
    //     if(v451) {
    //         LUMP::LUMP(v451);
    //         v452 = v148;
    //     } else {
    //         v452 = 0i64;
    //     }
    //     v584 = v452;
    //     a_lump = v452;
    //     LUMP::set_body(v452, offset_wire_body, 1);
    //     BODY::set_lump(offset_wire_body, a_lump, 1);
    //     last_shell = 0i64;
    //     while(its_wire) {
    //         next_wire = WIRE::next(its_wire, PAT_CAN_CREATE);
    //         v453 = (SHELL*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 1609, &alloc_file_index_3517);
    //         if(v453) {
    //             SHELL::SHELL(v453);
    //             v454 = v149;
    //         } else {
    //             v454 = 0i64;
    //         }
    //         v585 = v454;
    //         a_shell = v454;
    //         SHELL::set_wire(v454, its_wire, 1);
    //         WIRE::set_shell(its_wire, a_shell, 1);
    //         SHELL::set_lump(a_shell, a_lump, 1);
    //         if(last_shell) {
    //             SHELL::set_next(last_shell, a_shell, 1);
    //         } else {
    //             last_shell = a_shell;
    //             LUMP::set_shell(a_lump, a_shell, 1);
    //         }
    //         its_wire = next_wire;
    //     }
    //     if(!trim) {
    //         simple_wire = 1;
    //         ENTITY_LIST::ENTITY_LIST(&coeds);
    //         v150 = BODY::lump(offset_wire_body);
    //         for(a_shella = LUMP::shell(v150); a_shella; a_shella = SHELL::next(a_shella, PAT_CAN_CREATE)) {
    //             its_wirea = SHELL::wire(a_shella);
    //             get_coedges(its_wirea, &coeds, PAT_CAN_CREATE);
    //             for(index = 0;; ++index) {
    //                 v151 = ENTITY_LIST::count(&coeds);
    //                 if(index >= v151) break;
    //                 coed = (COEDGE*)ENTITY_LIST::operator[](&coeds, index);
    //                 COEDGE::set_wire(coed, its_wirea, 1);
    //                 if(simple_wire) {
    //                     if(COEDGE::partner(coed) || COEDGE::previous(coed) && (v152 = COEDGE::previous(coed), v153 = COEDGE::next(v152), v153 != coed) || COEDGE::next(coed) && (v154 = COEDGE::next(coed), v155 = COEDGE::previous(v154), v155 != coed) ||
    //                        COEDGE::start(coed) && (v156 = COEDGE::start(coed), VERTEX::count_edges(v156) != 1) || COEDGE::end(coed) && (v157 = COEDGE::end(coed), VERTEX::count_edges(v157) != 1)) {
    //                         simple_wire = 0;
    //                     }
    //                 }
    //             }
    //             ENTITY_LIST::clear(&coeds);
    //         }
    //         AcisVersion::AcisVersion(&v479, 10, 0, 0);
    //         v587 = v158;
    //         v588 = GET_ALGORITHMIC_VERSION(&v480);
    //         v159 = operator>=(v588, v587);
    //         in_R10 = v159;
    //         if(v159)
    //             repair_dupicated_vertices(offset_wire_body);
    //         else
    //             simple_wire = 0;
    //         v376 = 0i64;
    //         if(simple_wire) {
    //             ENTITY_LIST::ENTITY_LIST(&new_wires);
    //             v160 = BODY::lump(offset_wire_body);
    //             for(a_shellb = LUMP::shell(v160); a_shellb; a_shellb = SHELL::next(a_shellb, PAT_CAN_CREATE)) {
    //                 v376 = a_shellb;
    //                 its_wireb = SHELL::wire(a_shellb);
    //                 start_coed = WIRE::coedge(its_wireb);
    //                 previous = start_coed;
    //                 wire_periodic = 0;
    //                 if(start_coed) {
    //                     if(COEDGE::previous(start_coed)) {
    //                         v161 = COEDGE::previous(start_coed);
    //                         if(v161 != previous) {
    //                             v162 = COEDGE::previous(start_coed);
    //                             v589 = COEDGE::end(v162);
    //                             v163 = COEDGE::start(previous);
    //                             if(v589 == v163) {
    //                                 wire_periodic = 1;
    //                             } else {
    //                                 v591 = COEDGE::previous(start_coed);
    //                                 next = COEDGE::previous(start_coed);
    //                                 COEDGE::set_next(v591, next, 0, 1);
    //                                 COEDGE::set_previous(previous, previous, 0, 1);
    //                             }
    //                         }
    //                     }
    //                 }
    //                 reset_period = 0;
    //                 while(previous) {
    //                     coed_next = COEDGE::next(previous);
    //                     if(coed_next == previous) coed_next = 0i64;
    //                     COEDGE::set_wire(previous, its_wireb, 1);
    //                     if(coed_next) {
    //                         v592 = COEDGE::start(coed_next);
    //                         v164 = COEDGE::end(previous);
    //                         if(v592 != v164) {
    //                             COEDGE::set_next(previous, previous, 0, 1);
    //                             COEDGE::set_previous(coed_next, coed_next, 0, 1);
    //                             if(wire_periodic) {
    //                                 WIRE::set_coedge(its_wireb, coed_next);
    //                                 start_coed = coed_next;
    //                                 wire_periodic = 0;
    //                                 reset_period = 1;
    //                             } else {
    //                                 v455 = (WIRE*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 1715, &alloc_file_index_3517);
    //                                 if(v455) {
    //                                     WIRE::WIRE(v455, coed_next, 0i64);
    //                                     v456 = v165;
    //                                 } else {
    //                                     v456 = 0i64;
    //                                 }
    //                                 v593 = v456;
    //                                 its_wireb = v456;
    //                                 COEDGE::set_wire(coed_next, v456, 1);
    //                                 ENTITY_LIST::add(&new_wires, its_wireb, 1);
    //                             }
    //                         }
    //                     }
    //                     previous = coed_next;
    //                     if(!reset_period && previous == start_coed) previous = 0i64;
    //                     reset_period = 0;
    //                 }
    //             }
    //             ENTITY_LIST::init(&new_wires);
    //             for(m = (WIRE*)ENTITY_LIST::next(&new_wires); m; m = (WIRE*)ENTITY_LIST::next(&new_wires)) {
    //                 v457 = (SHELL*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 1742, &alloc_file_index_3517);
    //                 if(v457) {
    //                     SHELL::SHELL(v457);
    //                     v458 = v166;
    //                 } else {
    //                     v458 = 0i64;
    //                 }
    //                 v594 = v458;
    //                 a_shellc = v458;
    //                 SHELL::set_wire(v458, m, 1);
    //                 WIRE::set_shell(m, a_shellc, 1);
    //                 v167 = BODY::lump(offset_wire_body);
    //                 SHELL::set_lump(a_shellc, v167, 1);
    //                 SHELL::set_next(v376, a_shellc, 1);
    //                 v376 = a_shellc;
    //             }
    //             ENTITY_LIST::~ENTITY_LIST(&new_wires);
    //         }
    //         ENTITY_LIST::~ENTITY_LIST(&coeds);
    //         goto LABEL_319;
    //     }
    //     goto LABEL_250;
    // }
    // LABEL_319:
    // if(!inside_wire_trimmed && shell && (SHELL::next(shell, PAT_CAN_CREATE) || intersection_found)) {
    //     while(shell && law::constant(dist_law)) {
    //         v595 = SpaAcis::NullObj::get_double();
    //         v596 = SpaAcis::NullObj::get_int();
    //         X = law::eval(dist_law, 1.0, v596, v595);
    //         v168 = SHELL::wire(shell);
    //         v169 = WIRE::coedge(v168);
    //         if(COEDGE::sense(v169)) {
    //             v174 = SHELL::wire(shell);
    //             v175 = WIRE::coedge(v174);
    //             v172 = COEDGE::end(v175);
    //         } else {
    //             v170 = SHELL::wire(shell);
    //             v171 = WIRE::coedge(v170);
    //             v172 = COEDGE::start(v171);
    //         }
    //         v173 = VERTEX::geometry(v172);
    //         v459 = (SPAposition*)APOINT::coords(v173);
    //         p = v459;
    //         SPAposition::SPAposition(&testpos, v459);
    //         start_coedge = start_of_wire_chain(wire);
    //         this_coedgea = start_coedge;
    //         cutout = 0;
    //         while(1) {
    //             SPAposition::SPAposition(&cpoint);
    //             SPAtransf::SPAtransf(&v742);
    //             face_trans = v176;
    //             v599 = COEDGE::edge(this_coedgea);
    //             find_cls_pt_on_edge(&testpos, v599, &cpoint, face_trans);
    //             v177 = operator-(&v725, &cpoint, &testpos);
    //             testdist = SPAvector::len(v177);
    //             max_error = 0.0;
    //             find_max_tolerance(this_coedgea, &max_error);
    //             if(max_error < 0.001) max_error = DOUBLE_0_001;
    //             v601 = testdist + max_error;
    //             v178 = fabs_0(X);
    //             if(v178 > v601) break;
    //             prev_coedgeb = this_coedgea;
    //             this_coedgea = COEDGE::next(this_coedgea);
    //             if(this_coedgea == prev_coedgeb || this_coedgea == start_coedge) goto LABEL_348;
    //         }
    //         if(Debug_Break_Active("trim shells", "WIRE-OFFSET")) {
    //             if(get_breakpoint_callback()) {
    //                 v460 = get_breakpoint_callback();
    //                 v602 = v460->new_render_object;
    //                 v461 = v602(v460, 1);
    //             } else {
    //                 v461 = 0i64;
    //             }
    //             v299 = v461;
    //             if(v461) {
    //                 v179 = SHELL::wire(shell);
    //                 show_entity(v179, TOPOLOGY_NEW, v299);
    //                 insert_point = v299->insert_point;
    //                 insert_point(v299, &testpos);
    //                 v604 = v299->insert_point;
    //                 v604(v299, &cpoint);
    //                 v180 = operator-(&v726, &testpos, &cpoint);
    //                 v181 = normalise(&v727, v180);
    //                 operator*(&direction, v181, X);
    //                 v412 = (ellipse*)ACIS_OBJECT::operator new(0xB0ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 1801, &alloc_file_index_3517);
    //                 if(v412) {
    //                     ellipse::ellipse(v412, &cpoint, wire_normal, &direction, 1.0, 0.0);
    //                     v384 = (ellipse*)v182;
    //                 } else {
    //                     v384 = 0i64;
    //                 }
    //                 v605 = v384;
    //                 elli = v384;
    //                 show_curve(v384, -3.1415927, 3.1415927, v299);
    //                 v374 = elli;
    //                 if(elli) {
    //                     v606 = v374->~ellipse;
    //                     v607 = ((__int64(__fastcall*)(ellipse*, __int64))v606)(v374, 1i64);
    //                 } else {
    //                     v607 = 0i64;
    //                 }
    //                 acis_fprintf(debug_file_ptr, "sg_offset_planar_wire - trimming shells\n");
    //                 Debug_Break("trim shells", "WIRE-OFFSET", "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 1809);
    //                 if(get_breakpoint_callback()) {
    //                     v386 = get_breakpoint_callback();
    //                     v608 = v386->delete_render_object;
    //                     v608(v386, v299);
    //                 }
    //             }
    //         }
    //         cutout = 1;
    //     LABEL_348:
    //         nshell = SHELL::next(shell, PAT_CAN_CREATE);
    //         if(cutout) {
    //             extract_shell(shell, 0);
    //             del_entity(shell);
    //         }
    //         shell = nshell;
    //     }
    // }
    // ENTITY_LIST::ENTITY_LIST(&all_shells);
    // ENTITY_LIST::ENTITY_LIST(&all_lumps);
    // shells = api_get_shells(&v720, offset_wire_body, &all_shells, PAT_CAN_CREATE, 0i64);
    // v611 = shells;
    // check_outcome(shells);
    // outcome::~outcome(&v720);
    // lumps = api_get_lumps(&v721, offset_wire_body, &all_lumps, PAT_CAN_CREATE, 0i64);
    // v613 = lumps;
    // check_outcome(lumps);
    // outcome::~outcome(&v721);
    // v358 = ENTITY_LIST::count(&all_shells);
    // v183 = ENTITY_LIST::count(&all_lumps);
    // if(v358 != v183) {
    //     head_lump = 0i64;
    //     prev_lump = 0i64;
    //     for(n = 0; n < ENTITY_LIST::count(&all_shells); ++n) {
    //         current_shell = (SHELL*)ENTITY_LIST::operator[](&all_shells, n);
    //         v387 = (LUMP*)ACIS_OBJECT::operator new(0x50ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 1847, &alloc_file_index_3517);
    //         if(v387) {
    //             LUMP::LUMP(v387);
    //             v388 = v184;
    //         } else {
    //             v388 = 0i64;
    //         }
    //         v614 = v388;
    //         current_lump = v388;
    //         LUMP::set_body(v388, offset_wire_body, 1);
    //         LUMP::set_shell(current_lump, current_shell, 1);
    //         if(head_lump)
    //             LUMP::set_next(prev_lump, current_lump, 1);
    //         else
    //             head_lump = current_lump;
    //         prev_lump = current_lump;
    //         SHELL::set_lump(current_shell, current_lump, 1);
    //         SHELL::set_next(current_shell, 0i64, 1);
    //     }
    //     BODY::set_lump(offset_wire_body, head_lump, 1);
    //     for(kk = 0; kk < ENTITY_LIST::count(&all_lumps); ++kk) {
    //         current_lump = (LUMP*)ENTITY_LIST::operator[](&all_lumps, kk);
    //         LUMP::set_body(current_lump, 0i64, 1);
    //         LUMP::set_shell(current_lump, 0i64, 1);
    //         v615 = api_del_entity(&v722, current_lump, 0i64);
    //         v616 = v615;
    //         check_outcome(v615);
    //         outcome::~outcome(&v722);
    //     }
    // }
    // if(woffset_module_header.debug_level >= 0x1E) {
    //     acis_fprintf(debug_file_ptr, "!!! seg_list after sg_trim_offset_wire()\n");
    //     offset_segment_list::print(&seg_list, debug_file_ptr);
    //     acis_fprintf(debug_file_ptr, "\n");
    // }
    // if(woffset_module_header.debug_level >= 0x23) {
    //     acis_fprintf(debug_file_ptr, "*** offset_wire_body after sg_trim_offset_wire()\n");
    //     debug_entity(offset_wire_body, debug_file_ptr);
    //     acis_fprintf(debug_file_ptr, "\n");
    // }
    // if(woffset_module_header.debug_level >= 0xA) acis_fprintf(debug_file_ptr, "    Leaving sg_offset_planar_wire()\n");
    // ENTITY_LIST::ENTITY_LIST(&edges);
    // get_edges(offset_wire_body, &edges, PAT_CAN_CREATE);
    // if(ENTITY_LIST::count(&edges) > 0) {
    //     first_edge = (EDGE*)ENTITY_LIST::first(&edges);
    //     for(edge = first_edge; edge; edge = (EDGE*)ENTITY_LIST::next(&edges)) {
    //         if(EDGE::geometry(edge)) {
    //             v390 = EDGE::geometry(edge);
    //             edge_cur = v390->equation(v390);
    //             if(edge_cur->type((curve*)edge_cur) == 11) {
    //                 SPAposition::SPAposition(&pt);
    //                 SPAvector::SPAvector(&dpdt);
    //                 SPAvector::SPAvector(&dp2dt2);
    //                 _deriv_ptr[0] = &dpdt;
    //                 _deriv_ptr[1] = &dp2dt2;
    //                 v185 = EDGE::start_param(edge, &v690);
    //                 t_val = SPAparameter::operator double(v185);
    //                 v618 = EDGE::end_param(edge, &v689);
    //                 v619 = EDGE::start_param(edge, &v688);
    //                 v186 = operator-(v619, v618);
    //                 max_t_step = fabs_0(v186) / 10.0;
    //                 v620 = EDGE::end_param(edge, &v679);
    //                 v621 = EDGE::start_param(edge, &v680);
    //                 if(operator<(v621, v620))
    //                     v328 = 1;
    //                 else
    //                     v328 = -1;
    //                 dir = (double)v328;
    //                 evaluate = edge_cur->evaluate;
    //                 ((void(__fastcall*)(const curve*, __int64, SPAposition*, SPAvector**, int, int))evaluate)(edge_cur, v187, &pt, _deriv_ptr, 2, 2);
    //                 conv_curvature(&start_curvature, _deriv_ptr[0], _deriv_ptr[1]);
    //                 start_t_step = max_t_step;
    //                 v623 = SPAvector::len(&start_curvature);
    //                 v188 = safe_function_type<double>::operator double(&SPAresabs);
    //                 if(v623 > 1.0 / v188) {
    //                     v624 = SPAvector::len(_deriv_ptr[1]);
    //                     v189 = safe_function_type<double>::operator double(&SPAresmch);
    //                     if(v624 > v189) {
    //                         v625 = safe_function_type<double>::operator double(&SPAresabs);
    //                         v190 = SPAvector::len(_deriv_ptr[1]);
    //                         start_t_step = acis_sqrt(v625 / v190);
    //                     }
    //                     if(start_t_step > max_t_step) start_t_step = max_t_step;
    //                 }
    //                 v191 = EDGE::end_param(edge, &v681);
    //                 t_val = SPAparameter::operator double(v191);
    //                 v626 = edge_cur->evaluate;
    //                 ((void(__fastcall*)(const curve*, __int64, SPAposition*, SPAvector**, int, int))v626)(edge_cur, v192, &pt, _deriv_ptr, 2, 2);
    //                 conv_curvature(&end_curvature, _deriv_ptr[0], _deriv_ptr[1]);
    //                 end_t_step = max_t_step;
    //                 v627 = SPAvector::len(&end_curvature);
    //                 v193 = safe_function_type<double>::operator double(&SPAresabs);
    //                 if(v627 > 1.0 / v193) {
    //                     v628 = SPAvector::len(_deriv_ptr[1]);
    //                     v194 = safe_function_type<double>::operator double(&SPAresmch);
    //                     if(v628 > v194) {
    //                         v629 = safe_function_type<double>::operator double(&SPAresabs);
    //                         v195 = SPAvector::len(_deriv_ptr[1]);
    //                         end_t_step = acis_sqrt(v629 / v195);
    //                     }
    //                     if(end_t_step > max_t_step) end_t_step = max_t_step;
    //                 }
    //                 adjust_edge_start = 0;
    //                 adjust_edge_end = 0;
    //                 v196 = EDGE::start_param(edge, &v682);
    //                 start_par = SPAparameter::operator double(v196);
    //                 v197 = EDGE::end_param(edge, &v683);
    //                 end_par = SPAparameter::operator double(v197);
    //                 open_start_vert = 0;
    //                 open_end_vert = 0;
    //                 ENTITY_LIST::ENTITY_LIST(&start_vert_edges);
    //                 ENTITY_LIST::ENTITY_LIST(&end_vert_edges);
    //                 v198 = EDGE::start(edge);
    //                 get_edges(v198, &start_vert_edges, PAT_CAN_CREATE);
    //                 if(ENTITY_LIST::count(&start_vert_edges) == 1) {
    //                     v630 = EDGE::start(edge);
    //                     v199 = EDGE::end(edge);
    //                     if(v630 != v199) open_start_vert = 1;
    //                 }
    //                 v200 = EDGE::end(edge);
    //                 get_edges(v200, &end_vert_edges, PAT_CAN_CREATE);
    //                 if(ENTITY_LIST::count(&end_vert_edges) == 1) {
    //                     v631 = EDGE::start(edge);
    //                     v201 = EDGE::end(edge);
    //                     if(v631 != v201) open_end_vert = 1;
    //                 }
    //                 v632 = SPAvector::len(&start_curvature);
    //                 v202 = safe_function_type<double>::operator double(&SPAresabs);
    //                 if(v632 > 1.0 / v202) {
    //                     t_adjust = start_t_step;
    //                     div = DOUBLE_2_0;
    //                     curvature = 0.0;
    //                     SPAposition::SPAposition(&prev_pt);
    //                     if(start_t_step == max_t_step) {
    //                         v203 = EDGE::start(edge);
    //                         v204 = VERTEX::geometry(v203);
    //                         qmemcpy(&prev_pt, APOINT::coords(v204), sizeof(prev_pt));
    //                         while(1) {
    //                             v633 = edge_cur->evaluate;
    //                             ((void(__fastcall*)(const curve*, __int64, SPAposition*, SPAvector**, int, int))v633)(edge_cur, v205, &pt, _deriv_ptr, 2, 2);
    //                             v206 = conv_curvature(&v728, _deriv_ptr[0], _deriv_ptr[1]);
    //                             v634 = SPAvector::len(v206);
    //                             v207 = 1.0 / safe_function_type<double>::operator double(&SPAresabs);
    //                             if(v634 > v207) break;
    //                             v213 = safe_function_type<double>::operator double(&SPAresabs);
    //                             if(same_point(&prev_pt, &pt, v213)) start_t_step = t_adjust / div;
    //                             div = div * 2.0;
    //                             qmemcpy(&prev_pt, &pt, sizeof(prev_pt));
    //                         }
    //                         v366 = start_par + dir * t_adjust / div * 2.0;
    //                         v368 = start_par + dir * t_adjust / div;
    //                         while(1) {
    //                             v208 = safe_function_type<double>::operator double(&SPAresabs);
    //                             if(same_point(&prev_pt, &pt, v208)) break;
    //                             t = (v368 + v366) / 2.0;
    //                             v635 = edge_cur->evaluate;
    //                             ((void(__fastcall*)(const curve*, __int64, SPAposition*, SPAvector**, int, int))v635)(edge_cur, v209, &pt, _deriv_ptr, 2, 2);
    //                             v210 = conv_curvature(&v729, _deriv_ptr[0], _deriv_ptr[1]);
    //                             v636 = SPAvector::len(v210);
    //                             v211 = safe_function_type<double>::operator double(&SPAresabs);
    //                             if(v636 <= 1.0 / v211) {
    //                                 v366 = t;
    //                             } else {
    //                                 v637 = t - v368;
    //                                 v212 = safe_function_type<double>::operator double(&SPAresmch);
    //                                 if(v212 > v637) break;
    //                                 v368 = t;
    //                             }
    //                         }
    //                         start_t_step = fabs_0(v366 - start_par);
    //                     }
    //                     adjust_edge_start = 1;
    //                     start_par = start_par + start_t_step * dir;
    //                 }
    //                 v638 = SPAvector::len(&end_curvature);
    //                 v214 = 1.0 / safe_function_type<double>::operator double(&SPAresabs);
    //                 if(v638 > v214) {
    //                     v350 = end_t_step;
    //                     v311 = DOUBLE_2_0;
    //                     SPAposition::SPAposition(&v668);
    //                     if(end_t_step == max_t_step) {
    //                         v215 = EDGE::end(edge);
    //                         v216 = VERTEX::geometry(v215);
    //                         qmemcpy(&v668, APOINT::coords(v216), sizeof(v668));
    //                         while(1) {
    //                             v639 = edge_cur->evaluate;
    //                             ((void(__fastcall*)(const curve*, __int64, SPAposition*, SPAvector**, int, int))v639)(edge_cur, v217, &pt, _deriv_ptr, 2, 2);
    //                             v218 = conv_curvature(&v739, _deriv_ptr[0], _deriv_ptr[1]);
    //                             v640 = SPAvector::len(v218);
    //                             v219 = 1.0 / safe_function_type<double>::operator double(&SPAresabs);
    //                             if(v640 > v219) break;
    //                             v224 = safe_function_type<double>::operator double(&SPAresabs);
    //                             if(same_point(&v668, &pt, v224)) end_t_step = v350 / v311;
    //                             v311 = v311 * 2.0;
    //                             qmemcpy(&v668, &pt, sizeof(v668));
    //                         }
    //                         v364 = end_par - dir * v350 / v311 * 2.0;
    //                         v391 = end_par - dir * v350 / v311;
    //                         while(1) {
    //                             v220 = safe_function_type<double>::operator double(&SPAresabs);
    //                             if(same_point(&v668, &pt, v220)) break;
    //                             v365 = (v391 + v364) / 2.0;
    //                             v641 = edge_cur->evaluate;
    //                             ((void(__fastcall*)(const curve*, __int64, SPAposition*, SPAvector**, int, int))v641)(edge_cur, v221, &pt, _deriv_ptr, 2, 2);
    //                             v222 = conv_curvature(&v730, _deriv_ptr[0], _deriv_ptr[1]);
    //                             v642 = SPAvector::len(v222);
    //                             v223 = safe_function_type<double>::operator double(&SPAresabs);
    //                             if(v642 <= 1.0 / v223)
    //                                 v364 = v365;
    //                             else
    //                                 v391 = v365;
    //                         }
    //                         end_t_step = fabs_0(v364 - end_par);
    //                     }
    //                     adjust_edge_end = 1;
    //                     end_par = end_par - end_t_step * dir;
    //                 }
    //                 if(adjust_edge_start) {
    //                     v392 = EDGE::geometry(edge);
    //                     v644 = v392->equation_for_update(v392);
    //                     SPAinterval::SPAinterval(&v731, start_par, end_par);
    //                     given_range = v225;
    //                     curve::limit(v644, v225);
    //                     if(open_start_vert) {
    //                         eval_position = edge_cur->eval_position;
    //                         ((void(__fastcall*)(const curve*, SPAposition*, __int64, __int64, _DWORD))eval_position)(edge_cur, &start_pos, v226, 1i64, 0);
    //                         v399 = (APOINT*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 2028, &alloc_file_index_3517);
    //                         if(v399) {
    //                             APOINT::APOINT(v399, &start_pos);
    //                             v393 = v227;
    //                         } else {
    //                             v393 = 0i64;
    //                         }
    //                         v646 = v393;
    //                         start_point = v393;
    //                         v648 = EDGE::start(edge);
    //                         VERTEX::set_geometry(v648, start_point, 1);
    //                         EDGE::set_param_range(edge, 0i64);
    //                     } else {
    //                         tvert = 0i64;
    //                         v228 = EDGE::start(edge);
    //                         if(!is_TVERTEX(v228)) {
    //                             v394 = (APOINT*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 2035, &alloc_file_index_3517);
    //                             if(v394) {
    //                                 v229 = EDGE::start(edge);
    //                                 v230 = VERTEX::geometry(v229);
    //                                 here = (SPAposition*)APOINT::coords(v230);
    //                                 APOINT::APOINT(v394, here);
    //                                 v395 = v231;
    //                             } else {
    //                                 v395 = 0i64;
    //                             }
    //                             v650 = v395;
    //                             ap = v395;
    //                             v232 = EDGE::start(edge);
    //                             replace_vertex_with_tvertex(v232, &tvert);
    //                             VERTEX::set_geometry(tvert, ap, 1);
    //                             TVERTEX::set_tolerance(tvert, 0.0, 1);
    //                         }
    //                         EDGE::set_param_range(edge, 0i64);
    //                     }
    //                 }
    //                 if(adjust_edge_end) {
    //                     v396 = EDGE::geometry(edge);
    //                     v653 = v396->equation_for_update(v396);
    //                     SPAinterval::SPAinterval(&v732, start_par, end_par);
    //                     v652 = v233;
    //                     curve::limit(v653, v233);
    //                     if(open_end_vert) {
    //                         v654 = edge_cur->eval_position;
    //                         ((void(__fastcall*)(const curve*, SPAposition*, __int64, __int64, _DWORD))v654)(edge_cur, &end_pos, v234, 1i64, 0);
    //                         v397 = (APOINT*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 2049, &alloc_file_index_3517);
    //                         if(v397) {
    //                             APOINT::APOINT(v397, &end_pos);
    //                             v398 = v235;
    //                         } else {
    //                             v398 = 0i64;
    //                         }
    //                         v655 = v398;
    //                         end_point = v398;
    //                         v657 = EDGE::end(edge);
    //                         VERTEX::set_geometry(v657, end_point, 1);
    //                         EDGE::set_param_range(edge, 0i64);
    //                     } else {
    //                         this_tvertex = 0i64;
    //                         v236 = EDGE::end(edge);
    //                         if(!is_TVERTEX(v236)) {
    //                             v463 = (APOINT*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 2057, &alloc_file_index_3517);
    //                             if(v463) {
    //                                 v237 = EDGE::end(edge);
    //                                 v238 = VERTEX::geometry(v237);
    //                                 v658 = (SPAposition*)APOINT::coords(v238);
    //                                 APOINT::APOINT(v463, v658);
    //                                 v400 = v239;
    //                             } else {
    //                                 v400 = 0i64;
    //                             }
    //                             v659 = v400;
    //                             point = v400;
    //                             v240 = EDGE::end(edge);
    //                             replace_vertex_with_tvertex(v240, &this_tvertex);
    //                             VERTEX::set_geometry(this_tvertex, point, 1);
    //                             TVERTEX::set_tolerance(this_tvertex, 0.0, 1);
    //                         }
    //                         EDGE::set_param_range(edge, 0i64);
    //                     }
    //                 }
    //                 ENTITY_LIST::~ENTITY_LIST(&end_vert_edges);
    //                 ENTITY_LIST::~ENTITY_LIST(&start_vert_edges);
    //             }
    //         }
    //     }
    // }
    // if(bOutwards && offset_wire_body && !BODY::lump(offset_wire_body) && (!iEdgeSmoothMgr || ofst_edge_smooth_manager::is_last_iteration(iEdgeSmoothMgr))) {
    //     v241 = message_module::message_code(&spaacisds_api_errmod, 0);
    //     ofst_error(v241, 1, wire);
    // }
    // display_wire_offset_io(wire, dist_law, offset_wire_body);
    // v661 = offset_wire_body;
    // ENTITY_LIST::~ENTITY_LIST(&edges);
    // ENTITY_LIST::~ENTITY_LIST(&all_lumps);
    // ENTITY_LIST::~ENTITY_LIST(&all_shells);
    // offset_segment_list::~offset_segment_list(&seg_list);
    // return v661;
    return nullptr;
}