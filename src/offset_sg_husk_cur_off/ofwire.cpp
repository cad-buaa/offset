
#include "PrivateInterfaces/apiFinder.hxx"
#include "PrivateInterfaces/ofst_edge_smooth_manager.hxx"
#include "PrivateInterfaces/ofst_natural_extender.hxx"
#include "ProtectedInterfaces/off_cu.hxx"  //陆
#include "ProtectedInterfaces/ofsttools.hxx"
#include "PublicInterfaces/gme_sgcofrtn.hxx"
#include "acis/acismath.h"
#include "acis/acistype.hxx"
#include "acis/annotation.hxx"
#include "acis/api.err"
#include "acis/api_model_comparison.hxx"
#include "acis/attrib.hxx"
#include "acis/bl_sup.hxx"
#include "acis/boolapi.hxx"
#include "acis/boolopts.hxx"
#include "acis/bs3curve.hxx"
#include "acis/calctol.hxx"
#include "acis/ckoutcom.hxx"
#include "acis/cnstruct.hxx"
#include "acis/copyanno.hxx"
#include "acis/cucuint.hxx"
#include "acis/cur.hxx"
#include "acis/cur_off.err"
#include "acis/cur_off_anno.hxx"
#include "acis/curdef.hxx"
#include "acis/curextnd.hxx"
#include "acis/curve.hxx"
#include "acis/debug.hxx"
#include "acis/deltop.hxx"
#include "acis/dmexcept.hxx"
#include "acis/ellipse.hxx"
#include "acis/err_info_base.hxx"
#include "acis/errmsg.hxx"
#include "acis/geometry.hxx"
#include "acis/get_top.hxx"
#include "acis/intdef.hxx"
#include "acis/inteded.hxx"
#include "acis/interval.hxx"
#include "acis/kernapi.hxx"
#include "acis/law.hxx"
#include "acis/law_base.hxx"
#include "acis/list_header.hxx"
#include "acis/main_law.hxx"
#include "acis/module.hxx"
#include "acis/option.hxx"
#include "acis/point.hxx"
#include "acis/ptentrel.hxx"
#include "acis/remtop.hxx"
#include "acis/repedge.hxx"
#include "acis/sgcofrtn.hxx"
#include "acis/sgquery.hxx"
#include "acis/sp3crtn.hxx"
#include "acis/spd3rtn.hxx"
#include "acis/sps3crtn.hxx"
#include "acis/straight.hxx"
#include "acis/testwire.hxx"
#include "acis/vector.hxx"
#include "acis/wire_qry.hxx"
#include "acis/wire_utl.hxx"

extern APIFINDER apiFinderACIS;
extern PROCSTATE prostate;

module_debug woffset_module_header("woffset");

int sg_extend_coedges(law* dist_law, SPAposition& in_pos, COEDGE* in_coedge, curve* crv_geom, SPAvector& v_dist) {
    SPAinterval new_interval = crv_geom->param_range();
    double max_extension = (new_interval.length()) * 10.0;
    if(crv_geom->param_range().infinite() || crv_geom->closed()) return 1;
    double ext_param = crv_geom->param(in_pos);
    double v39 = fabs(ext_param - crv_geom->param_range().end_pt());
    double started = crv_geom->param_range().start_pt();
    int f_extend_start = v39 > fabs(ext_param - started);
    double ofst_dist = dist_law->eval(ext_param);
    SPAposition a_position;
    SPAvector first_deri;
    crv_geom->eval(ext_param, a_position, first_deri);  // 不确
    if(SPAresabs > first_deri.len()) return 0;
    SPAvector v14 = crv_geom->eval_curvature(ext_param);
    double curveature_norm = v14.len();
    double ofst_param;
    if(SPAresabs <= curveature_norm) {
        double ofst_len = 2.0 / curveature_norm * ofst_dist;
        if(ofst_len < 6.28) ofst_len = 6.28;
        ofst_param = ofst_len * first_deri.len();
        if(ofst_param > max_extension) ofst_param = max_extension;
    } else {
        first_deri = normalise(first_deri);
        double v15 = first_deri % v_dist;
        double ofst_param = 4.0 * fabs(v15);
        if(SPAresabs > ofst_param) ofst_param = max_extension;
    }
    SPAinterval delta;
    if(f_extend_start) {
        SPAinterval v61(-ofst_param, 0.0);
        delta = v61;
    } else {
        SPAinterval v62(0.0, ofst_param);
        delta = v62;
    }
    new_interval += delta;
    SPAinterval check_interval = extend_curve(*crv_geom, new_interval, 1);
    int bIsNotPeriodic = crv_geom->periodic() == 0;
    if(GET_ALGORITHMIC_VERSION() <= AcisVersion(13, 0, 6)) bIsNotPeriodic = 1;
    if((check_interval != new_interval) && bIsNotPeriodic) return 0;
    in_coedge->edge()->set_geometry(make_curve(*crv_geom), 1);
    return 1;
}

int comp_arc_center(SPAposition& iPoint1, SPAposition& iPoint2, const double iRadius, SPAposition& iRefPoint, SPAposition& oCenter) {
    int retOk = 0;
    SPAunit_vector v6 = normalise((iPoint1 - iRefPoint));
    SPAvector majorAxis1 = iRadius * v6;
    SPAunit_vector v8 = normalise(iPoint2 - iRefPoint);
    SPAvector majorAxis2 = iRadius * v8;
    SPAunit_vector planeNormal = normalise(majorAxis2 * majorAxis1);
    acis_exception error_info_holder(0);
    error_info_base* error_info_base_ptr = nullptr;
    exception_save exception_save_mark;
    curve_curve_int* intersections = nullptr;
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    ellipse* v28 = ACIS_NEW ellipse(iPoint1, planeNormal, majorAxis1, 1.0, 0.0);
    ellipse* v30 = ACIS_NEW ellipse(iPoint2, planeNormal, majorAxis2, 1.0, 0.0);

    if(v28 && v30) {
        SPAbox box_cur;
        SPAinterval v12 = v28->param_range();
        if(v12.infinite() && (v30->param_range().infinite())) {
            SPAinterval tmp_int(-10000.0, 10000.0);
            SPAbox v73(tmp_int, tmp_int, tmp_int);
            box_cur = v73;
        } else {
            SPAbox b2 = v30->bound(v30->param_range());
            SPAbox b1 = v28->bound(v28->param_range());
            box_cur = b1 | b2;
        }
        intersections = int_cur_cur(*v28, *v30, box_cur, SPAresabs * 0.1);
        if(intersections) {
            oCenter = intersections->int_point;
            const curve_curve_int* nxtIntersection = intersections->next;
            if(nxtIntersection) {
                double dst1 = (intersections->int_point - iRefPoint).len_sq();
                double dst2 = (nxtIntersection->int_point - iRefPoint).len_sq();
                if(dst1 > dst2) oCenter = nxtIntersection->int_point;
                nxtIntersection = nullptr;
            }
            retOk = 1;
        }
    }

    if(v28) {
        ACIS_DELETE v28;
        v28 = nullptr;
    }
    if(v30) {
        ACIS_DELETE v30;
        v30 = nullptr;
    }

    delete_curve_curve_ints(intersections);
    intersections = nullptr;

    if(acis_interrupted()) sys_error(0, error_info_base_ptr);
    return retOk;
}

// 输入一个中心点，两个coedge的偏置点，输入中心点的VERTEX(用于注释)，输入偏置法则，输入该段。用圆弧连接他们
void sg_close_with_arc(offset_segment* this_seg, law* dist_law, SPAposition& p, SPAposition& p1, SPAposition& p2, COEDGE* c1, COEDGE* c2, VERTEX* vert) {
    SPAposition arcCenter(p);
    SPAunit_vector v1 = normalise(p1 - p);
    SPAunit_vector v2 = normalise(p2 - p);
    double dist1 = (p1 - p).len();
    double dist2 = (p2 - p).len();
    double endpram = c1->edge()->param_range().end_pt();
    double dist = dist_law->eval(endpram);
    if(SPAresfit > (p1 - p2).len() && dist1 != dist2 && comp_arc_center(p1, p2, dist, p, arcCenter)) {
        dist = (p1 - arcCenter).len();
        SPAunit_vector v1 = normalise((p1 - arcCenter));
        SPAunit_vector v2 = normalise((p2 - arcCenter));
    }
    SPAunit_vector normal;
    if((v1 * v2).len() <= SPAresabs) {
        SPAunit_vector tangent = coedge_end_dir(this_seg->original_coedge());
        normal = normalise(tangent * v2);
        if(normal.is_zero(0.0)) {
            normal = normalise(v1 * tangent);
        }
    } else {
        normal = normalise(v1 * v2);
    }
    ELLIPSE* v66 = ACIS_NEW ELLIPSE(arcCenter, normal, fabs(dist) * v1, 1.0);
    VERTEX* vert_start = c1->end();
    VERTEX* vert_end = c2->start();  // 修改ida
    if(GET_ALGORITHMIC_VERSION() >= AcisVersion(20, 0, 0)) {
        if(!c1->edge()->geometry() || (!c2->edge()->geometry())) {
            SPAunit_vector c1_dir;
            SPAunit_vector c2_dir;
            COEDGE* orig_c1 = this_seg->original_coedge();
            COEDGE* orig_c2 = orig_c1->next();
            if(GET_ALGORITHMIC_VERSION() < AcisVersion(21, 0, 0)) {
                if(orig_c1->sense()) {
                    c1_dir = normalise(orig_c1->edge()->start_deriv());
                } else {
                    c1_dir = normalise(orig_c1->edge()->end_deriv());
                }
                if(orig_c2->sense()) {
                    c2_dir = normalise(orig_c2->edge()->end_deriv());
                } else {
                    c2_dir = normalise(orig_c2->edge()->start_deriv());
                }
            } else {
                c1_dir = coedge_end_dir(orig_c1);

                c2_dir = coedge_start_dir(orig_c2);
            }
            double angle = safe_acos(c1_dir % c2_dir);
            if(!c1->edge()->geometry()) {
                double param_end = v66->equation().param(c2->start_pos());
                SPAposition pos_start = v66->equation().eval_position(param_end);
                double dist_sq = (vert_start->geometry()->coords() - pos_start).len_sq();
                if(dist_sq <= SPAresfit * SPAresfit) {
                    if(dist_sq > SPAresabs * SPAresabs && !is_TVERTEX(vert_start)) {
                        TVERTEX* tvert_start = nullptr;
                        replace_vertex_with_tvertex(vert_start, tvert_start);
                        vert_start = tvert_start;
                    }
                } else {
                    APOINT* v74 = ACIS_NEW APOINT(pos_start);
                    VERTEX* v76 = ACIS_NEW VERTEX(v74);
                    vert_start = (TVERTEX*)v76;
                }
            }
            if(!c2->edge()->geometry()) {
                double param_start = v66->equation().param(c1->end_pos());
                SPAposition pos_end = v66->equation().eval_position(param_start);
                if((vert_end->geometry()->coords() - pos_end).len_sq() <= SPAresfit * SPAresfit) {
                    if((vert_end->geometry()->coords() - pos_end).len_sq() > SPAresabs * SPAresabs && !is_TVERTEX(vert_start)) {
                        TVERTEX* tvert_end = nullptr;
                        replace_vertex_with_tvertex(vert_end, tvert_end);
                        vert_end = tvert_end;
                    }
                } else {
                    APOINT* v83 = ACIS_NEW APOINT(pos_end);
                    VERTEX* v85 = ACIS_NEW VERTEX(v83);
                    vert_end = (TVERTEX*)v85;
                }
            }
        }
    }
    EDGE* circle_edge = ACIS_NEW EDGE(vert_start, vert_end, v66, 0);
    COEDGE* v62 = ACIS_NEW COEDGE(circle_edge, 0, c1, c2);
    option_header* anno = find_option("annotations");
    if(anno->on() && anno->on())  // 不懂
    {
        WIRE_OFFSET_ANNO* v87 = ACIS_NEW WIRE_OFFSET_ANNO(vert, circle_edge);
        __hookup__(v87);
    }
}

// void reduce_to_principal_param_range(SPAparameter& t, SPAinterval& range, double period, double tol) {
//     typedef void (*reduce_to_principal_param_range)(SPAparameter& t, SPAinterval& range, double period, double tol);
//     reduce_to_principal_param_range f = (reduce_to_principal_param_range)apiFinderACIS.GetAddress("?reduce_to_principal_param_range@@YAXAEAVSPAparameter@@AEBVSPAinterval@@NN@Z", prostate);
//     if(f) {
//         return f(t, range, period, tol);
//     } else {
//         return;
//     }
// }
int validate_intersection_as_extension(SPAposition& intersect_pos, COEDGE* c, curve* c_geom, int fixed_start) {
    int answer = 1;
    if(c_geom->periodic()) {
        if(GET_ALGORITHMIC_VERSION() >= AcisVersion(21, 0, 2)) {
            SPAinterval i = c->param_range();
            double period = c_geom->param_period();
            SPAparameter t = c_geom->param(intersect_pos);  // 不确
            int v14 = c->edge()->sense();
            if(c->sense(v14)) t = -t;
            if(GET_ALGORITHMIC_VERSION() >= AcisVersion(29, 0, 0)) {
                SPAinterval curveRange = c_geom->param_range();
                if(!(curveRange >> i)) {
                    SPAinterval range(i.start_pt(), i.start_pt() + period);
                    reduce_to_principal_param_range(t, range, period, SPAresnor);
                }
                SPAposition startPos = c_geom->eval_position(i.start_pt());
                SPAposition endPos = c_geom->eval_position(i.end_pt());
                SPAposition intersect_point = c_geom->eval_position(t.operator double());  // 不确
                double distFromStart = (startPos - intersect_point).len();
                double distFromEnd = (endPos - intersect_point).len();
                if(fixed_start) {
                    int v42 = i >> t || fabs(t - i.start_pt()) > fabs(i.end_pt() - t) || distFromStart > distFromEnd;
                    return v42;
                } else {
                    int v43 = i >> t || fabs(i.end_pt() - t) > fabs(t - i.start_pt()) || distFromEnd > distFromStart;
                    return v43;
                }
            } else {
                SPAinterval v93(i.start_pt(), i.start_pt() + period);
                reduce_to_principal_param_range(t, v93, period, SPAresnor);
                if(fixed_start) {
                    int v44 = i >> t || fabs(t - i.start_pt() - period) > fabs(i.end_pt() - t);
                    return v44;
                } else {
                    int v45 = i >> t || fabs(i.end_pt() - t) > fabs(t - i.start_pt() - period);
                    return v45;
                }
            }
        }
    } else {
        double d2 = c_geom->param(c->end()->geometry()->coords());
        double d1 = c_geom->param(c->start()->geometry()->coords());
        SPAinterval c_interval_old(d1, d2);
        double intersect_param = c_geom->param(intersect_pos);
        int v9 = c->edge()->sense();
        if(c->sense(v9)) {
            c_interval_old.negate();
            intersect_param = -intersect_param;
        }
        if(fixed_start) {
            return intersect_param > (c_interval_old.start_pt() + SPAresnor);
        } else {
            return (c_interval_old.end_pt() - SPAresnor) > intersect_param;
        }
    }
    return answer;
}
int extend_coedges_to_intersection(curve_curve_int* iIntersections, COEDGE* c1, COEDGE* c2, curve* c1_geom, curve* c2_geom, curve* master_crv, SPAposition& p) {
    int extDone = 0;
    if(iIntersections && c1 && c2) {
        curve_curve_int* nearest_int = nullptr;
        curve_curve_int* now_ints = iIntersections;
        double min_dist = INFINITY;
        SPAposition c2SPos = c2->start_pos();
        SPAposition c2EPos = c2->end_pos();
        SPAposition c1EPos = c1->end_pos();
        while(now_ints) {
            double dist = (now_ints->int_point - p).len();
            if(min_dist > dist) {
                int changeInt = 1;
                BOOL v53 = GET_ALGORITHMIC_VERSION() < AcisVersion(21, 0, 0) || min_dist != INFINITY;
                if(v53 && now_ints != iIntersections && (min_dist - dist) / dist < 0.01) {
                    double v95 = (now_ints->int_point - c2SPos).len_sq();
                    double v15 = (now_ints->int_point - c2EPos).len_sq();
                    if(v95 > v15) changeInt = 0;
                    if(changeInt && nearest_int) {
                        double v96 = (now_ints->int_point - c1EPos).len_sq();
                        double v18 = (now_ints->int_point - nearest_int->int_point).len_sq();
                        changeInt = v18 > v96;
                    }
                }
                if(changeInt) {
                    int valid_intersection = 1;
                    if(GET_ALGORITHMIC_VERSION() >= AcisVersion(21, 0, 0)) {
                        valid_intersection = validate_intersection_as_extension(now_ints->int_point, c1, c1_geom, 1);
                        if(valid_intersection) {
                            if(c1 != c2) valid_intersection = validate_intersection_as_extension(now_ints->int_point, c2, c2_geom, 0);
                        }
                    }
                    if(valid_intersection) {
                        min_dist = dist;
                        nearest_int = now_ints;
                    }
                }
            }
            now_ints = now_ints->next;
        }
        if(nearest_int) {
            APOINT* v74 = ACIS_NEW APOINT(nearest_int->int_point);
            int postR31 = GET_ALGORITHMIC_VERSION() >= AcisVersion(31, 0, 1);
            if(c1 == c2) {
                VERTEX* vert1 = c1->end();
                vert1->set_geometry(v74, 1);
                if(postR31) {
                    if(c1->sense()) {
                        VERTEX* v77 = c1->edge()->end();
                        c1->edge()->set_end(vert1, 1, 1);
                        v77->lose();
                    } else {
                        VERTEX* vtx = c1->edge()->start();
                        c1->edge()->set_start(vert1, 1, 1);
                        vtx->lose();
                    }
                } else {
                    c1->edge()->start()->lose();
                    c1->edge()->set_start(vert1, 1, 1);
                }
                SPAinterval new_interval(nearest_int->param1, nearest_int->param2);
                SPAinterval checkInterval = extend_curve(*master_crv, new_interval, 1, SpaAcis::NullObj::get_extension_info(), 1);
                if(checkInterval != new_interval) extend_offset_int_cur(master_crv, new_interval);
                CURVE* curve = make_curve(*master_crv);
                c1->edge()->set_geometry(curve, 1);
            } else {
                c1->end()->set_geometry(v74, 1);
                c1->edge()->set_sense(c1->edge()->sense(), 1);
                if(postR31 && (c2->start() == c2->end())) {
                    c2->edge()->start()->lose();
                    c2->edge()->set_start(c1->end(), 1, 1);
                    c2->edge()->set_end(c1->end(), 1, 1);
                } else if(c2->sense()) {
                    c2->edge()->set_end(c1->end(), 1, 1);
                    c2->edge()->end()->lose();
                } else {
                    c2->edge()->set_start(c1->end(), 1, 1);
                    c2->edge()->start()->lose();
                }
                double d2 = c1_geom->param(c1->end()->geometry()->coords());
                double d1 = c1_geom->param(c1->start()->geometry()->coords());
                SPAinterval new_range(d1, d2);
                SPAinterval v150 = extend_curve(*c1_geom, new_range, 1, SpaAcis::NullObj::get_extension_info(), 1);
                double v127 = c2_geom->param(c2->end()->geometry()->coords());
                double v128 = c2_geom->param(c2->start()->geometry()->coords());
                new_range = SPAinterval(v128, v127);
                SPAinterval v152 = extend_curve(*c2_geom, new_range, 1, SpaAcis::NullObj::get_extension_info(), 1);
                c1_geom->deep_calc_disc_info();
                c2_geom->deep_calc_disc_info();
                c1->edge()->set_geometry(make_curve(*c1_geom), 1);
                c2->edge()->set_geometry(make_curve(*c2_geom), 1);
                c2->edge()->set_bound(nullptr);
                if(GET_ALGORITHMIC_VERSION() >= AcisVersion(34, 0, 0)) {
                    c1->edge()->set_bound(nullptr);
                }
            }
            return 1;
        }
    }
    return extDone;
}

BODY* gme_sg_offset_planar_wire(WIRE* wire, const TRANSFORM* wire_transf, double offset_dist, const SPAunit_vector& wire_normal, sg_gap_type close_type, int add_attribs, int trim, int zero_length, int overlap) {
    acis_exception error_info_holder(0, (error_info_base*)0);
    exception_save exception_save_mark;
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    constant_law* v13 = ACIS_NEW constant_law(offset_dist);
    constant_law* v15 = ACIS_NEW constant_law(0.0);
    BODY* result = gme_sg_offset_planar_wire(wire, wire_transf, v13, v15, wire_normal, close_type, add_attribs, trim, zero_length, overlap, 0);
    if(acis_interrupted()) {
        sys_error(0, (error_info_base*)0);
    }
    v13->remove();
    v15->remove();
    return result;
}
BODY* gme_sg_offset_planar_wire(WIRE* wire, const TRANSFORM* wire_transf, law* dist_law, law* twist_law, const SPAunit_vector& wire_normal, sg_gap_type close_type, int add_attribs, int trim, int zero_length, int overlap, int iKeepMiniTopo) {
    AcisVersion vt2 = AcisVersion(20, 0, 0);
    AcisVersion vt1 = GET_ALGORITHMIC_VERSION();
    int useSmoothMgr = (vt1 >= vt2) && (!dist_law || dist_law->constant());

    option_header* no_smooth = find_option("ofst_planer_wire_no_smooth");
    if(no_smooth->on()) {
        useSmoothMgr = 0;
    }
    ofst_edge_smooth_manager edgeSmoothMgr;
    if(useSmoothMgr) {
        ENTITY_LIST edges;
        get_edges(wire, edges);
        edgeSmoothMgr.init(&edges, 0, 5);
    }
    BODY* offset_wire_body = nullptr;
    ofst_edge_smooth_manager* iEdgeSmoothMgr = nullptr;
    if(useSmoothMgr) {
        iEdgeSmoothMgr = &edgeSmoothMgr;
    }
    offset_wire_body = sg_offset_planar_wire_internal(wire, wire_transf, dist_law, twist_law, wire_normal, close_type, add_attribs, trim, zero_length, overlap, iKeepMiniTopo, iEdgeSmoothMgr);
    if(useSmoothMgr) {
        for(int numIter = 0; !offset_success(offset_wire_body) && edgeSmoothMgr.apply_more_smooth(numIter) && numIter < 10; ++numIter) {
            if(offset_wire_body) {
                delete_entity(offset_wire_body);
            }
            offset_wire_body = sg_offset_planar_wire_internal(wire, wire_transf, dist_law, twist_law, wire_normal, close_type, add_attribs, trim, zero_length, overlap, iKeepMiniTopo, &edgeSmoothMgr);
        }
    }
    return offset_wire_body;
}
int offset_success(BODY*& iOffsetBody) {
    int retOk = 0;
    if(iOffsetBody) {
        ENTITY_LIST wireList;
        get_wires(iOffsetBody, wireList);
        retOk = wireList.count() > 0;
    }
    return retOk;
}
void display_wire_offset_io(WIRE* in_wire, law* dist_law, BODY* out_wire) {
    char brkpt_str[60];
    if(out_wire)
        sprintf(brkpt_str, "%s", "wire offset - output");
    else
        sprintf(brkpt_str, "%s", "wire offset - inputs");
}
int is_open_wire(WIRE* w) {
    int bAnswer = 1;
    COEDGE* first_coedge = start_of_wire_chain(w);  // 找到WIRE链的开始coedge
    COEDGE* last_coedge = first_coedge;
    for(COEDGE* next_coedge = first_coedge->next(); next_coedge && next_coedge != last_coedge && next_coedge != first_coedge; next_coedge = next_coedge->next()) {
        last_coedge = next_coedge;
    }
    SPAposition start_pos(first_coedge->start()->geometry()->coords());
    SPAposition end_pos(last_coedge->end()->geometry()->coords());
    if(start_pos == end_pos) {
        return 0;
    }
    return bAnswer;
}
law* edge_dist_law_to_coedge_dist_law(COEDGE* c, EDGE* e, law* dist_law) {
    law* local_law;
    if(c->sense() == e->sense()) {
        local_law = dist_law;
        dist_law->add();
    } else if(dist_law->constant()) {
        double v3 = dist_law->eval(1.0);
        constant_law* v9 = ACIS_NEW constant_law(v3 * -1.0);
        if(!v9) return nullptr;
        return (law*)v9;
    } else {
        negate_law* v11 = ACIS_NEW negate_law(dist_law);
        if(!v11) return nullptr;
        return (law*)v11;
    }
    return local_law;
}
int curve_point_offset_curvature_problem(curve& cu, double param, double dist, int rev, const SPAunit_vector& norm) {
    SPAvector ka = cu.eval_curvature(param);
    SPAvector tang(normalise(cu.eval_deriv(param)));
    if(rev) tang = -tang;
    double v13;
    if(dist <= 0.0) {
        v13 = -1.0;
    } else {
        v13 = 1.0;
    }
    double max_offset_inv = (v13 * (tang * norm)) % ka;
    int ret_val = 0;
    if(max_offset_inv > SPAresabs) {
        return fabs(dist) > 1.0 / max_offset_inv;
    }
    return ret_val;
}
int coedge_endpoint_localized_offset_curvature_problem(COEDGE* ce, int at_start, double dist, const SPAunit_vector& norm) {
    if(!ce->edge()) return 0;
    if(!ce->edge()->geometry()) return 0;
    const curve& cu = ce->edge()->geometry()->equation();
    SPAparameter v15;
    if(at_start)
        v15 = ce->start_param();
    else
        v15 = ce->end_param();
    double par = v15.operator double();
    logical rev = ce->sense() != ce->edge()->sense();
    if(rev) {
        SPAparameter v16;
        if(at_start) {
            v16 = -ce->end_param();
        } else {
            v16 = -ce->start_param();
        }
        par = v16.operator double();
    }
    int ret_val = curve_point_offset_curvature_problem((curve&)cu, par, dist, rev, norm);
    if(ret_val) {
        double delta = (ce->param_range().length()) * 0.05;
        double para;
        if(at_start)
            para = par + delta;
        else
            para = par - delta;
        return curve_point_offset_curvature_problem((curve&)cu, para, dist, rev, norm) == 0;
    }
    return ret_val;
}
int make_tvertex(COEDGE* c, int start) {
    if(GET_ALGORITHMIC_VERSION() < AcisVersion(8, 0, 9)) {
        return 0;
    }

    if(GET_ALGORITHMIC_VERSION() >= AcisVersion(10, 0, 0)) {
        if(GET_ALGORITHMIC_VERSION() <= AcisVersion(10, 0, 2)) {
            return 0;
        }
    }
    int c_sense = c->sense();
    EDGE* e = c->edge();
    int e_sense = e->sense();
    int check_e_start = 1;
    if(start && e_sense == 1 || !start && !e_sense) {
        check_e_start = 0;
    }
    if(check_e_start && (is_TVERTEX(e->start()))) {
        if(SPAresabs > e->start()->get_tolerance()) return 0;
        if(!c_sense && c->previous() && c->previous() != c || c_sense == 1 && c->next() && c->next() != c) {
            return 1;
        }
    } else if(!check_e_start) {
        if(is_TVERTEX(e->end())) {
            if(SPAresabs > e->end()->get_tolerance()) return 0;
            if(!c_sense && c->next() && c->next() != c || c_sense == 1 && c->previous() && c->previous() != c) {
                return 1;
            }
        }
    }
    return 0;
}
int all_offset_degenerated(ENTITY_LIST& iCoedges) {
    int allDegenerated = 1;
    int numCoedges = iCoedges.count();
    for(int i = 0; i < numCoedges && allDegenerated; ++i) {
        COEDGE* thisCoedge = (COEDGE*)iCoedges[i];
        if(thisCoedge) {
            if(thisCoedge->edge()) {
                allDegenerated = thisCoedge->edge()->geometry() == nullptr;
            }
        }
    }
    return allDegenerated;
}

// curve* offset_geometry(curve const* geometry, SPAunit_vector const& in_normal, law* dist_law, law* twist_law, SPAinterval const& in_off_domain)
//{
//     double v5;                                 // xmm0_8
//     AcisVersion* v6;                           // rax
//     curve_law_data* v7;                        // rax
//     law* v8;                                   // rax
//     double v9;                                 // xmm0_8
//     double v10;                                // xmm0_8
//     double v11;                                // xmm0_8
//     const SPAvector* v12;                      // rax
//     const SPAvector* v13;                      // rax
//     __int64 v14;                               // rax
//     long double v15;                           // xmm0_8
//     __int64 v16;                               // rax
//     __int64 v17;                               // rax
//     long double v18;                           // xmm0_8
//     SPAvector* v19;                            // rax
//     long double v20;                           // xmm0_8
//     __int64 v21;                               // rax
//     intcurve* offset_curve;                    // [rsp+40h] [rbp-2E8h]
//     int i;                                     // [rsp+48h] [rbp-2E0h]
//     int j;                                     // [rsp+4Ch] [rbp-2DCh]
//     CURVE* simp_curv;                          // [rsp+50h] [rbp-2D8h]
//     int v27;                                   // [rsp+58h] [rbp-2D0h]
//     int num_ctrlpts;                           // [rsp+5Ch] [rbp-2CCh] BYREF
//     int is_degenerate;                         // [rsp+60h] [rbp-2C8h]
//     const ellipse* ell_geom;                   // [rsp+68h] [rbp-2C0h]
//     SPAposition* ctrlpts;                      // [rsp+70h] [rbp-2B8h] BYREF
//     law* geom_law;                             // [rsp+78h] [rbp-2B0h]
//     const straight* straight_geom;             // [rsp+80h] [rbp-2A8h]
//     long double off_radius;                    // [rsp+88h] [rbp-2A0h] BYREF
//     intcurve* v35;                             // [rsp+90h] [rbp-298h]
//     degenerate_curve* v36;                     // [rsp+98h] [rbp-290h]
//     intcurve* v37;                             // [rsp+A0h] [rbp-288h]
//     curve_law_data* v38;                       // [rsp+A8h] [rbp-280h]
//     curve_law_data* v39;                       // [rsp+B0h] [rbp-278h]
//     curve_law* v40;                            // [rsp+B8h] [rbp-270h]
//     law* v41;                                  // [rsp+C0h] [rbp-268h]
//     curve_law_data* cld;                       // [rsp+C8h] [rbp-260h]
//     straight* v43;                             // [rsp+D0h] [rbp-258h]
//     __int64 v44;                               // [rsp+D8h] [rbp-250h]
//     ellipse* v45;                              // [rsp+E0h] [rbp-248h]
//     intcurve* v46;                             // [rsp+E8h] [rbp-240h]
//     degenerate_curve* v47;                     // [rsp+F0h] [rbp-238h]
//     intcurve* v48;                             // [rsp+F8h] [rbp-230h]
//     SPAinterval off_domain;                    // [rsp+100h] [rbp-228h] BYREF
//     AcisVersion result;                        // [rsp+118h] [rbp-210h] BYREF
//     AcisVersion* vt2;                          // [rsp+120h] [rbp-208h]
//     AcisVersion* vt1;                          // [rsp+128h] [rbp-200h]
//     long double in_end;                        // [rsp+130h] [rbp-1F8h]
//     long double in_start;                      // [rsp+138h] [rbp-1F0h]
//     curve_law_data* v55;                       // [rsp+140h] [rbp-1E8h]
//     law* v56;                                  // [rsp+148h] [rbp-1E0h]
//     long double* guess;                        // [rsp+150h] [rbp-1D8h]
//     int* side;                                 // [rsp+158h] [rbp-1D0h]
//     double v59;                                // [rsp+160h] [rbp-1C8h]
//     long double* v60;                          // [rsp+168h] [rbp-1C0h]
//     int* v61;                                  // [rsp+170h] [rbp-1B8h]
//     long double offset_dist;                   // [rsp+178h] [rbp-1B0h]
//     SPAposition* root;                         // [rsp+180h] [rbp-1A8h]
//     __int64 v64;                               // [rsp+188h] [rbp-1A0h]
//     long double tol;                           // [rsp+190h] [rbp-198h]
//     long double fit_data;                      // [rsp+198h] [rbp-190h]
//     long double maj_len;                       // [rsp+1A0h] [rbp-188h]
//     intcurve* v68;                             // [rsp+1A8h] [rbp-180h]
//     intcurve* v69;                             // [rsp+1B0h] [rbp-178h]
//     long double v70;                           // [rsp+1B8h] [rbp-170h]
//     long double v71;                           // [rsp+1C0h] [rbp-168h]
//     long double v72;                           // [rsp+1C8h] [rbp-160h]
//     long double v73;                           // [rsp+1D0h] [rbp-158h]
//     bs3_curve_def* bs;                         // [rsp+1D8h] [rbp-150h]
//     double v75;                                // [rsp+1E0h] [rbp-148h]
//     long double resabs_sq;                     // [rsp+1E8h] [rbp-140h]
//     void(__fastcall * v77)(struct intcurve*);  // [rsp+1F0h] [rbp-138h]
//     __int64 v78;                               // [rsp+1F8h] [rbp-130h]
//     intcurve* v79;                             // [rsp+200h] [rbp-128h]
//     void* alloc_ptr;                           // [rsp+208h] [rbp-120h]
//     AcisVersion v81;                           // [rsp+210h] [rbp-118h] BYREF
//     SPAunit_vector normal;                     // [rsp+218h] [rbp-110h] BYREF
//     SPAunit_vector str_dir;                    // [rsp+230h] [rbp-F8h] BYREF
//     SPAunit_vector off_dir;                    // [rsp+248h] [rbp-E0h] BYREF
//     SPAposition str_root;                      // [rsp+260h] [rbp-C8h] BYREF
//     SPAvector off_major;                       // [rsp+278h] [rbp-B0h] BYREF
//     SPAposition this_pos;                      // [rsp+290h] [rbp-98h] BYREF
//     SPAvector v88;                             // [rsp+2A8h] [rbp-80h] BYREF
//     SPAvector v89;                             // [rsp+2C0h] [rbp-68h] BYREF
//     SPAposition v90;                           // [rsp+2D8h] [rbp-50h] BYREF
//     SPAvector v91;                             // [rsp+2F0h] [rbp-38h] BYREF
//
//     offset_curve = 0i64;
//     simp_curv = 0i64;
//     SPAunit_vector::SPAunit_vector(&normal, in_normal);
//     qmemcpy(&off_domain, in_off_domain, sizeof(off_domain));
//     if(geometry) {
//         if(geometry->type(geometry) == 11) {
//             if(law::constant(dist_law)) {
//                 v5 = safe_function_type<double>::operator double(&SPAresabs);
//                 if((unsigned int)law::zero(twist_law, v5)) {
//                     AcisVersion::AcisVersion(&v81, 12, 0, 0);
//                     vt2 = v6;
//                     vt1 = GET_ALGORITHMIC_VERSION(&result);
//                     if(operator>=(vt1, vt2)) {
//                         v38 = (curve_law_data*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 4416, &alloc_file_index_3517);
//                         if(v38) {
//                             in_end = SPAinterval::end_pt(&off_domain);
//                             in_start = SPAinterval::start_pt(&off_domain);
//                             curve_law_data::curve_law_data(v38, geometry, in_start, in_end);
//                             v39 = v7;
//                         } else {
//                             v39 = 0i64;
//                         }
//                         v55 = v39;
//                         cld = v39;
//                         v40 = (curve_law*)ACIS_OBJECT::operator new(0x50ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 4419, &alloc_file_index_3517);
//                         if(v40) {
//                             curve_law::curve_law(v40, cld);
//                             v41 = v8;
//                         } else {
//                             v41 = 0i64;
//                         }
//                         v56 = v41;
//                         geom_law = v41;
//                         law_data::remove(cld);
//                         simp_curv = test_for_line(v41, &off_domain);
//                         if(!simp_curv) simp_curv = test_for_circle(geom_law, &off_domain);
//                         if(simp_curv) geometry = (straight*)simp_curv->equation(simp_curv);
//                         law::remove(geom_law);
//                     }
//                 }
//             }
//         }
//     }
//     if(law::constant(dist_law) && (guess = SpaAcis::NullObj::get_double(), side = SpaAcis::NullObj::get_int(), v59 = safe_function_type<double>::operator double(&SPAresabs), v9 = law::eval(dist_law, 1.0, side, guess), v10 = fabs_0(v9), v59 > v10)) {
//         offset_curve = (intcurve*)geometry->make_copy(geometry);
//     } else {
//         v27 = geometry->type(geometry);
//         switch(v27) {
//             case 1:
//                 if(law::constant(dist_law) && (v11 = safe_function_type<double>::operator double(&SPAresabs), (unsigned int)law::zero(twist_law, v11))) {
//                     v60 = SpaAcis::NullObj::get_double();
//                     v61 = SpaAcis::NullObj::get_int();
//                     offset_dist = law::eval(dist_law, 1.0, v61, v60);
//                     straight_geom = geometry;
//                     SPAposition::SPAposition(&str_root, &geometry->root_point);
//                     SPAunit_vector::SPAunit_vector(&str_dir, &geometry->direction);
//                     v12 = operator*(&v88, &str_dir, &normal);
//                     normalise(&off_dir, v12);
//                     v43 = (straight*)ACIS_OBJECT::operator new(0x58ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 4454, &alloc_file_index_3517);
//                     if(v43) {
//                         v13 = operator*(&v89, offset_dist, &off_dir);
//                         root = operator+(&v90, &str_root, v13);
//                         straight::straight(v43, root, &str_dir, 1.0);
//                         v44 = v14;
//                     } else {
//                         v44 = 0i64;
//                     }
//                     v64 = v44;
//                     offset_curve = (intcurve*)v44;
//                     *(long double*)(v44 + 56) = straight_geom->param_scale;
//                 } else {
//                     tol = safe_function_type<double>::operator double(&SPAresabs);
//                     fit_data = safe_function_type<double>::operator double(&SPAresfit);
//                     offset_curve = (intcurve*)sg_offset_planar_curve(geometry, &off_domain, fit_data, dist_law, twist_law, &normal, 0, tol);
//                 }
//                 break;
//             case 2:
//                 ell_geom = (const ellipse*)geometry;
//                 off_radius = 0.0;
//                 if((unsigned int)get_circle_offset_radius(geometry, in_normal, dist_law, twist_law, &off_radius)) {
//                     maj_len = SPAvector::len(&ell_geom->major_axis);
//                     v15 = safe_function_type<double>::operator double(&SPAresabs);
//                     if(off_radius <= v15) {
//                         v47 = (degenerate_curve*)ACIS_OBJECT::operator new(0x38ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 4484, &alloc_file_index_3517);
//                         if(v47) {
//                             degenerate_curve::degenerate_curve(v47, &ell_geom->centre);
//                             v48 = (intcurve*)v17;
//                         } else {
//                             v48 = 0i64;
//                         }
//                         v69 = v48;
//                         offset_curve = v48;
//                     } else {
//                         operator*(&off_major, off_radius / maj_len, &ell_geom->major_axis);
//                         v45 = (ellipse*)ACIS_OBJECT::operator new(0xB0ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 4477, &alloc_file_index_3517);
//                         if(v45) {
//                             ellipse::ellipse(v45, &ell_geom->centre, &ell_geom->normal, &off_major, 1.0, 0.0);
//                             v46 = (intcurve*)v16;
//                         } else {
//                             v46 = 0i64;
//                         }
//                         v68 = v46;
//                         offset_curve = v46;
//                     }
//                 } else {
//                     v70 = safe_function_type<double>::operator double(&SPAresabs);
//                     v71 = 10.0 * safe_function_type<double>::operator double(&SPAresfit);
//                     offset_curve = (intcurve*)sg_offset_planar_curve(geometry, &off_domain, v71, dist_law, twist_law, &normal, 0, v70);
//                 }
//                 break;
//             case 11:
//                 v72 = safe_function_type<double>::operator double(&SPAresabs);
//                 v73 = safe_function_type<double>::operator double(&SPAresfit);
//                 offset_curve = (intcurve*)sg_offset_planar_curve(geometry, &off_domain, v73, dist_law, twist_law, &normal, 0, v72);
//                 if(offset_curve->type(offset_curve) == 11) {
//                     is_degenerate = 1;
//                     bs = intcurve::cur(offset_curve, -1.0, 0);
//                     num_ctrlpts = 0;
//                     ctrlpts = 0i64;
//                     bs3_curve_control_points(bs, &num_ctrlpts, &ctrlpts, 0);
//                     v75 = safe_function_type<double>::operator double(&SPAresabs);
//                     v18 = safe_function_type<double>::operator double(&SPAresabs);
//                     resabs_sq = v75 * v18;
//                     for(i = 0; i < num_ctrlpts && is_degenerate; ++i) {
//                         SPAposition::SPAposition(&this_pos, &ctrlpts[i]);
//                         for(j = num_ctrlpts - 1; j > i; --j) {
//                             v19 = operator-(&v91, &this_pos, &ctrlpts[j]);
//                             v20 = SPAvector::len_sq(v19);
//                             if(v20 >= resabs_sq) {
//                                 is_degenerate = 0;
//                                 break;
//                             }
//                         }
//                     }
//                     if(is_degenerate) {
//                         v35 = offset_curve;
//                         if(offset_curve) {
//                             v77 = v35->~intcurve;
//                             v78 = ((__int64(__fastcall*)(intcurve*, __int64))v77)(v35, 1i64);
//                         } else {
//                             v78 = 0i64;
//                         }
//                         v36 = (degenerate_curve*)ACIS_OBJECT::operator new(0x38ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 4528, &alloc_file_index_3517);
//                         if(v36) {
//                             degenerate_curve::degenerate_curve(v36, ctrlpts);
//                             v37 = (intcurve*)v21;
//                         } else {
//                             v37 = 0i64;
//                         }
//                         v79 = v37;
//                         offset_curve = v37;
//                     }
//                     alloc_ptr = ctrlpts;
//                     SPAposition::operator delete[](ctrlpts);
//                 }
//                 break;
//         }
//     }
//     if(simp_curv) simp_curv->lose(simp_curv);
//     return offset_curve;
//
//
// }

COEDGE* sg_offset_pl_coedge(COEDGE* c, law* dist_law, law* twist_law, const SPAunit_vector& n) {
    EDGE* e = c->edge();
    SPAinterval curve_range = e->param_range();
    int e_sense = e->sense();
    if(e_sense == 1) {
        curve_range = -curve_range;
    }  // 不理解
    law* local_law = edge_dist_law_to_coedge_dist_law(c, e, dist_law);
    COEDGE* coedge_offset = nullptr;
    int resignal_no = 0;
    acis_exception error_info_holder(0);
    error_info_base* error_info_base_ptr = nullptr;
    exception_save exception_save_mark;
    curve* curve_offset = nullptr;  // 修改ida
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    int error_no = 0;

    if(woffset_module_header.debug_level >= 30) {
        acis_fprintf(debug_file_ptr, " *** The curve to be offset ***\n");
        e->geometry()->equation().debug("\t");
        acis_fprintf(debug_file_ptr, "\n");
    }

    if(GET_ALGORITHMIC_VERSION() >= AcisVersion(21, 0, 2)) {
        if(dist_law->constant()) {
            if(twist_law->zero(SPAresabs)) {
                if(CUR_is_exact_intcurve(e->geometry()->equation())) {
                    int start_prob = coedge_endpoint_localized_offset_curvature_problem(c, 1, dist_law->eval(1.0), n);
                    int end_prob = coedge_endpoint_localized_offset_curvature_problem(c, 0, dist_law->eval(1.0), n);
                    if(start_prob || end_prob) {
                        const intcurve& ic = (const intcurve&)e->geometry()->equation();
                        SPAinterval new_range = e->param_range();
                        if(((intcurve&)ic).reversed()) {
                            new_range.negate();
                        }
                        if(e->sense() == 1) new_range.negate();
                        intcurve* temp_curve = nullptr;
                        int err_num = 0;
                        acis_exception v300(0);
                        error_info_base* e_info = nullptr;
                        exception_save v290;
                        v290.begin();
                        get_error_mark().buffer_init = 1;
                        bs3_curve_def* old_bs = ((intcurve&)ic).cur(-1.0, 0);
                        bs3_curve_def* bs3 = bs3_curve_subset(old_bs, new_range);
                        if(((intcurve&)ic).reversed()) {
                            bs3_curve_reverse(bs3);
                            new_range.negate();
                        }
                        SPAinterval cr = ((struct intcurve&)ic).param_range();  // 不确定，没用到
                        if(start_prob) {
                            SPAvector end_ka = bs3_curve_curvature(new_range.end_pt(), bs3);
                            SPAvector start_crv(0.0, 0.0, 0.0);
                            bs3_curve new_bs3 = bs3_curve_snap_end_curvatures(bs3, start_crv, end_ka, 0.0);
                            bs3_curve_compat(bs3, new_bs3);
                            if(bs3_cpt_dist(bs3, new_bs3) > SPAresfit) {
                                bs3_curve_delete(new_bs3);
                                new_bs3 = nullptr;
                            }
                            bs3_curve_delete(bs3);
                            bs3 = new_bs3;
                            new_bs3 = nullptr;
                        }
                        if(end_prob) {
                            SPAvector start_ka = bs3_curve_curvature(new_range.start_pt(), bs3);
                            SPAvector end_crv(0.0, 0.0, 0.0);
                            bs3_curve_def* b2 = bs3_curve_snap_end_curvatures(bs3, start_ka, end_crv, 0.0);
                            bs3_curve_compat(bs3, b2);
                            if(bs3_cpt_dist(bs3, b2) > SPAresfit) {
                                bs3_curve_delete(b2);
                                b2 = nullptr;
                            }
                            bs3_curve_delete(bs3);
                            bs3 = b2;
                            b2 = nullptr;
                        }
                        SPAinterval safe_range;
                        if(bs3) {
                            intcurve* temp_curve = ACIS_NEW intcurve(bs3, 0.0, SpaAcis::NullObj::get_surface(), SpaAcis::NullObj::get_surface(), (bs2_curve)0, (bs2_curve)0, safe_range, 0, 0);
                            bs3 = nullptr;
                            curve_offset = (intcurve*)offset_geometry(temp_curve, n, local_law, twist_law, curve_range);
                        }
                        if(temp_curve) {
                            temp_curve = nullptr;
                        }
                        if(bs3) {
                            bs3_curve_delete(bs3);
                            bs3 = nullptr;
                        }
                        if(err_num) err_num = 0;
                        if(err_num || acis_interrupted()) sys_error(err_num, e_info);
                    }
                }
            }
        }
    }
    if(!curve_offset) {
        // 传进来curve，平面法向量，coedge的dist_law，twist_law,一个coedge的edge()的参数范围，返回一个curve
        curve_offset = offset_geometry(&e->geometry()->equation(), n, local_law, twist_law, curve_range);  // 看不懂
    }
    local_law->remove();
    local_law = nullptr;

    if(curve_offset->type() == 2) {
        if(e->geometry()->equation().type() == 11) {
            SPAposition orig_start_pos;
            SPAvector orig_start_dir;
            e->geometry()->equation().eval(curve_range.start_pt(), orig_start_pos, orig_start_dir);
            double start_t = curve_offset->param(orig_start_pos);
            SPAvector off_dir(curve_offset->eval_direction(start_t));
            if((off_dir % orig_start_dir) < 0.0) {
                curve_offset->negate();
                start_t = curve_offset->param(orig_start_pos);
            }
            SPAposition orig_end_pos = e->geometry()->equation().eval_position(curve_range.end_pt());
            double end_t = curve_offset->param(orig_end_pos);
            if(start_t > end_t) end_t = end_t + 6.283185307179586;
            curve_range = SPAinterval(start_t, end_t);
        }
    }

    if(woffset_module_header.debug_level >= 30) {
        acis_fprintf(debug_file_ptr, " *** The untrimmed offset curve ***\n");
        if(curve_offset) {
            curve_offset->debug("\t", debug_file_ptr);
        } else {
            acis_fprintf(debug_file_ptr, " ****  NULL  **** ");
        }
        acis_fprintf(debug_file_ptr, "\n");
    }
    TVERTEX* start_offset = nullptr;
    VERTEX* end_offset = nullptr;
    if(curve_offset) {
        SPAposition start_pos = curve_offset->eval_position(curve_range.start_pt());
        if(make_tvertex(c, 1)) {
            APOINT* v145 = ACIS_NEW APOINT(start_pos);
            TVERTEX* v147 = ACIS_NEW TVERTEX(v145, -1.0);
            start_offset = v147;
            v147->mark_to_update();
        } else {
            APOINT* v149 = ACIS_NEW APOINT(start_pos);
            VERTEX* v150 = ACIS_NEW VERTEX(v149);
            TVERTEX* v151 = (TVERTEX*)v150;
            start_offset = v151;
        }
        if(!e_sense || GET_ALGORITHMIC_VERSION() < AcisVersion(8, 0, 9) || (GET_ALGORITHMIC_VERSION() >= AcisVersion(10, 0, 0)) && GET_ALGORITHMIC_VERSION() <= AcisVersion(10, 0, 2)) {
            copy_attrib(e->start(), start_offset);
        } else {
            copy_attrib(e->end(), start_offset);
        }
        SPAposition end_pos = curve_offset->eval_position(curve_range.end_pt());
        if((start_pos - end_pos).len() <= SPAresabs) {
            end_offset = start_offset;
        } else {
            if(make_tvertex(c, 0)) {
                APOINT* v153 = ACIS_NEW APOINT(end_pos);
                TVERTEX* v155 = ACIS_NEW TVERTEX(v153, -1.0);
                end_offset = v155;
                v155->mark_to_update();
            } else {
                APOINT* v157 = ACIS_NEW APOINT(end_pos);
                VERTEX* v159 = ACIS_NEW VERTEX(v157);
                end_offset = v159;
            }
            if(!e_sense || (GET_ALGORITHMIC_VERSION() < AcisVersion(8, 0, 9)) || (GET_ALGORITHMIC_VERSION() >= AcisVersion(10, 0, 0)) && (GET_ALGORITHMIC_VERSION() <= AcisVersion(10, 0, 2))) {
                copy_attrib(e->end(), end_offset);
            } else {
                copy_attrib(e->start(), end_offset);
            }
        }
        if(curve_offset->type() == 11) {
            if((end_pos - start_pos).len() > SPAresabs) {
                curve* extra = curve_offset->split(curve_range.start_pt(), start_pos);
                if(extra) {
                    extra = nullptr;
                }
                extra = curve_offset;
                curve_offset = (intcurve*)curve_offset->split(curve_range.end_pt(), end_pos);
                if(extra) {
                    extra = nullptr;
                }
                if(e->closed()) {
                    if(!curve_offset->closed()) {
                        if(GET_ALGORITHMIC_VERSION() >= AcisVersion(14, 0, 1)) {
                            int v75 = 0;
                            acis_exception v301(0);
                            error_info_base* v111 = nullptr;
                            exception_save v291;
                            bs3_curve_def* bs3Copy = nullptr;
                            bs3_curve_def* bs3StartSeg = nullptr;
                            bs3_curve_def* bs3EndSeg = nullptr;
                            curve* pExtraStartCur = nullptr;
                            curve* pExtraEndCur = nullptr;
                            intcurve* pMidCurve = nullptr;
                            curve_curve_int* pCCI = nullptr;
                            v291.begin();
                            get_error_mark().buffer_init = 1;
                            intcurve* intCurOffset = (intcurve*)curve_offset;  // 修改ida
                            double fReduce = curve_range.length() * 0.005;
                            double d2 = curve_range.start_pt() + fReduce;
                            double d1 = curve_range.start_pt();
                            SPAinterval startSegRange(d1, d2);
                            SPAinterval endSegRange(curve_range.end_pt() - fReduce, curve_range.end_pt());
                            bs3Copy = bs3_curve_copy(((intcurve*)intCurOffset)->cur(-1.0, 0));  // 修改ida
                            bs3StartSeg = bs3_curve_subset(bs3Copy, startSegRange);
                            bs3EndSeg = bs3_curve_subset(bs3Copy, endSegRange);
                            if(bs3StartSeg) {
                                if(bs3EndSeg) {
                                    pCCI = bs3_curve_bs3_curve_int(bs3StartSeg, bs3EndSeg, SPAresabs);
                                    if(pCCI) {
                                        double fStartParam = pCCI->param1;
                                        double fEndParam = pCCI->param2;
                                        pExtraStartCur = intCurOffset->split(fStartParam);
                                        pExtraEndCur = intCurOffset;
                                        intCurOffset = (intcurve*)pExtraEndCur->split(fEndParam);  // 不确
                                        curve_range = intCurOffset->param_range();

                                        double fMiddleParam = curve_range.mid_pt();
                                        pMidCurve = intCurOffset;
                                        intCurOffset = (intcurve*)pMidCurve->split(fMiddleParam);
                                        intCurOffset->join(*pMidCurve, -1);
                                        curve_range = intCurOffset->param_range();
                                        curve_offset = intCurOffset;
                                        intCurOffset->deep_calc_disc_info();
                                        SPAposition posStart = intCurOffset->eval_position(curve_range.start_pt());
                                        int bTolerant = is_TVERTEX(start_offset);
                                        start_offset->lose();
                                        end_offset->lose();

                                        if(bTolerant) {
                                            APOINT* v121 = ACIS_NEW APOINT(start_pos);
                                            TVERTEX* v123 = ACIS_NEW TVERTEX(v121, -1.0);
                                            start_offset = v123;
                                            v123->mark_to_update();
                                        } else {
                                            APOINT* v162 = ACIS_NEW APOINT(posStart);
                                            VERTEX* v118 = ACIS_NEW VERTEX(v162);
                                            TVERTEX* v119 = (TVERTEX*)v118;
                                            start_offset = v119;
                                        }
                                        end_offset = start_offset;
                                        if(!e_sense || (GET_ALGORITHMIC_VERSION() < AcisVersion(8, 0, 9)) || (GET_ALGORITHMIC_VERSION() >= AcisVersion(10, 0, 0)) && (GET_ALGORITHMIC_VERSION() <= AcisVersion(10, 0, 2))) {
                                            copy_attrib(e->start(), start_offset);
                                        } else {
                                            copy_attrib(e->end(), start_offset);
                                        }
                                    }
                                }
                            }
                            if(pMidCurve) {
                                pMidCurve = nullptr;
                            }
                            if(pExtraStartCur) {
                                pExtraStartCur = nullptr;
                            }
                            if(pExtraEndCur) {
                                pExtraEndCur = nullptr;
                            }
                            if(pCCI) {
                                curve_curve_int* pTmp;
                                for(curve_curve_int* pThisCci = pCCI; pThisCci; pThisCci = pTmp) {
                                    pTmp = pThisCci->next;
                                    ACIS_DELETE pThisCci;  // 不确
                                }
                                pCCI = nullptr;
                            }
                            if(bs3StartSeg) {
                                bs3_curve_delete(bs3StartSeg);
                                bs3StartSeg = nullptr;
                            }
                            if(bs3EndSeg) {
                                bs3_curve_delete(bs3EndSeg);
                                bs3EndSeg = nullptr;
                            }
                            if(bs3Copy) {
                                bs3_curve_delete(bs3Copy);
                                bs3Copy = nullptr;
                            }
                            if(v75 || acis_interrupted()) sys_error(v75, v111);
                        }
                    }
                }
            }
        }
        if(curve_offset->type() == 3) {
            curve_offset = nullptr;
        }
    }
    if(e_sense == 1) {
        VERTEX* tmp_v = start_offset;
        start_offset = (TVERTEX*)end_offset;
        end_offset = tmp_v;
    }
    CURVE* curve_entity_offset = nullptr;
    if(curve_offset) curve_entity_offset = make_curve(*curve_offset);
    EDGE* edge_offset = nullptr;
    if(curve_offset && curve_offset->type() == 11) {
        SPAinterval ofstRange = curve_offset->param_range();
        if(e_sense == 1) ofstRange = -ofstRange;
        EDGE* v126 = ACIS_NEW EDGE(start_offset, end_offset, curve_entity_offset, e_sense, EDGE_cvty_unknown, ofstRange);
        edge_offset = v126;
    } else {
        EDGE* v128 = ACIS_NEW EDGE(start_offset, end_offset, curve_entity_offset, e_sense, EDGE_cvty_unknown);
        edge_offset = v128;
    }
    copy_attrib(e, edge_offset);

    int coedge_sense = c->sense();
    COEDGE* v129 = ACIS_NEW COEDGE(edge_offset, coedge_sense, nullptr, nullptr);
    coedge_offset = v129;
    copy_attrib(c, v129);
    coedge_offset->set_next(coedge_offset, 0, 1);
    coedge_offset->set_previous(coedge_offset, 0, 1);
    if(curve_offset) {
        curve_offset = nullptr;
    }
    if(local_law) local_law->remove();
    if(resignal_no || acis_interrupted()) sys_error(resignal_no, error_info_base_ptr);
    return coedge_offset;
}

void make_same_vertex_pointers(VERTEX* vert1, VERTEX* vert2, COEDGE* this_coedge, COEDGE* next_coedge, bool reset_this_edge_range) {
    EDGE* this_edge = this_coedge->edge();
    EDGE* next_edge = next_coedge->edge();
    if(next_edge->start() == next_edge->end()) {
        if(this_edge->start() == this_edge->end()) {
            this_edge->set_start(vert2, 1, 1);
            this_edge->set_end(vert2, 1, 1);
            COEDGE* check_prev = this_coedge->previous();
            while(check_prev) {
                int num_fixed = 0;
                EDGE* tmp_ed = check_prev->edge();
                if(tmp_ed && tmp_ed->start() == vert1) {
                    num_fixed = 1;
                    tmp_ed->set_start(vert2, 1, 1);
                }
                if(tmp_ed && tmp_ed->end() == vert1) {
                    ++num_fixed;
                    tmp_ed->set_end(vert2, 1, 1);
                }
                if(num_fixed == 2)
                    check_prev = check_prev->previous();
                else
                    check_prev = nullptr;
            }
        } else if(this_coedge->sense()) {
            this_edge->set_start(vert2, 1, 1);
        } else {
            this_edge->set_end(vert2, 1, 1);
        }
        vert1->lose();
        SPAposition p1 = this_coedge->end_pos();
        SPAposition p2 = next_coedge->start_pos();
        if(p1 != p2) {
            SPAposition average_pos = interpolate(0.5, p1, p2);
            SPAvector v5 = p1 - p2;
            double diff = 0.5 * (v5.len());
            double tol = 1.001 * diff;
            TVERTEX* tvert = nullptr;
            replace_vertex_with_tvertex(vert2, tvert);
            if(tvert) {
                APOINT* v17 = ACIS_NEW APOINT(average_pos);
                tvert->set_geometry(v17, 1);
                tvert->set_tolerance(tol, 0);
                this_coedge->edge()->set_bound(nullptr);
                this_coedge->edge()->set_param_range(nullptr);
                next_coedge->edge()->set_bound(nullptr);
                next_coedge->edge()->set_param_range(nullptr);
            }
        }
    } else {
        if(next_coedge->sense())
            next_edge->set_end(vert1, 1, 1);
        else
            next_edge->set_start(vert1, 1, 1);
        vert2->lose();
        if(reset_this_edge_range) {
            this_coedge->edge()->set_bound(nullptr);
            this_coedge->edge()->set_param_range(nullptr);
        }
        next_coedge->edge()->set_bound(nullptr);
        next_coedge->edge()->set_param_range(nullptr);
        SPAposition v48 = this_coedge->end_pos();
        SPAposition v47 = next_coedge->start_pos();
        if(v48 != v47) {
            SPAposition here = interpolate(0.5, v48, v47);
            double v49 = 1.001 * (0.5 * ((v48 - v47).len()));
            TVERTEX* this_tvertex = nullptr;
            replace_vertex_with_tvertex(vert1, this_tvertex);
            if(this_tvertex) {
                APOINT* v19 = ACIS_NEW APOINT(here);
                this_tvertex->set_geometry(v19, 1);

                this_coedge->edge()->set_bound(nullptr);

                this_coedge->edge()->set_param_range(nullptr);

                next_coedge->edge()->set_bound(nullptr);

                next_coedge->edge()->set_param_range(nullptr);
            }
        }
    }
}
// 输入coedge和prev_coedge,看prev_coedge末端点与coedge的前端点是不是一个点，一般来说是同一个点，之后再判断coedge和prev_coedge所在平面与输入向量的方向是否同向，如果同向，则返回2，表示偏置后会存在间隙
int check_gap_type(COEDGE* this_orig_coedge, COEDGE* prev_orig_coedge, const SPAvector& plane_nv, double offset_dist) {
    int gap_status = 0;
    SPAposition posOrig;
    if(this_orig_coedge->sense() == 1) {
        posOrig = this_orig_coedge->edge()->end_pos();
    } else {
        posOrig = this_orig_coedge->edge()->start_pos();
    }
    SPAposition posPrev;
    if(prev_orig_coedge->sense()) {
        posPrev = prev_orig_coedge->edge()->start_pos();
    } else {
        posPrev = prev_orig_coedge->edge()->end_pos();
    }
    if(same_point(posOrig, posPrev, SPAresabs)) {
        SPAvector this_v;
        SPAvector prev_v;
        SPAposition prev_pos;
        SPAposition this_pos;
        const curve& a_curve = prev_orig_coedge->edge()->geometry()->equation();

        if(prev_orig_coedge->sense() == prev_orig_coedge->edge()->sense()) {
            a_curve.eval(prev_orig_coedge->edge()->end_param().operator double(), prev_pos, prev_v);
        } else {
            a_curve.eval(prev_orig_coedge->edge()->start_param().operator double(), prev_pos, prev_v);
            prev_v = -prev_v;
        }
        const curve& a_curvea = this_orig_coedge->edge()->geometry()->equation();

        if(this_orig_coedge->sense() == this_orig_coedge->edge()->sense()) {
            a_curvea.eval(this_orig_coedge->edge()->start_param().operator double(), this_pos, this_v);
        } else {
            a_curvea.eval(this_orig_coedge->edge()->end_param().operator double(), this_pos, this_v);
            this_v = -this_v;
        }
        if(offset_dist * ((prev_v * this_v) % plane_nv) >= 0.0)
            return 2;
        else
            return 1;
    }
    return gap_status;
}

int get_circle_offset_radius(curve* geometry, const SPAunit_vector& normal, law* dist_law, law* twist_law, double& off_radius) {
    int answer = 0;
    if(geometry->type() == 2 && *(double*)&geometry[3].subset_range.type_data == 1.0) {
        if(dist_law->constant()) {
            if(twist_law->zero(double(SPAresabs))) {
                double offset_dist = dist_law->eval(1.0);
                double maj_len = ((SPAvector*)&geometry[1].subset_range.type_data)->len();              // 不懂
                double dotProduct = *((const SPAvector*)&geometry[2].subset_range.type_data) % normal;  // 不确定
                if(GET_ALGORITHMIC_VERSION() < AcisVersion(31, 0, 0) || (fabs(dotProduct) > 1.0 - 10.0 * double(SPAresmch))) {
                    if(dotProduct <= 0.0)
                        off_radius = maj_len - offset_dist;
                    else
                        off_radius = maj_len + offset_dist;
                    return 1;
                }
            }
        }
    }
    return answer;
}
int is_circle_offset_by_radius(curve* geometry, const SPAunit_vector& normal, law* dist_law, law* twist_law) {
    int answer = 0;
    double offset_radius[2];
    offset_radius[0] = 0.0;
    if(get_circle_offset_radius(geometry, normal, dist_law, twist_law, *offset_radius)) {
        if(SPAresabs >= offset_radius[0]) {
            if(offset_radius[0] >= -SPAresabs) return 1;
        }
        return 0;
    }
    return answer;
}
// 计算0.5 * (tan1 + tan2)，返回给oTangent
int comp_tangent_at_coedge_commom_vertex(COEDGE* iCoedge1, COEDGE* iCoedge2, COEDGE* iOfstCoedge1, COEDGE* iOfstCoedge2, SPAunit_vector& oTangent, SPAunit_vector& oStartOfstDir, SPAunit_vector& oEndfstDir) {
    if(!iCoedge1 || !iCoedge2) return 0;
    SPAunit_vector tan1 = coedge_end_dir(iCoedge1);
    SPAunit_vector tan2 = coedge_start_dir(iCoedge2);
    oStartOfstDir = normalise(tan1);
    oEndfstDir = normalise(tan2);
    int retOK;
    if(antiparallel(tan1, tan2, SPAresnor)) {
        SPAposition p2 = coedge_start_pos(iOfstCoedge2);
        SPAposition p1 = coedge_end_pos(iOfstCoedge1);
        oTangent = normalise(p1 - p2);
        retOK = oTangent.is_zero() == 0;
    } else {
        oTangent = normalise(0.5 * (tan1 + tan2));
        retOK = 1;
    }
    /*if(Debug_Break_Active("showing offset directions", "WIRE-OFFSET"))
    {
        if(get_breakpoint_callback()) {
            breakpoint_callback = get_breakpoint_callback();
            v19 = breakpoint_callback->new_render_object(breakpoint_callback, 1i64);
        } else {
            v19 = 0i64;
        }
        if(v19) {
            show_entity_with_text(iCoedge1, "coedge1", WHITE, v19, 1);
            show_entity_with_text(iCoedge2, "coedge2", BLUE, v19, 1);
            v19->set_line_style(v19, SPA_DASHED_LINE);
            iCoedge1->end_pos(iCoedge1, &pos);
            show_vector_with_text(&pos, oStartOfstDir, "start offset dir", WHITE, iCoedge1, v19);
            show_vector_with_text(&pos, oEndfstDir, "end offset dir", BLUE, iCoedge1, v19);
            show_vector_with_text(&pos, oTangent, "offset dir", CYAN, iCoedge1, v19);
            Debug_Break("showing offset directions", "WIRE-OFFSET", "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 4639);
            if(get_breakpoint_callback()) {
                v20 = get_breakpoint_callback();
                v20->delete_render_object(v20, v19);
            }
        }
    }*/
    return retOK;
}
// 判断偏置coedge末端点方向与原始coedge末端点方向是否反向
int ofst_coedge_reversed(COEDGE* iOfstCoedge, COEDGE* iorigCoedge, int iAtStart) {
    int reversed = 0;
    if(iOfstCoedge && iorigCoedge) {
        SPAvector ofstTan(0.0, 0.0, 0.0);
        SPAvector origTan(0.0, 0.0, 0.0);
        if(iAtStart) {
            ofstTan = coedge_start_dir(iOfstCoedge);
            origTan = coedge_start_dir(iorigCoedge);
        } else {
            ofstTan = coedge_end_dir(iOfstCoedge);
            origTan = coedge_end_dir(iorigCoedge);
        }
        if(normalise(ofstTan) % normalise(origTan) < -0.707) return 1;
    }
    return reversed;
}
int get_coedge_data(COEDGE* iCoedge, VERTEX*& oVertex, SPAposition& oVrtPos, curve*& oCurGeom, int iNeedStartData) {
    int retOk = iCoedge && !oVertex && !oCurGeom;
    if(retOk) {
        VERTEX* v12;
        if(iNeedStartData)
            v12 = iCoedge->start();
        else
            v12 = iCoedge->end();
        oVertex = v12;
        if(oVertex && oVertex->geometry()) {
            oVrtPos = oVertex->geometry()->coords();
        } else {
            retOk = 0;
        }
    }
    if(retOk && iCoedge->edge() && (iCoedge->edge()->geometry())) {
        oCurGeom = iCoedge->edge()->geometry()->trans_curve();
    } else {
        return 0;
    }
    return retOk;
}

int trim_coedge(COEDGE*& ioCoedge, VERTEX*& ioSplitVertex, SPAposition& iSplitPos, SPAparameter iParam, curve* ioCurve, int iKeepFirstPart) {
    SPAparameter iParama = iParam.operator double();
    int retOk = 0;
    if(ioCoedge) {
        if(ioCoedge->edge()) {
            if(ioSplitVertex) {
                if(ioCurve) {
                    SPAinterval oldRange = ioCurve->param_range();
                    double started = oldRange.start_pt();
                    if(iParama > started) {
                        if(iParama < oldRange.end_pt()) {
                            SPAinterval newRange;
                            if(iKeepFirstPart) {
                                newRange = SPAinterval(oldRange.start_pt(), iParama.operator double());
                            } else {
                                newRange = SPAinterval(iParama.operator double(), oldRange.end_pt());
                            }
                            ioCurve->limit(newRange);
                            APOINT* v15 = ACIS_NEW APOINT(iSplitPos);
                            ioSplitVertex->set_geometry(v15, 1);
                            ioCoedge->edge()->set_sense(ioCoedge->edge()->sense());
                            ioCoedge->edge()->set_geometry(make_curve(*ioCurve), 1);
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return retOk;
}

int process_overlapping(COEDGE*& ioCoedge1, COEDGE*& ioCoedge2) {
    if(ioCoedge1 == ioCoedge2) return 0;
    acis_exception error_info_holder(0);
    error_info_base* error_info_base_ptr = nullptr;
    exception_save exception_save_mark;
    VERTEX* vrt1 = nullptr;
    VERTEX* vrt2 = nullptr;
    curve* cur1 = nullptr;
    curve* cur2 = nullptr;
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    SPAposition p1;
    SPAposition p2;
    SPAvector t1;
    SPAvector t2;
    BOOL v26 = get_coedge_data(ioCoedge1, vrt1, p1, cur1, 0) && get_coedge_data(ioCoedge2, vrt2, p2, cur2, 1);
    int retOk = v26;
    if(retOk) {
        SPAvector gapVec = p2 - p1;
        COEDGE* coedge = ioCoedge1;
        if(coedge_start_dir(coedge) % gapVec < 0.0) {
            if(coedge_start_dir(ioCoedge2) % gapVec < 0.0) {
                double gapLen2 = gapVec.len_sq();
                if((ioCoedge1->start_pos() - p1).len_sq() > gapLen2) {
                    if((ioCoedge2->end_pos() - p2).len_sq() > gapLen2) {
                        SPAposition newPos1;
                        SPAposition newPos2;
                        SPAparameter newParam1;
                        SPAparameter newParam2;
                        SPAparameter param_guess = SpaAcis::NullObj::get_parameter();
                        cur1->point_perp(p2, newPos1, param_guess, newParam1, 0);  // 第一条线垂足与参数
                        SPAparameter parameter = SpaAcis::NullObj::get_parameter();
                        cur2->point_perp(p1, newPos2, parameter, newParam2, 0);  // 第二条线垂足与参数
                        double overDst2 = 0.05 * SPAresfit;
                        if(overDst2 > (p2 - newPos1).len()) {
                            if(overDst2 > (p1 - newPos2).len()) {
                                if((p1 - p2).len() > overDst2) {
                                    retOk = 0;
                                    int v27 = ioCoedge1->sense() == ioCoedge1->edge()->sense();
                                    if(trim_coedge(ioCoedge1, vrt1, newPos1, newParam1, cur1, v27))  // 输入：第一条偏置coedge，第一个偏置边的末端点，点到第一条线的垂足，垂足参数，第一个边的几何，1。
                                    {
                                        int v28 = ioCoedge2->sense() != ioCoedge2->edge()->sense();
                                        retOk = trim_coedge(ioCoedge2, vrt2, newPos2, newParam2, cur2, v28);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if(cur1) {
        cur1 = nullptr;
    }
    if(cur2) {
        cur2 = nullptr;
    }
    if(acis_interrupted()) sys_error(0, error_info_base_ptr);

    return retOk;
}

int check_gap_intersection(double iGapDist, double iOfstDist, EDGE* iEdge1, EDGE* iEdge2) {
    double checkTol = 0.01 * iOfstDist;
    int needCheck = 0.01 * iOfstDist > iGapDist;
    if(0.01 * iOfstDist <= iGapDist && iEdge1 && iEdge2) {
        if(5.0 * checkTol <= iGapDist) {
            if(!iEdge2->geometry()) {
                SPAposition checkPoint = iEdge2->start_pos();
                SPAposition oClosePointOnEdge;
                SPAtransf v17;
                find_cls_pt_on_edge(checkPoint, iEdge1, oClosePointOnEdge, v17);
                return (checkTol > (oClosePointOnEdge - checkPoint).len());
            }
        } else if(iEdge1->geometry() && iEdge2->geometry()) {
            const curve& v12 = iEdge1->geometry()->equation();
            int v9 = 0;

            if(v12.type() == 2) {
                const curve& v4 = iEdge2->geometry()->equation();
                if(v4.type() == 2) return 1;
            }
            return v9;
        }
    }
    return needCheck;
}
void display_null_offset_coedge_to_be_deleted(COEDGE* this_off_coedge, COEDGE* this_orig_coedge) {
    return;
}
int sg_close_with_corner(offset_segment* this_seg, law* dist_law, SPAposition& p, SPAposition& p1, SPAposition& p2, COEDGE* c1, COEDGE* c2, VERTEX* vert, int iKeepMiniTopo) {
    SPAunit_vector v1 = normalise(p1 - p);
    SPAunit_vector v2 = normalise(p2 - p);
    SPAvector v1xv2 = v1 * v2;
    if(SPAresabs >= v1xv2.len()) {
        sg_close_with_arc(this_seg, dist_law, p, p1, p2, c1, c2, vert);
        return 1;
    }
    const SPAvector& v12 = v1xv2 * v1;
    SPAunit_vector d1 = normalise(v12);
    SPAunit_vector tangent = coedge_end_dir(this_seg->original_coedge());
    if(d1 % tangent < 0.0) {
        sg_close_with_arc(this_seg, dist_law, p, p1, p2, c1, c2, vert);
        return 1;
    }
    const SPAvector& v14 = v1xv2 * v2;
    SPAunit_vector d2 = normalise(v14);
    double v15 = d1 % d2;
    double sina = acis_sqrt((v15 + 1.0) / 2.0);
    double t1 = 0.5 * (p1 - p2).len() / sina;
    const SPAvector& v17 = t1 * d1;
    SPAposition int_pt = p1 + v17;
    APOINT* v48 = ACIS_NEW APOINT(int_pt);
    VERTEX* vert1 = c1->end();
    VERTEX* vert2 = c2->start();
    CURVE* c1_geom = c1->edge()->geometry();
    option_header* anno = find_option("annotations");
    if(c1_geom && c1_geom->identity(2) == STRAIGHT_TYPE && (auto_merge.count() > 0 || iKeepMiniTopo)) {
        vert1->set_geometry(v48);
        c1->edge()->set_sense(c1->edge()->sense(), 1);
        c1->edge()->set_bound(nullptr);
        c1->edge()->set_param_range(nullptr);
    } else {
        if(iKeepMiniTopo) return 0;
        VERTEX* v51 = ACIS_NEW VERTEX(v48);
        STRAIGHT* v53 = ACIS_NEW STRAIGHT(p1, d1);
        EDGE* v55 = ACIS_NEW EDGE(vert1, v51, v53, 0, EDGE_cvty_unknown);
        COEDGE* v57 = ACIS_NEW COEDGE(v55, 0, c1, c2);
        c1 = v57;
        vert1 = v57->end();

        if(anno->on()) {
            ENTITY* old_edge = nullptr;
            int subsubtype = ATTRIB_ANNOTATION_TYPE;
            int subtype = ATTRIB_SYS_TYPE;
            EDGE* owner = v57->previous()->edge();
            for(ATTRIB_ANNOTATION* this_att = (ATTRIB_ANNOTATION*)find_attrib(owner, subtype, subsubtype, -1, -1); this_att; this_att = (ATTRIB_ANNOTATION*)find_next_attrib(this_att, ATTRIB_SYS_TYPE, ATTRIB_ANNOTATION_TYPE, -1, -1)) {
                if(is_COPY_ANNOTATION(this_att->annotation())) {
                    COPY_ANNOTATION* anno1 = (COPY_ANNOTATION*)this_att->annotation();
                    old_edge = get_actual_live_entity(anno1->source());
                    break;
                }
            }
            if(anno->on()) {
                WIRE_OFFSET_ANNO* v59 = ACIS_NEW WIRE_OFFSET_ANNO(vert, v55, old_edge);  // 不确定 __hookup__
                __hookup__(v59);
            }
        }
    }
    CURVE* c2_geom = c2->edge()->geometry();

    if(c2_geom && c2_geom->identity(2) == STRAIGHT_TYPE && (auto_merge.count() > 0 || iKeepMiniTopo)) {
        if(c2->sense()) {
            c2->edge()->set_end(vert1, 1, 1);
        } else {
            c2->edge()->set_start(vert1, 1, 1);
        }
        vert2->lose();
    } else {
        if(iKeepMiniTopo) return 0;
        STRAIGHT* v61 = ACIS_NEW STRAIGHT(p2, d2);
        EDGE* v63 = ACIS_NEW EDGE(vert1, vert2, v61, 0, EDGE_cvty_unknown);
        COEDGE* v65 = ACIS_NEW COEDGE(v63, 0, c1, c2);
        if(anno->on()) {
            ENTITY* second_original_ent = nullptr;
            int v42 = ATTRIB_ANNOTATION_TYPE;
            int v43 = ATTRIB_SYS_TYPE;
            for(ATTRIB_ANNOTATION* i = (ATTRIB_ANNOTATION*)find_attrib(c2->edge(), v43, v42, -1, -1); i; i = (ATTRIB_ANNOTATION*)find_next_attrib(i, ATTRIB_SYS_TYPE, ATTRIB_ANNOTATION_TYPE, -1, -1)) {
                if(is_COPY_ANNOTATION(i->annotation())) {
                    COPY_ANNOTATION* v83 = (COPY_ANNOTATION*)i->annotation();
                    second_original_ent = get_actual_live_entity(v83->source());
                    break;
                }
            }
            if(anno->on()) {
                WIRE_OFFSET_ANNO* v67 = ACIS_NEW WIRE_OFFSET_ANNO(vert, v63, second_original_ent);  // 不理解有什么用
                __hookup__(v67);
            }
        }
    }
    return 1;
}

void display_gap_after_close(COEDGE* c1, COEDGE* c2, char* type) {
    // EDGE* v3;                                  // rax
    // EDGE* v4;                                  // rax
    // EDGE* v5;                                  // rax
    //_iobuf* v6;                                // rax
    //_iobuf* v7;                                // rax
    // breakpoint_callback* breakpoint_callback;  // [rsp+38h] [rbp-60h]
    // RenderingObject* v9;                       // [rsp+40h] [rbp-58h]
    // COEDGE* gap_coedge;                        // [rsp+48h] [rbp-50h]
    // breakpoint_callback* v11;                  // [rsp+50h] [rbp-48h]
    // char gaptype[30];                          // [rsp+68h] [rbp-30h] BYREF

    // if(c1 && c2 && Debug_Break_Active("wire after gap closure", "WIRE-OFFSET"))
    //{
    //     if(get_breakpoint_callback()) {
    //         breakpoint_callback = get_breakpoint_callback();
    //         v9 = breakpoint_callback->new_render_object(breakpoint_callback, 1i64);
    //     } else {
    //         v9 = 0i64;
    //     }
    //     if(v9) {
    //         v3 = COEDGE::edge(c1);
    //         show_entity(v3, ORANGE, v9);
    //         v4 = COEDGE::edge(c2);
    //         show_entity(v4, YELLOW, v9);
    //         gap_coedge = COEDGE::next(c1);
    //         if(gap_coedge == c2) {
    //             v7 = __acrt_iob_func(2u);
    //             acis_fprintf(v7, "no gap\n");
    //         } else {
    //             sprintf(gaptype, " %s gap closed", type);
    //             v5 = COEDGE::edge(gap_coedge);
    //             show_entity_with_text(v5, gaptype, MAGENTA, v9, 1);
    //             v6 = __acrt_iob_func(2u);
    //             acis_fprintf(v6, "after closing %s gap\n", type);
    //         }
    //         Debug_Break("wire after gap closure", "WIRE-OFFSET", "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 782);
    //         if(get_breakpoint_callback()) {
    //             v11 = get_breakpoint_callback();
    //             v11->delete_render_object(v11, v9);
    //         }
    //     }
    // }
    return;
}

void comp_vdist(SPAposition& p, SPAposition& p1, SPAposition& p2, SPAvector& oVDist1, SPAvector& oVDist2) {
    SPAvector v_dist = p2 - p1;
    SPAvector l1 = p - p1;
    SPAvector l2 = p - p2;
    SPAunit_vector ul1 = normalise(l1);
    SPAunit_vector ul2 = normalise(l2);
    SPAunit_vector v1 = normalise(v_dist);
    SPAunit_vector v2 = -v1;
    double costheta1 = ul1 % v1;
    double sintheta1 = acis_sqrt(1.0 - costheta1 * costheta1);
    double costheta2 = ul2 % v2;
    double sintheta2 = acis_sqrt(1.0 - costheta2 * costheta2);
    if(sintheta1 < 0.5 && sintheta1 > double(SPAresabs) && GET_ALGORITHMIC_VERSION() > AcisVersion(17, 0, 0)) {
        oVDist1 = v_dist * 1.0 / sintheta1;
    } else {
        oVDist1 = v_dist;
    }
    if(sintheta2 < 0.5 && sintheta2 > double(SPAresabs) && GET_ALGORITHMIC_VERSION() > AcisVersion(17, 0, 0)) {
        oVDist2 = (-v_dist) * (1.0 / sintheta2);
    } else {
        oVDist2 = -v_dist;
    }
}
int sg_close_with_natural2(offset_segment* this_seg, law* dist_law, SPAposition& p, SPAposition& p1, SPAposition& p2, COEDGE* c1, COEDGE* c2, VERTEX* vert) {
    if(c1->edge()->geometry() && c2->edge()->geometry()) {
        curve* master_crv = nullptr;
        EDGE* v11 = c1->edge();
        curve* c1_geom;
        curve* c2_geom;
        if(c1 == c2) {
            master_crv = v11->geometry()->trans_curve();
            double param = c1->edge()->param_range().mid_pt();
            c1_geom = master_crv->copy_curve();
            c2_geom = c1_geom->split(param);
        } else {
            c1_geom = v11->geometry()->trans_curve();
            c2_geom = c2->edge()->geometry()->trans_curve();
        }
        SPAvector v_dist1;
        SPAvector v_dist2;
        comp_vdist(p, p1, p2, v_dist1, v_dist2);
        int f_success = sg_extend_coedges(dist_law, p1, c1, c1_geom, v_dist1);
        if(f_success) f_success = sg_extend_coedges(dist_law, p2, c2, c2_geom, v_dist2);
        if(f_success) {
            SPAbox box_cur;
            if(c1_geom->param_range().infinite() && c2_geom->param_range().infinite()) {
                SPAinterval tmp_int(-10000.0, 10000.0);
                SPAbox v103(tmp_int, tmp_int, tmp_int);
                box_cur = v103;
            } else {
                SPAbox b2 = c2_geom->bound(c2_geom->param_range());
                SPAbox b1 = c1_geom->bound(c1_geom->param_range());
                box_cur = b1 | b2;
            }
            curve_curve_int* their_ints = int_cur_cur(*c1_geom, *c2_geom, box_cur, double(SPAresabs) * 0.1);

            if(their_ints) {
                extend_coedges_to_intersection(their_ints, c1, c2, c1_geom, c2_geom, master_crv, p);
                while(their_ints) {
                    curve_curve_int* next_int = their_ints->next;
                    their_ints = next_int;
                }
            } else {
                sg_close_with_arc(this_seg, dist_law, p, p1, p2, c1, c2, vert);
            }
            return 1i64;
        } else {
            int close_with_arc = 1;
            if(GET_ALGORITHMIC_VERSION() < AcisVersion(11, 0, 0) || (p1 - p2).len() > double(SPAresfit)) {
                sg_close_with_arc(this_seg, dist_law, p, p1, p2, c1, c2, vert);
            } else {
                close_with_arc = 0;
            }

            return close_with_arc;
        }
    } else {
        sg_close_with_arc(this_seg, dist_law, p, p1, p2, c1, c2, vert);
        return 1;
    }
}
int sg_close_offset_gap(offset_segment_list& seg_list, offset_segment*& this_seg, COEDGE* this_coedge, VERTEX* common_vertex, SPAunit_vector& iOffsetDir, SPAunit_vector& iStartOfstDir, SPAunit_vector& iEndOfstDir, law* dist_law, sg_gap_type gap_type,
                        int iKeepMiniTopo, int skipbackup) {
    VERTEX* common_vertexa = common_vertex;
    COEDGE* this_coedgea = this_coedge;
    COEDGE* next_coedge = this_coedge->next();
    VERTEX* vert1 = this_coedgea->end();
    VERTEX* vert2 = next_coedge->start();

    SPAposition p1(vert1->geometry()->coords());
    SPAposition p2(vert2->geometry()->coords());
    SPAposition common_pt(common_vertexa->geometry()->coords());
    SPAvector gap_vec = p2 - p1;
    double gap_size = gap_vec.len();
    sg_gap_type oldGapType = gap_type;
    BOOL v61 = gap_size > SPAresabs;
    int close_gap = v61;  // 是第一个coedge的末端点与第二个coedge的首端点是否距离大于SPAresabs，大于SPAresabs则为1
    if(close_gap && SPAresfit > gap_size) {
        if(2.0 * SPAresabs > gap_size) {  // 不理解
            COEDGE* origCoedge = this_seg->original_coedge();
            if(this_coedgea) {
                if(origCoedge) {
                    if(ofst_coedge_reversed(this_coedgea, origCoedge, 0)) {
                        close_gap = 0;
                    } else {
                        if(ofst_coedge_reversed(this_coedgea->next(), origCoedge->next(), 1)) close_gap = 0;
                    }
                }
            }
        }
        if(close_gap) gap_type = natural;
    }
    if(this_coedgea == next_coedge) {
        double parm1 = this_coedgea->edge()->end_param().operator double();
        double parm2 = this_coedgea->edge()->start_param().operator double();
        if(fabs(dist_law->eval(parm1) - dist_law->eval(parm2)) > SPAresabs) close_gap = 0;
    } else if(oldGapType != gap_type && close_gap) {
        process_overlapping(this_coedgea, next_coedge);
        vert1 = this_coedgea->end();
        vert2 = next_coedge->start();
        p1 = vert1->geometry()->coords();
        p2 = vert2->geometry()->coords();
    }
    if(close_gap && (gap_type == arc || oldGapType == arc)) {
        if(GET_ALGORITHMIC_VERSION() > AcisVersion(10, 0, 3)) {
            double endpram = this_coedgea->edge()->param_range().end_pt();
            double dist = dist_law->eval(endpram);
            if(dist < 0.0) {
                if(GET_ALGORITHMIC_VERSION() >= AcisVersion(10, 0, 7)) {
                    if(GET_ALGORITHMIC_VERSION() != AcisVersion(11, 0, 0)) dist = -dist;
                }
            }
            if(gap_type == oldGapType || dist < 0.02) {
                EDGE* iEdge2 = next_coedge->edge();
                EDGE* iEdge1 = this_coedgea->edge();
                if(check_gap_intersection(gap_size, dist, iEdge1, iEdge2)) {
                    if(!this_coedgea->edge()) return 0;
                    if(!this_coedgea->edge()->geometry()) return 0;
                    SPAtransf ctrans;
                    COEDGE* coedge = this_coedgea;
                    SPAunit_vector coedge_dir = coedge_end_dir(this_coedgea, ctrans, 1);
                    SPAunit_vector gap_dir = normalise(gap_vec);
                    if(coedge_dir % gap_dir < 0.0) {  //(p2-p1)与p1处方向
                        if(!next_coedge->edge()) return 0;
                        if(!next_coedge->edge()->geometry()) return 0;
                        SPAtransf v175;
                        SPAunit_vector next_coedge_dir = coedge_start_dir(next_coedge, v175, 1);
                        if(coedge_dir % next_coedge_dir > 0.9984) {
                            int extra_ccis = 0;
                            int resignal_no = 0;
                            acis_exception error_info_holder(0);
                            error_info_base* error_info_base_ptr = nullptr;
                            exception_save exception_save_mark;
                            curve_curve_int* cci = nullptr;
                            exception_save_mark.begin();
                            get_error_mark().buffer_init = 1;
                            int error_no = 0;
                            EDGE* e2 = next_coedge->edge();
                            EDGE* e1 = this_coedgea->edge();
                            sg_inter_ed_ed(e1, e2, cci, SPAresabs, SPAresnor);
                            if(!cci || cci->next) {
                                if(cci && cci->next) extra_ccis = 1;
                            } else {
                                double coedge_param = cci->param1;
                                SPAunit_vector dir1 = coedge_param_dir(this_coedgea, coedge_param);
                                double param2 = cci->param2;
                                SPAunit_vector dir2 = coedge_param_dir(next_coedge, param2);
                                if(dir1 % dir2 > -0.99) {
                                    close_gap = 0;
                                    APOINT* v74 = ACIS_NEW APOINT(cci->int_point);
                                    APOINT* ap = v74;
                                    vert1->set_geometry(v74, 1);
                                    vert2->set_geometry(ap, 1);
                                    this_coedgea->edge()->set_bound(nullptr);
                                    this_coedgea->edge()->set_param_range(nullptr);
                                    next_coedge->edge()->set_bound(nullptr);
                                    next_coedge->edge()->set_param_range(nullptr);
                                }
                            }
                            sg_delete_cci(cci);
                            if(resignal_no || acis_interrupted()) sys_error(resignal_no, error_info_base_ptr);
                            if(close_gap) {
                                point_entity_rel* v77 = ACIS_NEW point_entity_rel(vert1->geometry(), next_coedge->edge());
                                point_entity_rel* per = v77;
                                if(v77) {
                                    if(per->rel_type == point_on_entity) {
                                        vert2->set_geometry(vert1->geometry(), 1);
                                        next_coedge->edge()->set_bound(nullptr);
                                        next_coedge->edge()->set_param_range(nullptr);
                                        close_gap = 0;
                                    }
                                    per->lose();
                                    per = nullptr;
                                }
                                if(close_gap) {
                                    point_entity_rel* v79 = ACIS_NEW point_entity_rel(vert2->geometry(), this_coedgea->edge());
                                    per = v79;
                                    if(v79) {
                                        if(per->rel_type == point_on_entity) {
                                            vert1->set_geometry(vert2->geometry(), 1);
                                            this_coedgea->edge()->set_bound(nullptr);
                                            this_coedgea->edge()->set_param_range(nullptr);
                                            close_gap = 0;
                                        }
                                        per->lose();
                                        per = nullptr;
                                    }
                                }
                            }
                            if(close_gap && !extra_ccis) gap_type = natural;
                        }
                    }
                }
            }
        }
    }
    if(close_gap) {
        if(gap_type) {
            if(gap_type == corner) {
                close_gap = sg_close_with_corner(this_seg, dist_law, common_pt, p1, p2, this_coedgea, next_coedge, common_vertexa, iKeepMiniTopo);
                if(close_gap) display_gap_after_close(this_coedgea, next_coedge, (char*)"corner");
            } 
            else {
                if(gap_type != natural) return 0;
                if(GET_ALGORITHMIC_VERSION() >= AcisVersion(17, 0, 0)) {
                    ofst_natural_extender naturalExtender;
                    close_gap = naturalExtender.extend(this_seg, dist_law, this_coedgea, next_coedge, common_vertexa, iKeepMiniTopo, skipbackup);
                    if(close_gap) {
                        display_gap_after_close(this_coedgea, next_coedge, (char*)"natural");
                    } else if(iKeepMiniTopo) {
                        sys_error(OFFSET_CURVE_TOPO_CHANGED);
                    }
                } else {
                    close_gap = sg_close_with_natural2(this_seg, dist_law, common_pt, p1, p2, this_coedgea, next_coedge, common_vertexa);
                }
            }
        } else if(iKeepMiniTopo) {
            close_gap = 0;
        } else {
            sg_close_with_arc(this_seg, dist_law, common_pt, p1, p2, this_coedgea, next_coedge, common_vertexa);  //
            display_gap_after_close(this_coedgea, next_coedge, (char*)"arc");
        }
        COEDGE* gap_coedge = this_coedgea->next();
        WIRE* owner = this_coedgea->wire();
        while(gap_coedge != next_coedge && close_gap) {
            offset_segment* new_seg = ACIS_NEW offset_segment(gap_coedge, common_vertexa);
            seg_list.insert_segment(new_seg, this_seg);
            SPAunit_vector gapCoedgeDir = coedge_mid_dir(gap_coedge);
            /*   if(Debug_Break_Active("showing offset directions", "WIRE-OFFSET")) {
               if(get_breakpoint_callback()) {
                   breakpoint_callback = get_breakpoint_callback();
                   new_render_object = breakpoint_callback->new_render_object;
                   v83 = new_render_object(breakpoint_callback, 1);
               } else {
                   v83 = 0i64;
               }
               pRO = v83;
               if(v83) {
                   v44 = COEDGE::edge(gap_coedge);
                   show_entity_with_text(v44, "closing edge", MAGENTA, pRO, 1);
                   v68 = COEDGE::edge(gap_coedge);
                   mid_pos = v68->mid_pos;
                   mid_pos(v68, &mid, 1);
                   set_line_style = pRO->set_line_style;
                   set_line_style(pRO, SPA_DASHED_LINE);
                   show_vector_with_text(&mid, iOffsetDir, "offset dir", CYAN, 0i64, pRO);
                   set_line_width = pRO->set_line_width;
                   ((void(__fastcall*)(RenderingObject*))set_line_width)(pRO);
                   show_vector_with_text(&mid, &gapCoedgeDir, "\n closing edge dir at mid point", BLUE, 0i64, pRO);
                   Debug_Break("showing offset directions", "WIRE-OFFSET", "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 3251);
                   if(get_breakpoint_callback()) {
                       v69 = get_breakpoint_callback();
                       delete_render_object = v69->delete_render_object;
                       delete_render_object(v69, pRO);
                   }
               }
            }*/
            if(new_seg && iOffsetDir % gapCoedgeDir < 0.0) {
                SPAunit_vector tan1 = coedge_start_dir(gap_coedge);
                SPAunit_vector tan2 = coedge_end_dir(gap_coedge);
                /*      if(Debug_Break_Active("showing offset directions", "WIRE-OFFSET")) {
                        if(get_breakpoint_callback()) {
                            v70 = get_breakpoint_callback();
                            v146 = v70->new_render_object;
                            v71 = v146(v70, 1);
                        } else {
                            v71 = 0i64;
                        }
                        v52 = v71;
                        if(v71) {
                            v47 = COEDGE::edge(gap_coedge);
                            show_entity_with_text(v47, "closing edge", MAGENTA, v52, 1);
                            v147 = v52->set_line_style;
                            v147(v52, SPA_DASHED_LINE);
                            start_pos = gap_coedge->start_pos;
                            start_pos(gap_coedge, &start);
                            end_pos = gap_coedge->end_pos;
                            end_pos(gap_coedge, &end);
                            show_vector_with_text(&start, &tan1, "end dir of coedge1", CYAN, 0i64, v52);
                            show_vector_with_text(&end, &tan2, "start dir of coedge2", BLUE, 0i64, v52);
                            v150 = v52->set_line_width;
                            ((void(__fastcall*)(RenderingObject*))v150)(v52);
                            show_vector_with_text(&start, iStartOfstDir, "\n start offset dir", MAGENTA, 0i64, v52);
                            show_vector_with_text(&end, iEndOfstDir, "\n end offset dir", MAGENTA, 0i64, v52);
                            Debug_Break("showing offset directions", "WIRE-OFFSET", "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 3273);
                            if(get_breakpoint_callback()) {
                                v72 = get_breakpoint_callback();
                                v151 = v72->delete_render_object;
                                v151(v72, v52);
                            }
                        }
                    }*/
                if(tan1 % iStartOfstDir < 0.0) new_seg->set_start_in();
                if(tan2 % iEndOfstDir < 0.0) new_seg->set_end_in();
            }
            gap_coedge->set_wire(owner, 1);
            this_seg = new_seg;
            gap_coedge = gap_coedge->next();
        }
    }
    if(!close_gap && !skipbackup && vert1 != vert2) {
        make_same_vertex_pointers(vert1, vert2, this_coedgea, next_coedge, 0);
    }
    return close_gap;
}

logical sel_ed(EDGE* e1, EDGE* e2) {
    COEDGE* wire_start;
    wire_start = start_of_wire_chain((WIRE*)e1->coedge()->owner());
    return e1->coedge() == wire_start || e2->coedge() != wire_start;
}

BODY* sg_offset_planar_wire_internal(WIRE* wire, const TRANSFORM* wire_transf, law* dist_law, law* twist_law, const SPAunit_vector& wire_normal, sg_gap_type close_type, int add_attribs, int trim, int zero_length, int overlap, int iKeepMiniTopo,
                                     ofst_edge_smooth_manager* iEdgeSmoothMgr) {
    if(woffset_module_header.debug_level >= 10) {
        acis_fprintf(debug_file_ptr, " Entering sg_offset_planar_wire()\n");
        acis_fprintf(debug_file_ptr, " wire = ");
        debug_pointer(wire, debug_file_ptr);
        acis_fprintf(debug_file_ptr, " )\n");
    }
    display_wire_offset_io(wire, dist_law, nullptr);  // 打印wire offset - inputs
    if(branched_wire(wire)) {
        ofst_error(OFFSET_BRANCHED_WIRE, 1, wire);
    }  // 有分支，报错

    int bOutwards = 0;  // 偏置方向是否朝外
    if(GET_ALGORITHMIC_VERSION() >= AcisVersion(14, 0, 3)) {
        SPAunit_vector wireNormal;
        SPAposition centroid;
        if(!is_planar_wire(wire, centroid, wireNormal, 1, 1)) {
            sys_error(OFFSET_INPUT_NOT_PLANAR);
        }
        if(is_open_wire(wire) == 0) {  // 是openwire，返回1，不是（wire的开始点 = 终止点），返回0
            // 如果平行，扭曲为0，偏距为常数，bOutwards = 1;
            if(parallel(wireNormal, wire_normal)) {
                if(dist_law) {
                    if(dist_law->constant()) {
                        int a = dist_law->return_dim();
                        if(dist_law->eval(1.0) > 0.0) {
                            if(twist_law) {
                                if(twist_law->zero())  // 可以探究一下
                                {
                                    bOutwards = 1;
                                }
                            }
                        }
                    }
                }
            } else {
                if(biparallel(wireNormal, wire_normal)) {
                    if(dist_law) {
                        if(dist_law->constant()) {
                            if(dist_law->eval(1.0) < 0.0) {
                                if(twist_law) {
                                    if(twist_law->zero()) bOutwards = 1;
                                }
                            }
                        }
                    }
                } else {
                    bOutwards = 0;
                }
            }
        }
    }

    offset_segment_list seg_list;
    seg_list.set_distance(dist_law);
    seg_list.set_normal(wire_normal);
    seg_list.set_add_attribs(add_attribs);
    seg_list.set_gap_type(close_type);

    COEDGE* start_coedge = start_of_wire_chain(wire);
    COEDGE* this_coedge = start_coedge;
    COEDGE* prev_coedge;

    // 处理前后两段coedge是椭圆的情况。
    if(GET_ALGORITHMIC_VERSION() >= AcisVersion(10, 0, 4)) {
        do {
            prev_coedge = this_coedge;
            this_coedge = this_coedge->next();
            if(CUR_is_intcurve(prev_coedge->edge()->geometry()->equation())) {
                ENTITY_LIST new_edges;
                sg_split_edge_at_disc(prev_coedge->edge(), new_edges, 1);
            } else {
                if(GET_ALGORITHMIC_VERSION() >= AcisVersion(16, 0, 1)) {
                    if(dist_law->constant()) {
                        if(this_coedge) {
                            if(this_coedge != prev_coedge) {
                                if(CUR_is_ellipse(prev_coedge->edge()->geometry()->equation())) {
                                    if(CUR_is_ellipse(this_coedge->edge()->geometry()->equation())) {
                                        ELLIPSE* ell1 = (ELLIPSE*)prev_coedge->edge()->geometry();
                                        ELLIPSE* ell2 = (ELLIPSE*)this_coedge->edge()->geometry();
                                        if(ell1->radius_ratio() == 1.0 && ell2->radius_ratio() == 1.0) {
                                            double radius = ((SPAvector&)ell1->major_axis()).len();
                                            double radius2 = ((SPAvector&)ell2->major_axis()).len();
                                            if(SPAresabs > fabs(radius - fabs(dist_law->eval(1.0)))) {  // 看不懂eval()
                                                if(SPAresabs > fabs(radius - radius2)) {
                                                    SPAposition p2 = ell2->centre();
                                                    SPAposition p1 = ell1->centre();  // 不确定
                                                    if(SPAresabs > (p1 - p2).len()) {
                                                        SPAvector curvature_vec1 = ell1->centre() - prev_coedge->start()->geometry()->coords();
                                                        SPAvector curvature_vec2 = ell2->centre() - this_coedge->start()->geometry()->coords();
                                                        SPAunit_vector started = coedge_start_dir(prev_coedge);
                                                        SPAvector start_tang1 = SPAvector(started);
                                                        SPAvector start_tang2 = SPAvector(coedge_start_dir(this_coedge));
                                                        SPAvector vec1 = curvature_vec1 * start_tang1;  // 半径向量（中心点-初始点）叉乘初始点切向量
                                                        SPAvector vec2 = curvature_vec2 * start_tang2;
                                                        // 保证vec1与vec2同向
                                                        if((vec1 % wire_normal > 0.0) == (vec2 % wire_normal > 0.0) && bOutwards == (vec1 % wire_normal > 0.0)) {
                                                            if(GET_ALGORITHMIC_VERSION() > AcisVersion(24, 0, 0)) {
                                                                merge_vertex(prev_coedge->end(), sel_ed);  // 不懂
                                                            } else {
                                                                api_clean_entity(prev_coedge->edge());
                                                            }
                                                            if(prev_coedge->next() != this_coedge) this_coedge = prev_coedge->next();  // 看不懂这一步是做什么
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } while(this_coedge != prev_coedge && this_coedge != start_coedge);
        this_coedge = start_coedge;
    }

    WIRE* v407 = ACIS_NEW WIRE(nullptr, nullptr);
    WIRE* offset_wire = v407;
    wire_law_data* v409 = ACIS_NEW wire_law_data(wire);
    wire_law_data* wld = v409;

    int dist_law_size;
    law** dist_laws = v409->map_laws(dist_law, &dist_law_size, 0);  // 怎么实现把dist_law_size变为3的？
    // 输出的法则分为一系列法则，寄存在dist_laws中，每个dist_laws数组元素对应一个基础曲线，同时wire的参数域被适当地重新映射
    int twist_law_size;
    law** twist_laws = wld->map_laws(twist_law, &twist_law_size, 0);
    wld->remove();

    int coedge_count = 0;

    int resignal_no = 0;
    acis_exception error_info_holder(0);
    error_info_base* error_info_base_ptr = nullptr;
    exception_save exception_save_mark;
    ENTITY_LIST off_coedge_list;
    ENTITY_LIST orig_coedge_list;
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    int error_no = 0;
    int openWire = is_open_wire(wire);

    COEDGE* prev_coedgea;
    do {
        COEDGE* offset_coedge = sg_offset_pl_coedge(this_coedge, dist_laws[coedge_count], twist_laws[coedge_count], wire_normal);  // 真正实现每个coedge的偏置
        // 空几何，WIRE开放,原曲线非圆
        if(offset_coedge) {
            if(!offset_coedge->edge()->geometry()) {  // 不理解
                if(openWire) {
                    if(!is_circular(&(this_coedge->edge()->geometry()->equation())))  // coedge不是圆时
                    {
                        if(offset_coedge == offset_coedge->next() || !offset_coedge->next() || (offset_coedge == offset_coedge->previous()) || !offset_coedge->previous()) {
                            api_delent(offset_coedge);
                            offset_coedge = nullptr;
                        }
                    }
                }
            }
        }
        ++coedge_count;
        off_coedge_list.add(offset_coedge, 1);  // 所有偏移的coedge
        orig_coedge_list.add(this_coedge, 1);   // 所有原始的coedge
        prev_coedgea = this_coedge;
        this_coedge = this_coedge->next();
    } while(this_coedge != prev_coedgea && this_coedge != start_coedge);  // 遍历所有的coedge一次，不懂为何上面判断openwire？当this_coedge->next()==this_coedge（openwire）或this_coedge->next()==start_coedge（闭合wire）
    off_coedge_list.init();                                               // 有待探究
    orig_coedge_list.init();
    if(!off_coedge_list[0] || off_coedge_list.count() > 1 && all_offset_degenerated(off_coedge_list))  // 所有的coedge的edge都没有CURVE
    {
        delete_entity(offset_wire);
        offset_wire = nullptr;
    } 
    else 
    {
        COEDGE* c_original = (COEDGE*)orig_coedge_list[0];
        COEDGE* c_offset = (COEDGE*)off_coedge_list[0];
        offset_segment* v418 = ACIS_NEW offset_segment(c_offset, c_original);
        offset_segment* new_seg = v418;
        offset_segment* v60 = seg_list.first_segment();
        offset_segment* curr_seg = v60->previous();
        seg_list.insert_segment(new_seg, curr_seg);  // 后插法
        curr_seg = new_seg;
        coedge_count = off_coedge_list.count();
        COEDGE* prev_off_coedge = (COEDGE*)off_coedge_list[0];
        COEDGE* prev_orig_coedge = (COEDGE*)orig_coedge_list[0];
        prev_off_coedge->set_wire(offset_wire, 1);
        COEDGE* first_off_coedge = prev_off_coedge;
        option_header* anno = find_option("annotations");  // 不懂两个anno
        if(anno->on()) {
            ANNOTATION* v511;
            if(anno->on()) {
                WIRE_OFFSET_ANNO* v420 = ACIS_NEW WIRE_OFFSET_ANNO(((COEDGE*)off_coedge_list[0])->edge(), ((COEDGE*)off_coedge_list[0])->edge());
                v511 = __hookup__(v420);
                v511 = nullptr;
            }
        }
        int start = 1;
        int i = 1;
    LABEL_106:
        logical degen_off_coed_deleted;
        if(i < coedge_count) 
        {
            degen_off_coed_deleted = 0;
            COEDGE* this_orig_coedge = nullptr;
            int nni = -1;
            int ii = 1;
            int old_i = i;
            if(close_type == natural) {
                if(GET_ALGORITHMIC_VERSION() >= AcisVersion(20, 0, 0)) {
                    int j;
                    for(j = i;; ++j) {
                        if(j >= coedge_count) goto LABEL_114;
                        if(((COEDGE*)off_coedge_list[j])->edge()->geometry()) break;
                    }
                    nni = j;
                LABEL_114:
                    if(nni > i) {
                        ii = 0;
                        i = nni;
                    }
                }
            }                    // 不懂
            int skipbackup = 0;  // 看不懂
            while(1) 
            {
                skipbackup = ii == 0;
                COEDGE* this_off_coedge = (COEDGE*)off_coedge_list[i];
                this_orig_coedge = (COEDGE*)orig_coedge_list[i];
                if(anno->on()) {
                    ANNOTATION* v517;
                    if(anno->on()) {
                        WIRE_OFFSET_ANNO* v422 = ACIS_NEW WIRE_OFFSET_ANNO(this_orig_coedge->edge(), this_off_coedge->edge());
                        v517 = __hookup__(v422);
                    }
                    v517 = nullptr;
                }

                option_header* remnulledge = find_option("rem_null_edge");
                if(prev_off_coedge->edge()->geometry() || (this_off_coedge->edge()->geometry()) || !remnulledge->on()) {
                    if(this_off_coedge->edge()->geometry()) goto LABEL_134;
                    if(!remnulledge->on()) goto LABEL_134;
                    if(SPAresfit * SPAresfit <= (prev_off_coedge->end()->geometry()->coords() - this_off_coedge->next()->start()->geometry()->coords()).len_sq()) {
                    LABEL_134:
                        this_off_coedge->set_wire(offset_wire, 1);
                        this_off_coedge->set_previous(prev_off_coedge, 0, 1);
                        prev_off_coedge->set_next(this_off_coedge, 0, 1);
                        if(is_TVERTEX(prev_off_coedge->end()) && (is_TVERTEX(this_off_coedge->start()))) {
                            SPAposition average_pos(prev_off_coedge->end()->geometry()->coords());
                            SPAposition temp(this_off_coedge->start()->geometry()->coords());
                            SPAvector diff = 0.5 * (temp - average_pos);
                            average_pos += diff;
                            prev_off_coedge->end()->geometry()->set_coords(average_pos);
                            this_off_coedge->start()->geometry()->set_coords(average_pos);
                            double tol = 1.001 * diff.len();
                            ((TVERTEX*)(prev_off_coedge->end()))->set_tolerance(tol, 0);
                            ((TVERTEX*)(this_off_coedge->start()))->set_tolerance(tol, 0);
                            if(GET_ALGORITHMIC_VERSION() >= AcisVersion(20, 0, 0)) {
                                VERTEX* vert2 = this_off_coedge->start();
                                VERTEX* vert1 = prev_off_coedge->end();
                                make_same_vertex_pointers(vert1, vert2, prev_off_coedge, this_off_coedge, 1);
                            }
                        } 
                        else {
                            if(twist_law->zero(SPAresabs)) {
                                int dist_law_index = i - 1;
                                double offset_dist = 0.0;
                                if(GET_ALGORITHMIC_VERSION() < AcisVersion(10, 0, 4)) {
                                    dist_law_index = i;
                                    offset_dist = dist_law->eval(this_orig_coedge->edge()->start_param().operator double());
                                } else {
                                    offset_dist = dist_laws[i]->eval(this_orig_coedge->edge()->start_param().operator double());
                                }
                                int check_status;
                                if(ii == 1) {
                                    check_status = check_gap_type(this_orig_coedge, prev_orig_coedge, wire_normal, offset_dist);
                                } else {
                                    check_status = 2;
                                    degen_off_coed_deleted = 0;
                                }
                                int skipStartDegOffsetCoedges = 0;
                                if(openWire) {
                                    if(GET_ALGORITHMIC_VERSION() >= AcisVersion(18, 0, 0)) {
                                        int check_offset = GET_ALGORITHMIC_VERSION() >= AcisVersion(20, 0, 2);
                                        int is_offset_by_radius = 0;
                                        if(i == start) {
                                            if(check_offset) {
                                                law* local_law = edge_dist_law_to_coedge_dist_law(prev_orig_coedge, prev_orig_coedge->edge(), dist_law);
                                                const curve& v110 = prev_orig_coedge->edge()->geometry()->equation();
                                                is_offset_by_radius = is_circle_offset_by_radius(&((curve&)v110), wire_normal, local_law, twist_law);
                                                local_law->remove();
                                            }
                                            skipStartDegOffsetCoedges = (!prev_off_coedge || (!(prev_off_coedge->edge()->geometry())) && !is_offset_by_radius);
                                        } 
                                        else if(i + 1 == coedge_count && prev_off_coedge)
                                        {
                                            if(check_offset) {
                                                COEDGE* next_orig_coedge = prev_orig_coedge->next();
                                                law* v431 = edge_dist_law_to_coedge_dist_law(next_orig_coedge, next_orig_coedge->edge(), dist_law);
                                                is_offset_by_radius = is_circle_offset_by_radius(&((curve&)next_orig_coedge->edge()->geometry()->equation()), wire_normal, v431, twist_law);
                                                v431->remove();
                                            }
                                            COEDGE* next_coedge = prev_off_coedge->next();
                                            int skipStartDegOffsetCoedges = (!next_coedge || (!next_coedge->edge()->geometry()) && !is_offset_by_radius);
                                            next_coedge = nullptr;
                                        }
                                    }
                                }
                                int did_close = 1;
                                if(!skipStartDegOffsetCoedges) {
                                    SPAunit_vector offsetDir(0.0, 0.0, 0.0);
                                    SPAunit_vector endOfstDir(0.0, 0.0, 0.0);
                                    SPAunit_vector startOfstDir(0.0, 0.0, 0.0);
                                    COEDGE* next_off_coedge;
                                    if(prev_off_coedge)
                                        next_off_coedge = prev_off_coedge->next();
                                    else
                                        next_off_coedge = nullptr;
                                    comp_tangent_at_coedge_commom_vertex(prev_orig_coedge, this_orig_coedge, prev_off_coedge, next_off_coedge, offsetDir, startOfstDir, endOfstDir);
                                    if(check_status == 1 && trim) {
                                        law* v545 = dist_laws[dist_law_index];
                                        VERTEX* common_vertex = prev_orig_coedge->end();
                                        did_close = sg_close_offset_gap(seg_list, curr_seg, prev_off_coedge, common_vertex, offsetDir, startOfstDir, endOfstDir, dist_laws[dist_law_index], arc, iKeepMiniTopo, skipbackup);
                                    } 
                                    else if(check_status == 2) {
                                        law* v547 = dist_laws[dist_law_index];
                                        VERTEX* v548 = prev_orig_coedge->end();
                                        did_close = sg_close_offset_gap(seg_list, curr_seg, prev_off_coedge, v548, offsetDir, startOfstDir, endOfstDir, v547, close_type, iKeepMiniTopo, skipbackup);
                                    }
                                    if(skipbackup) {
                                        if(did_close) {
                                            ii = 1;
                                            for(int jj = old_i; jj < i; ++jj) {
                                                COEDGE* ce = (COEDGE*)off_coedge_list[jj];
                                                VERTEX* v433 = ce->edge()->start();
                                                v433->lose();
                                                EDGE* v434 = ce->edge();
                                                v434->lose();
                                                ce->lose();
                                            }
                                        } else {
                                            i = old_i;
                                        }
                                    }
                                }
                            }
                        }
                        ++ii;
                        goto LABEL_183;
                    }
                    display_null_offset_coedge_to_be_deleted(this_off_coedge, this_orig_coedge);
                    VERTEX* v425 = this_off_coedge->edge()->start();
                    v425->lose();
                    EDGE* v426 = this_off_coedge->edge();
                    v426->lose();
                    this_off_coedge->lose();
                    degen_off_coed_deleted = 1;
                } else {
                    SPAposition prev_pos(prev_off_coedge->start()->geometry()->coords());
                    SPAposition this_pos(this_off_coedge->start()->geometry()->coords());
                    if(SPAresabs <= (prev_pos - this_pos).len()) goto LABEL_134;
                    display_null_offset_coedge_to_be_deleted(this_off_coedge, this_orig_coedge);
                    VERTEX* v423 = this_off_coedge->edge()->start();
                    v423->lose();
                    EDGE* v424 = this_off_coedge->edge();
                    v424->lose();
                    this_off_coedge->lose();
                    degen_off_coed_deleted = 1;
                }
            LABEL_183:
                if(ii >= 2 || degen_off_coed_deleted) {
                    if(!degen_off_coed_deleted) {
                        offset_segment* v436 = ACIS_NEW offset_segment(this_off_coedge, (COEDGE*)orig_coedge_list[i]);
                        new_seg = v436;
                        seg_list.insert_segment(v436, curr_seg);
                        curr_seg = new_seg;
                        prev_off_coedge = this_off_coedge;
                    }
                    prev_orig_coedge = this_orig_coedge;
                    ++i;
                    goto LABEL_106;
                }
            }
        }
        if(start_coedge->previous() != start_coedge || (start_coedge->start() == start_coedge->end())) {
            COEDGE* last_off_coedge = prev_off_coedge;
            first_off_coedge->set_previous(prev_off_coedge, 0, 1);
            last_off_coedge->set_next(first_off_coedge, 0, 1);
            if(is_TVERTEX(last_off_coedge->end()) && (is_TVERTEX(first_off_coedge->start()))) {
                SPAposition new_coords(last_off_coedge->end()->geometry()->coords());
                SPAposition v703(first_off_coedge->start()->geometry()->coords());
                const SPAvector& v128 = v703 - new_coords;
                SPAvector v = 0.5 * v128;
                new_coords += v;
                last_off_coedge->end()->geometry()->set_coords(new_coords);
                first_off_coedge->start()->geometry()->set_coords(new_coords);
                double in_tol = 1.001 * v.len();
                TVERTEX* v554 = (TVERTEX*)last_off_coedge->end();
                v554->set_tolerance(in_tol, 0);
                TVERTEX* v555 = (TVERTEX*)first_off_coedge->start();
                v555->set_tolerance(in_tol, 0);
                if(GET_ALGORITHMIC_VERSION() >= AcisVersion(20, 0, 0)) {
                    make_same_vertex_pointers(last_off_coedge->end(), first_off_coedge->start(), last_off_coedge, first_off_coedge, 1);
                }
            }

            else {
                if(twist_law->zero(SPAresabs)) {
                    SPAunit_vector oTangent(0.0, 0.0, 0.0);
                    SPAunit_vector iStartOfstDir(0.0, 0.0, 0.0);
                    SPAunit_vector iEndOfstDir(0.0, 0.0, 0.0);
                    comp_tangent_at_coedge_commom_vertex(prev_orig_coedge, start_coedge, last_off_coedge, first_off_coedge, oTangent, iStartOfstDir, iEndOfstDir);
                    double v564 = dist_law->eval(start_coedge->edge()->start_param().operator double());
                    if(check_gap_type(start_coedge, prev_orig_coedge, wire_normal, v564) == 1 && trim) {
                        sg_close_offset_gap(seg_list, curr_seg, last_off_coedge, prev_orig_coedge->end(), oTangent, iStartOfstDir, iEndOfstDir, dist_laws[dist_law_size - 1], arc, iKeepMiniTopo, 0);
                    } else if(check_gap_type(start_coedge, prev_orig_coedge, wire_normal, v564) == 2) {
                        sg_close_offset_gap(seg_list, curr_seg, last_off_coedge, prev_orig_coedge->end(), oTangent, iStartOfstDir, iEndOfstDir, dist_laws[dist_law_size - 1], close_type, iKeepMiniTopo, 0);
                    }
                }
            }
        }
        offset_wire->set_coedge(first_off_coedge);
    }
    for(int lawi = 0; lawi < dist_law_size; ++lawi) {
        dist_laws[lawi]->remove();
        twist_laws[lawi]->remove();
    }
    ACIS_DELETE[] STD_CAST dist_laws;
    dist_laws = nullptr;
    ACIS_DELETE[] STD_CAST twist_laws;
    twist_laws = nullptr;
    if(resignal_no || acis_interrupted()) sys_error(resignal_no, error_info_base_ptr);
    BODY* offset_wire_body = ACIS_NEW BODY(offset_wire);
    return offset_wire_body;
    TRANSFORM* v441 = nullptr;
    if(wire_transf) {
        v441 = ACIS_NEW TRANSFORM((SPAtransf)wire_transf->transform());  // 不确定
    }
    offset_wire_body->set_transform(v441, 1);


    if(woffset_module_header.debug_level >= 30) {
        acis_fprintf(debug_file_ptr, "!!! seg_list before sg_trim_offset_wire()\n");
        seg_list.print(debug_file_ptr);
        acis_fprintf(debug_file_ptr, "\n");
    }
    if(woffset_module_header.debug_level >= 35) {
        acis_fprintf(debug_file_ptr, "!!! offset_wire_body before sg_trim_offset_wire()\n");
        debug_entity(offset_wire_body, debug_file_ptr);
        acis_fprintf(debug_file_ptr, "\n");
    }
    if(woffset_module_header.debug_level >= 40) {
        acis_fprintf(debug_file_ptr, "!!! Creating wb.sat file\n\n");
        _iobuf* fptr = fopen("wb.sat", "w");
        ENTITY_LIST elist;
        elist.add(offset_wire_body, 1);
        api_save_entity_list(fptr, 1, elist);
        fclose(fptr);
    }
    int intersection_found = 0;
    int inside_wire_trimmed = 0;

    if(trim && (twist_law->zero(SPAresabs))) {
        seg_list.set_base_wire(wire, coedge_count);
        seg_list.set_edge_smooth_manager(iEdgeSmoothMgr);

        intersection_found = sg_trim_offset_wire(seg_list, zero_length, overlap);

        inside_wire_trimmed = seg_list.trim_inside_wire_done();
        if(seg_list.mSelfIntesectError) {
            delete_entity(offset_wire_body);
            return NULL;
        }
    }

    else {
        offset_segment* this_seg = seg_list.first_segment();
        offset_segment* last_seg = seg_list.last_segment()->next();
        while(this_seg != last_seg) {
            offset_segment* next_seg = this_seg->next();
            COEDGE* ofc = this_seg->coedge();
            COEDGE* pgc = this_seg->original_coedge();
            if(is_intcurve_edge(ofc->edge())) {
                if(is_intcurve_edge(pgc->edge())) {
                    const intcurve& ofintc = (const intcurve&)ofc->edge()->geometry()->equation();
                    const intcurve& pgintc = (const intcurve&)pgc->edge()->geometry()->equation();
                    double t1 = bs3_curve_angle(((intcurve&)pgintc).cur(-1.0, 0));
                    double t2 = bs3_curve_angle(((intcurve&)ofintc).cur(-1.0, 0));
                    if(t2 > t1 + 3.0) {
                        double v581 = ((intcurve&)ofintc).fitol();
                        double actual_tol = 0.0;
                        intcurve& ofintc_upd = (intcurve&)ofc->edge()->geometry()->equation_for_update();
                        SPAinterval the_int = ofintc_upd.param_range();
                        bs3_curve_def* new_bs3 = nullptr;
                        int err_num = 0;
                        acis_exception v696(0);
                        error_info_base* e_info = nullptr;
                        exception_save v663;
                        v663.begin();
                        get_error_mark().buffer_init = 1;
                        new_bs3 = bs3_curve_make_approx(ofintc_upd, the_int, v581, actual_tol, 0);
                        if(new_bs3) {
                            double rem_tol = actual_tol;
                            if(SPAresabs > rem_tol) rem_tol = SPAresabs;
                            bs3_curve_rem_extra_knots(new_bs3, rem_tol);
                            double t3 = bs3_curve_angle(new_bs3);
                            if(t1 + 3.0 > t3) {
                                ofintc_upd.set_cur(new_bs3, rem_tol, 1, 0);
                                new_bs3 = nullptr;
                            }
                        }
                        if(new_bs3) {
                            bs3_curve_delete(new_bs3);
                            new_bs3 = nullptr;
                        }
                        if(err_num || acis_interrupted()) sys_error(err_num, e_info);
                    }
                }
            }
            this_seg = next_seg;
        }
    }
  


    SHELL* shell = nullptr;
    if(offset_wire_body->lump()) {
    LABEL_250:
        shell = offset_wire_body->lump()->shell();
        goto LABEL_319;
    }
    if(offset_wire_body->wire()) {
        WIRE* its_wire = offset_wire_body->wire();
        offset_wire_body->set_wire(nullptr, 1);
        LUMP* v452 = ACIS_NEW LUMP();
        LUMP* a_lump = v452;
        v452->set_body(offset_wire_body, 1);
        offset_wire_body->set_lump(a_lump, 1);
        SHELL* last_shell = nullptr;
        while(its_wire) {
            WIRE* next_wire = its_wire->next(PAT_CAN_CREATE);
            SHELL* v454 = ACIS_NEW SHELL();
            SHELL* a_shell = v454;
            v454->set_wire(its_wire, 1);
            its_wire->set_shell(a_shell, 1);
            a_shell->set_lump(a_lump, 1);
            if(last_shell) {
                last_shell->set_next(a_shell, 1);
            } else {
                last_shell = a_shell;
                a_lump->set_shell(a_shell, 1);
            }
            its_wire = next_wire;
        }
        if(!trim) {
            int simple_wire = 1;
            ENTITY_LIST coeds;
            for(SHELL* a_shella = offset_wire_body->lump()->shell(); a_shella; a_shella = a_shella->next(PAT_CAN_CREATE)) {
                WIRE* its_wirea = a_shella->wire();
                get_coedges(its_wirea, coeds, PAT_CAN_CREATE);
                for(int index = 0;; ++index) {
                    if(index >= coeds.count()) break;
                    COEDGE* coed = (COEDGE*)coeds[index];
                    coed->set_wire(its_wirea, 1);
                    if(simple_wire) {
                        if(coed->partner() || coed->previous() && (coed->previous()->next() != coed) || coed->next() && (coed->next()->previous() != coed) || coed->start() && (coed->start()->count_edges() != 1) ||
                           coed->end() && (coed->end()->count_edges() != 1)) {
                            simple_wire = 0;
                        }
                    }
                }
                coeds.clear();
            }

            int in_R10 = GET_ALGORITHMIC_VERSION() >= AcisVersion(10, 0, 0);
            if(in_R10)
                repair_dupicated_vertices(offset_wire_body);
            else
                simple_wire = 0;
            SHELL* v376 = nullptr;
            if(simple_wire) {
                ENTITY_LIST new_wires;
                for(SHELL* a_shellb = offset_wire_body->lump()->shell(); a_shellb; a_shellb = a_shellb->next(PAT_CAN_CREATE)) {
                    v376 = a_shellb;
                    WIRE* its_wireb = a_shellb->wire();
                    COEDGE* start_coed = its_wireb->coedge();
                    COEDGE* previous = start_coed;
                    int wire_periodic = 0;
                    if(start_coed) {
                        if(start_coed->previous()) {
                            if(start_coed->previous() != previous) {
                                if(start_coed->previous()->end() == previous->start()) {
                                    wire_periodic = 1;
                                } else {
                                    COEDGE* v591 = start_coed->previous();
                                    COEDGE* next = start_coed->previous();
                                    v591->set_next(next, 0, 1);
                                    previous->set_previous(previous, 0, 1);
                                }
                            }
                        }
                    }
                    int reset_period = 0;
                    while(previous) {
                        COEDGE* coed_next = previous->next();
                        if(coed_next == previous) coed_next = nullptr;
                        previous->set_wire(its_wireb, 1);
                        if(coed_next) {
                            if(coed_next->start() != previous->end()) {
                                previous->set_next(previous, 0, 1);
                                coed_next->set_previous(coed_next, 0, 1);
                                if(wire_periodic) {
                                    its_wireb->set_coedge(coed_next);
                                    start_coed = coed_next;
                                    wire_periodic = 0;
                                    reset_period = 1;
                                } else {
                                    WIRE* v456 = ACIS_NEW WIRE(coed_next, nullptr);
                                    its_wireb = v456;
                                    coed_next->set_wire(v456, 1);
                                    new_wires.add(its_wireb, 1);
                                }
                            }
                        }
                        previous = coed_next;
                        if(!reset_period && previous == start_coed) previous = nullptr;
                        reset_period = 0;
                    }
                }
                new_wires.init();
                for(WIRE* m = (WIRE*)new_wires.next(); m; m = (WIRE*)new_wires.next()) {
                    SHELL* v458 = ACIS_NEW SHELL();
                    SHELL* a_shellc = v458;
                    v458->set_wire(m, 1);
                    m->set_shell(a_shellc, 1);
                    a_shellc->set_lump(offset_wire_body->lump(), 1);
                    v376->set_next(a_shellc, 1);
                    v376 = a_shellc;
                }
            }
            goto LABEL_319;
        }
        goto LABEL_250;
    }
LABEL_319:
    if(!inside_wire_trimmed && shell && (shell->next(PAT_CAN_CREATE) || intersection_found)) {
        while(shell && dist_law->constant()) {
            double X = dist_law->eval(1.0);
            VERTEX* v172;
            if(shell->wire()->coedge()->sense()) {
                v172 = shell->wire()->coedge()->end();
            } else {
                v172 = shell->wire()->coedge()->start();
            }

            SPAposition& v459 = (SPAposition&)v172->geometry()->coords();
            SPAposition testpos(v459);
            start_coedge = start_of_wire_chain(wire);
            COEDGE* this_coedgea = start_coedge;
            int cutout = 0;
            while(1) {
                SPAposition cpoint;
                SPAtransf face_trans;
                EDGE* v599 = this_coedgea->edge();
                find_cls_pt_on_edge(testpos, v599, cpoint, face_trans);
                SPAvector& v177 = (SPAvector&)(cpoint - testpos);
                double testdist = v177.len();
                double max_error = 0.0;
                find_max_tolerance(this_coedgea, max_error);
                if(max_error < 0.001) max_error = 0.001;
                if(fabs(X) > testdist + max_error) break;
                COEDGE* prev_coedgeb = this_coedgea;
                this_coedgea = this_coedgea->next();
                if(this_coedgea == prev_coedgeb || this_coedgea == start_coedge) goto LABEL_348;
            }
            /*if(Debug_Break_Active("trim shells", "WIRE-OFFSET"))
            {
                if(get_breakpoint_callback()) {
                    v460 = get_breakpoint_callback();
                    v602 = v460->new_render_object;
                    v461 = v602(v460, 1);
                } else {
                    v461 = 0i64;
                }
                v299 = v461;
                if(v461) {
                    v179 = SHELL::wire(shell);
                    show_entity(v179, TOPOLOGY_NEW, v299);
                    insert_point = v299->insert_point;
                    insert_point(v299, &testpos);
                    v604 = v299->insert_point;
                    v604(v299, &cpoint);
                    v180 = operator-(&v726, &testpos, &cpoint);
                    v181 = normalise(&v727, v180);
                    operator*(&direction, v181, X);
                    v412 = (ellipse*)ACIS_OBJECT::operator new(0xB0ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 1801, &alloc_file_index_3517);
                    if(v412) {
                        ellipse::ellipse(v412, &cpoint, wire_normal, &direction, 1.0, 0.0);
                        v384 = (ellipse*)v182;
                    } else {
                        v384 = 0i64;
                    }
                    v605 = v384;
                    elli = v384;
                    show_curve(v384, -3.1415927, 3.1415927, v299);
                    v374 = elli;
                    if(elli) {
                        v606 = v374->~ellipse;
                        v607 = ((__int64(__fastcall*)(ellipse*, __int64))v606)(v374, 1i64);
                    } else {
                        v607 = 0i64;
                    }
                    acis_fprintf(debug_file_ptr, "sg_offset_planar_wire - trimming shells\n");
                    Debug_Break("trim shells", "WIRE-OFFSET", "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofwire.cpp", 1809);
                    if(get_breakpoint_callback()) {
                        v386 = get_breakpoint_callback();
                        v608 = v386->delete_render_object;
                        v608(v386, v299);
                    }
                }
            }*/
            cutout = 1;
        LABEL_348:
            SHELL* nshell = shell->next(PAT_CAN_CREATE);
            if(cutout) {
                extract_shell(shell, 0);
                del_entity(shell);
            }
            shell = nshell;
        }
    }
    ENTITY_LIST all_shells;
    ENTITY_LIST all_lumps;
    check_outcome(api_get_shells(offset_wire_body, all_shells));
    check_outcome(api_get_lumps(offset_wire_body, all_lumps));
    if(all_shells.count() != all_lumps.count()) {
        LUMP* head_lump = nullptr;
        LUMP* prev_lump = nullptr;
        LUMP* current_lump;
        for(int n = 0; n < all_shells.count(); ++n) {
            SHELL* current_shell = (SHELL*)all_shells[n];
            LUMP* v388 = ACIS_NEW LUMP();
            current_lump = v388;
            v388->set_body(offset_wire_body, 1);
            current_lump->set_shell(current_shell, 1);
            if(head_lump)
                prev_lump->set_next(current_lump, 1);
            else
                head_lump = current_lump;
            prev_lump = current_lump;
            current_shell->set_lump(current_lump, 1);
            current_shell->set_next(nullptr, 1);
        }
        offset_wire_body->set_lump(head_lump, 1);
        for(int kk = 0; kk < all_lumps.count(); ++kk) {
            current_lump = (LUMP*)all_lumps[kk];
            current_lump->set_body(nullptr, 1);
            current_lump->set_shell(nullptr, 1);
            check_outcome(api_del_entity(current_lump));
        }
    }

    if(woffset_module_header.debug_level >= 30) {
        acis_fprintf(debug_file_ptr, "!!! seg_list after sg_trim_offset_wire()\n");
        seg_list.print(debug_file_ptr);
        acis_fprintf(debug_file_ptr, "\n");
    }
    if(woffset_module_header.debug_level >= 35) {
        acis_fprintf(debug_file_ptr, "*** offset_wire_body after sg_trim_offset_wire()\n");
        debug_entity(offset_wire_body, debug_file_ptr);
        acis_fprintf(debug_file_ptr, "\n");
    }
    if(woffset_module_header.debug_level >= 10) acis_fprintf(debug_file_ptr, "    Leaving sg_offset_planar_wire()\n");

    ENTITY_LIST edges;
    get_edges(offset_wire_body, edges, PAT_CAN_CREATE);
    if(edges.count() > 0) {
        EDGE* first_edge = (EDGE*)edges.first();
        for(EDGE* edge = first_edge; edge; edge = (EDGE*)edges.next()) {
            if(edge->geometry()) {
                const curve& edge_cur = edge->geometry()->equation();
                if(((curve&)edge_cur).type() == 11) 
                {
                    SPAposition pt;
                    SPAvector dpdt;
                    SPAvector dp2dt2;
                    SPAvector* _deriv_ptr[3];
                    _deriv_ptr[0] = &dpdt;
                    _deriv_ptr[1] = &dp2dt2;
                    double t_val = edge->start_param().operator double();
                    double v186 = edge->start_param() - edge->end_param();
                    double max_t_step = fabs(v186) / 10.0;
                    double dir;
                    if(edge->start_param() < edge->end_param())
                        dir = 1;
                    else
                        dir = -1;
                    edge_cur.evaluate(t_val, pt, _deriv_ptr, 2, evaluate_curve_unknown);  // 不确定
                    SPAvector start_curvature = conv_curvature(*_deriv_ptr[0], *_deriv_ptr[1]);
                    double start_t_step = max_t_step;
                    if(start_curvature.len() > 1.0 / SPAresabs) {
                        if(_deriv_ptr[1]->len() > SPAresmch) {
                            start_t_step = acis_sqrt(SPAresabs / _deriv_ptr[1]->len());
                        }
                        if(start_t_step > max_t_step) start_t_step = max_t_step;
                    }
                    t_val = edge->end_param().operator double();
                    edge_cur.evaluate(t_val, pt, _deriv_ptr, 2, evaluate_curve_unknown);
                    SPAvector end_curvature = conv_curvature(*_deriv_ptr[0], *_deriv_ptr[1]);
                    double end_t_step = max_t_step;
                    if(end_curvature.len() > 1.0 / SPAresabs) {
                        if(_deriv_ptr[1]->len() > SPAresmch) {
                            end_t_step = acis_sqrt(SPAresabs / _deriv_ptr[1]->len());
                        }
                        if(end_t_step > max_t_step) end_t_step = max_t_step;
                    }
                    int adjust_edge_start = 0;
                    int adjust_edge_end = 0;
                    double start_par = edge->start_param().operator double();
                    double end_par = edge->end_param().operator double();
                    int open_start_vert = 0;
                    int open_end_vert = 0;
                    ENTITY_LIST start_vert_edges;
                    ENTITY_LIST end_vert_edges;
                    get_edges(edge->start(), start_vert_edges, PAT_CAN_CREATE);
                    if(start_vert_edges.count() == 1) {
                        if(edge->start() != edge->end()) open_start_vert = 1;
                    }
                    get_edges(edge->end(), end_vert_edges, PAT_CAN_CREATE);
                    if(end_vert_edges.count() == 1) {
                        if(edge->start() != edge->end()) open_end_vert = 1;
                    }
                    if(start_curvature.len() > 1.0 / SPAresabs) {
                        double t_adjust = start_t_step;
                        double div = 2.0;
                        double curvature = 0.0;
                        SPAposition prev_pt;
                        if(start_t_step == max_t_step) {
                            prev_pt = edge->start()->geometry()->coords();
                            while(1) {
                                edge_cur.evaluate(start_par + dir * t_adjust, pt, _deriv_ptr, 2);  // 不确
                                if((conv_curvature(*_deriv_ptr[0], *_deriv_ptr[1])).len() > 1.0 / SPAresabs) break;
                                if(same_point(prev_pt, pt, SPAresabs)) start_t_step = t_adjust / div;
                                div = div * 2.0;
                                prev_pt = pt;
                            }
                            double v366 = start_par + dir * t_adjust / div * 2.0;
                            double v368 = start_par + dir * t_adjust / div;
                            while(1) {
                                if(same_point(prev_pt, pt, SPAresabs)) break;
                                double t = (v368 + v366) / 2.0;
                                edge_cur.evaluate(t, pt, _deriv_ptr, 2);
                                if(conv_curvature(*_deriv_ptr[0], *_deriv_ptr[1]).len() <= 1.0 / SPAresabs) {
                                    v366 = t;
                                } else {
                                    if(SPAresmch > t - v368) break;
                                    v368 = t;
                                }
                            }
                            start_t_step = fabs(v366 - start_par);
                        }
                        adjust_edge_start = 1;
                        start_par = start_par + start_t_step * dir;
                    }
                    if(end_curvature.len() > 1.0/ SPAresabs) {
                        double v350 = end_t_step;
                        double v311 = 2.0;
                        SPAposition v668;
                        if(end_t_step == max_t_step) {
                            v668 = edge->end()->geometry()->coords();
                            while(1) {
                                edge_cur.evaluate(end_par - dir * v350, pt, _deriv_ptr, 2);  // 不确
                                if(conv_curvature(*_deriv_ptr[0], *_deriv_ptr[1]).len() > SPAresabs) break;
                                if(same_point(v668, pt, SPAresabs)) end_t_step = v350 / v311;
                                v311 = v311 * 2.0;
                                v668 = pt;
                            }
                            double v364 = end_par - dir * v350 / v311 * 2.0;
                            double v391 = end_par - dir * v350 / v311;
                            while(1) {
                                if(same_point(v668, pt, SPAresabs)) break;
                                double v365 = (v391 + v364) / 2.0;
                                int v641 = edge_cur.evaluate(v365, pt, _deriv_ptr, 2,evaluate_curve_unknown);
                                if(conv_curvature(*_deriv_ptr[0], *_deriv_ptr[1]).len() <= 1.0 / SPAresabs)
                                    v364 = v365;
                                else
                                    v391 = v365;
                            }
                            end_t_step = fabs(v364 - end_par);
                        }
                        adjust_edge_end = 1;
                        end_par = end_par - end_t_step * dir;
                    }
                    if(adjust_edge_start) {
                        SPAinterval given_range(start_par, end_par);
                        edge->geometry()->equation_for_update().limit(given_range);
                        if(open_start_vert) {
                            SPAposition start_pos = edge_cur.eval_position(start_par, 1);
                            APOINT* v393 = ACIS_NEW APOINT(start_pos);
                            APOINT* start_point = v393;
                            edge->start()->set_geometry(start_point, 1);
                            edge->set_param_range(nullptr);
                        } else {
                            TVERTEX* tvert = nullptr;
                            if(!is_TVERTEX(edge->start())) {
                                APOINT* v395 = ACIS_NEW APOINT(edge->start()->geometry()->coords());
                                APOINT* ap = v395;
                                replace_vertex_with_tvertex(edge->start(), tvert);
                                tvert->set_geometry(ap, 1);
                                tvert->set_tolerance(0.0, 1);
                            }
                            edge->set_param_range(nullptr);
                        }
                    }
                    if(adjust_edge_end) {
                        SPAinterval v233(start_par, end_par);
                        edge->geometry()->equation_for_update().limit(v233);
                        if(open_end_vert) {
                            SPAposition end_pos = edge_cur.eval_position(end_par, 1, 0);
                            APOINT* v398 = ACIS_NEW APOINT(end_pos);
                            APOINT* end_point = v398;
                            edge->end()->set_geometry(end_point, 1);
                            edge->set_param_range(nullptr);
                        } else {
                            TVERTEX* this_tvertex = nullptr;
                            if(!is_TVERTEX(edge->end())) {
                                APOINT* v400 = ACIS_NEW APOINT(edge->end()->geometry()->coords());
                                APOINT* point = v400;
                                replace_vertex_with_tvertex(edge->end(), this_tvertex);
                                this_tvertex->set_geometry(point, 1);
                                this_tvertex->set_tolerance(0.0, 1);
                            }
                            edge->set_param_range(nullptr);
                        }
                    }
                }
            }
        }
    }
    if(bOutwards && offset_wire_body && !offset_wire_body->lump() && (!iEdgeSmoothMgr || iEdgeSmoothMgr->is_last_iteration())) {
        ofst_error(OFFSET_BRANCHED_WIRE, 1, wire);
    }
    display_wire_offset_io(wire, dist_law, offset_wire_body);
    return offset_wire_body;
}
BODY* gme_sg_offset_planar_wire(BODY* wire_body, double offset_dist,const SPAunit_vector& wire_normal, sg_gap_type close_type, int add_attribs, int trim, int zero_length, int overlap) {
    BODY* result_body = nullptr;
    if(wire_body) {
        TRANSFORM* wire_transf = wire_body->transform();
        SPAunit_vector transf_wire_normal;
        if(wire_transf) {
            const SPAtransf& v8 = (const SPAtransf&)wire_transf->transform().inverse();
            SPAunit_vector v9 = wire_normal * v8;
            SPAvector tmp(v9);
            SPAunit_vector transf_wire_normal = normalise(tmp);
        } else {
            transf_wire_normal = wire_normal;
        }
        for(WIRE* wire = wire_body->wire(); wire; wire = wire->next(PAT_CAN_CREATE)) {
            BODY* this_body = sg_offset_planar_wire(wire, wire_transf, offset_dist, transf_wire_normal, close_type, add_attribs, trim, zero_length, overlap);
            if(result_body) {
                do_boolean(this_body, result_body, UNION, SpaAcis::NullObj::get_body_ptr(), SpaAcis::NullObj::get_body_ptr(), NDBOOL_KEEP_NEITHER, SpaAcis::NullObj::get_body_ptr(), nullptr, 0);  //
            }

            else {
                result_body = this_body;
            }
        }
        for(LUMP* lump = wire_body->lump(); lump; lump = lump->next(PAT_CAN_CREATE)) {
            for(SHELL* shell = lump->shell(); shell; shell = shell->next(PAT_CAN_CREATE)) {
                for(WIRE* i = shell->wire(); i; i = i->next(PAT_CAN_CREATE)) {
                    BODY* tool_body = sg_offset_planar_wire(i, wire_transf, offset_dist, transf_wire_normal, close_type, add_attribs, trim, zero_length, overlap);
                    if(result_body) {
                        do_boolean(tool_body, result_body, UNION, SpaAcis::NullObj::get_body_ptr(), SpaAcis::NullObj::get_body_ptr(), NDBOOL_KEEP_NEITHER, SpaAcis::NullObj::get_body_ptr(), nullptr, 0);  
                    } else {
                        result_body = tool_body;
                    }
                }
            }
        }
    }
    return result_body;
}
