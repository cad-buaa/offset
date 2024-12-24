#include "acis/acistype.hxx"
#include "acis/at_col.hxx"
#include "acis/at_ct.hxx"
#include "acis/at_name.hxx"
#include "acis/at_real.hxx"
#include "acis/cnstruct.hxx"
#include "acis/coedge.hxx"
#include "acis/compcurv.hxx"
#include "acis/container_utils.hxx"
#include "acis/cucuint.hxx"
#include "acis/cur.hxx"
#include "acis/curdef.hxx"
// #include "acis/container_utils_scm.hxx"
#include "PrivateInterfaces/apiFinder.hxx"
#include "PrivateInterfaces/off_trim.hxx"
#include "PrivateInterfaces/ofst_bad_intersections_remover.hxx"
#include "PrivateInterfaces/ofst_edge_smooth_manager.hxx"
#include "PrivateInterfaces/ofst_intersection_containment_remover.hxx"
#include "PrivateInterfaces/ofst_natural_extender.hxx"
#include "PrivateInterfaces/ofst_trim_util.hxx"
#include "ProtectedInterfaces/at_rel.hxx"
#include "ProtectedInterfaces/off_cu.hxx"
#include "ProtectedInterfaces/ofsttools.hxx"
#include "acis/ellipse.hxx"
#include "acis/ent_tool.hxx"
#include "acis/errorbase.hxx"
#include "acis/esplit.hxx"
#include "acis/euler.hxx"
#include "acis/exct_int.hxx"
#include "acis/geometry.hxx"
#include "acis/get_top.hxx"
#include "acis/getbox.hxx"
#include "acis/intcur.err"
#include "acis/intdef.hxx"
#include "acis/loop_utl.hxx"
#include "acis/module.hxx"
#include "acis/off_misc.hxx"
#include "acis/off_utl.hxx"
#include "acis/option.hxx"
#include "acis/param.hxx"
#include "acis/point.hxx"
#include "acis/remtop.hxx"
#include "acis/repedge.hxx"
#include "acis/sgcofrtn.hxx"
#include "acis/spd3rtn.hxx"
#include "acis/sps3crtn.hxx"
#include "acis/stch_utl.hxx"
#include "acis/strdef.hxx"
#include "acis/undefc.hxx"
#include "acis/wire_qry.hxx"
#include "acis/wire_utl.hxx"

extern APIFINDER apiFinderACIS;
extern PROCSTATE prostate;

option_header gme_check_offst_self_intersection("gme_check_offset_selfint", 1);

module_debug trim_offset_wire_module_header("trim_offset_wire");
class option_header;
class offset_segment_list;

class SPAdouble_array : public ACIS_OBJECT {
    int blk_size;
    int blk_cap;
    int blk_mult_i;
    double* blk_data;

  public:
    enum Exact_flag { EXACT = 1 };
    SPAdouble_array(SPAdouble_array& src);
    SPAdouble_array(int size, int blmult);
    void Push(const double& val);
    virtual void Copy_block(const double* tgt, const const double* src, int count);
    virtual void Swap_block(const double* tgt, const double* src, int count);
    virtual void Swap(const double* tgt, const double* src);
    SPAdouble_array& operator=(SPAdouble_array& src);
    ~SPAdouble_array();
    int Size() const;
    int Capacity();
    double& operator[](int i);
    const double& operator[](int i) const;
    SPAdouble_array& Insert(int index, int count, const double& val);
    SPAdouble_array& Insert(int index, int count);
    SPAdouble_array& Remove(int index, int count);
    SPAdouble_array& Need(int new_size, SPAdouble_array::Exact_flag __formal);
    SPAdouble_array& Need(int new_size);
    SPAdouble_array& Grow(int new_size, SPAdouble_array::Exact_flag __formal);
    SPAdouble_array& Grow(int new_size);
    SPAdouble_array& Wipe();
    int Pop(double& val);
    int Top(double& val);
    void Free_data();
    void Alloc_block(int new_cap);
    void Realloc_block(int new_cap);
};

void sg_add_extensions(offset_segment_list& l) {
    WIRE* coedge_owner = (WIRE*)l.first_segment()->original_coedge()->owner();
    if(coedge_owner) {
        if(coedge_owner->identity(1) == WIRE_TYPE) {
            WIRE* w = coedge_owner;
            COEDGE* first_coedge = start_of_wire_chain(coedge_owner);
            COEDGE* last_coedge = first_coedge;
            for(COEDGE* next_coedge = first_coedge->next(); next_coedge && next_coedge != last_coedge && next_coedge != first_coedge; next_coedge = next_coedge->next()) {
                last_coedge = next_coedge;
            }

            SPAposition start_pos(first_coedge->start()->geometry()->coords());
            SPAposition end_pos(last_coedge->end()->geometry()->coords());
            if(!(start_pos == end_pos)) {
                law* dist_law = l.distance();
                if(!dist_law->constant()) {
                    double dist = dist_law->eval(1.0);
                    if(trim_offset_wire_module_header.debug_level >= 20) acis_fprintf(debug_file_ptr, "...Adding extensions\n");
                    COEDGE* first_off_coedge = l.first_segment()->coedge();
                    COEDGE* last_off_coedge = l.last_segment()->coedge();
                    SPAunit_vector coedge_dir = coedge_start_dir(first_coedge);
                    SPAvector major = (l.normal() * coedge_dir) * dist;
                    APOINT* v40 = ACIS_NEW APOINT(start_pos + major);
                    VERTEX* v42 = ACIS_NEW VERTEX(v40);
                    SPAunit_vector cir_normal = normalise(coedge_dir * major);

                    ELLIPSE* v44 = ACIS_NEW ELLIPSE(start_pos, cir_normal, major, 1.0);
                    EDGE* v46 = ACIS_NEW EDGE(v42, first_off_coedge->start(), v44, 0, EDGE_cvty_unknown);
                    COEDGE* v49 = ACIS_NEW COEDGE(v46, 0, nullptr, first_off_coedge);
                    v49->set_previous(v49, 0, 1);
                    coedge_dir = coedge_end_dir(last_coedge);
                    major = (coedge_dir * l.normal()) * dist;
                    APOINT* v52 = ACIS_NEW APOINT(end_pos - major);
                    VERTEX* v54 = ACIS_NEW VERTEX(v52);
                    cir_normal = normalise(major * coedge_dir);
                    ELLIPSE* v56 = ACIS_NEW ELLIPSE(end_pos, cir_normal, major, 1.0);
                    EDGE* v58 = ACIS_NEW EDGE(last_off_coedge->end(), v54, v56, 0, EDGE_cvty_unknown);
                    COEDGE* v61 = ACIS_NEW COEDGE(v58, 0, last_off_coedge, nullptr);
                    v61->set_next(v61, 0, 1);
                    offset_segment* v63 = ACIS_NEW offset_segment(v49, first_coedge->start());
                    v63->set_extension();
                    offset_segment* v65 = ACIS_NEW offset_segment(v61, last_coedge->end());
                    v65->set_extension();
                    l.insert_segment(v63, l.first_segment()->previous());
                    l.insert_segment(v65, l.last_segment());
                }
            }
        }
    }
}

void display_segment_in_colors(offset_segment_list& iSegList, int iNumColors) {
    return;
}

bs3_curve_def* smooth_curve(curve* iCurve, SPAinterval& iParamRange, double iFitTolerance, int iMoreSamples) {
    bs3_curve_def* retCurve = nullptr;
    int v11;
    if(iMoreSamples)
        v11 = 20;
    else
        v11 = 9;
    SPAposition* v17 = ACIS_NEW SPAposition[v11];
    if(v17) {
        double sParam = iParamRange.start_pt();
        double eParam = iParamRange.end_pt();
        SPAunit_vector sDir;
        SPAunit_vector eDir;
        SPAvector tmpDir;
        iCurve->eval(sParam, v17[0], tmpDir);
        sDir = normalise(tmpDir);
        iCurve->eval(eParam, v17[v11 - 1], tmpDir);
        eDir = normalise(tmpDir);
        double dParam = (eParam - sParam) / (double)(v11 - 1);
        double param = sParam + dParam;
        for(int i = 1; i < v11 - 1; ++i) {
            v17[i] = iCurve->eval_position(param, 0, 0);
            param = param + dParam;
        }

        retCurve = bs3_curve_interp(v11, v17, sDir, eDir, iFitTolerance);
        bs3_curve_reparam(sParam, eParam, retCurve);
        ACIS_DELETE[] v17;
    }
    return retCurve;
}
curve* filter_high_curvature(law* iDistLaw, SPAunit_vector& iNormal, curve* offsetCurve, SPAinterval& iParamRange, int iSingleSegment) {
    if(!iDistLaw || !iDistLaw->constant() || !offsetCurve) return 0;
    acis_exception error_info_holder(0);
    exception_save exception_save_mark;
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    offset_int_cur& intCur = (offset_int_cur&)((intcurve*)offsetCurve)->get_int_cur();  // 不确定
    const curve& baseCurve = intCur.get_orig_curve();
    bs3_curve_def* newBaseCurve = smooth_curve(&(curve&)baseCurve, iParamRange, 0.01 * fabs(iDistLaw->eval(1.0)), iSingleSegment);  // 不确定

    intcurve* v17 = ACIS_NEW intcurve((bs3_curve)newBaseCurve, 0.0, SpaAcis::NullObj::get_surface(), SpaAcis::NullObj::get_surface());
    constant_law* v20 = ACIS_NEW constant_law(0.0);
    SPAinterval tmp = ((curve&)baseCurve).param_range();
    curve* newOffsetCurve = sg_offset_planar_curve(*v17, tmp, SPAresfit, iDistLaw, v20, iNormal, 0, SPAresabs);
    v20->remove();

    if(v17) v17 = nullptr;

    if(acis_interrupted()) sys_error(0, (error_info_base*)nullptr);  // 不确定

    return newOffsetCurve;
}

void display_segment_trim_status(offset_segment_list& iSegList) {
    return;
}
int coedge_overlap_with_next_coedge(COEDGE* iCoedge, SPAposition& iCheckPosition) {
    int coedgeOpverlap = 0;
    if(iCoedge) {
        COEDGE* nextCoedge = iCoedge->next();
        if(nextCoedge) {
            if(nextCoedge != iCoedge) {
                if(is_STRAIGHT(iCoedge->edge()->geometry())) {
                    if(is_STRAIGHT(nextCoedge->edge()->geometry())) {
                        SPAposition p1 = iCoedge->start_pos();
                        SPAposition p2 = nextCoedge->end_pos();
                        return (p1 == iCheckPosition) || (p2 == iCheckPosition);
                    }
                }
            }
        }
    }
    return coedgeOpverlap;
}
int consecutive_coedge_overlap(COEDGE* iCoedge, SPAposition& iCheckPosition) {
    int retVal = 0;
    COEDGE* prevCoedge = iCoedge->previous();
    if(prevCoedge != iCoedge) retVal = coedge_overlap_with_next_coedge(prevCoedge, iCheckPosition) != 0;
    if(!retVal) {
        COEDGE* nextCoedge = iCoedge->next();
        if(prevCoedge != nextCoedge && nextCoedge != iCoedge && coedge_overlap_with_next_coedge(iCoedge, iCheckPosition)) return 2;
    }
    return retVal;
}
void delete_overlap_coedges(COEDGE*& ioCoedge1, COEDGE*& ioCoedge2) {
    /*  if(!*ioCoedge1 || !*ioCoedge2) _wassert(L"ioCoedge1 && ioCoedge2", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\trmofwi.cpp", 0x1A9Bu);*/
    VERTEX* midVrt = ioCoedge1->end();
    EDGE* v2 = ioCoedge1->edge();
    midVrt->delete_edge(v2);
    EDGE* v3 = ioCoedge2->edge();
    midVrt->delete_edge(v3);
    VERTEX* v8 = ioCoedge1->start();
    EDGE* edge = ioCoedge1->edge();
    v8->delete_edge(edge);
    VERTEX* v10 = ioCoedge2->end();
    EDGE* v9 = ioCoedge2->edge();
    v10->delete_edge(v9);
    EDGE* v5 = ioCoedge1->edge();
    v5->lose();
    ioCoedge1->lose();
    EDGE* v6 = ioCoedge2->edge();
    v6->lose();
    ioCoedge2->lose();
    midVrt->lose();
    ioCoedge1 = nullptr;
    ioCoedge2 = nullptr;
}

void remove_overlap_coedges(COEDGE* overLapCoedge1) {
    COEDGE* nextCoedge;
    COEDGE* overLapCoedge1a = overLapCoedge1;
    COEDGE* prevCoedge = overLapCoedge1a->previous();
    COEDGE* overLapCoedge2 = overLapCoedge1a->next();
    if(overLapCoedge2)
        nextCoedge = overLapCoedge2->next();
    else
        nextCoedge = nullptr;
    if(prevCoedge && nextCoedge) {
        prevCoedge->set_next(nextCoedge, 0, 1);
        nextCoedge->set_previous(prevCoedge, 0, 1);
        if(GET_ALGORITHMIC_VERSION() >= AcisVersion(31, 0, 1)) {
            LOOP* owner = (LOOP*)overLapCoedge1a->owner();
            if(is_LOOP(owner)) {
                if(owner->start() == overLapCoedge1a || owner->start() == overLapCoedge2) owner->set_start(nextCoedge, 1);
            } else if(is_WIRE(owner)) {
                COEDGE* v4 = ((WIRE*)owner)->coedge();
                if(v4 == overLapCoedge1a || ((WIRE*)owner)->coedge() == overLapCoedge2) ((WIRE*)owner)->set_coedge(nextCoedge);
            }
        }
        delete_overlap_coedges(overLapCoedge1a, overLapCoedge2);
        EDGE* e1 = prevCoedge->edge();
        VERTEX* v1 = prevCoedge->end();
        VERTEX* v2 = nextCoedge->start();
        /*if(!e1 || !v1 || !v2) _wassert(L"e1 && v1 && v2", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\trmofwi.cpp", 0x1AD5u);*/  // 不确定
        v1->set_edge(nullptr, 1);
        if(prevCoedge->sense())
            e1->set_start(v2, 1, 1);
        else
            e1->set_end(v2, 1, 1);
        v1->lose();
    }
}

int remove_consecutive_overlaps(BODY* wire_body) {
    ENTITY_LIST shell_list;
    ENTITY_LIST coedge_list;  // 不确定，上面有ENTITY_LIST*
    get_shells(wire_body, shell_list, PAT_CAN_CREATE);
    int removeDone = 0;
    ENTITY_LIST overlapCoedges;
    for(int ii = 0; ii < shell_list.count(); ++ii) {
        SHELL* a_shell = (SHELL*)shell_list[ii];
        get_coedges(a_shell, coedge_list, PAT_CAN_CREATE);
        for(int ij = 0; ij < coedge_list.count(); ++ij) {
            ATTRIB_GEN_REAL* named_att = (ATTRIB_GEN_REAL*)find_named_attrib(coedge_list[ij], "tee_intersection");
            while(named_att) {
                if(named_att->isa(ATTRIB_GEN_REAL::id())) {
                    COEDGE* thisCoedge = (COEDGE*)coedge_list[ij];
                    double crrSplitParam = named_att->value();
                    SPAparameter v2 = thisCoedge->start_param();
                    double sParam = v2.operator double();
                    SPAparameter v3 = thisCoedge->end_param();
                    double eParam = v3.operator double();
                    if(crrSplitParam > sParam + SPAresabs) {
                        if(eParam - SPAresabs > crrSplitParam) {
                            SPAposition splitPos = coedge_param_pos(thisCoedge, crrSplitParam);
                            int needSplit = consecutive_coedge_overlap(thisCoedge, splitPos);
                            if(needSplit) {
                                split_coedge_at_params(thisCoedge, 1, &crrSplitParam);
                                COEDGE* startOverLapCoedge = thisCoedge->next();
                                if(needSplit == 1) startOverLapCoedge = thisCoedge->previous();
                                removeDone = 1;
                                overlapCoedges.add(startOverLapCoedge, 1);
                                overlapCoedges.add(startOverLapCoedge->next(), 1);
                            }
                        }
                    }
                }
                ATTRIB_GEN_REAL* prev_att = named_att;
                named_att = (ATTRIB_GEN_REAL*)find_next_named_attrib(named_att);
                prev_att->lose();
            }
        }
        coedge_list.clear();
    }
    overlapCoedges.init();
    for(int i = 0; i < overlapCoedges.count(); i += 2) {
        remove_overlap_coedges((COEDGE*)overlapCoedges[i]);
    }
    overlapCoedges.clear();
    return removeDone;
}

void sg_add_offset_coedge(WIRE*& current_wire, WIRE* wire_list, COEDGE* offset_coedge) {
    SPAposition test_pos(offset_coedge->start()->geometry()->coords());
    COEDGE* curr_coedge = current_wire->coedge();
    COEDGE* last_coedge = curr_coedge->previous();
    SPAposition end_pos(last_coedge->end()->geometry()->coords());
    if(!(test_pos == end_pos)) {
        SPAposition testPos2(offset_coedge->end()->geometry()->coords());
        SPAposition start_pos(last_coedge->start()->geometry()->coords());
        if(testPos2 == end_pos && test_pos == start_pos) return;
        for(current_wire = wire_list; current_wire; current_wire = current_wire->next(PAT_CAN_CREATE)) {
            curr_coedge = current_wire->coedge();
            COEDGE* last_coedge = curr_coedge->previous();
            SPAposition end_pos = last_coedge->end()->geometry()->coords();
            if(test_pos == end_pos) break;
        }
    }
    if(current_wire) {
        offset_coedge->set_wire(current_wire, 1);
        curr_coedge->set_previous(offset_coedge, 0, 1);
        last_coedge->set_next(offset_coedge, 0, 1);
        offset_coedge->set_previous(last_coedge, 0, 1);
        offset_coedge->set_next(curr_coedge, 0, 1);
        if(offset_coedge->start() != last_coedge->end()) {
            if(offset_coedge->sense()) {
                offset_coedge->edge()->set_end(last_coedge->end(), 1, 1);
            } else {
                offset_coedge->edge()->set_start(last_coedge->end(), 1, 1);
            }
        }
        offset_coedge->end()->set_edge(offset_coedge->edge(), 1);
    } else {
        WIRE* last_wire = wire_list;
        for(WIRE* i = wire_list->next(PAT_CAN_CREATE); i; i = last_wire->next(PAT_CAN_CREATE)) last_wire = last_wire->next(PAT_CAN_CREATE);  // 不确定
        offset_coedge->set_next(offset_coedge, 0, 1);
        offset_coedge->set_previous(offset_coedge, 0, 1);
        WIRE* v23 = ACIS_NEW WIRE(offset_coedge, nullptr);
        last_wire->set_next(v23, 1);
        current_wire = v23;
        offset_coedge->set_wire(current_wire, 1);
        offset_coedge->start()->set_edge(offset_coedge->edge(), 1);
        offset_coedge->end()->set_edge(offset_coedge->edge(), 1);
    }
}

offset_segment* get_prev_out_seg(offset_segment*& curr_seg, offset_segment_list* seg_list) {
    offset_segment* prev_seg = curr_seg->previous();
    do {
        if(prev_seg->start_out() && prev_seg->end_out()) break;
        if(prev_seg->singular_offset()) break;
        prev_seg = prev_seg->previous();
    } while(prev_seg);
    offset_segment* segment = seg_list->first_segment();
    if(segment->previous() == prev_seg)
        return nullptr;
    else
        return prev_seg;
}

int trim_coedge_0(VERTEX*& ioSplitVertex, const SPAposition& iSplitPos, SPAparameter& iParam, int iKeepFirstPart, COEDGE*& ioCoedge) {
    int retOk = 0;
    if(!ioCoedge || !ioCoedge->edge() || !ioSplitVertex) return 0;
    CURVE* v15 = ioCoedge->edge()->geometry();
    SPAtransf v37;
    curve* ioCurve = v15->trans_curve(v37, 0);  // 不确定   CURVE*curve
    SPAinterval v32 = ioCurve->param_range();
    if(v32 >> iParam.operator double()) {
        SPAinterval newRange;
        if(ioCoedge->sense() == ioCoedge->edge()->sense()) {
            if(iKeepFirstPart) {
                SPAinterval v35(v32.start_pt(), iParam.operator double());
                newRange = v35;
            } else {
                SPAinterval v36(iParam.operator double(), v32.end_pt());
                newRange = v36;
            }
        } else if(iKeepFirstPart) {
            SPAinterval v33(iParam.operator double(), v32.end_pt());
            newRange = v33;
        } else {
            SPAinterval v34(v32.start_pt(), iParam.operator double());
            newRange = v34;
        }
        ioCurve->limit(newRange);
        CURVE* curve1 = make_curve(*ioCurve);
        APOINT* v17 = ACIS_NEW APOINT(iSplitPos);
        ioSplitVertex->set_geometry(v17, 1);
        ioCoedge->edge()->set_geometry(curve1, 1);
        if(ioCurve) ioCurve = nullptr;
        return 1;
    }
    return retOk;
}

offset_segment* get_next_out_seg(offset_segment*& curr_seg, offset_segment_list* seg_list) {
    offset_segment* next_seg = curr_seg->next();
    do {
        if(next_seg->start_out() && next_seg->end_out()) break;
        if(next_seg->singular_offset()) break;
        next_seg = next_seg->next();
    } while(next_seg);
    offset_segment* segment = seg_list->last_segment();
    if(segment->next() == next_seg)
        return nullptr;
    else
        return next_seg;
}

int trim_edge(offset_segment_list* seg_list, bool start_too_close, int iNumEdges, EDGE*& edgeToTrim) {
    offset_segment* curr_seg = seg_list->first_segment();
    offset_segment* segment = seg_list->last_segment();
    offset_segment* last_seg = segment->next();
    while(curr_seg != last_seg) {
        if(curr_seg->coedge()) {
            EDGE* tmp_edge = curr_seg->coedge()->edge();
            if(tmp_edge == edgeToTrim) break;
        }
        curr_seg = curr_seg->next();
    }
    int required_trim_point_found = 0;
    double param = 0.0;
    SPAinterval oldRange = edgeToTrim->param_range();
    SPAposition inPoint;
    SPAtransf v105;
    curve* curve_to_trim = edgeToTrim->geometry()->trans_curve(v105);
    while(!required_trim_point_found) {
        offset_segment* adjacent_seg = nullptr;
        COEDGE* adjacent_coedge = nullptr;
        SPAunit_vector coedge_dir;
        SPAposition center;
        if(start_too_close) {
            adjacent_seg = get_prev_out_seg(curr_seg, seg_list);
            if(!adjacent_seg) break;
            adjacent_coedge = adjacent_seg->original_coedge();
            coedge_dir = coedge_start_dir(adjacent_coedge);
            center = adjacent_coedge->start_pos();
        } else {
            adjacent_seg = get_next_out_seg(curr_seg, seg_list);
            if(!adjacent_seg) break;
            adjacent_coedge = adjacent_seg->original_coedge();
            coedge_dir = coedge_end_dir(adjacent_coedge);
            center = adjacent_coedge->end_pos();
        }
        const law* dist_law = seg_list->distance();

        double offsetDist = ((law*)dist_law)->eval(1.0);
        double checkDist = fabs(offsetDist);
        double ratio = 1.0;
        const SPAvector& v10 = (seg_list->normal()) * coedge_dir;
        const SPAvector& maj_axis = v10 * offsetDist;
        const SPAvector& v11 = coedge_dir * maj_axis;
        const SPAunit_vector& cir_normal = normalise(v11);
        ellipse circle(center, cir_normal, maj_axis, ratio, 0.0);
        // if(Debug_Break_Active("trim_wire", "WIRE-OFFSET")) {
        //     if(get_breakpoint_callback()) {
        //         breakpoint_callback = get_breakpoint_callback();
        //         new_render_object = breakpoint_callback->new_render_object;
        //         v41 = new_render_object(breakpoint_callback, 1);
        //     } else {
        //         v41 = 0i64;
        //     }
        //     pRO = v41;
        //     if(v41) {
        //         next_edge = COEDGE::edge(adjacent_coedge);
        //         show_entity_with_text(next_edge, "adjacent edge", GREEN, pRO, 1);
        //         show_entity_with_text(*edgeToTrim, "edge to trim", MAGENTA, pRO, 1);
        //         box = SpaAcis::NullObj::get_box();
        //         ellipse::param_range(&circle, &v95, box);
        //         circle_param = &v95;
        //         ent_with_transf = *edgeToTrim;
        //         end_param = SPAinterval::end_pt(&v95);
        //         start_param = SPAinterval::start_pt((SPAinterval*)circle_param);
        //         show_curve(&circle, start_param, end_param, RED, ent_with_transf, pRO);
        //         acis_fprintf(debug_file_ptr, "trim_wire before expand\n");
        //         Debug_Break("trim_wire", "WIRE-OFFSET", "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\trmofwi.cpp", 5617);
        //         if(get_breakpoint_callback()) {
        //             v43 = get_breakpoint_callback();
        //             delete_render_object = v43->delete_render_object;
        //             delete_render_object(v43, pRO);
        //         }
        //     }
        // }

        curve_curve_int* intersections = int_cur_cur(*curve_to_trim, circle, SpaAcis::NullObj::get_box(), SPAresabs * 0.1);
        if(intersections) {
            const curve_curve_int* tmpInts = intersections;
            int param_in_bound_found = 0;
            const WIRE* baseWire = seg_list->get_base_wire();
            while(tmpInts) {
                inPoint = tmpInts->int_point;
                double param = curve_to_trim->param(inPoint);
                if(oldRange >> param) {
                    param_in_bound_found = 1;
                    SPAposition basePos;
                    int iSmallTol = iNumEdges == 1;
                    if(!wire_dist_too_close(inPoint, (WIRE*)baseWire, checkDist, basePos, iSmallTol)) {
                        required_trim_point_found = 1;
                        break;
                    }
                    tmpInts = tmpInts->next;
                } else {
                    tmpInts = tmpInts->next;
                }
            }
            delete_curve_curve_ints(intersections);
            intersections = nullptr;
            if(!param_in_bound_found) {
                if(curr_seg->original_coedge()) {
                    if(curve_to_trim) {
                        curve_to_trim = nullptr;
                    }

                    return 0;
                }
            }
        }
        curr_seg = adjacent_seg;
    }
    if(required_trim_point_found) {
        int iKeepFirstPart = 0;
        VERTEX* vert = edgeToTrim->coedge()->start();
        COEDGE* coedgeToTrim = edgeToTrim->coedge();
        ENTITY_LIST split_edges;
        if(!start_too_close) {
            iKeepFirstPart = 1;
            vert = edgeToTrim->coedge()->end();
        }
        curve* subsetcurve = curve_to_trim->subset(oldRange);
        (edgeToTrim)->set_geometry(make_curve(*subsetcurve), 1);
        SPAparameter iParam(param);
        int retOk = trim_coedge_0(vert, inPoint, iParam, iKeepFirstPart, coedgeToTrim);  // 不确定，llvm里没有
        // if(Debug_Break_Active("trim_wire", "WIRE-OFFSET")) {
        //     if(get_breakpoint_callback()) {
        //         v45 = get_breakpoint_callback();
        //         v82 = v45->new_render_object;
        //         v46 = v82(v45, 1);
        //     } else {
        //         v46 = 0i64;
        //     }
        //     v34 = v46;
        //     if(v46) {
        //         show_entity(*edgeToTrim, MAGENTA, v34);
        //         acis_fprintf(debug_file_ptr, "trim_wire after trim\n");
        //         Debug_Break("trim_wire", "WIRE-OFFSET", "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\trmofwi.cpp", 5709);
        //         if(get_breakpoint_callback()) {
        //             v47 = get_breakpoint_callback();
        //             v83 = v47->delete_render_object;
        //             v83(v47, v34);
        //         }
        //     }
        // }
        if(curve_to_trim) {
            curve_to_trim = nullptr;
        }
        if(subsetcurve) {
            subsetcurve = nullptr;
        }
        return 1;
    } else {
        if(curve_to_trim) {
            curve_to_trim = nullptr;
        }

        return 0;
    }
}

void remove_edge_from_wire(WIRE*& wire_list, EDGE* edge_to_remove) {
    WIRE* curr_wire = wire_list;
    WIRE* previous_wire = nullptr;
    while(curr_wire) {
        ENTITY_LIST edges;
        get_edges(curr_wire, edges, PAT_CAN_CREATE);
        int edgeCount = edges.iteration_count();
        int found = 0;
        COEDGE* first_coedge = curr_wire->coedge();
        COEDGE* cur_coedge = first_coedge;
        COEDGE* next;
        COEDGE* previous;
        while(cur_coedge->edge() != edge_to_remove) {
            cur_coedge = cur_coedge->next();
            if(cur_coedge == first_coedge) goto LABEL_12;
        }
        next = cur_coedge->next();
        previous = cur_coedge->previous();
        if(edgeCount == 1) {
            curr_wire->set_coedge(nullptr);
        } else {
            if(cur_coedge == first_coedge) curr_wire->set_coedge(next);
            previous->set_next(next, 0, 1);
            next->set_previous(previous, 0, 1);
        }
        found = 1;
    LABEL_12:
        if(found == 1 && edgeCount == 1) {
            WIRE* v11 = curr_wire->next(PAT_CAN_CREATE);
            if(previous_wire)
                previous_wire->set_next(v11, 1);
            else
                wire_list = v11;
        }
        if(found == 1) {
            return;
        }
        previous_wire = curr_wire;
        curr_wire = curr_wire->next(PAT_CAN_CREATE);
    }
}

void trim_the_wire(offset_segment_list* seg_list, int wire_closed, WIRE*& wire_list) {
    WIRE* curr_wire = wire_list;
    law* dist_law = seg_list->distance();
    double offsetDist = dist_law->eval(1.0);
    double checkDist = fabs(offsetDist);
    const WIRE* baseWire = seg_list->get_base_wire();
    /* if(Debug_Break_Active("trim_wire", "WIRE-OFFSET")) {
         if(get_breakpoint_callback()) {
             breakpoint_callback = get_breakpoint_callback();
             new_render_object = breakpoint_callback->new_render_object;
             v23 = new_render_object(breakpoint_callback, 1);
         } else {
             v23 = 0i64;
         }
         pRO = v23;
         if(v23) {
             ENTITY_LIST::ENTITY_LIST(&edges);
             get_edges(*wire_list, &edges, PAT_CAN_CREATE);
             ENTITY_LIST::init(&edges);
             while(1) {
                 ed = (EDGE*)ENTITY_LIST::next(&edges);
                 if(!ed) break;
                 show_entity(ed, MAGENTA, pRO);
             }
             acis_fprintf(debug_file_ptr, "trim_wire_input\n");
             Debug_Break("trim_wire", "WIRE-OFFSET", "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\trmofwi.cpp", 5796);
             if(get_breakpoint_callback()) {
                 v25 = get_breakpoint_callback();
                 delete_render_object = v25->delete_render_object;
                 delete_render_object(v25, pRO);
             }
             ENTITY_LIST::~ENTITY_LIST(&edges);
         }
     }*/
    while(curr_wire) {
        ENTITY_LIST edge_list;
        get_edges(curr_wire, edge_list, PAT_CAN_CREATE);
        int iNumEdges = edge_list.iteration_count();
        COEDGE* v26 = ((EDGE*)edge_list[0])->coedge();
        SPAposition startPos = v26->start_pos();
        COEDGE* v27 = ((EDGE*)edge_list[iNumEdges - 1])->coedge();
        SPAposition endPos = v27->end_pos();
        SPAposition basePos;
        int iSmallTol = iNumEdges == 1;
        int start_too_close = wire_dist_too_close(startPos, (WIRE*)baseWire, checkDist, basePos, iSmallTol);
        int v16 = iNumEdges == 1;
        int end_too_close = wire_dist_too_close(endPos, (WIRE*)baseWire, checkDist, basePos, v16);
        /*       if(Debug_Break_Active("trim_wire", "WIRE-OFFSET")) {
                   if(get_breakpoint_callback()) {
                       v30 = get_breakpoint_callback();
                       v43 = v30->new_render_object;
                       v31 = v43(v30, 1);
                   } else {
                       v31 = 0i64;
                   }
                   v20 = v31;
                   if(v31) {
                       v5 = ENTITY_LIST::operator[](&edge_list, 0);
                       show_entity_with_text(v5, "first_edge", GREEN, v20, 1);
                       v6 = ENTITY_LIST::operator[](&edge_list, iNumEdges - 1);
                       show_entity_with_text(v6, "last_edge", MAGENTA, v20, 1);
                       acis_fprintf(debug_file_ptr, "trim_wire\n");
                       Debug_Break("trim_wire", "WIRE-OFFSET", "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\trmofwi.cpp", 5822);
                       if(get_breakpoint_callback()) {
                           v32 = get_breakpoint_callback();
                           v44 = v32->delete_render_object;
                           v44(v32, v20);
                       }
                   }
               }*/
        if(start_too_close || end_too_close) {
            edge_list.init();
            int removed_edges = 0;
            if(start_too_close) {
                int res = 0;
                int cnt = 0;
                while(!res && iNumEdges) {
                    EDGE* edgeToTrim = (EDGE*)edge_list[cnt];
                    res = trim_edge(seg_list, 1, iNumEdges, edgeToTrim);
                    if(!res) {
                        remove_edge_from_wire(wire_list, edgeToTrim);
                        --iNumEdges;
                        ++removed_edges;
                    }
                    ++cnt;
                }
            }
            if(end_too_close) {
                int v12 = 0;
                int index = removed_edges + iNumEdges - 1;
                while(!v12 && iNumEdges) {
                    EDGE* edge_to_remove = (EDGE*)edge_list[index];
                    v12 = trim_edge(seg_list, 0, iNumEdges, edge_to_remove);
                    if(!v12) {
                        remove_edge_from_wire(wire_list, edge_to_remove);
                        --iNumEdges;
                    }
                    --index;
                }
            }
        }
        curr_wire = curr_wire->next(PAT_CAN_CREATE);
    }
}

int is_common_point(SPAposition& common, offset_segment_list& segment_list) {
    double v2 = 2.0 * SPAresabs;
    offset_segment* this_seg = segment_list.first_segment();
    offset_segment* segment = segment_list.last_segment();
    offset_segment* last_seg = segment->next();
    do {
        if(this_seg->original_coedge()) {
            if(!same_point(common, this_seg->coedge()->start()->geometry()->coords(), v2)) {
                if(!same_point(common, this_seg->coedge()->end()->geometry()->coords(), v2)) return 0;
            }
        }
        this_seg = this_seg->next();
    } while(this_seg != last_seg);
    return 1;
}

void sg_remove_attributes(WIRE* this_wire) {
    // acis_exception error_info_holder(0);
    // error_info_base* error_info_base_ptr = nullptr;
    // exception_save exception_save_mark;
    // ENTITY_LIST new_list;
    // exception_save_mark.begin();
    // get_error_mark().buffer_init = 1;
    // sg_seed_for_delete(this_wire, new_list);
    // int count = new_list.count();
    // for(int i = 0; i < count; ++i) {
    //     ENTITY* this_ent = new_list[i];
    //     if(this_ent->identity(1) == COEDGE_TYPE) {
    //         ATTRIB_OFFREL* cur_attrib = (ATTRIB_OFFREL*)find_attrib(this_ent, ATTRIB_SG_TYPE, ATTRIB_OFFREL_TYPE, -1, -1);//不懂
    //         if(cur_attrib) cur_attrib->lose();
    //     }
    // }
    // if(acis_interrupted()) sys_error(0, error_info_base_ptr);
}

void sg_clean_and_close_wires(WIRE*& wire_list, ENTITY_LIST& delete_list, int orig_wire_closed) {
    WIRE* prev_wire = nullptr;
    for(WIRE* this_wire = wire_list; this_wire; this_wire = this_wire->next(PAT_CAN_CREATE)) {
        COEDGE* first_coedge = this_wire->coedge();
        COEDGE* last_coedge = first_coedge->previous();
        SPAposition start_pos(first_coedge->start()->geometry()->coords());
        SPAposition end_pos(last_coedge->end()->geometry()->coords());
        if(start_pos == end_pos) {
            if(last_coedge->end() != first_coedge->start()) {
                if(last_coedge->sense()) {
                    last_coedge->edge()->set_start(first_coedge->start(), 1, 1);
                } else {
                    last_coedge->edge()->set_end(first_coedge->start(), 1, 1);
                }
                first_coedge->start()->set_edge(last_coedge->edge(), 1);
            }
        } else if(orig_wire_closed) {
            if(wire_list == this_wire) {
                wire_list = this_wire->next(PAT_CAN_CREATE);
                sg_remove_attributes(this_wire);
                sg_seed_for_delete(this_wire, delete_list);
            } else {
                prev_wire->set_next(this_wire->next(PAT_CAN_CREATE), 1);
                sg_remove_attributes(this_wire);
                sg_seed_for_delete(this_wire, delete_list);
                this_wire = prev_wire;
            }
        } else {
            first_coedge->set_previous(first_coedge, 0, 1);
            last_coedge->set_next(last_coedge, 0, 1);
        }
        prev_wire = this_wire;
    }
}

// 输入wire_body，偏置表，0,0
void sg_collect_segments(BODY* wire_body, offset_segment_list& seg_list, int zero_length, int iOverlap) {
    ENTITY_LIST vlist;
    get_vertices(wire_body, vlist, PAT_CAN_CREATE);
    for(int ii = 0; ii < vlist.count(); ++ii) {
        VERTEX* v147 = (VERTEX*)vlist[ii];
        v147->set_edge(nullptr, 1);
    }
    int resignal_no = 0;
    acis_exception error_info_holder(0);
    error_info_base* error_info_base_ptr = nullptr;
    exception_save exception_save_mark;
    ENTITY_LIST start_list;
    ENTITY_LIST finish_list;
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    int error_no = 0;
    ENTITY_LIST eds_coeds_verts0;
    get_entities(wire_body, eds_coeds_verts0, 5122, 0, PAT_CAN_CREATE);
    sg_seed_for_delete(wire_body, start_list);

    WIRE* wire_list = nullptr;
    WIRE* curr_wire = nullptr;
    offset_segment* this_seg = seg_list.first_segment();
    offset_segment* last_seg = seg_list.last_segment();
    for(int j = this_seg->extension(); j; j = this_seg->extension()) {
        seg_list.remove_segment(this_seg);
        void* alloc_ptr = this_seg;  // 不确定
        ACIS_DELETE this_seg;
        this_seg = seg_list.first_segment();
    }
    while(last_seg->extension()) {
        seg_list.remove_segment(last_seg);
        void* v149 = last_seg;
        ACIS_DELETE last_seg;
        last_seg = seg_list.last_segment();
    }

    SPAposition start_pos(this_seg->coedge()->start()->geometry()->coords());
    SPAposition end_pos(last_seg->coedge()->end()->geometry()->coords());
    int wire_closed = start_pos == end_pos;
    int needCleanCloseWire = 1;
    if(GET_ALGORITHMIC_VERSION() > AcisVersion(17, 0, 0)) {
        ofst_wires_extractor wireExtractor;
        int iRemoveOverlap = !iOverlap && wire_closed;
        wireExtractor.extract(&seg_list, wire_list, wire_closed, iRemoveOverlap);  // 看不懂
        needCleanCloseWire = wireExtractor.get_overlap_status() != 2;
    }
    if(!(GET_ALGORITHMIC_VERSION() > AcisVersion(17, 0, 0)) || !wire_list && !wire_closed) {
        last_seg = last_seg->next();
        while(this_seg != last_seg) {
            if(this_seg->outside()) {
                COEDGE* this_coedge = this_seg->coedge();
                if(seg_list.add_attribs()) {
                    ATTRIB_OFFREL* v107 = ACIS_NEW ATTRIB_OFFREL(this_coedge, *this_seg);  // 不确定
                    this_coedge->set_attrib(v107);
                }
                if(wire_list) {
                    sg_add_offset_coedge(curr_wire, wire_list, this_coedge);
                } else {
                    this_coedge->set_next(this_coedge, 0, 1);
                    this_coedge->set_previous(this_coedge, 0, 1);
                    WIRE* v109 = ACIS_NEW WIRE(this_coedge, nullptr);
                    wire_list = v109;
                    curr_wire = v109;  //
                    this_coedge->set_wire(v109, 1);
                    this_coedge->start()->set_edge(this_coedge->edge(), 1);
                    this_coedge->end()->set_edge(this_coedge->edge(), 1);
                }
            }
            this_seg = this_seg->next();
        }
    }

    int numOf_org_edges = seg_list.get_number_of_offset_coedges();
    if(GET_ALGORITHMIC_VERSION() > AcisVersion(32, 0, 1) && wire_list && numOf_org_edges > 1) {
        int num_out_coedge = 0;
        offset_segment* curr_seg = seg_list.first_segment();
        offset_segment* segment = seg_list.last_segment();
        offset_segment* last_seg1 = segment->next();
        while(curr_seg != last_seg1) {
            if(curr_seg->end_out() && curr_seg->start_out()) ++num_out_coedge;
            curr_seg = curr_seg->next();
        }
        int num_edge_in_wire_list = 0;
        WIRE* curr_wire1 = wire_list;
        while(curr_wire1) {
            ENTITY_LIST edges;
            get_edges(curr_wire1, edges, PAT_CAN_CREATE);
            int v20 = edges.iteration_count();
            num_edge_in_wire_list += v20;
            curr_wire1 = curr_wire1->next(PAT_CAN_CREATE);
        }
        if(num_out_coedge != num_edge_in_wire_list) trim_the_wire(&seg_list, wire_closed, wire_list);  // 不确定
    }

    COEDGE* degen_coed = nullptr;
    if(zero_length && !wire_list) {
        int f_common = 0;
        SPAposition common;
        offset_segment* this_sega = seg_list.first_segment();
        last_seg = seg_list.last_segment()->next();
        do {
            if(this_sega->original_coedge()) {
                common = this_sega->coedge()->start()->geometry()->coords();
                f_common = is_common_point(common, seg_list);
                if(!f_common) {
                    common = this_sega->coedge()->end()->geometry()->coords();
                    f_common = is_common_point(common, seg_list);
                }
                if(f_common == 1) break;
            }
            this_sega = this_sega->next();
        } while(this_sega != last_seg);
        if(f_common) {
            APOINT* v115 = ACIS_NEW APOINT(common);
            VERTEX* v117 = ACIS_NEW VERTEX(v115);
            VERTEX* start_v = v117;
            APOINT* v119 = ACIS_NEW APOINT(common);
            VERTEX* v121 = ACIS_NEW VERTEX(v119);
            VERTEX* end_v = v121;
            EDGE* v123 = ACIS_NEW EDGE(start_v, end_v, nullptr, 0, EDGE_cvty_unknown);
            EDGE* degen_edge = v123;
            COEDGE* v125 = ACIS_NEW COEDGE(degen_edge, 0, nullptr, nullptr);
            degen_coed = v125;

            degen_coed->set_next(degen_coed, 0, 1);
            degen_coed->set_previous(degen_coed, 0, 1);
            degen_coed->start()->set_edge(degen_coed->edge(), 1);
            degen_coed->end()->set_edge(degen_coed->edge(), 1);
            if(seg_list.add_attribs()) {
                offset_segment* this_segb = seg_list.first_segment();
                ATTRIB_OFFREL* v127 = ACIS_NEW ATTRIB_OFFREL(degen_coed, *this_segb);
                degen_coed->set_attrib(v127);
            }
        }
    }

    int orig_wire_closed = wire_closed && needCleanCloseWire;
    sg_clean_and_close_wires(wire_list, start_list, orig_wire_closed);
    SHELL* prev_shell = nullptr;
    WIRE* next_wire;
    for(curr_wire = wire_list; curr_wire; curr_wire = next_wire) {
        next_wire = curr_wire->next(PAT_CAN_CREATE);
        SHELL* v129 = ACIS_NEW SHELL(curr_wire, nullptr, prev_shell);
        prev_shell = v129;
        curr_wire->set_next(nullptr, 1);
    }
    LUMP* v131 = ACIS_NEW LUMP(prev_shell, wire_body->lump());
    LUMP* lump = v131;
    lump->set_body(wire_body, 1);
    wire_body->set_lump(lump, 1);
    wire_body->set_wire(nullptr, 1);
    /* if(Debug_Break_Active("sg_collect_segments", "WIRE-OFFSET")) {
         if(get_breakpoint_callback()) {
             v133 = get_breakpoint_callback();
             v183 = v133->new_render_object;
             v134 = v183(v133, 1);
         } else {
             v134 = 0i64;
         }
         v96 = v134;
         if(v134) {
             if(BODY::lump(wire_body)) {
                 v37 = BODY::lump(wire_body);
                 if(LUMP::shell(v37)) {
                     v38 = BODY::lump(wire_body);
                     v39 = LUMP::shell(v38);
                     tmp_wire = SHELL::wire(v39);
                     while(tmp_wire) {
                         ENTITY_LIST::ENTITY_LIST(&edge_list);
                         get_edges(tmp_wire, &edge_list, PAT_CAN_CREATE);
                         for(i = 0;; ++i) {
                             v40 = ENTITY_LIST::count(&edge_list);
                             if(i >= v40) break;
                             e = ENTITY_LIST::operator[](&edge_list, i);
                             show_entity(e, GREEN, v96);
                         }
                         tmp_wire = WIRE::next(tmp_wire, PAT_CAN_CREATE);
                         ENTITY_LIST::~ENTITY_LIST(&edge_list);
                     }
                     Debug_Break("sg_collect_segments", "WIRE-OFFSET", "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\trmofwi.cpp", 6146);
                 }
             }
             if(get_breakpoint_callback()) {
                 v135 = get_breakpoint_callback();
                 v185 = v135->delete_render_object;
                 v185(v135, v96);
             }
         }
     }*/
    ENTITY_LIST eds_coeds_verts1;
    get_entities(wire_body, eds_coeds_verts1, 5122, 0, PAT_CAN_CREATE);
    ENTITY_LIST orphans;
    for(ENTITY* ent = eds_coeds_verts0.first(); ent; ent = eds_coeds_verts0.next()) {
        if(eds_coeds_verts1.lookup(ent) == -1) orphans.add(ent, 1);
    }
    for(ENTITY* owner = orphans.first(); owner; owner = orphans.next()) {
        ATTRIB* attr = find_attrib(owner, ATTRIB_CT_TYPE, ATTRIB_SPACOLLECTION_TYPE, -1, -1);
        while(attr) {
            ATTRIB* temp = attr;
            attr = find_next_attrib(attr, ATTRIB_CT_TYPE, ATTRIB_SPACOLLECTION_TYPE, -1, -1);
            temp->lose();
        }
    }
    sg_seed_for_delete(wire_body, finish_list);
    /*  if(Debug_Break_Active("sg_collect_segments", "WIRE-OFFSET")) {
          if(get_breakpoint_callback()) {
              v137 = get_breakpoint_callback();
              v186 = v137->new_render_object;
              v138 = v186(v137, 1);
          } else {
              v138 = 0i64;
          }
          v98 = v138;
          if(v138) {
              if(BODY::lump(wire_body)) {
                  v41 = BODY::lump(wire_body);
                  if(LUMP::shell(v41)) {
                      v42 = BODY::lump(wire_body);
                      v43 = LUMP::shell(v42);
                      v83 = SHELL::wire(v43);
                      while(v83) {
                          ENTITY_LIST::ENTITY_LIST(&v84);
                          get_edges(v83, &v84, PAT_CAN_CREATE);
                          for(index = 0;; ++index) {
                              v44 = ENTITY_LIST::count(&v84);
                              if(index >= v44) break;
                              v187 = ENTITY_LIST::operator[](&v84, index);
                              show_entity(v187, GREEN, v98);
                          }
                          v83 = WIRE::next((WIRE*)v83, PAT_CAN_CREATE);
                          ENTITY_LIST::~ENTITY_LIST(&v84);
                      }
                      Debug_Break("sg_collect_segments", "WIRE-OFFSET", "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\trmofwi.cpp", 6206);
                  }
              }
              if(get_breakpoint_callback()) {
                  v139 = get_breakpoint_callback();
                  v188 = v139->delete_render_object;
                  v188(v139, v98);
              }
          }
      }*/
    start_list.init();
    for(ENTITY* this_entity = start_list.next(); this_entity; this_entity = start_list.next()) {
        if(finish_list.lookup(this_entity) == -1) this_entity->lose();
    }
    if(degen_coed) {
        WIRE* v141 = ACIS_NEW WIRE(degen_coed, nullptr);
        WIRE* degen_wire = v141;
        degen_coed->set_wire(v141, 1);
        SHELL* v144 = ACIS_NEW SHELL(degen_wire, nullptr, nullptr);
        SHELL* v99 = v144;
        wire_body->lump()->set_shell(v99, 1);
        v99->set_lump(wire_body->lump(), 1);
    }
    if(resignal_no || acis_interrupted()) sys_error(resignal_no, error_info_base_ptr);
}

void close_coedges_at_vertex(generic_graph* base_graph, VERTEX* gvent) {
    int count;
    const gvertex* target = base_graph->find_vertex_by_entity(gvent, 0);
    gedge** adj_ges = base_graph->get_adjacent_edges(target, count);
    EDGE* v2 = (EDGE*)(*adj_ges)->get_entity();
    COEDGE* coed_1 = v2->coedge();
    if(count == 2) {
        EDGE* v3 = (EDGE*)adj_ges[1]->get_entity();
        COEDGE* coed_2 = v3->coedge();
        if(coed_1->start() == gvent) {
            coed_1->set_previous(coed_2, 0, 1);
            coed_2->set_next(coed_1, 0, 1);
        } else {
            coed_1->set_next(coed_2, 0, 1);
            coed_2->set_previous(coed_1, 0, 1);
        }
    }
    ENTITY_LIST edge_list;
    base_graph->get_entities_from_edge(edge_list);
    int index = 0;
    int add_edge = 1;
    for(EDGE* ref_edge = gvent->edge(0); ref_edge; ref_edge = gvent->edge(index)) {
        if(edge_list.lookup(ref_edge) > -1) {
            add_edge = 0;
            break;
        }
        ++index;
    }
    if(add_edge) {
        gvent->add_edge(coed_1->edge());
    }
    for(int ii = 0; ii < count; ++ii) adj_ges[ii]->remove();
    ACIS_DELETE[] adj_ges;
}

void repair_wire_at_branches(BODY* wire_body) {
    ENTITY_LIST edge_list;
    ENTITY_LIST shell_list;
    ENTITY_LIST ent_list;
    get_shells(wire_body, shell_list, PAT_CAN_CREATE);
    generic_graph** out_graphs = nullptr;
    SHELL* last_shell = sg_last_shell(wire_body->lump());
    SPAunit_vector test_uv;
    SPAposition test_pos;
    for(int ii = 0;; ++ii) {
        if(ii >= shell_list.count()) break;
        WIRE* a_wire = ((SHELL*)shell_list[ii])->wire();
        if(a_wire && is_planar_wire(a_wire, test_pos, test_uv, 1, 0)) {
            edge_list.clear();
            get_edges(shell_list[ii], edge_list, PAT_CAN_CREATE);
            generic_graph* body_graph = create_graph_from_edges(edge_list);
            int count = body_graph->split_branches(out_graphs);
            int v5 = count * count;
            int* connect = ACIS_NEW int[v5];
            int ik;
            for(int ij = 0; ij < count * count; ++ij) connect[ij] = 1;
            for(int ija = 0; ija < count - 1; ++ija) {
                connect[(count + 1) * ija] = 1;
                for(ik = ija + 1; ik < count; ++ik) {
                    generic_graph* v58 = out_graphs[ija];
                    generic_graph* int_graph = v58->intersect(out_graphs[ik]);
                    int_graph->get_entities_from_vertex(ent_list, 0);
                    if(ent_list.count() <= 0) {
                        connect[ik + count * ija] = -1;
                    } else {
                        connect[ik + count * ija] = ija;
                        for(int im = 0;; ++im) {
                            int v6 = ent_list.count();
                            if(im >= v6) break;
                            VERTEX* gvent = (VERTEX*)ent_list[im];
                            generic_graph* base_graph = out_graphs[ija];
                            close_coedges_at_vertex(base_graph, gvent);
                            VERTEX* v61 = (VERTEX*)ent_list[im];
                            generic_graph* v62 = out_graphs[ik];
                            close_coedges_at_vertex(v62, v61);
                        }
                        ent_list.clear();
                    }
                    int_graph->remove();
                }
            }
            int overlap = 0;
            COEDGE* a_coed;
            for(int ijb = 0; ijb < count; ++ijb) {
                ent_list.clear();
                generic_graph* v63 = out_graphs[ijb];
                v63->get_entities_from_edge(ent_list);
                if(ent_list.count() > 1) {
                    EDGE* v7 = (EDGE*)ent_list[0];
                    a_coed = v7->coedge();
                    int zero_area_loop = 0;
                    if(closed_coedge_chain(a_coed, ik)) {
                        SPAunit_vector nv;
                        find_planar_chain_normal(a_coed, nv, zero_area_loop);
                    }
                    if(zero_area_loop) {
                        overlap = 1;
                        connect[(count + 1) * ijb] = 0;
                        ENTITY_LIST vertices;
                        for(ik = 0;; ++ik) {
                            int v8 = ent_list.count();
                            if(ik >= v8) break;
                            EDGE* v9 = (EDGE*)ent_list[ik];
                            a_coed = v9->coedge();
                            VERTEX* v65 = a_coed->end();
                            EDGE* edge = a_coed->edge();
                            v65->delete_edge(edge);
                            VERTEX* v67 = a_coed->start();
                            EDGE* v66 = a_coed->edge();
                            v67->delete_edge(v66);
                            VERTEX* v10 = a_coed->start();
                            vertices.add(v10, 1);
                            VERTEX* v11 = a_coed->end();
                            vertices.add(v11, 1);
                            EDGE* v48 = a_coed->edge();
                            v48->lose();
                            a_coed->lose();
                        }
                        for(int ik = 0;; ++ik) {
                            if(ik >= vertices.count()) break;
                            VERTEX* avert = (VERTEX*)vertices[ik];
                            if(!avert->count_edges()) avert->lose();
                        }
                    }
                }
            }
            if(overlap) {
                int new_shell = 0;
                int zero_shell = 1;
                for(int ijc = 0; ijc < count; ++ijc) {
                    if(connect[(count + 1) * ijc] == 1) {
                        ent_list.clear();
                        generic_graph* v68 = out_graphs[ijc];
                        v68->get_entities_from_edge(ent_list);
                        EDGE* v13 = (EDGE*)ent_list[0];
                        a_coed = v13->coedge();
                        if(new_shell) {
                            SHELL* v50 = ACIS_NEW SHELL();
                            SHELL* add_shell = v50;
                            LUMP* v15 = wire_body->lump();
                            add_shell->set_lump(v15, 1);
                            WIRE* v52 = ACIS_NEW WIRE(a_coed, nullptr);
                            WIRE* v41 = v52;
                            for(int im = 0;; ++im) {
                                int v17 = ent_list.count();
                                if(im >= v17) break;
                                EDGE* v18 = (EDGE*)ent_list[im];
                                a_coed = v18->coedge();
                                a_coed->set_wire(v41, 1);
                            }
                            for(ik = ijc + 1; ik < count; ++ik) {
                                if(connect[ik + count * ijc] == ijc && connect[(count + 1) * ik] == 1) {
                                    connect[(count + 1) * ik] = 0;
                                    ent_list.clear();
                                    generic_graph* v71 = out_graphs[ik];
                                    v71->get_entities_from_edge(ent_list);
                                    for(int im = 0;; ++im) {
                                        int v19 = ent_list.count();
                                        if(im >= v19) break;
                                        EDGE* v20 = (EDGE*)ent_list[im];
                                        a_coed = v20->coedge();
                                        a_coed->set_wire(v41, 1);
                                    }
                                }
                            }
                            v41->set_shell(add_shell, 1);
                            add_shell->set_wire(v41, 1);
                            last_shell->set_next(add_shell, 1);
                            last_shell = add_shell;
                        } else {
                            WIRE* v72 = a_coed->wire();
                            v72->set_coedge(a_coed);
                            new_shell = 1;
                            for(ik = ijc + 1; ik < count; ++ik) {
                                if(connect[ik + count * ijc] == ijc) connect[(count + 1) * ik] = 0;
                            }
                        }
                        zero_shell = 0;
                    }
                }
                if(zero_shell) {
                    SHELL* empty_shell = (SHELL*)shell_list[ii];
                    extract_shell(empty_shell, 0);
                    WIRE* v54 = empty_shell->wire();
                    v54->lose();
                    empty_shell->lose();
                }
            }

            ACIS_DELETE[] STD_CAST connect;
            for(int ijd = 0; ijd < count; ++ijd) out_graphs[ijd]->remove();
            ACIS_DELETE[] STD_CAST out_graphs;
            body_graph->remove();
        }
    }
}

logical sg_trim_offset_wire(offset_segment_list& orig_list, int zero_length, int overlap) {
    int intersection_found = 0;
    offset_segment* segment = orig_list.first_segment();
    if(segment->coedge()) {
        ofst_edge_smooth_manager* edgeSmoothMgr = orig_list.get_edge_smooth_manager();  // 看不懂
        ENTITY* coedge_owner = orig_list.first_segment()->coedge()->owner();            // 获取偏置wire
        if(!is_WIRE(coedge_owner)) return 0;
        WIRE* this_wire = (WIRE*)coedge_owner;
        BODY* wire_body = nullptr;
        if(((WIRE*)coedge_owner)->body()) {
            wire_body = this_wire->body();
        } else {
            wire_body = this_wire->shell()->lump()->body();
        }
        // 不确定
        int resignal_no = 0;
        acis_exception error_info_holder(0);
        error_info_base* error_info_base_ptr = nullptr;
        exception_save exception_save_mark;
        law* dist = orig_list.distance();
        offset_segment_list segment_list(dist, orig_list.normal(), orig_list.add_attribs(), orig_list.get_gap_type(), 0);
        // 初始化一个segment_list
        law* local_dist_law = nullptr;
        option_header* cur_check = find_option("careful_curve_self_int_check");
        if(GET_ALGORITHMIC_VERSION() >= AcisVersion(20, 0, 0)) cur_check->push(0);
        exception_save_mark.begin();
        get_error_mark().buffer_init = 1;
        int error_no = 0;
        sg_add_extensions(orig_list);  // 有待探究,
        display_segment_in_colors(orig_list, 5);
        int origConnected = orig_list.segments_connected();  // 判断所有偏置段coedge是否是相连的
        offset_segment* this_seg = orig_list.first_segment();
        offset_segment* last_seg = orig_list.last_segment()->next();
        int segmentConnectionId = 0;
        int singleSegment = orig_list.get_number_of_offset_coedges() == 1;
        offset_segment* next_seg;
        if(gme_check_offst_self_intersection.on()) {
            SPAunit_vector off_normal = orig_list.normal();
            while(this_seg != last_seg) {
                law* dist_law = orig_list.distance();
                next_seg = this_seg->next();
                int v95 = this_seg->coedge()->sense();
                int v13 = this_seg->coedge()->edge()->sense();
                if(v95 == v13) {
                    local_dist_law = dist_law;
                    dist_law->add();
                } else {
                    negate_law* v119 = ACIS_NEW negate_law(dist_law);
                    local_dist_law = v119;
                }
                if(GET_ALGORITHMIC_VERSION() > AcisVersion(17, 0, 0)) {
                    int a = this_seg->c_underlying == nullptr;
                    if(dist_law && this_seg->coedge_offset() && !this_seg->simple_offset()) {
                        if(edgeSmoothMgr) {
                            int hasError = 0;
                            this_seg->do_self_intersect(local_dist_law, off_normal, singleSegment, segmentConnectionId, edgeSmoothMgr, hasError);
                        } else {
                            SPAinterval edgeParamRange;
                            SPAtransf iTransf;
                            COEDGE* iCoedge = this_seg->coedge();
                            curve* offsetCurve = get_curve_from_coedge(iCoedge, edgeParamRange, iTransf);
                            if(!this_seg->do_self_intersect_new(offsetCurve, local_dist_law, off_normal, singleSegment, segmentConnectionId, 1, 0)) {
                                curve* newOffsetCurve = filter_high_curvature(local_dist_law, off_normal, offsetCurve, edgeParamRange, singleSegment);
                                set_curve_to_coedge(this_seg->coedge(), newOffsetCurve);
                                if(!this_seg->do_self_intersect_new(newOffsetCurve, local_dist_law, off_normal, singleSegment, segmentConnectionId, 1, 0)) {
                                    SPAtransf iTransf;
                                    curve* offsetCurve2 = get_curve_from_coedge(this_seg->coedge(), edgeParamRange, iTransf);
                                    curve* newOffsetCurve2 = filter_high_curvature(local_dist_law, off_normal, offsetCurve2, edgeParamRange, singleSegment);
                                    if(offsetCurve2) {
                                        if(newOffsetCurve2) {
                                            set_curve_to_coedge(this_seg->coedge(), newOffsetCurve2);
                                            if(!this_seg->do_self_intersect_new(newOffsetCurve2, local_dist_law, off_normal, singleSegment, segmentConnectionId, 1, 0))
                                                this_seg->do_self_intersect_new(newOffsetCurve2, local_dist_law, off_normal, singleSegment, segmentConnectionId, 1, 1);
                                            if(offsetCurve2) offsetCurve2 = nullptr;
                                            if(newOffsetCurve2) newOffsetCurve2 = nullptr;
                                        }
                                    }
                                }
                                if(newOffsetCurve) {
                                    newOffsetCurve = nullptr;
                                }
                            }
                            if(offsetCurve) {
                                offsetCurve = nullptr;
                            }
                        }
                    }
                } else {
                    this_seg->do_self_intersect(local_dist_law, off_normal, segmentConnectionId);
                }
                this_seg = next_seg;
                local_dist_law->remove();
                local_dist_law = nullptr;
            }
        }

        offset_segment* this_sega = orig_list.first_segment();
        last_seg = orig_list.last_segment()->next();

        if(GET_ALGORITHMIC_VERSION() > AcisVersion(20, 0, 0) && gme_check_offst_self_intersection.on()) {
            while(this_sega != last_seg) {
                next_seg = this_sega->next();
                this_sega->split_at_cusps(orig_list.distance());  // 有待探究
                this_sega = next_seg;
            }
            display_segment_in_colors(orig_list, 5);
            display_segment_trim_status(orig_list);
        }
        if(!orig_list.segments_connected() && origConnected) {
            orig_list.mSelfIntesectError = 1;
            return 0;
        }
        orig_list.set_last_connectionId(segmentConnectionId);
        display_segment_in_colors(orig_list, 5);
        offset_segment* this_segb = orig_list.first_segment();
        next_seg = this_segb->next();
        RenderingObject* in_segment_pRO = nullptr;

        orig_list.init_connection();
        int start_segment_connectionId = orig_list.get_start_segment_connectionId();
        segment_list.set_start_segment_connectionId(start_segment_connectionId);
        int last_connectionId = orig_list.get_last_connectionId();
        segment_list.set_last_connectionId(last_connectionId);
        int iNumCoedges = orig_list.get_number_of_offset_coedges();
        WIRE* iBaseWire = orig_list.get_base_wire();
        segment_list.set_base_wire(iBaseWire, iNumCoedges);

        while(this_segb != last_seg) {
            orig_list.remove_segment(this_segb);
            EDGE* pEdge = this_segb->coedge()->edge();
            // double a = pEdge->param_range().mid_pt();
            // SPAinterval ab = pEdge->geometry()->equation().subset_range;
            // double b = pEdge->start_param().operator double();
            // double c = pEdge->end_param().operator double();

            ////SPAinterval ac(interval_finite_above, c, SpaAcis::NullObj::get_double());
            ////SPAinterval ad(interval_finite_below, c, SpaAcis::NullObj::get_double());
            ////ab &=ac;
            ////ab &=ad;

            // curve* b1 =&((curve&)pEdge->geometry()->equation());
            // curve* new_crv = b1->copy_curve();
            // curve* c1 = new_crv->split(b, pEdge->start_pos());
            // curve* d1 = new_crv->split(c, pEdge->end_pos());
            // curve* d2 = new_crv->split(10);
            // b->change_event()
            ////b->split(a);
            ////CURVE* new_cur = make_curve(*b);
            ////pEdge->set_geometry(new_cur);
            // SPAinterval ac(interval_finite_above, 2.0);

            // int a = ac.empty();
            // int b = ac.unbounded_above();
            // int c = ac.unbounded_below();
            if(pEdge) {
                trim_edge_geom(this_segb->coedge()->edge());  // 关键核心
                int bContinue = 1;
                if(is_intcurve_edge(pEdge)) {
                    const intcurve& rIntcurve = (const intcurve&)pEdge->geometry()->equation();
                    if(!((intcurve&)rIntcurve).cur(-1.0, 0)) bContinue = 0;
                }
                if(bContinue) {
                    if(segment_list.add_segment(this_segb, in_segment_pRO) == 1) intersection_found = 1;  // 关键核心
                } else {
                    offset_segment* pPrevSegment = this_segb->previous();
                    offset_segment* pNextSegment = this_segb->next();
                    if(pPrevSegment) pPrevSegment->set_next(pNextSegment);
                    if(pNextSegment) pNextSegment->set_previous(pPrevSegment);
                    this_segb->set_previous(nullptr);
                    this_segb->set_next(nullptr);
                    // void* alloc_ptr = this_segb;  // 不懂
                    ACIS_DELETE this_segb;
                }
            }
            this_segb = next_seg;
            next_seg = next_seg->next();
        }

        segment_list.close_segments();

        display_segment_in_colors(segment_list, 5);
        orig_list.empty_list();

        offset_segment* this_segc = segment_list.first_segment();
        last_seg = segment_list.last_segment()->next();
        int inside_rating = 0;
        // 将每段segment的10的情况都变为00;
        do {
            if(!inside_rating) inside_rating = this_segc->start_out() == 0;
            if(this_segc->end_out()) inside_rating = 0;  // 将10的情况都变为00;
            if(inside_rating) {
                this_segc->set_start_in();
                this_segc->set_end_in();
            }
            this_segc = this_segc->next();
        } while(this_segc != last_seg);
        offset_segment* this_segd = segment_list.last_segment();
        offset_segment* first_seg = segment_list.first_segment()->previous();
        int inside_ratinga = 0;
        // 将每段segment的01的情况都变为00;
        do {
            if(!inside_ratinga) inside_ratinga = this_segd->end_out() == 0;
            if(this_segd->start_out()) inside_ratinga = 0;
            if(inside_ratinga) {
                this_segd->set_start_in();
                this_segd->set_end_in();
            }
            this_segd = this_segd->previous();
        } while(this_segd != first_seg);
        if(segment_list.get_number_of_offset_coedges() == 1) {
            offset_segment* thisSeg = segment_list.first_segment();
            offset_segment* lastSeg = segment_list.last_segment();
            if(!lastSeg->start_out()) {
                if(lastSeg->end_out()) {
                    if(!thisSeg->start_out()) {
                        SPAposition start_pos(thisSeg->coedge()->start()->geometry()->coords());
                        SPAposition end_pos(lastSeg->coedge()->end()->geometry()->coords());
                        if(start_pos != end_pos) lastSeg->set_end_in();
                    }
                }
            }
            lastSeg = lastSeg->next();
            while(thisSeg != lastSeg) {
                if(thisSeg->start_out() || !thisSeg->end_out()) {
                    if(thisSeg->start_out() && !thisSeg->end_out()) thisSeg->set_end_out();
                } else {
                    thisSeg->set_start_out();
                }
                thisSeg = thisSeg->next();
            }
        }
        if(trim_offset_wire_module_header.debug_level >= 30) segment_list.print(debug_file_ptr);

        // 处理intcurve的情况
        offset_segment* v78 = segment_list.first_segment();
        offset_segment* v163 = segment_list.last_segment()->next();
        while(v78 != v163) {
            offset_segment* v170 = v78->next();
            COEDGE* ofc = v78->coedge();
            COEDGE* pgc = v78->original_coedge();
            if(is_intcurve_edge(ofc->edge())) {
                if(is_intcurve_edge(pgc->edge())) {
                    const intcurve& ofintc = (const intcurve&)ofc->edge()->geometry()->equation();
                    const intcurve& pgintc = (const intcurve&)pgc->edge()->geometry()->equation();
                    double t1 = bs3_curve_angle(((intcurve&)pgintc).cur(-1.0, 0));
                    double t2 = bs3_curve_angle(((intcurve&)ofintc).cur(-1.0, 0));
                    if(t2 > t1 + 3.0) {
                        double tol = ((intcurve&)ofintc).fitol();
                        double actual_tol = 0.0;
                        intcurve& ofintc_upd = (intcurve&)ofc->edge()->geometry()->equation_for_update();
                        SPAinterval the_int = ofintc_upd.param_range();
                        bs3_curve_def* new_bs3 = nullptr;
                        int err_num = 0;
                        acis_exception v180(0);
                        error_info_base* e_info = nullptr;
                        exception_save v176;
                        v176.begin();
                        get_error_mark().buffer_init = 1;
                        new_bs3 = bs3_curve_make_approx(ofintc_upd, the_int, tol, actual_tol, 0, (const double*)0, 0, 0, 0);
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
            v78 = v170;
        }
        display_segment_in_colors(segment_list, 5);
        display_segment_trim_status(segment_list);

        sg_collect_segments(wire_body, segment_list, zero_length, overlap);
        orig_list.set_trim_inside_wire_done(segment_list.trim_inside_wire_done());

        repair_dupicated_vertices(wire_body);

        if(!overlap) {
            remove_consecutive_overlaps(wire_body);  // 已检查
            if(GET_ALGORITHMIC_VERSION() >= AcisVersion(23, 0, 0)) repair_wire_at_branches(wire_body);
        }

        tolerate_wire_edges(wire_body);
        if(error_no) orig_list.mSelfIntesectError = 1;
        if(local_dist_law) local_dist_law->remove();
        edgeSmoothMgr = nullptr;

        option_header* care_cur_chk = find_option("careful_curve_self_int_check");
        if(GET_ALGORITHMIC_VERSION() >= AcisVersion(20, 0, 0)) care_cur_chk->pop();
        if(acis_interrupted()) sys_error(resignal_no, error_info_base_ptr);
    }
    return intersection_found;
}

COEDGE* start_of_wire_chain(WIRE* w) {
    COEDGE* wire_coedge = w->coedge();
    COEDGE* this_coedge = wire_coedge;
    COEDGE* prev_coedge = nullptr;
    for(prev_coedge = wire_coedge->previous(); prev_coedge != this_coedge && prev_coedge != wire_coedge && prev_coedge; prev_coedge = prev_coedge->previous()) {
        this_coedge = prev_coedge;
    }
    if(prev_coedge) {
        return prev_coedge;
    }
    return this_coedge;
}
void display_segment_intersection_points(EDGE* iOfstEdge, curve_curve_int* iIntersections, EDGE* iOfstEdge2) {
    return;
}
int is_seg_reversed(offset_segment* iSeg, double iCheckParam) {
    int reversed = 0;
    // if(!iSeg) apiFinderACIS.GetAddress("_wassert", prostate);
    //_wassert(L"iSeg != NULL", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\trmofwi.cpp", 0x478u);
    COEDGE* ofstCoed = iSeg->coedge();
    COEDGE* baseCoed = iSeg->original_coedge();
    if(baseCoed) {
        if(ofstCoed) {
            if(ofstCoed->edge()) {
                if(baseCoed->edge()) {
                    if(baseCoed->edge()->geometry()) {
                        if(ofstCoed->edge()) {
                            if(ofstCoed->edge()->geometry()) {
                                SPAparameter ofstParam(iCheckParam);
                                SPAparameter baseParam(iCheckParam);
                                if(ofstCoed->edge()->sense() == 1) ofstParam = -baseParam.operator double();
                                if(baseCoed->edge()->sense() == 1) baseParam = -baseParam.operator double();
                                double coedge_param = ofstParam.operator double();
                                SPAposition ofstPos = coedge_param_pos(ofstCoed, coedge_param);
                                SPAtransf v22;
                                SPAunit_vector ofstTan = normalise(coedge_param_dir(ofstCoed, ofstParam.operator double()));
                                const curve& baseCurve = baseCoed->edge()->geometry()->equation();
                                SPAparameter adjustedBaseParam;
                                SPAposition basePos;
                                ((curve&)baseCurve).point_perp(ofstPos, basePos, baseParam, adjustedBaseParam, 0);  // 不確定
                                SPAtransf v25;
                                SPAunit_vector baseTan = normalise(coedge_param_dir(baseCoed, adjustedBaseParam.operator double()));
                                return (baseTan % ofstTan) < 0.0;
                            }
                        }
                    }
                }
            }
        }
    }
    return reversed;
}
int rate_single_seg_case(offset_segment* iSeg, curve_curve_int* iIntsect, sg_seg_rating* ioRating) {
    int rateDone = 0;
    if(iSeg && iIntsect && ioRating && iIntsect->param1 != iIntsect->param2) {
        int headSegRev = is_seg_reversed(iSeg, iIntsect->param1);
        int tailSegRev = is_seg_reversed(iSeg, iIntsect->param2);
        if(!headSegRev || tailSegRev) {
            if(!headSegRev && tailSegRev) {
                ioRating->low_rating = seg_in;
                ioRating->high_rating = seg_out;
                return 1;
            }
        } else {
            ioRating->low_rating = seg_out;
            ioRating->high_rating = seg_in;
            return 1;
        }
    }
    return rateDone;
}

curve* smooth_curve1(intcurve* iCurve, SPAunit_vector& iNormal, int iLevel, law* iDstLaw) {
    if(!iCurve || iLevel <= 0 || !iDstLaw) return nullptr;
    straight* newOffsetCurve = nullptr;
    /*   const int_cur& intCur = ((intcurve*)iCurve)->get_int_cur();*/
    const int_cur& intCur = iCurve->get_int_cur();
    offset_int_cur& ofstIntcur = (offset_int_cur&)intCur;  // 不确定
    const curve& baseCurve = ofstIntcur.get_orig_curve();
    SPAinterval paramRange = baseCurve.param_range();
    bs3_curve_def* newBs3Curve = nullptr;
    double ofstDst = fabs(iDstLaw->eval(1.0));
    double v24;
    if(ofstDst <= 1.0)
        v24 = SPAresfit;
    else
        v24 = ofstDst * SPAresfit;
    double fitTol = v24;
    if(baseCurve.type() == intcurve_type) {
        if(CUR_is_exact_intcurve(baseCurve)) {
            bs3_curve bs3Curve = ((intcurve&)baseCurve).cur(-1.0, 0);
            // bs3_curve_def* bs3Curve = baseCurve.cur(-1.0, 0);
            newBs3Curve = smooth_bs3_curve(bs3Curve, paramRange, fitTol, iLevel);
        } else {
            bs3_curve_def* iBs3Curve = bs3_curve_make_approx(baseCurve, paramRange, fitTol);  //
            if(iLevel <= 1) {
                newBs3Curve = iBs3Curve;
            } else {
                newBs3Curve = smooth_bs3_curve(iBs3Curve, paramRange, fitTol, iLevel - 1);
                bs3_curve_delete(iBs3Curve);
                iBs3Curve = nullptr;
            }
        }
    }

    if(newBs3Curve) {
        intcurve* v26 = ACIS_NEW intcurve(newBs3Curve, 0.0, SpaAcis::NullObj::get_surface(), SpaAcis::NullObj::get_surface());
        curve* temp_geom = v26;
        constant_law* v28 = ACIS_NEW constant_law(0.0);
        law* twistLaw = v28;

        newOffsetCurve = (straight*)sg_offset_planar_curve(*temp_geom, paramRange, SPAresfit, iDstLaw, twistLaw, iNormal, 0, SPAresabs);  // 不确定
        twistLaw->remove();
        twistLaw = nullptr;
    }
    return newOffsetCurve;
}
bs3_curve_def* smooth_bs3_curve(bs3_curve_def*& iBs3Curve, SPAinterval& iParamRange, double iFitTolerance, int iLevel) {
    if(iLevel < 1) return nullptr;
    bs3_curve_def* smoothedBs3Curve = nullptr;
    acis_exception error_info_holder(0);
    error_info_base* error_info_base_ptr = nullptr;
    exception_save exception_save_mark;
    double* curParams = nullptr;
    int* tmpCont = nullptr;
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    int numknots;
    bs3_curve_unique_knots(iBs3Curve, numknots, curParams, tmpCont, -1.0);

    SPAposition* curPositions = ACIS_NEW SPAposition[numknots];
    SPAvector sTan;
    bs3_curve_def* cur = iBs3Curve;
    double param = *curParams;
    bs3_curve_eval(param, cur, *curPositions, sTan);
    int numCurSmps = 1;
    SPAvector eTan;
    SPAposition ePos;
    bs3_curve_def* v23 = iBs3Curve;
    double v24 = curParams[numknots - 1];
    bs3_curve_eval(v24, v23, ePos, eTan);
    int smpInd = 1;
    for(int i = iLevel; i < numknots - 1; i += iLevel) bs3_curve_evaluate(curParams[i], iBs3Curve, curPositions[smpInd++]);
    curPositions[smpInd] = ePos;

    SPAunit_vector end_dir = normalise(eTan);    // 不确定
    SPAunit_vector start_dir = normalise(sTan);  //
    smoothedBs3Curve = bs3_curve_interp(smpInd + 1, curPositions, start_dir, end_dir, iFitTolerance);
    if(smoothedBs3Curve) {
        double end = iParamRange.end_pt();
        double start = iParamRange.start_pt();
        bs3_curve_reparam(start, end, smoothedBs3Curve);
    }
    if(curParams) {
        ACIS_DELETE[] STD_CAST curParams;  // 不确定
    }
    if(curPositions) {
        ACIS_DELETE[] curPositions;
    }
    if(tmpCont) {
        ACIS_DELETE[] STD_CAST tmpCont;
    }
    if(acis_interrupted()) sys_error(0, error_info_base_ptr);
    return smoothedBs3Curve;
}
int check_split_position(curve* iCurveGeom, const double iSplitParam, const double iStartParam, const double iEndParam, SPAposition& iSplitPos, SPAposition& iStartPos, SPAposition& iEndPos) {
    int splitPosition = 1;
    double v19;
    if(GET_ALGORITHMIC_VERSION() >= AcisVersion(29, 0, 2)) {
        v19 = 0.01;
    } else
        v19 = 0.5;
    double prmTol = v19 * SPAresabs;
    double dstTol = 1.5 * SPAresabs;
    double rangeTol = 0.2 * fabs(iEndParam - iStartParam);
    double paramDst = fabs(iSplitParam - iStartParam);
    if(prmTol <= paramDst) {
        if(rangeTol > paramDst) {
            if(dstTol > (iSplitPos - iStartPos).len()) {
                double midParam = 0.5 * (iStartParam + iSplitParam);
                SPAposition midPos = iCurveGeom->eval_position(midParam);
                if(dstTol > (midPos - iStartPos).len()) splitPosition = 0;
            }
        }
    } else {
        splitPosition = 0;
    }
    double paramDsta = fabs(iSplitParam - iEndParam);
    if(prmTol <= paramDsta) {
        if(rangeTol > paramDsta) {
            if(dstTol > (iSplitPos - iEndPos).len()) {
                SPAposition p1 = iCurveGeom->eval_position(0.5 * (iEndParam + iSplitParam));
                if(dstTol > (p1 - iEndPos).len()) return 2;
            }
        }
    } else {
        return 2;
    }
    return splitPosition;
}
void display_segment_split_status(EDGE* iEdge1, EDGE* iEdge2, curve* iCurve, SPAposition* iPoint) {
    return;
}
curve* get_curve_from_coedge(COEDGE* iCoedge, SPAinterval& oParamRange, SPAtransf& iTransf) {
    curve* retCurve = nullptr;

    if(iCoedge) {
        EDGE* givenEdge = iCoedge->edge();
        if(givenEdge) {
            oParamRange = givenEdge->param_range();
            CURVE* edgeGeom = givenEdge->geometry();
            if(edgeGeom) return edgeGeom->trans_curve(iTransf, 0);
        }
    }
    return retCurve;
}
void add_intersectins(curve_curve_int*& ioMainList, curve_curve_int* iIntersections) {
    while(iIntersections) {
        curve_curve_int* v6 = ACIS_NEW curve_curve_int(ioMainList, iIntersections->int_point, iIntersections->param1, iIntersections->param1);
        ioMainList = v6;
        (ioMainList)->low_rel = iIntersections->low_rel;
        (ioMainList)->high_rel = iIntersections->high_rel;
        (ioMainList)->uv_set = iIntersections->uv_set;
        curve_curve_int* v8 = ACIS_NEW curve_curve_int(ioMainList, iIntersections->int_point, iIntersections->param2, iIntersections->param2);
        ioMainList = v8;
        (ioMainList)->low_rel = iIntersections->low_rel;
        (ioMainList)->high_rel = iIntersections->high_rel;
        (ioMainList)->uv_set = iIntersections->uv_set;
        iIntersections = iIntersections->next;
    }
}
int ofst_params_over_lapping(double iParam1, double iParam2, double iParam3, double iParam4) {
    double iParam4a = iParam4;
    double iParam3a = iParam3;
    double iParam2a = iParam2;
    double iParam1a = iParam1;
    if(iParam1 > iParam2) {
        iParam1a = iParam2;
        iParam2a = iParam1;
    }
    if(iParam3 > iParam4) {
        iParam3a = iParam4;
        iParam4a = iParam3;
    }
    int overlapping = iParam1a == iParam3a || iParam2a == iParam4a;
    if(!overlapping) {
        if(iParam3a <= iParam1a || iParam2a <= iParam3a) {
            if(iParam1a > iParam3a && iParam4a > iParam1a) return iParam2a > iParam4a;
        } else {
            return iParam4a > iParam2a;
        }
    }
    return overlapping;
}
int has_high_curvature(curve_curve_int* iIntersections, int iSingleSegment) {
    int numIntersections = 0;
    for(curve_curve_int* intsect1 = iIntersections; intsect1; intsect1 = intsect1->next) ++numIntersections;
    BOOL v9 = !iSingleSegment && numIntersections >= 15;
    int hasHighCurvature = v9;
    if(!v9 && (!iSingleSegment || numIntersections < 30)) {
        curve_curve_int* intsect1a = iIntersections;
        int checkIntersectionOk = 1;
        while(intsect1a && checkIntersectionOk) {
            checkIntersectionOk = intsect1a->low_rel == cur_cur_normal && intsect1a->high_rel == cur_cur_normal;
            for(curve_curve_int* intsect2 = intsect1a->next; intsect2 && checkIntersectionOk; intsect2 = intsect2->next)
                checkIntersectionOk = !(intsect1a->int_point == intsect2->int_point) && ofst_params_over_lapping(intsect1a->param1, intsect1a->param2, intsect2->param1, intsect2->param2) == 0;
            intsect1a = intsect1a->next;
        }
        return checkIntersectionOk == 0;
    }
    return hasHighCurvature;
}
int coedge_connected(COEDGE* iCoedge1, COEDGE* iCoedge2) {
    int coedgesConnected = 1;
    if(iCoedge1 && iCoedge2) {
        SPAposition sPos1 = iCoedge1->start_pos();
        SPAposition ePos1 = iCoedge1->end_pos();
        SPAposition sPos2 = iCoedge2->start_pos();
        SPAposition ePos2 = iCoedge2->end_pos();
        return same_point(sPos1, sPos2) || same_point(sPos1, ePos2) || same_point(ePos1, sPos2) || same_point(ePos1, ePos2);
    }
    return coedgesConnected;
}

// curve_curve_int* __fastcall line_ell_int(EDGE* edge1, EDGE* edge2, long double dist_tol_in, long double angle_tol_in, bool* difficult_config) {
//     AcisVersion* v5;                                                                                                          // rax
//     bool v6;                                                                                                                  // al
//     __int64 v8;                                                                                                               // r8
//     const SPAposition* v9;                                                                                                    // rax
//     __int64 v10;                                                                                                              // r8
//     const SPAposition* v11;                                                                                                   // rax
//     __int64 v12;                                                                                                              // r8
//     const SPAposition* v13;                                                                                                   // rax
//     __int64 v14;                                                                                                              // r8
//     const SPAposition* v15;                                                                                                   // rax
//     __int64 v16;                                                                                                              // r8
//     __int64 v17;                                                                                                              // r8
//     __int64 v18;                                                                                                              // r8
//     __int64 v19;                                                                                                              // r8
//     const SPAvector* v20;                                                                                                     // rax
//     double v21;                                                                                                               // xmm0_8
//     long double v22;                                                                                                          // xmm0_8
//     long double v23;                                                                                                          // xmm0_8
//     long double v24;                                                                                                          // xmm0_8
//     long double v25;                                                                                                          // xmm0_8
//     const SPAvector* v26;                                                                                                     // rax
//     long double v27;                                                                                                          // xmm0_8
//     long double v28;                                                                                                          // xmm0_8
//     long double v29;                                                                                                          // xmm0_8
//     __int64 v30;                                                                                                              // r8
//     __int64 v31;                                                                                                              // r8
//     SPAvector* v32;                                                                                                           // rax
//     SPAvector* v33;                                                                                                           // rax
//     const SPAvector* v34;                                                                                                     // rax
//     const SPAvector* v35;                                                                                                     // rax
//     __int64 v36;                                                                                                              // r8
//     __int64 v37;                                                                                                              // r8
//     __int64 v38;                                                                                                              // r8
//     const SPAvector* v39;                                                                                                     // rax
//     double* v40;                                                                                                              // rax
//     __int64 v41;                                                                                                              // rax
//     double v42;                                                                                                               // xmm0_8
//     SPAunit_vector* v43;                                                                                                      // rax
//     long double v44;                                                                                                          // xmm0_8
//     long double v45;                                                                                                          // xmm0_8
//     SPAvector* v46;                                                                                                           // rax
//     SPAvector* v47;                                                                                                           // rax
//     const SPAvector* v48;                                                                                                     // rax
//     const SPAvector* v49;                                                                                                     // rax
//     __int64 v50;                                                                                                              // r8
//     __int64 v51;                                                                                                              // r8
//     __int64 v52;                                                                                                              // r8
//     const SPAvector* v53;                                                                                                     // rax
//     double* v54;                                                                                                              // rax
//     __int64 v55;                                                                                                              // rax
//     __int64 v56;                                                                                                              // r8
//     __int64 v57;                                                                                                              // r8
//     *v58;                                                                                                                     // rax
//     *v59;                                                                                                                     // rax
//     *v60;                                                                                                                     // rax
//     const SPAvector* v61;                                                                                                     // rax
//     const SPAvector* v62;                                                                                                     // rax
//     __int64 v63;                                                                                                              // r8
//     __int64 v64;                                                                                                              // r8
//     __int64 v65;                                                                                                              // r8
//     const SPAvector* v66;                                                                                                     // rax
//     long double v67;                                                                                                          // xmm0_8
//     long double v68;                                                                                                          // xmm0_8
//     SPAvector* v69;                                                                                                           // rax
//     double* v70;                                                                                                              // rax
//     __int64 v71;                                                                                                              // rax
//     double* v72;                                                                                                              // rax
//     __int64 v73;                                                                                                              // rax
//     double* v74;                                                                                                              // rax
//     __int64 v75;                                                                                                              // rax
//     SPAvector* v76;                                                                                                           // rax
//     const SPAvector* v77;                                                                                                     // rax
//     long double v78;                                                                                                          // xmm0_8
//     const SPAvector* v79;                                                                                                     // rax
//     __int64 v80;                                                                                                              // r8
//     double* v81;                                                                                                              // rax
//     __int64 v82;                                                                                                              // rax
//     int v83;                                                                                                                  // eax
//     const SPAposition* v84;                                                                                                   // rax
//     int v85;                                                                                                                  // eax
//     const SPAposition* v86;                                                                                                   // rax
//     __int64 v87;                                                                                                              // rax
//     int v88;                                                                                                                  // eax
//     const SPAposition* v89;                                                                                                   // rax
//     __int64 v90;                                                                                                              // r8
//     double* v91;                                                                                                              // rax
//     _QWORD* v92;                                                                                                              // rax
//     __int64 v93;                                                                                                              // r8
//     __int64 v94;                                                                                                              // rax
//     int v95;                                                                                                                  // eax
//     const SPAposition* v96;                                                                                                   // rax
//     __int64 v97;                                                                                                              // r8
//     __int64 v98;                                                                                                              // r8
//     __int64 v99;                                                                                                              // rax
//     ellipse* ell_eq;                                                                                                          // [rsp+30h] [rbp-16C8h]
//     long double dist_tol;                                                                                                     // [rsp+38h] [rbp-16C0h]
//     curve_curve_int* ccrel_data;                                                                                              // [rsp+40h] [rbp-16B8h] BYREF
//     int i;                                                                                                                    // [rsp+48h] [rbp-16B0h]
//     straight* st_eq;                                                                                                          // [rsp+50h] [rbp-16A8h]
//     bool pt_at_ell_end;                                                                                                       // [rsp+58h] [rbp-16A0h] BYREF
//     bool v106;                                                                                                                // [rsp+59h] [rbp-169Fh] BYREF
//     bool v107;                                                                                                                // [rsp+5Ah] [rbp-169Eh] BYREF
//     bool v108;                                                                                                                // [rsp+5Bh] [rbp-169Dh] BYREF
//     bool v109;                                                                                                                // [rsp+5Ch] [rbp-169Ch] BYREF
//     int intr_count;                                                                                                           // [rsp+60h] [rbp-1698h]
//     bool pt_at_str_end[2];                                                                                                    // [rsp+64h] [rbp-1694h] BYREF
//     bool inters1_matches_end;                                                                                                 // [rsp+66h] [rbp-1692h]
//     bool inters2_matches_end;                                                                                                 // [rsp+67h] [rbp-1691h]
//     bool check_edge_ends_for_solution;                                                                                        // [rsp+68h] [rbp-1690h]
//     int not_circle;                                                                                                           // [rsp+6Ch] [rbp-168Ch]
//     int str_str_sol;                                                                                                          // [rsp+70h] [rbp-1688h]
//     int str_end_sol;                                                                                                          // [rsp+74h] [rbp-1684h]
//     int ell_str_sol;                                                                                                          // [rsp+78h] [rbp-1680h]
//     int ell_end_sol;                                                                                                          // [rsp+7Ch] [rbp-167Ch]
//     bool end_pt_sol;                                                                                                          // [rsp+80h] [rbp-1678h]
//     bool v121;                                                                                                                // [rsp+81h] [rbp-1677h]
//     bool line_touches_or_above_ellipse;                                                                                       // [rsp+82h] [rbp-1676h]
//     bool ellipse_tangent;                                                                                                     // [rsp+83h] [rbp-1675h]
//     int j;                                                                                                                    // [rsp+84h] [rbp-1674h]
//     int error_no;                                                                                                             // [rsp+88h] [rbp-1670h]
//     int resignal_no;                                                                                                          // [rsp+8Ch] [rbp-166Ch]
//     long double perp_dist;                                                                                                    // [rsp+90h] [rbp-1668h]
//     long double circle_radius;                                                                                                // [rsp+98h] [rbp-1660h]
//     int tangent_root;                                                                                                         // [rsp+A0h] [rbp-1658h]
//     SPAdouble_array ell_prospective_params;                                                                                   // [rsp+A8h] [rbp-1650h] BYREF
//     int intersects;                                                                                                           // [rsp+C0h] [rbp-1638h]
//     int inters1;                                                                                                              // [rsp+C4h] [rbp-1634h]
//     int inters2;                                                                                                              // [rsp+C8h] [rbp-1630h]
//     SPAposition perp_foot;                                                                                                    // [rsp+D0h] [rbp-1628h] BYREF
//     long double ratio;                                                                                                        // [rsp+E8h] [rbp-1610h]
//     long double v136;                                                                                                         // [rsp+F0h] [rbp-1608h] BYREF
//     SPAparameter v137;                                                                                                        // [rsp+F8h] [rbp-1600h] BYREF
//     SPAposition_array ell_prospective_sols;                                                                                   // [rsp+100h] [rbp-15F8h] BYREF
//     SPAposition_array str_prospective_sols;                                                                                   // [rsp+118h] [rbp-15E0h] BYREF
//     int v140;                                                                                                                 // [rsp+130h] [rbp-15C8h]
//     int v141;                                                                                                                 // [rsp+134h] [rbp-15C4h]
//     BOOL v142;                                                                                                                // [rsp+138h] [rbp-15C0h]
//     BOOL v143;                                                                                                                // [rsp+13Ch] [rbp-15BCh]
//     BOOL v144;                                                                                                                // [rsp+140h] [rbp-15B8h]
//     BOOL v145;                                                                                                                // [rsp+144h] [rbp-15B4h]
//     BOOL v146;                                                                                                                // [rsp+148h] [rbp-15B0h]
//     curve* ed_cur1;                                                                                                           // [rsp+150h] [rbp-15A8h]
//     curve* ed_cur2;                                                                                                           // [rsp+158h] [rbp-15A0h]
//     int v149;                                                                                                                 // [rsp+160h] [rbp-1598h]
//     BOOL v150;                                                                                                                // [rsp+164h] [rbp-1594h]
//     int v151;                                                                                                                 // [rsp+168h] [rbp-1590h]
//     int inters;                                                                                                               // [rsp+16Ch] [rbp-158Ch]
//     BOOL v153;                                                                                                                // [rsp+170h] [rbp-1588h]
//     BOOL v154;                                                                                                                // [rsp+174h] [rbp-1584h]
//     BOOL v155;                                                                                                                // [rsp+178h] [rbp-1580h]
//     BOOL v156;                                                                                                                // [rsp+17Ch] [rbp-157Ch]
//     BOOL v157;                                                                                                                // [rsp+180h] [rbp-1578h]
//     BOOL v158;                                                                                                                // [rsp+184h] [rbp-1574h]
//     int tang_root;                                                                                                            // [rsp+188h] [rbp-1570h]
//     long double angle_tol;                                                                                                    // [rsp+190h] [rbp-1568h]
//     BOOL v161;                                                                                                                // [rsp+198h] [rbp-1560h]
//     BOOL v162;                                                                                                                // [rsp+19Ch] [rbp-155Ch]
//     SPAposition inter1;                                                                                                       // [rsp+1A0h] [rbp-1558h] BYREF
//     long double v164;                                                                                                         // [rsp+1B8h] [rbp-1540h] BYREF
//     SPAparameter str_param;                                                                                                   // [rsp+1C0h] [rbp-1538h] BYREF
//     long double actual_par;                                                                                                   // [rsp+1C8h] [rbp-1530h] BYREF
//     long double param;                                                                                                        // [rsp+1D0h] [rbp-1528h] BYREF
//     SPAposition inter2;                                                                                                       // [rsp+1D8h] [rbp-1520h] BYREF
//     SPAvector perp_offset;                                                                                                    // [rsp+1F0h] [rbp-1508h] BYREF
//     SPAvector maj_dir;                                                                                                        // [rsp+208h] [rbp-14F0h] BYREF
//     curve* v171;                                                                                                              // [rsp+220h] [rbp-14D8h]
//     curve* v172;                                                                                                              // [rsp+228h] [rbp-14D0h]
//     SPAparameter v173;                                                                                                        // [rsp+230h] [rbp-14C8h] BYREF
//     long double v174;                                                                                                         // [rsp+238h] [rbp-14C0h] BYREF
//     long double angle;                                                                                                        // [rsp+240h] [rbp-14B8h]
//     long double v176[3];                                                                                                      // [rsp+248h] [rbp-14B0h] BYREF
//     error_info_base* error_info_base_ptr;                                                                                     // [rsp+260h] [rbp-1498h]
//     int inter_ed_ed_robustness;                                                                                               // [rsp+268h] [rbp-1490h]
//     int coin_ss;                                                                                                              // [rsp+26Ch] [rbp-148Ch]
//     int coin_ee;                                                                                                              // [rsp+270h] [rbp-1488h]
//     int coin_se;                                                                                                              // [rsp+274h] [rbp-1484h]
//     int coin_es;                                                                                                              // [rsp+278h] [rbp-1480h]
//     BOOL v183;                                                                                                                // [rsp+27Ch] [rbp-147Ch]
//     SPAvector min_dir;                                                                                                        // [rsp+280h] [rbp-1478h] BYREF
//     SPAdouble_array str_prospective_params;                                                                                   // [rsp+298h] [rbp-1460h] BYREF
//     char* v186;                                                                                                               // [rsp+2B0h] [rbp-1448h]
//     char* v187;                                                                                                               // [rsp+2B8h] [rbp-1440h]
//     SPAparameter ell_param;                                                                                                   // [rsp+2C0h] [rbp-1438h] BYREF
//     long double bx1_len;                                                                                                      // [rsp+2C8h] [rbp-1430h]
//     long double bx2_len;                                                                                                      // [rsp+2D0h] [rbp-1428h]
//     double v191;                                                                                                              // [rsp+2D8h] [rbp-1420h]
//     long double VALID_COINCIDENC_CHECK_TOL;                                                                                   // [rsp+2E0h] [rbp-1418h]
//     long double SMALL_FEATURE_SIZE;                                                                                           // [rsp+2E8h] [rbp-1410h]
//     long double v194;                                                                                                         // [rsp+2F0h] [rbp-1408h]
//     long double offset;                                                                                                       // [rsp+2F8h] [rbp-1400h]
//     long double line_param;                                                                                                   // [rsp+300h] [rbp-13F8h] BYREF
//     curve_curve_int* v197;                                                                                                    // [rsp+308h] [rbp-13F0h]
//     __int64 v198;                                                                                                             // [rsp+310h] [rbp-13E8h]
//     double v199;                                                                                                              // [rsp+318h] [rbp-13E0h]
//     double v200;                                                                                                              // [rsp+320h] [rbp-13D8h]
//     double v201;                                                                                                              // [rsp+328h] [rbp-13D0h]
//     double v202;                                                                                                              // [rsp+330h] [rbp-13C8h]
//     double v203;                                                                                                              // [rsp+338h] [rbp-13C0h]
//     double v204;                                                                                                              // [rsp+340h] [rbp-13B8h]
//     long double d;                                                                                                            // [rsp+348h] [rbp-13B0h]
//     long double v206;                                                                                                         // [rsp+350h] [rbp-13A8h] BYREF
//     curve_curve_int* v207;                                                                                                    // [rsp+358h] [rbp-13A0h]
//     __int64 v208;                                                                                                             // [rsp+360h] [rbp-1398h]
//     double v209;                                                                                                              // [rsp+368h] [rbp-1390h]
//     double v210;                                                                                                              // [rsp+370h] [rbp-1388h]
//     double v211;                                                                                                              // [rsp+378h] [rbp-1380h]
//     double v212;                                                                                                              // [rsp+380h] [rbp-1378h]
//     double v213;                                                                                                              // [rsp+388h] [rbp-1370h]
//     double v214;                                                                                                              // [rsp+390h] [rbp-1368h]
//     long double v215;                                                                                                         // [rsp+398h] [rbp-1360h]
//     long double FUZZ_TOL;                                                                                                     // [rsp+3A0h] [rbp-1358h]
//     long double v217;                                                                                                         // [rsp+3A8h] [rbp-1350h] BYREF
//     curve_curve_int* v218;                                                                                                    // [rsp+3B0h] [rbp-1348h]
//     __int64 v219;                                                                                                             // [rsp+3B8h] [rbp-1340h]
//     curve_curve_int* v220;                                                                                                    // [rsp+3C0h] [rbp-1338h]
//     __int64 v221;                                                                                                             // [rsp+3C8h] [rbp-1330h]
//     curve_curve_int* v222;                                                                                                    // [rsp+3D0h] [rbp-1328h]
//     __int64 v223;                                                                                                             // [rsp+3D8h] [rbp-1320h]
//     CURVE* v224;                                                                                                              // [rsp+3E0h] [rbp-1318h]
//     const curve* v225;                                                                                                        // [rsp+3E8h] [rbp-1310h]
//     long double v226;                                                                                                         // [rsp+3F0h] [rbp-1308h] BYREF
//     curve_curve_int* v227;                                                                                                    // [rsp+3F8h] [rbp-1300h]
//     __int64 v228;                                                                                                             // [rsp+400h] [rbp-12F8h]
//     curve_curve_int* v229;                                                                                                    // [rsp+408h] [rbp-12F0h]
//     __int64 v230;                                                                                                             // [rsp+410h] [rbp-12E8h]
//     long double t;                                                                                                            // [rsp+418h] [rbp-12E0h] BYREF
//     curve_curve_int* v232;                                                                                                    // [rsp+420h] [rbp-12D8h]
//     __int64 v233;                                                                                                             // [rsp+428h] [rbp-12D0h]
//     __int64 v234;                                                                                                             // [rsp+430h] [rbp-12C8h]
//     curve_curve_int* v235;                                                                                                    // [rsp+438h] [rbp-12C0h]
//     CURVE* v236;                                                                                                              // [rsp+440h] [rbp-12B8h]
//     const curve* v237;                                                                                                        // [rsp+448h] [rbp-12B0h]
//     CURVE* v238;                                                                                                              // [rsp+450h] [rbp-12A8h]
//     const curve* v239;                                                                                                        // [rsp+458h] [rbp-12A0h]
//     CURVE* v240;                                                                                                              // [rsp+460h] [rbp-1298h]
//     const curve* v241;                                                                                                        // [rsp+468h] [rbp-1290h]
//     CURVE* v242;                                                                                                              // [rsp+470h] [rbp-1288h]
//     const curve* v243;                                                                                                        // [rsp+478h] [rbp-1280h]
//     exception_save exception_save_mark;                                                                                       // [rsp+480h] [rbp-1278h] BYREF
//     SPAvector centre_offset;                                                                                                  // [rsp+490h] [rbp-1268h] BYREF
//     SPAunit_vector line_dir;                                                                                                  // [rsp+4A8h] [rbp-1250h] BYREF
//     SPAposition inter;                                                                                                        // [rsp+4C0h] [rbp-1238h] BYREF
//     SPAposition*(__fastcall * v248)(struct straight*, SPAposition*, long double, int, int);                                   // [rsp+4D8h] [rbp-1220h]
//     char* v249;                                                                                                               // [rsp+4E0h] [rbp-1218h]
//     long double started;                                                                                                      // [rsp+4E8h] [rbp-1210h] BYREF
//     long double v251[2];                                                                                                      // [rsp+4F0h] [rbp-1208h] BYREF
//     SPAposition*(__fastcall * v252)(struct ellipse*, SPAposition*, long double, int, int);                                    // [rsp+500h] [rbp-11F8h]
//     __int64 v253;                                                                                                             // [rsp+508h] [rbp-11F0h]
//     SPAposition*(__fastcall * v254)(struct ellipse*, SPAposition*, long double, int, int);                                    // [rsp+510h] [rbp-11E8h]
//     SPAposition* p2;                                                                                                          // [rsp+518h] [rbp-11E0h]
//     SPAposition* p1;                                                                                                          // [rsp+520h] [rbp-11D8h]
//     SPAposition* v257;                                                                                                        // [rsp+528h] [rbp-11D0h]
//     SPAposition* v258;                                                                                                        // [rsp+530h] [rbp-11C8h]
//     SPAposition* v259;                                                                                                        // [rsp+538h] [rbp-11C0h]
//     SPAposition* v260;                                                                                                        // [rsp+540h] [rbp-11B8h]
//     SPAposition* v261;                                                                                                        // [rsp+548h] [rbp-11B0h]
//     SPAposition* v262;                                                                                                        // [rsp+550h] [rbp-11A8h]
//     SPAparameter* parameter;                                                                                                  // [rsp+558h] [rbp-11A0h]
//     SPAparameter* v264;                                                                                                       // [rsp+560h] [rbp-1198h]
//     SPAposition* v265;                                                                                                        // [rsp+568h] [rbp-1190h]
//     int(__fastcall * test_point_tol)(struct straight*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);  // [rsp+570h] [rbp-1188h]
//     SPAparameter* v267;                                                                                                       // [rsp+578h] [rbp-1180h]
//     SPAparameter* v268;                                                                                                       // [rsp+580h] [rbp-1178h]
//     SPAposition* v269;                                                                                                        // [rsp+588h] [rbp-1170h]
//     int(__fastcall * v270)(struct straight*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);            // [rsp+590h] [rbp-1168h]
//     SPAparameter* v271;                                                                                                       // [rsp+598h] [rbp-1160h]
//     SPAparameter* v272;                                                                                                       // [rsp+5A0h] [rbp-1158h]
//     SPAposition* v273;                                                                                                        // [rsp+5A8h] [rbp-1150h]
//     int(__fastcall * v274)(struct ellipse*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);             // [rsp+5B0h] [rbp-1148h]
//     SPAparameter* v275;                                                                                                       // [rsp+5B8h] [rbp-1140h]
//     SPAparameter* v276;                                                                                                       // [rsp+5C0h] [rbp-1138h]
//     SPAposition* v277;                                                                                                        // [rsp+5C8h] [rbp-1130h]
//     int(__fastcall * v278)(struct ellipse*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);             // [rsp+5D0h] [rbp-1128h]
//     void(__fastcall * v279)(curve*);                                                                                          // [rsp+5D8h] [rbp-1120h]
//     __int64 v280;                                                                                                             // [rsp+5E0h] [rbp-1118h]
//     void(__fastcall * v281)(curve*);                                                                                          // [rsp+5E8h] [rbp-1110h]
//     __int64 v282;                                                                                                             // [rsp+5F0h] [rbp-1108h]
//     curve_curve_int* v283;                                                                                                    // [rsp+5F8h] [rbp-1100h]
//     SPAvector* v2;                                                                                                            // [rsp+600h] [rbp-10F8h]
//     long double other_angle;                                                                                                  // [rsp+608h] [rbp-10F0h]
//     double v286;                                                                                                              // [rsp+610h] [rbp-10E8h]
//     SPAvector* v287;                                                                                                          // [rsp+618h] [rbp-10E0h]
//     SPAvector* v1;                                                                                                            // [rsp+620h] [rbp-10D8h]
//     SPAvector* v289;                                                                                                          // [rsp+628h] [rbp-10D0h]
//     SPAvector* v290;                                                                                                          // [rsp+630h] [rbp-10C8h]
//     SPAvector* v291;                                                                                                          // [rsp+638h] [rbp-10C0h]
//     SPAvector* v292;                                                                                                          // [rsp+640h] [rbp-10B8h]
//     SPAparameter* v293;                                                                                                       // [rsp+648h] [rbp-10B0h]
//     int(__fastcall * v294)(struct ellipse*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);             // [rsp+650h] [rbp-10A8h]
//     long double v295;                                                                                                         // [rsp+658h] [rbp-10A0h]
//     SPAposition*(__fastcall * v296)(struct ellipse*, SPAposition*, long double, int, int);                                    // [rsp+660h] [rbp-1098h]
//     SPAposition* v297;                                                                                                        // [rsp+668h] [rbp-1090h]
//     SPAposition* v298;                                                                                                        // [rsp+670h] [rbp-1088h]
//     SPAposition* v299;                                                                                                        // [rsp+678h] [rbp-1080h]
//     SPAposition* v300;                                                                                                        // [rsp+680h] [rbp-1078h]
//     long double min_box_len;                                                                                                  // [rsp+688h] [rbp-1070h]
//     SPAparameter* v302;                                                                                                       // [rsp+690h] [rbp-1068h]
//     SPAparameter* v303;                                                                                                       // [rsp+698h] [rbp-1060h]
//     int(__fastcall * v304)(struct ellipse*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);             // [rsp+6A0h] [rbp-1058h]
//     SPAparameter* v305;                                                                                                       // [rsp+6A8h] [rbp-1050h]
//     SPAparameter* v306;                                                                                                       // [rsp+6B0h] [rbp-1048h]
//     int(__fastcall * v307)(struct ellipse*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);             // [rsp+6B8h] [rbp-1040h]
//     SPAvector*(__fastcall * v308)(struct ellipse*, SPAvector*, long double, int, int);                                        // [rsp+6C0h] [rbp-1038h]
//     double* v309;                                                                                                             // [rsp+6C8h] [rbp-1030h]
//     long double mid_pt;                                                                                                       // [rsp+6D0h] [rbp-1028h] BYREF
//     SPApar_pos* _uv;                                                                                                          // [rsp+6D8h] [rbp-1020h]
//     long double d2;                                                                                                           // [rsp+6E0h] [rbp-1018h]
//     long double d1;                                                                                                           // [rsp+6E8h] [rbp-1010h]
//     __int64 v314;                                                                                                             // [rsp+6F0h] [rbp-1008h]
//     double v315;                                                                                                              // [rsp+6F8h] [rbp-1000h]
//     SPAvector* v316;                                                                                                          // [rsp+700h] [rbp-FF8h]
//     SPAvector* v317;                                                                                                          // [rsp+708h] [rbp-FF0h]
//     SPAposition* v318;                                                                                                        // [rsp+710h] [rbp-FE8h]
//     SPAposition* v319;                                                                                                        // [rsp+718h] [rbp-FE0h]
//     SPAposition* v320;                                                                                                        // [rsp+720h] [rbp-FD8h]
//     SPAposition* v321;                                                                                                        // [rsp+728h] [rbp-FD0h]
//     double v322;                                                                                                              // [rsp+730h] [rbp-FC8h]
//     SPAparameter* v323;                                                                                                       // [rsp+738h] [rbp-FC0h]
//     SPAparameter* v324;                                                                                                       // [rsp+740h] [rbp-FB8h]
//     int(__fastcall * v325)(struct ellipse*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);             // [rsp+748h] [rbp-FB0h]
//     SPAparameter* v326;                                                                                                       // [rsp+750h] [rbp-FA8h]
//     SPAparameter* v327;                                                                                                       // [rsp+758h] [rbp-FA0h]
//     int(__fastcall * v328)(struct ellipse*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);             // [rsp+760h] [rbp-F98h]
//     SPAvector*(__fastcall * v329)(struct ellipse*, SPAvector*, long double, int, int);                                        // [rsp+768h] [rbp-F90h]
//     double* v330;                                                                                                             // [rsp+770h] [rbp-F88h]
//     long double range_mid_pt;                                                                                                 // [rsp+778h] [rbp-F80h] BYREF
//     SPApar_pos* v332;                                                                                                         // [rsp+780h] [rbp-F78h]
//     long double v333;                                                                                                         // [rsp+788h] [rbp-F70h]
//     long double v334;                                                                                                         // [rsp+790h] [rbp-F68h]
//     __int64 v335;                                                                                                             // [rsp+798h] [rbp-F60h]
//     SPAparameter* v336;                                                                                                       // [rsp+7A0h] [rbp-F58h]
//     int(__fastcall * v337)(struct ellipse*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);             // [rsp+7A8h] [rbp-F50h]
//     long double v338;                                                                                                         // [rsp+7B0h] [rbp-F48h]
//     SPAposition*(__fastcall * v339)(struct ellipse*, SPAposition*, long double, int, int);                                    // [rsp+7B8h] [rbp-F40h]
//     *v340;                                                                                                                    // [rsp+7C0h] [rbp-F38h]
//     SPAposition* v341;                                                                                                        // [rsp+7C8h] [rbp-F30h]
//     SPAposition* v342;                                                                                                        // [rsp+7D0h] [rbp-F28h]
//     SPAposition* v343;                                                                                                        // [rsp+7D8h] [rbp-F20h]
//     double v344;                                                                                                              // [rsp+7E0h] [rbp-F18h]
//     SPAparameter* v345;                                                                                                       // [rsp+7E8h] [rbp-F10h]
//     SPAparameter* v346;                                                                                                       // [rsp+7F0h] [rbp-F08h]
//     int(__fastcall * v347)(struct ellipse*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);             // [rsp+7F8h] [rbp-F00h]
//     SPAparameter* v348;                                                                                                       // [rsp+800h] [rbp-EF8h]
//     SPAparameter* v349;                                                                                                       // [rsp+808h] [rbp-EF0h]
//     int(__fastcall * v350)(struct ellipse*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);             // [rsp+810h] [rbp-EE8h]
//     long double v351;                                                                                                         // [rsp+818h] [rbp-EE0h]
//     SPAvector*(__fastcall * eval_deriv)(struct ellipse*, SPAvector*, long double, int, int);                                  // [rsp+820h] [rbp-ED8h]
//     long double half_chord;                                                                                                   // [rsp+828h] [rbp-ED0h]
//     SPAvector* v354;                                                                                                          // [rsp+830h] [rbp-EC8h]
//     SPAvector* v355;                                                                                                          // [rsp+838h] [rbp-EC0h]
//     double* v356;                                                                                                             // [rsp+840h] [rbp-EB8h]
//     long double v357;                                                                                                         // [rsp+848h] [rbp-EB0h] BYREF
//     SPApar_pos* v358;                                                                                                         // [rsp+850h] [rbp-EA8h]
//     long double v359;                                                                                                         // [rsp+858h] [rbp-EA0h]
//     long double v360;                                                                                                         // [rsp+860h] [rbp-E98h]
//     __int64 v361;                                                                                                             // [rsp+868h] [rbp-E90h]
//     double* v362;                                                                                                             // [rsp+870h] [rbp-E88h]
//     long double v363;                                                                                                         // [rsp+878h] [rbp-E80h] BYREF
//     SPApar_pos* v364;                                                                                                         // [rsp+880h] [rbp-E78h]
//     long double v365;                                                                                                         // [rsp+888h] [rbp-E70h]
//     long double v366;                                                                                                         // [rsp+890h] [rbp-E68h]
//     __int64 v367;                                                                                                             // [rsp+898h] [rbp-E60h]
//     double* v368;                                                                                                             // [rsp+8A0h] [rbp-E58h]
//     long double v369;                                                                                                         // [rsp+8A8h] [rbp-E50h] BYREF
//     SPApar_pos* v370;                                                                                                         // [rsp+8B0h] [rbp-E48h]
//     long double v371;                                                                                                         // [rsp+8B8h] [rbp-E40h]
//     long double v372;                                                                                                         // [rsp+8C0h] [rbp-E38h]
//     curve_curve_int* n;                                                                                                       // [rsp+8C8h] [rbp-E30h]
//     __int64 v374;                                                                                                             // [rsp+8D0h] [rbp-E28h]
//     long double res;                                                                                                          // [rsp+8D8h] [rbp-E20h]
//     SPAposition* v376;                                                                                                        // [rsp+8E0h] [rbp-E18h]
//     long double v377;                                                                                                         // [rsp+8E8h] [rbp-E10h]
//     SPAposition* v378;                                                                                                        // [rsp+8F0h] [rbp-E08h]
//     long double v379;                                                                                                         // [rsp+8F8h] [rbp-E00h]
//     SPAposition* v380;                                                                                                        // [rsp+900h] [rbp-DF8h]
//     long double v381;                                                                                                         // [rsp+908h] [rbp-DF0h]
//     SPAposition* v382;                                                                                                        // [rsp+910h] [rbp-DE8h]
//     long double v383;                                                                                                         // [rsp+918h] [rbp-DE0h]
//     SPAposition* v384;                                                                                                        // [rsp+920h] [rbp-DD8h]
//     long double v385;                                                                                                         // [rsp+928h] [rbp-DD0h]
//     SPAposition* v386;                                                                                                        // [rsp+930h] [rbp-DC8h]
//     long double v387;                                                                                                         // [rsp+938h] [rbp-DC0h]
//     SPAposition* v388;                                                                                                        // [rsp+940h] [rbp-DB8h]
//     long double v389;                                                                                                         // [rsp+948h] [rbp-DB0h]
//     SPAposition* v390;                                                                                                        // [rsp+950h] [rbp-DA8h]
//     long double dist;                                                                                                         // [rsp+958h] [rbp-DA0h]
//     SPAparameter* v392;                                                                                                       // [rsp+960h] [rbp-D98h]
//     SPAparameter* v393;                                                                                                       // [rsp+968h] [rbp-D90h]
//     int(__fastcall * v394)(curve*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);                      // [rsp+970h] [rbp-D88h]
//     double* v395;                                                                                                             // [rsp+978h] [rbp-D80h]
//     long double v396;                                                                                                         // [rsp+980h] [rbp-D78h] BYREF
//     SPApar_pos* par_pos;                                                                                                      // [rsp+988h] [rbp-D70h]
//     long double v398;                                                                                                         // [rsp+990h] [rbp-D68h]
//     long double v399;                                                                                                         // [rsp+998h] [rbp-D60h]
//     __int64 v400;                                                                                                             // [rsp+9A0h] [rbp-D58h]
//     SPApar_pos* v401;                                                                                                         // [rsp+9A8h] [rbp-D50h]
//     long double v402;                                                                                                         // [rsp+9B0h] [rbp-D48h]
//     long double v403;                                                                                                         // [rsp+9B8h] [rbp-D40h]
//     curve_curve_int* v404;                                                                                                    // [rsp+9C0h] [rbp-D38h]
//     __int64 v405;                                                                                                             // [rsp+9C8h] [rbp-D30h]
//     SPAparameter* v406;                                                                                                       // [rsp+9D0h] [rbp-D28h]
//     int(__fastcall * v407)(struct ellipse*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);             // [rsp+9D8h] [rbp-D20h]
//     double* v408;                                                                                                             // [rsp+9E0h] [rbp-D18h]
//     long double v409;                                                                                                         // [rsp+9E8h] [rbp-D10h] BYREF
//     long double tol;                                                                                                          // [rsp+9F0h] [rbp-D08h]
//     long double v411;                                                                                                         // [rsp+9F8h] [rbp-D00h]
//     long double d_high;                                                                                                       // [rsp+A00h] [rbp-CF8h]
//     long double d_low;                                                                                                        // [rsp+A08h] [rbp-CF0h]
//     long double v414;                                                                                                         // [rsp+A10h] [rbp-CE8h]
//     SPAposition*(__fastcall * v415)(struct ellipse*, SPAposition*, long double, int, int);                                    // [rsp+A18h] [rbp-CE0h]
//     SPApar_pos* v416;                                                                                                         // [rsp+A20h] [rbp-CD8h]
//     long double v417;                                                                                                         // [rsp+A28h] [rbp-CD0h]
//     long double v418;                                                                                                         // [rsp+A30h] [rbp-CC8h]
//     curve_curve_int* v419;                                                                                                    // [rsp+A38h] [rbp-CC0h]
//     __int64 v420;                                                                                                             // [rsp+A40h] [rbp-CB8h]
//     SPAparameter* v421;                                                                                                       // [rsp+A48h] [rbp-CB0h]
//     int(__fastcall * v422)(struct straight*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);            // [rsp+A50h] [rbp-CA8h]
//     long double v423;                                                                                                         // [rsp+A58h] [rbp-CA0h]
//     long double v424;                                                                                                         // [rsp+A60h] [rbp-C98h]
//     long double v425;                                                                                                         // [rsp+A68h] [rbp-C90h]
//     long double v426;                                                                                                         // [rsp+A70h] [rbp-C88h]
//     long double v427;                                                                                                         // [rsp+A78h] [rbp-C80h]
//     SPAposition*(__fastcall * v428)(struct straight*, SPAposition*, long double, int, int);                                   // [rsp+A80h] [rbp-C78h]
//     SPApar_pos* v429;                                                                                                         // [rsp+A88h] [rbp-C70h]
//     long double v430;                                                                                                         // [rsp+A90h] [rbp-C68h]
//     long double v431;                                                                                                         // [rsp+A98h] [rbp-C60h]
//     curve_curve_int* v432;                                                                                                    // [rsp+AA0h] [rbp-C58h]
//     __int64 v433;                                                                                                             // [rsp+AA8h] [rbp-C50h]
//     void(__fastcall * v434)(curve*);                                                                                          // [rsp+AB0h] [rbp-C48h]
//     __int64 v435;                                                                                                             // [rsp+AB8h] [rbp-C40h]
//     void(__fastcall * v436)(curve*);                                                                                          // [rsp+AC0h] [rbp-C38h]
//     __int64 v437;                                                                                                             // [rsp+AC8h] [rbp-C30h]
//     AcisVersion v438;                                                                                                         // [rsp+AD0h] [rbp-C28h] BYREF
//     AcisVersion result;                                                                                                       // [rsp+AD4h] [rbp-C24h] BYREF
//     _iobuf* v440;                                                                                                             // [rsp+AD8h] [rbp-C20h]
//     void(__fastcall * debug)(curve*, const char*, _iobuf*);                                                                   // [rsp+AE0h] [rbp-C18h]
//     _iobuf* v442;                                                                                                             // [rsp+AE8h] [rbp-C10h]
//     void(__fastcall * v443)(curve*, const char*, _iobuf*);                                                                    // [rsp+AF0h] [rbp-C08h]
//     AcisVersion* vt2;                                                                                                         // [rsp+AF8h] [rbp-C00h]
//     AcisVersion* vt1;                                                                                                         // [rsp+B00h] [rbp-BF8h]
//     char* v446;                                                                                                               // [rsp+B08h] [rbp-BF0h]
//     long double val;                                                                                                          // [rsp+B10h] [rbp-BE8h] BYREF
//     long double v448[2];                                                                                                      // [rsp+B18h] [rbp-BE0h] BYREF
//     SPAposition*(__fastcall * eval_position)(struct straight*, SPAposition*, long double, int, int);                          // [rsp+B28h] [rbp-BD0h]
//     __int64 v450;                                                                                                             // [rsp+B30h] [rbp-BC8h]
//     SPAvector chord_offset;                                                                                                   // [rsp+B38h] [rbp-BC0h] BYREF
//     SPAposition ln_root;                                                                                                      // [rsp+B50h] [rbp-BA8h] BYREF
//     SPAposition pos2;                                                                                                         // [rsp+B68h] [rbp-B90h] BYREF
//     SPAposition pos1;                                                                                                         // [rsp+B80h] [rbp-B78h] BYREF
//     SPAposition v455;                                                                                                         // [rsp+B98h] [rbp-B60h] BYREF
//     SPAposition v456;                                                                                                         // [rsp+BB0h] [rbp-B48h] BYREF
//     acis_exception error_info_holder;                                                                                         // [rsp+BC8h] [rbp-B30h] BYREF
//     long double SMALL_FEATURE_FACTOR;                                                                                         // [rsp+BE8h] [rbp-B10h]
//     long double v459;                                                                                                         // [rsp+BF0h] [rbp-B08h]
//     long double v460;                                                                                                         // [rsp+BF8h] [rbp-B00h]
//     SPAinterval ed1_param_range;                                                                                              // [rsp+C00h] [rbp-AF8h] BYREF
//     SPAinterval str_param_range;                                                                                              // [rsp+C18h] [rbp-AE0h] BYREF
//     SPAinterval ed2_param_range;                                                                                              // [rsp+C30h] [rbp-AC8h] BYREF
//     SPAinterval ell_param_range;                                                                                              // [rsp+C48h] [rbp-AB0h] BYREF
//     SPAunit_vector str_dir;                                                                                                   // [rsp+C60h] [rbp-A98h] BYREF
//     SPAvector parallel_offset;                                                                                                // [rsp+C78h] [rbp-A80h] BYREF
//     SPAunit_vector v467;                                                                                                      // [rsp+C90h] [rbp-A68h] BYREF
//     SPAposition v468;                                                                                                         // [rsp+CA8h] [rbp-A50h] BYREF
//     SPAunit_vector v469;                                                                                                      // [rsp+CC0h] [rbp-A38h] BYREF
//     SPAposition pl_root;                                                                                                      // [rsp+CD8h] [rbp-A20h] BYREF
//     SPAposition v471;                                                                                                         // [rsp+CF0h] [rbp-A08h] BYREF
//     SPAposition v472;                                                                                                         // [rsp+D08h] [rbp-9F0h] BYREF
//     SPAparameter v473;                                                                                                        // [rsp+D20h] [rbp-9D8h] BYREF
//     char v474[24];                                                                                                            // [rsp+D68h] [rbp-990h] BYREF
//     SPAposition ell_pos;                                                                                                      // [rsp+D80h] [rbp-978h] BYREF
//     SPAposition new_perp_foot;                                                                                                // [rsp+D98h] [rbp-960h] BYREF
//     SPAposition str_pos1;                                                                                                     // [rsp+DB0h] [rbp-948h] BYREF
//     SPAposition str_pos2;                                                                                                     // [rsp+DC8h] [rbp-930h] BYREF
//     SPAposition v479;                                                                                                         // [rsp+DE0h] [rbp-918h] BYREF
//     SPAposition v480;                                                                                                         // [rsp+DF8h] [rbp-900h] BYREF
//     SPAposition v481;                                                                                                         // [rsp+E10h] [rbp-8E8h] BYREF
//     SPAposition v482;                                                                                                         // [rsp+E28h] [rbp-8D0h] BYREF
//     SPAposition v483;                                                                                                         // [rsp+E40h] [rbp-8B8h] BYREF
//     SPAunit_vector pl_nor;                                                                                                    // [rsp+E58h] [rbp-8A0h] BYREF
//     SPAunit_vector ln_nor;                                                                                                    // [rsp+E70h] [rbp-888h] BYREF
//     SPAposition p;                                                                                                            // [rsp+E88h] [rbp-870h] BYREF
//     SPAposition v487;                                                                                                         // [rsp+EA0h] [rbp-858h] BYREF
//     SPAposition v488;                                                                                                         // [rsp+EB8h] [rbp-840h] BYREF
//     char v489[24];                                                                                                            // [rsp+ED0h] [rbp-828h] BYREF
//     SPAinterval v490;                                                                                                         // [rsp+EE8h] [rbp-810h] BYREF
//     char v491[24];                                                                                                            // [rsp+F00h] [rbp-7F8h] BYREF
//     char v492[24];                                                                                                            // [rsp+F18h] [rbp-7E0h] BYREF
//     SPAvector v493;                                                                                                           // [rsp+F30h] [rbp-7C8h] BYREF
//     SPAvector v494;                                                                                                           // [rsp+F48h] [rbp-7B0h] BYREF
//     SPAvector v495;                                                                                                           // [rsp+F60h] [rbp-798h] BYREF
//     SPAvector v496;                                                                                                           // [rsp+F78h] [rbp-780h] BYREF
//     SPAvector v497;                                                                                                           // [rsp+F90h] [rbp-768h] BYREF
//     SPAvector v498;                                                                                                           // [rsp+FA8h] [rbp-750h] BYREF
//     SPAvector v499;                                                                                                           // [rsp+FC0h] [rbp-738h] BYREF
//     SPAvector v500;                                                                                                           // [rsp+FD8h] [rbp-720h] BYREF
//     SPAvector v501;                                                                                                           // [rsp+FF0h] [rbp-708h] BYREF
//     SPAunit_vector v502;                                                                                                      // [rsp+1008h] [rbp-6F0h] BYREF
//     SPAvector v503;                                                                                                           // [rsp+1020h] [rbp-6D8h] BYREF
//     SPAvector v504;                                                                                                           // [rsp+1038h] [rbp-6C0h] BYREF
//     SPAvector v505;                                                                                                           // [rsp+1050h] [rbp-6A8h] BYREF
//     SPAposition v506;                                                                                                         // [rsp+1068h] [rbp-690h] BYREF
//     SPAposition v507;                                                                                                         // [rsp+1080h] [rbp-678h] BYREF
//     SPAvector v508;                                                                                                           // [rsp+1098h] [rbp-660h] BYREF
//     SPAposition v509;                                                                                                         // [rsp+10B0h] [rbp-648h] BYREF
//     SPAposition v510;                                                                                                         // [rsp+10C8h] [rbp-630h] BYREF
//     SPAvector v511;                                                                                                           // [rsp+10E0h] [rbp-618h] BYREF
//     SPAvector v512;                                                                                                           // [rsp+10F8h] [rbp-600h] BYREF
//     SPAvector v513;                                                                                                           // [rsp+1110h] [rbp-5E8h] BYREF
//     char v514[24];                                                                                                            // [rsp+1128h] [rbp-5D0h] BYREF
//     SPAvector v515;                                                                                                           // [rsp+1140h] [rbp-5B8h] BYREF
//     SPAunit_vector v516;                                                                                                      // [rsp+1158h] [rbp-5A0h] BYREF
//     SPAvector v517;                                                                                                           // [rsp+1170h] [rbp-588h] BYREF
//     SPAvector v518;                                                                                                           // [rsp+1188h] [rbp-570h] BYREF
//     SPAvector v519;                                                                                                           // [rsp+11A0h] [rbp-558h] BYREF
//     SPAvector v520;                                                                                                           // [rsp+11B8h] [rbp-540h] BYREF
//     SPAposition v521;                                                                                                         // [rsp+11D0h] [rbp-528h] BYREF
//     SPAposition v522;                                                                                                         // [rsp+11E8h] [rbp-510h] BYREF
//     SPAposition v523;                                                                                                         // [rsp+1200h] [rbp-4F8h] BYREF
//     SPAvector v524;                                                                                                           // [rsp+1218h] [rbp-4E0h] BYREF
//     SPAposition v525;                                                                                                         // [rsp+1230h] [rbp-4C8h] BYREF
//     SPAposition v526;                                                                                                         // [rsp+1248h] [rbp-4B0h] BYREF
//     SPAvector v527;                                                                                                           // [rsp+1260h] [rbp-498h] BYREF
//     SPAvector v528;                                                                                                           // [rsp+1278h] [rbp-480h] BYREF
//     SPAvector v529;                                                                                                           // [rsp+1290h] [rbp-468h] BYREF
//     char v530[24];                                                                                                            // [rsp+12A8h] [rbp-450h] BYREF
//     SPAvector v531;                                                                                                           // [rsp+12C0h] [rbp-438h] BYREF
//     SPAposition v532;                                                                                                         // [rsp+12D8h] [rbp-420h] BYREF
//     SPAposition v533;                                                                                                         // [rsp+12F0h] [rbp-408h] BYREF
//     SPAvector v534;                                                                                                           // [rsp+1308h] [rbp-3F0h] BYREF
//     SPAposition v535;                                                                                                         // [rsp+1320h] [rbp-3D8h] BYREF
//     SPAposition v536;                                                                                                         // [rsp+1338h] [rbp-3C0h] BYREF
//     SPAvector v537;                                                                                                           // [rsp+1350h] [rbp-3A8h] BYREF
//     SPAvector v538;                                                                                                           // [rsp+1368h] [rbp-390h] BYREF
//     SPAvector v539;                                                                                                           // [rsp+1380h] [rbp-378h] BYREF
//     char v540[24];                                                                                                            // [rsp+1398h] [rbp-360h] BYREF
//     SPAvector v541;                                                                                                           // [rsp+13B0h] [rbp-348h] BYREF
//     SPAvector v542;                                                                                                           // [rsp+13C8h] [rbp-330h] BYREF
//     SPAvector v543;                                                                                                           // [rsp+13E0h] [rbp-318h] BYREF
//     SPAposition v544;                                                                                                         // [rsp+13F8h] [rbp-300h] BYREF
//     SPAposition v545;                                                                                                         // [rsp+1410h] [rbp-2E8h] BYREF
//     SPAvector v546;                                                                                                           // [rsp+1428h] [rbp-2D0h] BYREF
//     SPAposition v547;                                                                                                         // [rsp+1440h] [rbp-2B8h] BYREF
//     SPAvector v548;                                                                                                           // [rsp+1458h] [rbp-2A0h] BYREF
//     SPAvector v549;                                                                                                           // [rsp+1470h] [rbp-288h] BYREF
//     SPAvector v550;                                                                                                           // [rsp+1488h] [rbp-270h] BYREF
//     SPAposition v551;                                                                                                         // [rsp+14A0h] [rbp-258h] BYREF
//     SPAinterval v552;                                                                                                         // [rsp+14B8h] [rbp-240h] BYREF
//     char v553[24];                                                                                                            // [rsp+14D0h] [rbp-228h] BYREF
//     char v554[24];                                                                                                            // [rsp+14E8h] [rbp-210h] BYREF
//     SPAbox bx1;                                                                                                               // [rsp+1500h] [rbp-1F8h] BYREF
//     SPAbox bx2;                                                                                                               // [rsp+1550h] [rbp-1A8h] BYREF
//     SPAbox v557;                                                                                                              // [rsp+15A0h] [rbp-158h] BYREF
//     SPAbox v558;                                                                                                              // [rsp+15F0h] [rbp-108h] BYREF
//     SPAbox v559;                                                                                                              // [rsp+1640h] [rbp-B8h] BYREF
//     SPAbox v560;                                                                                                              // [rsp+1690h] [rbp-68h] BYREF
//
//     if(sg_inter_module_header.debug_level >= 0x1E) {
//         acis_fprintf(debug_file_ptr, "STRAIGHT EDGE Debug..\n");
//         v236 = EDGE::geometry(edge1);
//         v237 = v236->equation(v236);
//         debug = v237->debug;
//         v440 = debug_file_ptr;
//         debug((curve*)v237, "\t", debug_file_ptr);
//         debug_newline(debug_file_ptr);
//         acis_fprintf(debug_file_ptr, "ELLIPSE EDGE Debug..\n");
//         v238 = EDGE::geometry(edge2);
//         v239 = v238->equation(v238);
//         v443 = v239->debug;
//         v442 = debug_file_ptr;
//         v443((curve*)v239, "\t", debug_file_ptr);
//         debug_newline(debug_file_ptr);
//     }
//
//     ccrel_data = 0i64;
//     *difficult_config = 0;
//     tangent_root = 0;
//     inter_ed_ed_robustness;
//     AcisVersion::AcisVersion(&v438, 22, 0, 0);
//     vt2 = v5;
//     vt1 = GET_ALGORITHMIC_VERSION(&result);
//     v6 = operator>=(vt1, vt2);
//     inter_ed_ed_robustness = v6;
//     if(!v6) return line_ell_int_old(edge1, edge2, dist_tol_in, angle_tol_in);
//     resignal_no = 0;
//     acis_exception::acis_exception(&error_info_holder, 0, 0i64, 0i64, 0);
//     error_info_base_ptr = 0i64;
//     exception_save::exception_save(&exception_save_mark);
//     ed_cur1 = 0i64;
//     ed_cur2 = 0i64;
//     exception_save::begin(&exception_save_mark);
//     get_error_mark()->buffer_init = 1;
//     error_no = 0;
//     dist_tol = dist_tol_in;
//     angle_tol = angle_tol_in;
//     v240 = EDGE::geometry(edge1);
//     v241 = v240->equation(v240);
//     ed_cur1 = v241->make_copy(v241);
//
//     v242 = EDGE::geometry(edge2);
//     v243 = v242->equation(v242);
//     ed_cur2 = v243->make_copy(v243);
//
//     st_eq = (straight*)ed_cur1;
//     ell_eq = (ellipse*)ed_cur2;
//
//     ed1_param_range;
//
//     /*  EDGE::param_range(edge1, &ed1_param_range);
//       if(EDGE::sense(edge1) == 1) {
//           v186 = (char*)operator-(&v552, &ed1_param_range);
//       } else {
//           qmemcpy(v489, &ed1_param_range, sizeof(v489));
//           v186 = v489;
//       }
//       v446 = v186;*/
//     str_param_range;         ////直线的参数域
//     str_prospective_params;  // 储存了edge1的开始参数与end参数
//     str_prospective_sols;    // str_prospective_sols储存了edge1的开始点与结尾点
//     ell_prospective_params;
//     ell_prospective_sols;
//     qmemcpy(&str_param_range, v186, sizeof(str_param_range));
//     SPAdouble_array::SPAdouble_array(&str_prospective_params, 0, 2);
//     val = SPAinterval::start_pt(&str_param_range);
//     SPAdouble_array::Push(&str_prospective_params, &val);
//     ////str_prospective_params储存了edge1的开始参数与end参数
//     v448[0] = SPAinterval::end_pt(&str_param_range);
//     SPAdouble_array::Push(&str_prospective_params, v448);
//
//     SPAposition_array::SPAposition_array(&str_prospective_sols, 0, 2);
//     eval_position = st_eq->eval_position;
//     v448[1] = *SPAdouble_array::operator[](&str_prospective_params, 0);
//     v9 = (const SPAposition*)((__int64(__fastcall*)(straight*, char*, __int64, _QWORD, _DWORD))eval_position)(st_eq, v553, v8, 0i64, 0);
//     SPAposition_array::Push(&str_prospective_sols, v9);
//     v248 = st_eq->eval_position;
//     v450 = *(_QWORD*)SPAdouble_array::operator[](&str_prospective_params, 1);
//     v11 = (const SPAposition*)((__int64(__fastcall*)(straight*, char*, __int64, _QWORD, _DWORD))v248)(st_eq, v554, v10, 0i64, 0);
//     SPAposition_array::Push(&str_prospective_sols, v11);
//     EDGE::param_range(edge2, &ed2_param_range);
//
//     if(EDGE::sense(edge2) == 1) {
//         v187 = (char*)operator-(&v490, &ed2_param_range);
//     } else {
//         qmemcpy(v474, &ed2_param_range, sizeof(v474));
//         v187 = v474;
//     }
//     v249 = v187;
//     qmemcpy(&ell_param_range, v187, sizeof(ell_param_range));
//     SPAdouble_array::SPAdouble_array(&ell_prospective_params, 0, 2);
//     started = SPAinterval::start_pt(&ell_param_range);
//     SPAdouble_array::Push(&ell_prospective_params, &started);
//     v251[0] = SPAinterval::end_pt(&ell_param_range);
//     SPAdouble_array::Push(&ell_prospective_params, v251);
//     SPAposition_array::SPAposition_array(&ell_prospective_sols, 0, 2);
//     v252 = ell_eq->eval_position;
//     v251[1] = *SPAdouble_array::operator[](&ell_prospective_params, 0);
//     v13 = (const SPAposition*)((__int64(__fastcall*)(ellipse*, char*, __int64, _QWORD, _DWORD))v252)(ell_eq, v491, v12, 0i64, 0);
//     SPAposition_array::Push(&ell_prospective_sols, v13);
//     v254 = ell_eq->eval_position;
//     v253 = *(_QWORD*)SPAdouble_array::operator[](&ell_prospective_params, 1);
//     v15 = (const SPAposition*)((__int64(__fastcall*)(ellipse*, char*, __int64, _QWORD, _DWORD))v254)(ell_eq, v492, v14, 0i64, 0);
//     SPAposition_array::Push(&ell_prospective_sols, v15);
//     // 同理
//
//     str_str_sol = 0;
//     str_end_sol = 0;
//     ell_str_sol = 0;
//     ell_end_sol = 0;
//     p2 = SPAposition_array::operator[](&ell_prospective_sols, 0);
//     p1 = SPAposition_array::operator[](&str_prospective_sols, 0);
//     coin_ss = same_point(p1, p2, dist_tol);
//     v257 = SPAposition_array::operator[](&ell_prospective_sols, 1);
//     v258 = SPAposition_array::operator[](&str_prospective_sols, 1);
//     coin_ee = same_point(v258, v257, dist_tol);
//     v259 = SPAposition_array::operator[](&ell_prospective_sols, 1);
//     v260 = SPAposition_array::operator[](&str_prospective_sols, 0);
//     coin_se = same_point(v260, v259, dist_tol);
//     v261 = SPAposition_array::operator[](&ell_prospective_sols, 0);
//     v262 = SPAposition_array::operator[](&str_prospective_sols, 1);
//     coin_es = same_point(v262, v261, dist_tol);
//     if(coin_ss && coin_ee) {
//         *difficult_config = 1;
//     } else if(coin_se && coin_es) {
//         *difficult_config = 1;
//     }
//     if(!*difficult_config) {
//         test_point_tol = st_eq->test_point_tol;
//         parameter = SpaAcis::NullObj::get_parameter();
//         v264 = SpaAcis::NullObj::get_parameter();
//         v265 = SPAposition_array::operator[](&ell_prospective_sols, 0);
//         ell_str_sol = ((__int64(__fastcall*)(straight*, SPAposition*, __int64, SPAparameter*, SPAparameter*))test_point_tol)(st_eq, v265, v16, v264, parameter);
//         v270 = st_eq->test_point_tol;
//         v267 = SpaAcis::NullObj::get_parameter();
//         v268 = SpaAcis::NullObj::get_parameter();
//         v269 = SPAposition_array::operator[](&ell_prospective_sols, 1);
//         ell_end_sol = ((__int64(__fastcall*)(straight*, SPAposition*, __int64, SPAparameter*, SPAparameter*))v270)(st_eq, v269, v17, v268, v267);
//         if(ell_str_sol) {
//             if(ell_end_sol && !EDGE::closed(edge2)) *difficult_config = 1;
//         }
//     }
//     if(!*difficult_config) {
//         v274 = ell_eq->test_point_tol;
//         v271 = SpaAcis::NullObj::get_parameter();
//         v272 = SpaAcis::NullObj::get_parameter();
//         v273 = SPAposition_array::operator[](&str_prospective_sols, 0);
//         str_str_sol = ((__int64(__fastcall*)(ellipse*, SPAposition*, __int64, SPAparameter*, SPAparameter*))v274)(ell_eq, v273, v18, v272, v271);
//         v278 = ell_eq->test_point_tol;
//         v275 = SpaAcis::NullObj::get_parameter();
//         v276 = SpaAcis::NullObj::get_parameter();
//         v277 = SPAposition_array::operator[](&str_prospective_sols, 1);
//         str_end_sol = ((__int64(__fastcall*)(ellipse*, SPAposition*, __int64, SPAparameter*, SPAparameter*))v278)(ell_eq, v277, v19, v276, v275);
//         if(str_str_sol) {
//             if(str_end_sol) *difficult_config = 1;
//         }
//     }
//     if(*difficult_config) {
//         v171 = ed_cur1;
//         if(ed_cur1) {
//             v279 = v171->~curve;
//             v280 = ((__int64(__fastcall*)(curve*, __int64))v279)(v171, 1i64);
//         } else {
//             v280 = 0i64;
//             sg_inter_ed_ed
//         }
//         v172 = ed_cur2;
//         if(ed_cur2) {
//             v281 = v172->~curve;
//             v282 = ((__int64(__fastcall*)(curve*, __int64))v281)(v172, 1i64);
//         } else {
//             v282 = 0i64;
//         }
//         v283 = ccrel_data;
//         SPAposition_array::~SPAposition_array(&ell_prospective_sols);
//         SPAdouble_array::~SPAdouble_array(&ell_prospective_params);
//         SPAposition_array::~SPAposition_array(&str_prospective_sols);
//         SPAdouble_array::~SPAdouble_array(&str_prospective_params);
//         exception_save::~exception_save(&exception_save_mark);
//         acis_exception::~acis_exception(&error_info_holder);
//         return v283;
//     }
//     // angle
//     // pl_root 椭圆中心
//     // pl_nor 椭圆法向量
//     // ln_root 直线根点
//     // ln_nor 直线方向
//
//     else {
//         angle = operator%(&st_eq->direction, &ell_eq->normal);
//         v2 = &ell_eq->normal;
//         v20 = operator-(&v493, &st_eq->root_point, &ell_eq->centre);
//         other_angle = operator%(v20, v2);
//         if(dist_tol <= fabs_0(angle) || dist_tol <= fabs_0(other_angle)) {
//             if(fabs_0(angle) > dist_tol) {
//                 if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Line and ellipse are not in the same plane.\n");
//                 SPAposition::SPAposition(&pl_root, &ell_eq->centre);
//                 SPAunit_vector::SPAunit_vector(&pl_nor, &ell_eq->normal);
//                 SPAposition::SPAposition(&ln_root, &st_eq->root_point);
//                 SPAunit_vector::SPAunit_vector(&ln_nor, &st_eq->direction);
//                 SPAposition::SPAposition(&inter);
//                 v76 = operator-(&v548, &ln_root, &pl_root);
//                 if(dist_tol <= SPAvector::len(v76)) {
//                     v77 = operator-(&v549, &pl_root, &ln_root);
//                     v78 = operator%(v77, &pl_nor);
//                     dist = v78 / angle;
//                     v79 = operator*(&v550, v78 / angle, &ln_nor);
//                     qmemcpy(&inter, operator+(&v551, &ln_root, v79), sizeof(inter));
//                 } else {
//                     qmemcpy(&inter, &ln_root, sizeof(inter));
//                 }
//                 v226 = 0.0;
//                 v176[0] = 0.0;
//                 v224 = EDGE::geometry(edge2);
//                 v225 = v224->equation(v224);
//                 v394 = v225->test_point_tol;
//                 v392 = SpaAcis::NullObj::get_parameter();
//                 v393 = SpaAcis::NullObj::get_parameter();
//                 v144 = ((unsigned int(__fastcall*)(const curve*, SPAposition*, __int64, SPAparameter*, SPAparameter*))v394)(v225, &inter, v80, v393, v392) && sg_pt_on_in_edge(&inter, edge1, &v226, dist_tol, 0i64) &&
//                        sg_pt_on_in_edge(&inter, edge2, v176, dist_tol, 0i64);
//                 v183 = v144;
//
//                 if(v144) {
//                     if(ell_eq->periodic(ell_eq)) {
//                         v395 = SPAdouble_array::operator[](&ell_prospective_params, 0);
//                         v81 = SPAdouble_array::operator[](&ell_prospective_params, 1);
//                         v396 = (*v395 + *v81) / 2.0;
//                         reduce_to_ell_range(&v396, v176);
//                     }
//                     v227 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 1865, &alloc_file_index_2127);
//                     if(v227) {
//                         par_pos = SpaAcis::NullObj::get_par_pos();
//                         v398 = v176[0];
//                         v399 = v226;
//                         curve_curve_int::curve_curve_int(v227, 0i64, &inter, v226, v176[0], par_pos);
//                         v228 = v82;
//                     } else {
//                         v228 = 0i64;
//                     }
//                     v400 = v228;
//                     ccrel_data = (curve_curve_int*)v228;
//                     *(_DWORD*)(v228 + 32) = 1;
//                     ccrel_data->high_rel = cur_cur_normal;
//                 }
//             }
//         }
//         // centre_offset centre-root_point
//         //  line_dir     direction
//         // circle_radius  major_axis
//         // ratio
//         // maj_dir  单位长轴
//         // min_dir  单位短轴
//
//         else {
//             if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Line and ellipse in the same plane.\n");
//             operator-(&centre_offset, &ell_eq->centre, &st_eq->root_point);
//             SPAunit_vector::SPAunit_vector(&line_dir, &st_eq->direction);
//             circle_radius = SPAvector::len(&ell_eq->major_axis);
//             ratio = ell_eq->radius_ratio;
//             v286 = fabs_0(ratio - 1.0);
//             v21 = safe_function_type<double>::operator double(&SPAresmch);
//             v149 = v286 > v21;
//             not_circle = v149;
//             SPAvector::SPAvector(&maj_dir);
//             SPAvector::SPAvector(&min_dir);
//             if(not_circle) {
//                 qmemcpy(&maj_dir, operator/(&v494, &ell_eq->major_axis, circle_radius), sizeof(maj_dir));
//                 qmemcpy(&min_dir, operator*(&v495, &ell_eq->normal, &maj_dir), sizeof(min_dir));
//                 v22 = operator%(&centre_offset, &min_dir);
//                 v287 = operator*(&v496, v22 / ratio, &min_dir);
//                 v23 = operator%(&centre_offset, &maj_dir);
//                 v1 = operator*(&v497, v23, &maj_dir);
//                 qmemcpy(&centre_offset, operator+(&v498, v1, v287), sizeof(centre_offset));
//                 v24 = operator%(&line_dir, &min_dir);
//                 v289 = operator*(&v499, v24 / ratio, &min_dir);
//                 v25 = operator%(&line_dir, &maj_dir);
//                 v290 = operator*(&v500, v25, &maj_dir);
//                 v26 = operator+(&v501, v290, v289);
//                 qmemcpy(&line_dir, normalise(&v502, v26), sizeof(line_dir));
//             }
//             v27 = operator%(&centre_offset, &line_dir);
//             operator*(&parallel_offset, v27, &line_dir);
//             operator-(&perp_offset, &centre_offset, &parallel_offset);
//             perp_dist = SPAvector::len(&perp_offset);
//             if(not_circle) {
//                 v28 = operator(&perp_offset % min_dir);
//                 v291 = operator*(&v503, v28 * ratio, &min_dir);
//                 v29 = operator%(&perp_offset, &maj_dir);
//                 v292 = operator*(&v504, v29, &maj_dir);
//                 qmemcpy(&perp_offset, operator+(&v505, v292, v291), sizeof(perp_offset));
//             }
//             perp_foot = ell_eq->centre - perp_offset;
//             if(sg_inter_module_header.debug_level >= 0x28) {
//                 acis_fprintf(debug_file_ptr, "Circle Radius : %0.15f\tPerp Dist : %0.15f\n", (double)circle_radius, (double)perp_dist);
//                 acis_fprintf(debug_file_ptr, "Dist Tolerance : %.15f\n", (double)dist_tol);
//                 acis_fprintf(debug_file_ptr, "perp_offset : ");
//                 SPAvector::debug(&perp_offset, debug_file_ptr);
//                 debug_newline(debug_file_ptr);
//             }
//             intersects = 0;
//             SPAposition::SPAposition(&inter1);
//             SPAposition::SPAposition(&inter2);
//             if(perp_dist <= circle_radius + dist_tol) {
//                 if(dist_tol <= fabs_0(circle_radius - perp_dist)) {
//                     intersects = 1;
//                     tang_root = 0;
//                     FUZZ_TOL = 3.0 * dist_tol;
//                     if(not_circle) {
//                         SPAparameter param1;
//                         if(ell_eq->test_point_tol(perp_foot, FUZZ_TOL, SpaAcis::NullObj::get_parameter(), param1)) {
//                             SPAposition v468 = ell_eq->eval_position(param1.operator double());
//                             SPAvector v58 = v468 - perp_foot;
//                             bool ellipse_tangent = dist_tol > v58.len();
//                             if(ellipse_tangent) {
//                                 SPAposition v481 = interpolate(0.5, perp_foot, v468);
//                                 perp_foot = v481;
//                                 SPAbox v559 = get_edge_box(edge1);
//                                 SPAbox v560 = get_edge_box(edge2);
//                                 SPAposition v340 = v559.low();
//                                 SPAposition v341 = v559.high();
//                                 SPAvector v59 = v341 - v340;
//                                 double v209 = v59.len();
//                                 SPAposition v342 = v560.low();
//                                 SPAposition v343 = v560.high();
//                                 SPAvector v60 = v343 - v342;
//                                 double v210 = v60.len();
//                                 // v210椭圆
//                                 if(v210 <= v209)
//                                     v211 = v210;
//                                 else
//                                     v211 = v209;
//                                 v460 = 20.0;
//                                 double v213 = v211 / 20.0;
//                                 double v212 = 25.0 * dist_tol;
//                                 if(v211 / 20.0 <= 25.0 * dist_tol)
//                                     v214 = v213;
//                                 else
//                                     v214 = v212;
//                                 SPAunit_vector v469(st_eq->direction);
//                                 const SPAvector v61 = v214 * v469;
//                                 SPAposition v482 = perp_foot - v61;
//                                 const SPAvector v62 = v214 * v469;
//                                 SPAposition v483 = perp_foot + v62;
//                                 v158 = !edge2->closed() && (ell_str_sol || ell_end_sol) || str_str_sol || str_end_sol;
//                                 pt_at_str_end[1] = v158;
//                                 if(v158 &&
//                                    (v347 = ell_eq->test_point_tol, v345 = SpaAcis::NullObj::get_parameter(), v346 = SpaAcis::NullObj::get_parameter(),
//                                     ((unsigned int(__fastcall*)(ellipse*, SPAposition*, __int64, SPAparameter*, SPAparameter*))v347)(ell_eq, &v482, v63, v346, v345)) &&
//                                    (v350 = ell_eq->test_point_tol, v348 = SpaAcis::NullObj::get_parameter(), v349 = SpaAcis::NullObj::get_parameter(),
//                                     ((unsigned int(__fastcall*)(ellipse*, SPAposition*, __int64, SPAparameter*, SPAparameter*))v350)(ell_eq, &v483, v64, v349, v348))) {
//                                     *difficult_config = 1;
//                                 } else {
//                                     eval_deriv = ell_eq->eval_deriv;
//                                     v351 = SPAparameter::operator double(&v173);
//                                     v66 = (const SPAvector*)((__int64(__fastcall*)(ellipse*, char*, __int64, _QWORD, _DWORD))eval_deriv)(ell_eq, v540, v65, 0i64, 0);
//                                     if(biparallel(&st_eq->direction, v66, angle_tol)) {
//                                         tang_root = 1;
//                                     } else {
//                                         if(sg_inter_module_header.debug_level >= 0x1E)
//                                             acis_fprintf(debug_file_ptr,
//                                                          "Tangency point is a solution, but its not a tangent classification, discarding it and relying on"
//                                                          " general intersectors.\n");
//                                         *difficult_config = 1;
//                                     }
//                                 }
//                             } else {
//                                 *difficult_config = 1;
//                             }
//                         }
//                     } else if(FUZZ_TOL > circle_radius - perp_dist) {
//                         *difficult_config = 1;
//                     }
//                     if(*difficult_config) {
//                         intersects = 0;
//                     } else if(tang_root) {
//                         qmemcpy(&inter2, &perp_foot, sizeof(inter2));
//                         qmemcpy(&inter1, &inter2, sizeof(inter1));
//                     } else {
//                         half_chord = acis_sqrt(circle_radius * circle_radius - perp_dist * perp_dist);
//                         operator*(&chord_offset, half_chord, &line_dir);
//                         if(not_circle) {
//                             v67 = operator%(&chord_offset, &min_dir);
//                             v354 = operator*(&v541, v67 * ratio, &min_dir);
//                             v68 = operator%(&chord_offset, &maj_dir);
//                             v355 = operator*(&v542, v68, &maj_dir);
//                             qmemcpy(&chord_offset, operator+(&v543, v355, v354), sizeof(chord_offset));
//                         }
//                         qmemcpy(&inter1, operator-(&v544, &perp_foot, &chord_offset), sizeof(inter1));
//                         qmemcpy(&inter2, operator+(&v545, &perp_foot, &chord_offset), sizeof(inter2));
//                     }
//                 } else {
//                     v153 = perp_dist > circle_radius || (v315 = circle_radius - perp_dist, v42 = safe_function_type<double>::operator double(&SPAresmch), v42 > v315);
//                     line_touches_or_above_ellipse = v153;
//                     perp_dist = (circle_radius + perp_dist) / 2.0;
//                     qmemcpy(&perp_offset, operator-(&v515, &centre_offset, &parallel_offset), sizeof(perp_offset));
//                     v43 = normalise(&v516, &perp_offset);
//                     qmemcpy(&perp_offset, operator*(&v517, perp_dist, v43), sizeof(perp_offset));
//                     if(not_circle) {
//                         v44 = operator%(&perp_offset, &min_dir);
//                         v316 = operator*(&v518, v44 * ratio, &min_dir);
//                         v45 = operator%(&perp_offset, &maj_dir);
//                         v317 = operator*(&v519, v45, &maj_dir);
//                         qmemcpy(&perp_offset, operator+(&v520, v317, v316), sizeof(perp_offset));
//                     }
//                     qmemcpy(&perp_foot, operator-(&v521, &ell_eq->centre, &perp_offset), sizeof(perp_foot));
//                     get_edge_box(&v557, edge1, 0i64, 0, 0i64);
//                     get_edge_box(&v558, edge2, 0i64, 0, 0i64);
//                     v318 = SPAbox::low(&v557, &v522);
//                     v319 = SPAbox::high(&v557, &v523);
//                     v46 = operator-(&v524, v319, v318);
//                     // 直线对角线向量
//                     v199 = SPAvector::len(v46);
//                     v320 = SPAbox::low(&v558, &v525);
//                     v321 = SPAbox::high(&v558, &v526);
//                     v47 = operator-(&v527, v321, v320);
//                     v200 = SPAvector::len(v47);
//                     // 椭圆包围盒对角线向量长度
//                     if(v200 <= v199)
//                         v201 = v200;
//                     else
//                         v201 = v199;
//                     v322 = v201;
//                     v459 = DOUBLE_20_0;
//                     v203 = v201 / 20.0;
//                     v202 = 25.0 * dist_tol;
//                     if(v201 / 20.0 <= 25.0 * dist_tol)
//                         v204 = v203;
//                     else
//                         v204 = v202;
//                     d = v204;
//                     SPAunit_vector::SPAunit_vector(&v467, &st_eq->direction);
//                     v48 = operator*(&v528, d, &v467);
//                     operator-(&v479, &perp_foot, v48);
//                     v49 = operator*(&v529, d, &v467);
//                     operator+(&v480, &perp_foot, v49);
//                     v154 = !EDGE::closed(edge2) && (ell_str_sol || ell_end_sol) || str_str_sol || str_end_sol;
//                     v121 = v154;
//                     if(v154 &&
//                        (v325 = ell_eq->test_point_tol, v323 = SpaAcis::NullObj::get_parameter(), v324 = SpaAcis::NullObj::get_parameter(),
//                         ((unsigned int(__fastcall*)(ellipse*, SPAposition*, __int64, SPAparameter*, SPAparameter*))v325)(ell_eq, &v479, v50, v324, v323)) &&
//                        (v328 = ell_eq->test_point_tol, v326 = SpaAcis::NullObj::get_parameter(), v327 = SpaAcis::NullObj::get_parameter(),
//                         ((unsigned int(__fastcall*)(ellipse*, SPAposition*, __int64, SPAparameter*, SPAparameter*))v328)(ell_eq, &v480, v51, v327, v326))) {
//                         *difficult_config = 1;
//                     } else {
//                         v206 = 0.0;
//                         param = 0.0;
//                         v106 = 0;
//                         v107 = 0;
//                         v155 = sg_pt_on_in_edge(&perp_foot, edge1, &v206, dist_tol, &v106) && sg_pt_on_in_edge(&perp_foot, edge2, &param, dist_tol, &v107);
//                         v156 = v155;
//                         if(v155) {
//                             if(v106 || !EDGE::closed(edge2) && v107 ||
//                                not_circle && (v329 = ell_eq->eval_deriv, v53 = (const SPAvector*)((__int64(__fastcall*)(ellipse*, char*, __int64, _QWORD, _DWORD))v329)(ell_eq, v530, v52, 0i64, 0), !biparallel(&st_eq->direction, v53, angle_tol))) {
//                                 if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Tangency point is a solution, but relationships are tricky, discarding it and relying on general intersectors.\n");
//                                 v156 = 0;
//                                 *difficult_config = 1;
//                             } else {
//                                 if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Keeping Intersection.\n");
//                                 tangent_root = 1;
//                                 if(ell_eq->periodic(ell_eq)) {
//                                     v330 = SPAdouble_array::operator[](&ell_prospective_params, 0);
//                                     v54 = SPAdouble_array::operator[](&ell_prospective_params, 1);
//                                     range_mid_pt = (*v330 + *v54) / 2.0;
//                                     reduce_to_ell_range(&range_mid_pt, &param);
//                                 }
//                                 v207 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 1519, &alloc_file_index_2127);
//                                 if(v207) {
//                                     v332 = SpaAcis::NullObj::get_par_pos();
//                                     v333 = param;
//                                     v334 = v206;
//                                     curve_curve_int::curve_curve_int(v207, 0i64, &perp_foot, v206, param, v332);
//                                     v208 = v55;
//                                 } else {
//                                     v208 = 0i64;
//                                 }
//                                 v335 = v208;
//                                 ccrel_data = (curve_curve_int*)v208;
//                                 *(_DWORD*)(v208 + 32) = 2;
//                                 ccrel_data->high_rel = cur_cur_tangent;
//                             }
//                         } else if(!line_touches_or_above_ellipse) {
//                             *difficult_config = 1;
//                         }
//                     }
//                 }
//             } else if(not_circle) {
//                 SPAparameter::SPAparameter(&ell_param);
//                 v294 = ell_eq->test_point_tol;
//                 v293 = SpaAcis::NullObj::get_parameter();
//                 if(((unsigned int(__fastcall*)(ellipse*, SPAposition*, __int64, SPAparameter*, SPAparameter*))v294)(ell_eq, &perp_foot, v30, v293, &ell_param)) {
//                     v296 = ell_eq->eval_position;
//                     v295 = SPAparameter::operator double(&ell_param);
//                     ((void(__fastcall*)(ellipse*, SPAposition*, __int64, _QWORD, _DWORD))v296)(ell_eq, &ell_pos, v31, 0i64, 0);
//                     interpolate(&new_perp_foot, 0.5, &perp_foot, &ell_pos);
//                     qmemcpy(&perp_foot, &new_perp_foot, sizeof(perp_foot));
//                     get_edge_box(&bx1, edge1, 0i64, 0, 0i64);
//                     get_edge_box(&bx2, edge2, 0i64, 0, 0i64);
//                     v297 = SPAbox::low(&bx1, &v506);
//                     v298 = SPAbox::high(&bx1, &v507);
//                     v32 = operator-(&v508, v298, v297);
//                     bx1_len = SPAvector::len(v32);
//                     v299 = SPAbox::low(&bx2, &v509);
//                     v300 = SPAbox::high(&bx2, &v510);
//                     v33 = operator-(&v511, v300, v299);
//                     bx2_len = SPAvector::len(v33);
//                     if(bx2_len <= bx1_len)
//                         v191 = bx2_len;
//                     else
//                         v191 = bx1_len;
//                     min_box_len = v191;
//                     SMALL_FEATURE_FACTOR = DOUBLE_20_0;
//                     SMALL_FEATURE_SIZE = v191 / 20.0;
//                     VALID_COINCIDENC_CHECK_TOL = 25.0 * dist_tol;
//                     if(v191 / 20.0 <= 25.0 * dist_tol)
//                         v194 = SMALL_FEATURE_SIZE;
//                     else
//                         v194 = VALID_COINCIDENC_CHECK_TOL;
//                     offset = v194;
//                     SPAunit_vector::SPAunit_vector(&str_dir, &st_eq->direction);
//                     v34 = operator*(&v512, offset, &str_dir);
//                     operator-(&str_pos1, &perp_foot, v34);
//                     v35 = operator*(&v513, offset, &str_dir);
//                     operator+(&str_pos2, &perp_foot, v35);
//                     v150 = !EDGE::closed(edge2) && (ell_str_sol || ell_end_sol) || str_str_sol || str_end_sol;
//                     end_pt_sol = v150;
//                     if(v150 &&
//                        (v304 = ell_eq->test_point_tol, v302 = SpaAcis::NullObj::get_parameter(), v303 = SpaAcis::NullObj::get_parameter(),
//                         ((unsigned int(__fastcall*)(ellipse*, SPAposition*, __int64, SPAparameter*, SPAparameter*))v304)(ell_eq, &str_pos1, v36, v303, v302)) &&
//                        (v307 = ell_eq->test_point_tol, v305 = SpaAcis::NullObj::get_parameter(), v306 = SpaAcis::NullObj::get_parameter(),
//                         ((unsigned int(__fastcall*)(ellipse*, SPAposition*, __int64, SPAparameter*, SPAparameter*))v307)(ell_eq, &str_pos2, v37, v306, v305))) {
//                         *difficult_config = 1;
//                     } else {
//                         line_param = 0.0;
//                         actual_par = 0.0;
//                         pt_at_str_end[0] = 0;
//                         pt_at_ell_end = 0;
//                         v151 = sg_pt_on_in_edge(&perp_foot, edge1, &line_param, dist_tol, pt_at_str_end) && sg_pt_on_in_edge(&perp_foot, edge2, &actual_par, dist_tol, &pt_at_ell_end);
//                         inters = v151;
//                         if(v151) {
//                             if(!pt_at_str_end[0] && (EDGE::closed(edge2) || !pt_at_ell_end) &&
//                                (v308 = ell_eq->eval_deriv, v39 = (const SPAvector*)((__int64(__fastcall*)(ellipse*, char*, __int64, _QWORD, _DWORD))v308)(ell_eq, v514, v38, 0i64, 0), biparallel(&st_eq->direction, v39, angle_tol))) {
//                                 if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Keeping Intersection.\n");
//                                 tangent_root = 1;
//                                 if(ell_eq->periodic(ell_eq)) {
//                                     v309 = SPAdouble_array::operator[](&ell_prospective_params, 0);
//                                     v40 = SPAdouble_array::operator[](&ell_prospective_params, 1);
//                                     mid_pt = (*v309 + *v40) / 2.0;
//                                     reduce_to_ell_range(&mid_pt, &actual_par);
//                                 }
//                                 v197 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 1421, &alloc_file_index_2127);
//                                 if(v197) {
//                                     _uv = SpaAcis::NullObj::get_par_pos();
//                                     d2 = actual_par;
//                                     d1 = line_param;
//                                     curve_curve_int::curve_curve_int(v197, 0i64, &perp_foot, line_param, actual_par, _uv);
//                                     v198 = v41;
//                                 } else {
//                                     v198 = 0i64;
//                                 }
//                                 v314 = v198;
//                                 ccrel_data = (curve_curve_int*)v198;
//                                 *(_DWORD*)(v198 + 32) = 2;
//                                 ccrel_data->high_rel = cur_cur_tangent;
//                             } else {
//                                 if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Tangency point is a solution, but relationships are tricky, discarding it and relying on general intersectors.\n");
//                                 inters = 0;
//                                 *difficult_config = 1;
//                             }
//                         }
//                     }
//                 }
//             }
//             if(intersects) {
//                 if(sg_inter_module_header.debug_level >= 0x1E) {
//                     acis_fprintf(debug_file_ptr, "Intersections found are : ");
//                     SPAposition::debug(&inter1, debug_file_ptr);
//                     acis_fprintf(debug_file_ptr, "\t");
//                     SPAposition::debug(&inter2, debug_file_ptr);
//                     debug_newline(debug_file_ptr);
//                 }
//                 v69 = operator-(&v546, &inter1, &inter2);
//                 if(dist_tol <= SPAvector::len(v69)) {
//                     if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Regular Intersection.\n");
//                     v164 = 0.0;
//                     v136 = 0.0;
//                     if(sg_inter_module_header.debug_level >= 0x1E) {
//                         acis_fprintf(debug_file_ptr, "First Intersection point : ");
//                         SPAposition::debug(&inter1, debug_file_ptr);
//                         debug_newline(debug_file_ptr);
//                     }
//                     v140 = sg_pt_on_in_edge(&inter1, edge1, &v164, dist_tol, 0i64) && sg_pt_on_in_edge(&inter1, edge2, &v136, dist_tol, 0i64);
//                     inters1 = v140;
//                     if(v140) {
//                         if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Keeping the First Intersection.\n");
//                         if(ell_eq->periodic(ell_eq)) {
//                             v362 = SPAdouble_array::operator[](&ell_prospective_params, 0);
//                             v72 = SPAdouble_array::operator[](&ell_prospective_params, 1);
//                             v363 = (*v362 + *v72) / 2.0;
//                             reduce_to_ell_range(&v363, &v136);
//                         }
//                         v220 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 1756, &alloc_file_index_2127);
//                         if(v220) {
//                             v364 = SpaAcis::NullObj::get_par_pos();
//                             v365 = v136;
//                             v366 = v164;
//                             curve_curve_int::curve_curve_int(v220, 0i64, &inter1, v164, v136, v364);
//                             v221 = v73;
//                         } else {
//                             v221 = 0i64;
//                         }
//                         v367 = v221;
//                         ccrel_data = (curve_curve_int*)v221;
//                         *(_DWORD*)(v221 + 32) = 1;
//                         ccrel_data->high_rel = cur_cur_normal;
//                     }
//                     if(sg_inter_module_header.debug_level >= 0x1E) {
//                         acis_fprintf(debug_file_ptr, "Second Intersection point : ");
//                         SPAposition::debug(&inter2, debug_file_ptr);
//                         debug_newline(debug_file_ptr);
//                     }
//                     v141 = sg_pt_on_in_edge(&inter2, edge1, &v164, dist_tol, 0i64) && sg_pt_on_in_edge(&inter2, edge2, &v136, dist_tol, 0i64);
//                     inters2 = v141;
//                     if(v141) {
//                         if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Keeping the Second Intersection.\n");
//                         if(ell_eq->periodic(ell_eq)) {
//                             v368 = SPAdouble_array::operator[](&ell_prospective_params, 0);
//                             v74 = SPAdouble_array::operator[](&ell_prospective_params, 1);
//                             v369 = (*v368 + *v74) / 2.0;
//                             reduce_to_ell_range(&v369, &v136);
//                         }
//                         v222 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 1783, &alloc_file_index_2127);
//                         if(v222) {
//                             v370 = SpaAcis::NullObj::get_par_pos();
//                             v371 = v136;
//                             v372 = v164;
//                             n = ccrel_data;
//                             curve_curve_int::curve_curve_int(v222, ccrel_data, &inter2, v164, v136, v370);
//                             v223 = v75;
//                         } else {
//                             v223 = 0i64;
//                         }
//                         v374 = v223;
//                         ccrel_data = (curve_curve_int*)v223;
//                         *(_DWORD*)(v223 + 32) = 1;
//                         ccrel_data->high_rel = cur_cur_normal;
//                     }
//                     if(!inters1 || inters2) {
//                         if(!inters1 && inters2) {
//                             v383 = safe_function_type<double>::operator double(&SPAresabs);
//                             v384 = SPAposition_array::operator[](&str_prospective_sols, 0);
//                             v143 = same_point(&inter2, v384, v383) || (v385 = safe_function_type<double>::operator double(&SPAresabs), v386 = SPAposition_array::operator[](&str_prospective_sols, 1), same_point(&inter2, v386, v385)) ||
//                                    (v387 = safe_function_type<double>::operator double(&SPAresabs), v388 = SPAposition_array::operator[](&ell_prospective_sols, 0), same_point(&inter2, v388, v387)) ||
//                                    (v389 = safe_function_type<double>::operator double(&SPAresabs), v390 = SPAposition_array::operator[](&ell_prospective_sols, 1), same_point(&inter2, v390, v389));
//                             inters2_matches_end = v143;
//                             if((str_str_sol || str_end_sol || ell_str_sol || ell_end_sol) && !inters2_matches_end) *difficult_config = 1;
//                         }
//                     } else {
//                         res = safe_function_type<double>::operator double(&SPAresabs);
//                         v376 = SPAposition_array::operator[](&str_prospective_sols, 0);
//                         v142 = same_point(&inter1, v376, res) || (v377 = safe_function_type<double>::operator double(&SPAresabs), v378 = SPAposition_array::operator[](&str_prospective_sols, 1), same_point(&inter1, v378, v377)) ||
//                                (v379 = safe_function_type<double>::operator double(&SPAresabs), v380 = SPAposition_array::operator[](&ell_prospective_sols, 0), same_point(&inter1, v380, v379)) ||
//                                (v381 = safe_function_type<double>::operator double(&SPAresabs), v382 = SPAposition_array::operator[](&ell_prospective_sols, 1), same_point(&inter1, v382, v381));
//                         inters1_matches_end = v142;
//                         if((str_str_sol || str_end_sol || ell_str_sol || ell_end_sol) && !inters1_matches_end) *difficult_config = 1;
//                     }
//                     if(!*difficult_config && ccrel_data && ccrel_data->next) sg_sort_cci(&ccrel_data, 0, 0);
//                 } else {
//                     if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Tangent Intersection.\n");
//                     v217 = 0.0;
//                     v174 = 0.0;
//                     qmemcpy(&inter1, interpolate(&v547, 0.5, &inter1, &inter2), sizeof(inter1));
//                     v108 = 0;
//                     v109 = 0;
//                     v161 = sg_pt_on_in_edge(&inter1, edge1, &v217, dist_tol, &v108) && sg_pt_on_in_edge(&inter1, edge2, &v174, dist_tol, &v109);
//                     v162 = v161;
//                     if(v161) {
//                         if(v108 || !EDGE::closed(edge2) && v109) {
//                             if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Tangency point is a solution, but relationships are tricky, discarding it and relying on general intersectors.\n");
//                             v162 = 0;
//                             *difficult_config = 1;
//                         } else {
//                             if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Keeping Intersection.\n");
//                             tangent_root = 1;
//                             if(ell_eq->periodic(ell_eq)) {
//                                 v356 = SPAdouble_array::operator[](&ell_prospective_params, 0);
//                                 v70 = SPAdouble_array::operator[](&ell_prospective_params, 1);
//                                 v357 = (*v356 + *v70) / 2.0;
//                                 reduce_to_ell_range(&v357, &v174);
//                             }
//                             v218 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 1711, &alloc_file_index_2127);
//                             if(v218) {
//                                 v358 = SpaAcis::NullObj::get_par_pos();
//                                 v359 = v174;
//                                 v360 = v217;
//                                 curve_curve_int::curve_curve_int(v218, 0i64, &inter1, v217, v174, v358);
//                                 v219 = v71;
//                             } else {
//                                 v219 = 0i64;
//                             }
//                             v361 = v219;
//                             ccrel_data = (curve_curve_int*)v219;
//                             *(_DWORD*)(v219 + 32) = 2;
//                             ccrel_data->high_rel = cur_cur_tangent;
//                         }
//                     } else {
//                         *difficult_config = 1;
//                     }
//                 }
//             }
//         }
//         intr_count = curve_curve_int_size(ccrel_data);
//         if(tangent_root || *difficult_config) {
//             v146 = 0;
//         } else {
//             v145 = intr_count < 2;
//             v146 = v145;
//         }
//         check_edge_ends_for_solution = v146;
//         if(v146) {
//             for(i = 0;; ++i) {
//                 v83 = SPAposition_array::Size(&str_prospective_sols);
//                 if(i >= v83 || intr_count >= 2) break;
//                 v84 = SPAposition_array::operator[](&str_prospective_sols, i);
//                 SPAposition::SPAposition(&pos1, v84);
//                 if(is_position_not_used(&pos1, ccrel_data, dist_tol)) {
//                     for(j = 0;; ++j) {
//                         v85 = SPAposition_array::Size(&ell_prospective_sols);
//                         if(j >= v85 || intr_count >= 2) break;
//                         v86 = SPAposition_array::operator[](&ell_prospective_sols, j);
//                         SPAposition::SPAposition(&pos2, v86);
//                         if(is_position_not_used(&pos2, ccrel_data, dist_tol) && same_point(&pos1, &pos2, dist_tol)) {
//                             ++intr_count;
//                             interpolate(&p, 0.5, &pos1, &pos2);
//                             v229 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 1901, &alloc_file_index_2127);
//                             if(v229) {
//                                 v401 = SpaAcis::NullObj::get_par_pos();
//                                 v402 = *SPAdouble_array::operator[](&ell_prospective_params, j);
//                                 v403 = *SPAdouble_array::operator[](&str_prospective_params, i);
//                                 v404 = ccrel_data;
//                                 curve_curve_int::curve_curve_int(v229, ccrel_data, &p, v403, v402, v401);
//                                 v230 = v87;
//                             } else {
//                                 v230 = 0i64;
//                             }
//                             v405 = v230;
//                             ccrel_data = (curve_curve_int*)v230;
//                             *(_DWORD*)(v230 + 32) = 1;
//                             ccrel_data->high_rel = cur_cur_normal;
//                         }
//                     }
//                 }
//             }
//             for(i = 0;; ++i) {
//                 v88 = SPAposition_array::Size(&str_prospective_sols);
//                 if(i >= v88 || intr_count >= 2) break;
//                 v89 = SPAposition_array::operator[](&str_prospective_sols, i);
//                 SPAposition::SPAposition(&v455, v89);
//                 if(is_position_not_used(&v455, ccrel_data, dist_tol)) {
//                     SPAparameter::SPAparameter(&v137);
//                     v407 = ell_eq->test_point_tol;
//                     v406 = SpaAcis::NullObj::get_parameter();
//                     if(((unsigned int(__fastcall*)(ellipse*, SPAposition*, __int64, SPAparameter*, SPAparameter*))v407)(ell_eq, &v455, v90, v406, &v137)) {
//                         if(ell_eq->periodic(ell_eq)) {
//                             v408 = SPAdouble_array::operator[](&ell_prospective_params, 0);
//                             v91 = SPAdouble_array::operator[](&ell_prospective_params, 1);
//                             v409 = (*v408 + *v91) / 2.0;
//                             t = SPAparameter::operator double(&v137);
//                             reduce_to_ell_range(&v409, &t);
//                             SPAparameter::SPAparameter(&v473, t);
//                             *(SPAparameter*)&v137.val = (SPAparameter)*v92;
//                         }
//                         tol = safe_function_type<double>::operator double(&SPAresnor);
//                         v411 = SPAparameter::operator double(&v137);
//                         d_high = *SPAdouble_array::operator[](&ell_prospective_params, 1);
//                         d_low = *SPAdouble_array::operator[](&ell_prospective_params, 0);
//                         if(sg_within(d_low, d_high, v411, tol)) {
//                             v415 = ell_eq->eval_position;
//                             v414 = SPAparameter::operator double(&v137);
//                             ((void(__fastcall*)(ellipse*, SPAposition*, __int64, _QWORD, _DWORD))v415)(ell_eq, &v487, v93, 0i64, 0);
//                             interpolate(&v471, 0.5, &v455, &v487);
//                             if(is_position_not_used(&v471, ccrel_data, dist_tol)) {
//                                 ++intr_count;
//                                 v232 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 1936, &alloc_file_index_2127);
//                                 if(v232) {
//                                     v416 = SpaAcis::NullObj::get_par_pos();
//                                     v417 = SPAparameter::operator double(&v137);
//                                     v418 = *SPAdouble_array::operator[](&str_prospective_params, i);
//                                     v419 = ccrel_data;
//                                     curve_curve_int::curve_curve_int(v232, ccrel_data, &v471, v418, v417, v416);
//                                     v233 = v94;
//                                 } else {
//                                     v233 = 0i64;
//                                 }
//                                 v420 = v233;
//                                 ccrel_data = (curve_curve_int*)v233;
//                                 *(_DWORD*)(v233 + 32) = 1;
//                                 ccrel_data->high_rel = cur_cur_normal;
//                             }
//                         }
//                     }
//                 }
//             }
//             for(i = 0;; ++i) {
//                 v95 = SPAposition_array::Size(&ell_prospective_sols);
//                 if(i >= v95 || intr_count >= 2) break;
//                 v96 = SPAposition_array::operator[](&ell_prospective_sols, i);
//                 SPAposition::SPAposition(&v456, v96);
//                 if(is_position_not_used(&v456, ccrel_data, dist_tol)) {
//                     SPAparameter::SPAparameter(&str_param);
//                     v422 = st_eq->test_point_tol;
//                     v421 = SpaAcis::NullObj::get_parameter();
//                     if(((unsigned int(__fastcall*)(straight*, SPAposition*, __int64, SPAparameter*, SPAparameter*))v422)(st_eq, &v456, v97, v421, &str_param)) {
//                         v423 = safe_function_type<double>::operator double(&SPAresnor);
//                         v424 = SPAparameter::operator double(&str_param);
//                         v425 = *SPAdouble_array::operator[](&str_prospective_params, 1);
//                         v426 = *SPAdouble_array::operator[](&str_prospective_params, 0);
//                         if(sg_within(v426, v425, v424, v423)) {
//                             v428 = st_eq->eval_position;
//                             v427 = SPAparameter::operator double(&str_param);
//                             ((void(__fastcall*)(straight*, SPAposition*, __int64, _QWORD, _DWORD))v428)(st_eq, &v488, v98, 0i64, 0);
//                             interpolate(&v472, 0.5, &v456, &v488);
//                             if(is_position_not_used(&v472, ccrel_data, dist_tol)) {
//                                 ++intr_count;
//                                 v235 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 1963, &alloc_file_index_2127);
//                                 if(v235) {
//                                     v429 = SpaAcis::NullObj::get_par_pos();
//                                     v430 = *SPAdouble_array::operator[](&ell_prospective_params, i);
//                                     v431 = SPAparameter::operator double(&str_param);
//                                     v432 = ccrel_data;
//                                     curve_curve_int::curve_curve_int(v235, ccrel_data, &v472, v431, v430, v429);
//                                     v234 = v99;
//                                 } else {
//                                     v234 = 0i64;
//                                 }
//                                 v433 = v234;
//                                 ccrel_data = (curve_curve_int*)v234;
//                                 *(_DWORD*)(v234 + 32) = 1;
//                                 ccrel_data->high_rel = cur_cur_normal;
//                             }
//                         }
//                     }
//                 }
//             }
//             if(ccrel_data && ccrel_data->next) sg_sort_cci(&ccrel_data, 0, 0);
//         }
//         SPAposition_array::~SPAposition_array(&ell_prospective_sols);
//         SPAdouble_array::~SPAdouble_array(&ell_prospective_params);
//         SPAposition_array::~SPAposition_array(&str_prospective_sols);
//         SPAdouble_array::~SPAdouble_array(&str_prospective_params);
//         if(ed_cur1) {
//             *(_QWORD*)&v176[1] = ed_cur1;
//             v434 = ed_cur1->~curve;
//             v435 = ((__int64(__fastcall*)(curve*, __int64))v434)(ed_cur1, 1i64);
//         }
//         if(ed_cur2) {
//             *(_QWORD*)&v176[2] = ed_cur2;
//             v436 = ed_cur2->~curve;
//             v437 = ((__int64(__fastcall*)(curve*, __int64))v436)(ed_cur2, 1i64);
//         }
//         if(error_no) sg_delete_cci_list(ccrel_data);
//         exception_save::~exception_save(&exception_save_mark);
//         if(resignal_no || acis_interrupted()) sys_error(resignal_no, error_info_base_ptr);
//         acis_exception::~acis_exception(&error_info_holder);
//         return ccrel_data;
//     }
// }
//
// curve_curve_int* eded_int(EDGE* edge1, EDGE* edge2, long double dist_tol_in, long double angle_tol_in) {
//     VERTEX* v4;              // rax
//     APOINT* v5;              // rax
//     const SPAposition* v6;   // rax
//     VERTEX* v7;              // rax
//     APOINT* v8;              // rax
//     const SPAposition* v9;   // rax
//     VERTEX* v10;             // rax
//     APOINT* v11;             // rax
//     const SPAposition* v12;  // rax
//     VERTEX* v13;             // rax
//     APOINT* v14;             // rax
//     const SPAposition* v15;  // rax
//     SPAunit_vector* v16;     // rax
//     SPAunit_vector* v17;     // rax
//     AcisVersion* v18;        // rax
//     const SPAvector* v19;    // rax
//     long double v20;         // xmm0_8
//     const SPAvector* v21;    // rax
//     SPAvector* v22;          // rax
//     __int64 v23;             // rax
//     curve_curve_int* v24;    // rax
//     SPAvector* v25;          // rax
//     SPAvector* v26;          // rax
//     long double v27;         // xmm0_8
//     SPAvector* v28;          // rax
//     SPAvector* v29;          // rax
//     SPAvector* v30;          // rax
//     curve_curve_int* v31;    // rax
//     double v32;              // xmm0_8
//     __int64 v33;             // r8
//     const SPAposition* v34;  // rax
//     __int64 v35;             // r8
//     const SPAposition* v36;  // rax
//     __int64 v37;             // r8
//     const SPAposition* v38;  // rax
//     __int64 v39;             // r8
//     const SPAposition* v40;  // rax
//     int v41;                 // eax
//     const SPAposition* v42;  // rax
//     SPAvector* v43;          // rax
//     SPAvector* v44;          // rax
//     __int64 v45;             // r8
//     __int64 v46;             // r8
//     AcisVersion* v47;        // rax
//     __int64 v48;             // r8
//     AcisVersion* v49;        // rax
//     __int64 v50;             // r8
//     COEDGE* v51;             // rax
//     COEDGE* v52;             // rax
//     COEDGE* v53;             // rax
//     LOOP* v54;               // rax
//     COEDGE* v55;             // rax
//     LOOP* v56;               // rax
//     COEDGE* v57;             // rax
//     COEDGE* v58;             // rax
//     COEDGE* v59;             // rax
//     COEDGE* v60;             // rax
//     COEDGE* v61;             // rax
//     LOOP* v62;               // rax
//     COEDGE* v63;             // rax
//     LOOP* v64;               // rax
//     COEDGE* v65;             // rax
//     COEDGE* v66;             // rax
//     COEDGE* v67;             // rax
//     COEDGE* v68;             // rax
//     COEDGE* v69;             // rax
//     COEDGE* v70;             // rax
//     COEDGE* v71;             // rax
//     LOOP* v72;               // rax
//     COEDGE* v73;             // rax
//     COEDGE* v74;             // rax
//     LOOP* v75;               // rax
//     COEDGE* v76;             // rax
//     COEDGE* v77;             // rax
//     COEDGE* v78;             // rax
//     COEDGE* v79;             // rax
//     COEDGE* v80;             // rax
//     COEDGE* v81;             // rax
//     COEDGE* v82;             // rax
//     COEDGE* v83;             // rax
//     COEDGE* v84;             // rax
//     LOOP* v85;               // rax
//     COEDGE* v86;             // rax
//     COEDGE* v87;             // rax
//     LOOP* v88;               // rax
//     COEDGE* v89;             // rax
//     COEDGE* v90;             // rax
//     int v91;                 // eax
//     int v92;                 // eax
//     const surface* v93;      // rax
//     AcisVersion* v94;        // rax
//     PCURVE* v95;             // rax
//     AcisVersion* v96;        // rax
//     PCURVE* v97;             // rax
//     ccrel_data;              // [rsp+50h] [rbp-D98h] BYREF
//
//     bool check_vertices;                                                                                            // [rsp+60h] [rbp-D88h]
//     bool difficult_config;                                                                                          // [rsp+61h] [rbp-D87h] BYREF
//     bool v103;                                                                                                      // [rsp+62h] [rbp-D86h] BYREF
//     curve* ed_cur2;                                                                                                 // [rsp+68h] [rbp-D80h]
//     *cur1;                                                                                                          // [rsp+70h] [rbp-D78h]
//     CURVE* cur2;                                                                                                    // [rsp+78h] [rbp-D70h]
//     curve* ed_cur1;                                                                                                 // [rsp+80h] [rbp-D68h]
//     int error_no;                                                                                                   // [rsp+88h] [rbp-D60h]
//     int resignal_no;                                                                                                // [rsp+8Ch] [rbp-D5Ch]
//     COEDGE* coed1;                                                                                                  // [rsp+90h] [rbp-D58h]
//     COEDGE* coed2;                                                                                                  // [rsp+98h] [rbp-D50h]
//     SPAposition* end_pt;                                                                                            // [rsp+A0h] [rbp-D48h] BYREF
//     const straight* eq2;                                                                                            // [rsp+A8h] [rbp-D40h]
//     int i;                                                                                                          // [rsp+B0h] [rbp-D38h]
//     int j;                                                                                                          // [rsp+B4h] [rbp-D34h]
//     SPAposition* start_pt;                                                                                          // [rsp+B8h] [rbp-D30h] BYREF
//     const straight* eq1;                                                                                            // [rsp+C0h] [rbp-D28h]
//     long double angle_tol;                                                                                          // [rsp+C8h] [rbp-D20h]
//     FACE* common_face;                                                                                              // [rsp+D0h] [rbp-D18h]
//     long double smaller_len;                                                                                        // [rsp+D8h] [rbp-D10h]
//     int v121;                                                                                                       // [rsp+E0h] [rbp-D08h]
//     int R22_onwards;                                                                                                // [rsp+E4h] [rbp-D04h]
//     int v123;                                                                                                       // [rsp+E8h] [rbp-D00h]
//     int v124;                                                                                                       // [rsp+ECh] [rbp-CFCh]
//     int negate_pcu2;                                                                                                // [rsp+F0h] [rbp-CF8h]
//     int v126;                                                                                                       // [rsp+F4h] [rbp-CF4h]
//     SURFACE* common_SF;                                                                                             // [rsp+F8h] [rbp-CF0h]
//     COEDGE* coed;                                                                                                   // [rsp+100h] [rbp-CE8h]
//     int negate_pcu1;                                                                                                // [rsp+108h] [rbp-CE0h]
//     const surface* common_sf;                                                                                       // [rsp+110h] [rbp-CD8h]
//     PCURVE* new_pcu;                                                                                                // [rsp+118h] [rbp-CD0h]
//     SPAposition start1;                                                                                             // [rsp+120h] [rbp-CC8h] BYREF
//     int intersects;                                                                                                 // [rsp+138h] [rbp-CB0h]
//     curve_curve_rel v134;                                                                                           // [rsp+13Ch] [rbp-CACh] BYREF
//     curve_curve_rel v135;                                                                                           // [rsp+140h] [rbp-CA8h] BYREF
//     int full_ellipse;                                                                                               // [rsp+144h] [rbp-CA4h] BYREF
//     int same_sense;                                                                                                 // [rsp+148h] [rbp-CA0h] BYREF
//     int v138;                                                                                                       // [rsp+14Ch] [rbp-C9Ch]
//     int v139;                                                                                                       // [rsp+150h] [rbp-C98h]
//     int pc_periodic;                                                                                                // [rsp+154h] [rbp-C94h]
//     long double t2[3];                                                                                              // [rsp+158h] [rbp-C90h] BYREF
//     error_info_base_ptr;                                                                                            // [rsp+170h] [rbp-C78h]
//     SPAposition start2;                                                                                             // [rsp+178h] [rbp-C70h] BYREF
//     const curve* v144;                                                                                              // [rsp+190h] [rbp-C58h]
//     const curve* v145;                                                                                              // [rsp+198h] [rbp-C50h]
//     SURFACE* v146;                                                                                                  // [rsp+1A0h] [rbp-C48h]
//     PCURVE* v147;                                                                                                   // [rsp+1A8h] [rbp-C40h]
//     PCURVE* v148;                                                                                                   // [rsp+1B0h] [rbp-C38h]
//     PCURVE* v149;                                                                                                   // [rsp+1B8h] [rbp-C30h]
//     PCURVE* v150;                                                                                                   // [rsp+1C0h] [rbp-C28h]
//     COEDGE* v151;                                                                                                   // [rsp+1C8h] [rbp-C20h]
//     const curve* v152;                                                                                              // [rsp+1D0h] [rbp-C18h]
//     const curve* v153;                                                                                              // [rsp+1D8h] [rbp-C10h]
//     char* v154;                                                                                                     // [rsp+1E0h] [rbp-C08h]
//     char* v155;                                                                                                     // [rsp+1E8h] [rbp-C00h]
//     v156;                                                                                                           // [rsp+1F0h] [rbp-BF8h]
//     const curve* v157;                                                                                              // [rsp+1F8h] [rbp-BF0h]
//     SPAunit_vector* v158;                                                                                           // [rsp+200h] [rbp-BE8h]
//     SPAunit_vector* v159;                                                                                           // [rsp+208h] [rbp-BE0h]
//     SPAposition* p_end1;                                                                                            // [rsp+210h] [rbp-BD8h]
//     SPAposition* p_start1;                                                                                          // [rsp+218h] [rbp-BD0h]
//     curve_curve_int* v162;                                                                                          // [rsp+220h] [rbp-BC8h]
//     __int64 v163;                                                                                                   // [rsp+228h] [rbp-BC0h]
//     curve_curve_int* v164;                                                                                          // [rsp+230h] [rbp-BB8h]
//     curve_curve_int* v165;                                                                                          // [rsp+238h] [rbp-BB0h]
//     long double t1;                                                                                                 // [rsp+240h] [rbp-BA8h] BYREF
//     long double min_dist;                                                                                           // [rsp+248h] [rbp-BA0h]
//     long double dist_to_inter;                                                                                      // [rsp+250h] [rbp-B98h]
//     long double ptst_len;                                                                                           // [rsp+258h] [rbp-B90h]
//     long double pten_len;                                                                                           // [rsp+260h] [rbp-B88h]
//     curve_curve_int* v171;                                                                                          // [rsp+268h] [rbp-B80h]
//     const curve* v172;                                                                                              // [rsp+270h] [rbp-B78h]
//     long double param;                                                                                              // [rsp+278h] [rbp-B70h] BYREF
//     const curve* v174;                                                                                              // [rsp+280h] [rbp-B68h]
//     const curve* v175;                                                                                              // [rsp+288h] [rbp-B60h]
//     const curve* v176;                                                                                              // [rsp+290h] [rbp-B58h]
//     const curve* v177;                                                                                              // [rsp+298h] [rbp-B50h]
//     const curve* v178;                                                                                              // [rsp+2A0h] [rbp-B48h]
//     curve_curve_int* v179;                                                                                          // [rsp+2A8h] [rbp-B40h]
//     SPAposition end1;                                                                                               // [rsp+2B0h] [rbp-B38h] BYREF
//     SPAposition pt2;                                                                                                // [rsp+2C8h] [rbp-B20h] BYREF
//     SPAposition_array prospective_sols;                                                                             // [rsp+2E0h] [rbp-B08h] BYREF
//     SPAinterval cur2_range;                                                                                         // [rsp+2F8h] [rbp-AF0h] BYREF
//                                                                                                                     // [rsp+310h] [rbp-AD8h] BYREF
//     SPAunit_vector dir2;                                                                                            // [rsp+328h] [rbp-AC0h] BYREF
//     SPAunit_vector dir1;                                                                                            // [rsp+340h] [rbp-AA8h] BYREF
//     AcisVersion v187;                                                                                               // [rsp+358h] [rbp-A90h] BYREF
//     char* v188;                                                                                                     // [rsp+360h] [rbp-A88h]
//     char* v189;                                                                                                     // [rsp+368h] [rbp-A80h]
//     SPAunit_vector* __that;                                                                                         // [rsp+370h] [rbp-A78h]
//     SPAunit_vector* v191;                                                                                           // [rsp+378h] [rbp-A70h]
//     SPAparameter* param_actual;                                                                                     // [rsp+380h] [rbp-A68h]
//     SPAparameter* param_guess;                                                                                      // [rsp+388h] [rbp-A60h]
//     SPAparameter* parameter;                                                                                        // [rsp+390h] [rbp-A58h]
//     SPAparameter* v195;                                                                                             // [rsp+398h] [rbp-A50h]
//     AcisVersion* vt2;                                                                                               // [rsp+3A0h] [rbp-A48h]
//     AcisVersion* vt1;                                                                                               // [rsp+3A8h] [rbp-A40h]
//     long double cross_pr_len;                                                                                       // [rsp+3B0h] [rbp-A38h]
//     long double proj_dist;                                                                                          // [rsp+3B8h] [rbp-A30h]
//     SPAposition* temp;                                                                                              // [rsp+3C0h] [rbp-A28h]
//     SPAparameter* v201;                                                                                             // [rsp+3C8h] [rbp-A20h]
//     SPAposition* v202;                                                                                              // [rsp+3D0h] [rbp-A18h]
//     long double(__fastcall * v203)(struct straight*, const SPAposition*, const SPAparameter*);                      // [rsp+3D8h] [rbp-A10h]
//     SPAparameter* v204;                                                                                             // [rsp+3E0h] [rbp-A08h]
//     SPAposition* v205;                                                                                              // [rsp+3E8h] [rbp-A00h]
//     long double(__fastcall * v206)(struct straight*, const SPAposition*, const SPAparameter*);                      // [rsp+3F0h] [rbp-9F8h]
//     SPApar_pos* _uv;                                                                                                // [rsp+3F8h] [rbp-9F0h]
//     long double d2;                                                                                                 // [rsp+400h] [rbp-9E8h]
//     long double d1;                                                                                                 // [rsp+408h] [rbp-9E0h]
//     SPAposition* p;                                                                                                 // [rsp+410h] [rbp-9D8h]
//     __int64 v211;                                                                                                   // [rsp+418h] [rbp-9D0h]
//     SPAparameter* v212;                                                                                             // [rsp+420h] [rbp-9C8h]
//     SPAposition* v213;                                                                                              // [rsp+428h] [rbp-9C0h]
//     long double(__fastcall * v214)(struct straight*, const SPAposition*, const SPAparameter*);                      // [rsp+430h] [rbp-9B8h]
//     SPAparameter* v215;                                                                                             // [rsp+438h] [rbp-9B0h]
//     SPAposition* v216;                                                                                              // [rsp+440h] [rbp-9A8h]
//     long double(__fastcall * v217)(struct straight*, const SPAposition*, const SPAparameter*);                      // [rsp+448h] [rbp-9A0h]
//     SPApar_pos* v218;                                                                                               // [rsp+450h] [rbp-998h]
//     long double v219;                                                                                               // [rsp+458h] [rbp-990h]
//     long double v220;                                                                                               // [rsp+460h] [rbp-988h]
//     SPAposition* v221;                                                                                              // [rsp+468h] [rbp-980h]
//     curve_curve_int* n;                                                                                             // [rsp+470h] [rbp-978h]
//     curve_curve_int* v223;                                                                                          // [rsp+478h] [rbp-970h]
//     long double line_len;                                                                                           // [rsp+480h] [rbp-968h]
//     SPAparameter* v225;                                                                                             // [rsp+488h] [rbp-960h]
//     long double(__fastcall * v226)(struct straight*, const SPAposition*, const SPAparameter*);                      // [rsp+490h] [rbp-958h]
//     SPAparameter* v227;                                                                                             // [rsp+498h] [rbp-950h]
//     long double(__fastcall * v228)(struct straight*, const SPAposition*, const SPAparameter*);                      // [rsp+4A0h] [rbp-948h]
//     SPApar_pos* par_pos;                                                                                            // [rsp+4A8h] [rbp-940h]
//     long double v230;                                                                                               // [rsp+4B0h] [rbp-938h]
//     long double v231;                                                                                               // [rsp+4B8h] [rbp-930h]
//     curve_curve_int* v232;                                                                                          // [rsp+4C0h] [rbp-928h]
//     SPAposition*(__fastcall * start_pos)(EDGE*, SPAposition*);                                                      // [rsp+4C8h] [rbp-920h]
//     SPAparameter* v234;                                                                                             // [rsp+4D0h] [rbp-918h]
//     SPAparameter* v235;                                                                                             // [rsp+4D8h] [rbp-910h]
//     __int64 v236;                                                                                                   // [rsp+4E0h] [rbp-908h]
//     int(__fastcall * test_point_tol)(curve*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);  // [rsp+4E8h] [rbp-900h]
//     SPAposition*(__fastcall * v238)(EDGE*, SPAposition*);                                                           // [rsp+4F0h] [rbp-8F8h]
//     SPAposition*(__fastcall * end_pos)(EDGE*, SPAposition*);                                                        // [rsp+4F8h] [rbp-8F0h]
//     SPAparameter* v240;                                                                                             // [rsp+500h] [rbp-8E8h]
//     SPAparameter* v241;                                                                                             // [rsp+508h] [rbp-8E0h]
//     __int64 v242;                                                                                                   // [rsp+510h] [rbp-8D8h]
//     int(__fastcall * v243)(curve*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);            // [rsp+518h] [rbp-8D0h]
//     SPAposition*(__fastcall * v244)(EDGE*, SPAposition*);                                                           // [rsp+520h] [rbp-8C8h]
//     SPAposition*(__fastcall * v245)(EDGE*, SPAposition*);                                                           // [rsp+528h] [rbp-8C0h]
//     SPAparameter* v246;                                                                                             // [rsp+530h] [rbp-8B8h]
//     SPAparameter* v247;                                                                                             // [rsp+538h] [rbp-8B0h]
//     __int64 v248;                                                                                                   // [rsp+540h] [rbp-8A8h]
//     int(__fastcall * v249)(curve*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);            // [rsp+548h] [rbp-8A0h]
//     SPAposition*(__fastcall * v250)(EDGE*, SPAposition*);                                                           // [rsp+550h] [rbp-898h]
//     SPAposition*(__fastcall * v251)(EDGE*, SPAposition*);                                                           // [rsp+558h] [rbp-890h]
//     SPAparameter* v252;                                                                                             // [rsp+560h] [rbp-888h]
//     SPAparameter* v253;                                                                                             // [rsp+568h] [rbp-880h]
//     __int64 v254;                                                                                                   // [rsp+570h] [rbp-878h]
//     int(__fastcall * v255)(curve*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);            // [rsp+578h] [rbp-870h]
//     SPAposition*(__fastcall * v256)(EDGE*, SPAposition*);                                                           // [rsp+580h] [rbp-868h]
//     SPAposition* p2;                                                                                                // [rsp+588h] [rbp-860h]
//     SPAposition* p1;                                                                                                // [rsp+590h] [rbp-858h]
//     SPAposition* v259;                                                                                              // [rsp+598h] [rbp-850h]
//     SPAposition* v260;                                                                                              // [rsp+5A0h] [rbp-848h]
//     long double diag2_len;                                                                                          // [rsp+5A8h] [rbp-840h]
//     long double diag1_len;                                                                                          // [rsp+5B0h] [rbp-838h]
//     SPAvector* v;                                                                                                   // [rsp+5B8h] [rbp-830h]
//     SPAposition* p_int_point;                                                                                       // [rsp+5C0h] [rbp-828h]
//     SPAvector* v265;                                                                                                // [rsp+5C8h] [rbp-820h]
//     SPAposition* v266;                                                                                              // [rsp+5D0h] [rbp-818h]
//     SPAparameter* v267;                                                                                             // [rsp+5D8h] [rbp-810h]
//     SPAparameter* v268;                                                                                             // [rsp+5E0h] [rbp-808h]
//     int(__fastcall * v269)(curve*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);            // [rsp+5E8h] [rbp-800h]
//     SPAparameter* v270;                                                                                             // [rsp+5F0h] [rbp-7F8h]
//     SPAparameter* v271;                                                                                             // [rsp+5F8h] [rbp-7F0h]
//     int(__fastcall * v272)(curve*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);            // [rsp+600h] [rbp-7E8h]
//     AcisVersion* v273;                                                                                              // [rsp+608h] [rbp-7E0h]
//     AcisVersion* v274;                                                                                              // [rsp+610h] [rbp-7D8h]
//     SPAposition*(__fastcall * mid_pos)(EDGE*, SPAposition*, int);                                                   // [rsp+618h] [rbp-7D0h]
//     SPAparameter* v276;                                                                                             // [rsp+620h] [rbp-7C8h]
//     SPAparameter* v277;                                                                                             // [rsp+628h] [rbp-7C0h]
//     __int64 v278;                                                                                                   // [rsp+630h] [rbp-7B8h]
//     int(__fastcall * v279)(curve*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);            // [rsp+638h] [rbp-7B0h]
//     AcisVersion* v280;                                                                                              // [rsp+640h] [rbp-7A8h]
//     AcisVersion* v281;                                                                                              // [rsp+648h] [rbp-7A0h]
//     SPAposition*(__fastcall * v282)(EDGE*, SPAposition*, int);                                                      // [rsp+650h] [rbp-798h]
//     SPAparameter* v283;                                                                                             // [rsp+658h] [rbp-790h]
//     SPAparameter* v284;                                                                                             // [rsp+660h] [rbp-788h]
//     __int64 v285;                                                                                                   // [rsp+668h] [rbp-780h]
//     int(__fastcall * v286)(curve*, const SPAposition*, long double, const SPAparameter*, SPAparameter*);            // [rsp+670h] [rbp-778h]
//     SPAbox* region_of_interest;                                                                                     // [rsp+678h] [rbp-770h]
//     curve* v288;                                                                                                    // [rsp+680h] [rbp-768h]
//     curve* v289;                                                                                                    // [rsp+688h] [rbp-760h]
//     LOOP* v290;                                                                                                     // [rsp+690h] [rbp-758h]
//     LOOP* v291;                                                                                                     // [rsp+698h] [rbp-750h]
//     LOOP* v292;                                                                                                     // [rsp+6A0h] [rbp-748h]
//     LOOP* v293;                                                                                                     // [rsp+6A8h] [rbp-740h]
//     SPApar_box*(__fastcall * param_range)(surface*, SPApar_box*, const SPAbox*);                                    // [rsp+6B0h] [rbp-738h]
//     PCURVE* v295;                                                                                                   // [rsp+6B8h] [rbp-730h]
//     PCURVE* v296;                                                                                                   // [rsp+6C0h] [rbp-728h]
//     AcisVersion* v297;                                                                                              // [rsp+6C8h] [rbp-720h]
//     AcisVersion* v298;                                                                                              // [rsp+6D0h] [rbp-718h]
//     PCURVE* v299;                                                                                                   // [rsp+6D8h] [rbp-710h]
//     void(__fastcall * set_geometry)(COEDGE*, PCURVE*, int);                                                         // [rsp+6E0h] [rbp-708h]
//     AcisVersion* v301;                                                                                              // [rsp+6E8h] [rbp-700h]
//     AcisVersion* v302;                                                                                              // [rsp+6F0h] [rbp-6F8h]
//     PCURVE* v303;                                                                                                   // [rsp+6F8h] [rbp-6F0h]
//     void(__fastcall * v304)(COEDGE*, PCURVE*, int);                                                                 // [rsp+700h] [rbp-6E8h]
//     PCURVE* v305;                                                                                                   // [rsp+708h] [rbp-6E0h]
//     SPAinterval*(__fastcall * v306)(COEDGE*, SPAinterval*);                                                         // [rsp+710h] [rbp-6D8h]
//     void(__fastcall * v307)(curve*);                                                                                // [rsp+718h] [rbp-6D0h]
//     __int64 v308;                                                                                                   // [rsp+720h] [rbp-6C8h]
//     void(__fastcall * v309)(curve*);                                                                                // [rsp+728h] [rbp-6C0h]
//     __int64 v310;                                                                                                   // [rsp+730h] [rbp-6B8h]
//     exception_save exception_save_mark;                                                                             // [rsp+738h] [rbp-6B0h] BYREF
//     AcisVersion v312;                                                                                               // [rsp+748h] [rbp-6A0h] BYREF
//     AcisVersion v313;                                                                                               // [rsp+74Ch] [rbp-69Ch] BYREF
//     AcisVersion v314;                                                                                               // [rsp+750h] [rbp-698h] BYREF
//     AcisVersion v315;                                                                                               // [rsp+754h] [rbp-694h] BYREF
//     AcisVersion v316;                                                                                               // [rsp+758h] [rbp-690h] BYREF
//     AcisVersion v317;                                                                                               // [rsp+75Ch] [rbp-68Ch] BYREF
//     AcisVersion v318;                                                                                               // [rsp+760h] [rbp-688h] BYREF
//     AcisVersion v319;                                                                                               // [rsp+764h] [rbp-684h] BYREF
//     AcisVersion v320;                                                                                               // [rsp+768h] [rbp-680h] BYREF
//     SPAposition end2;                                                                                               // [rsp+770h] [rbp-678h] BYREF
//     SPAposition foot;                                                                                               // [rsp+788h] [rbp-660h] BYREF
//     SPAinterval ed1_range;                                                                                          // [rsp+7A0h] [rbp-648h] BYREF
//     SPAinterval ed2_range;                                                                                          // [rsp+7B8h] [rbp-630h] BYREF
//     pcurve pcu2;                                                                                                    // [rsp+7D0h] [rbp-618h] BYREF
//     pcurve pcu1;                                                                                                    // [rsp+7F8h] [rbp-5F0h] BYREF
//     SPAunit_vector curve_dir1;                                                                                      // [rsp+820h] [rbp-5C8h] BYREF
//     SPAunit_vector curve_dir2;                                                                                      // [rsp+838h] [rbp-5B0h] BYREF
//     SPAposition pt1;                                                                                                // [rsp+850h] [rbp-598h] BYREF
//     SPAposition pt1_plus_smaller_len;                                                                               // [rsp+868h] [rbp-580h] BYREF
//     SPAposition pt1_minus_smaller_len;                                                                              // [rsp+880h] [rbp-568h] BYREF
//     SPAinterval pcurve_range;                                                                                       // [rsp+898h] [rbp-550h] BYREF
//     acis_exception error_info_holder;                                                                               // [rsp+8B0h] [rbp-538h] BYREF
//     pcurve temp_pcurve;                                                                                             // [rsp+8D0h] [rbp-518h] BYREF
//     char v335[24];                                                                                                  // [rsp+938h] [rbp-4B0h] BYREF
//     char v336[24];                                                                                                  // [rsp+950h] [rbp-498h] BYREF
//     SPAvector cross_prod;                                                                                           // [rsp+968h] [rbp-480h] BYREF
//     SPAposition proj_pt;                                                                                            // [rsp+980h] [rbp-468h] BYREF
//     SPAinterval coedge_range;                                                                                       // [rsp+998h] [rbp-450h] BYREF
//     SPAvector v340;                                                                                                 // [rsp+9B0h] [rbp-438h] BYREF
//     SPAvector v341;                                                                                                 // [rsp+9C8h] [rbp-420h] BYREF
//     SPAvector v342;                                                                                                 // [rsp+9E0h] [rbp-408h] BYREF
//     SPAvector v343;                                                                                                 // [rsp+9F8h] [rbp-3F0h] BYREF
//     SPAvector v344;                                                                                                 // [rsp+A10h] [rbp-3D8h] BYREF
//     SPAvector v345;                                                                                                 // [rsp+A28h] [rbp-3C0h] BYREF
//     SPAvector v346;                                                                                                 // [rsp+A40h] [rbp-3A8h] BYREF
//     SPAvector v347;                                                                                                 // [rsp+A58h] [rbp-390h] BYREF
//     SPAposition v348;                                                                                               // [rsp+A70h] [rbp-378h] BYREF
//     SPAposition v349;                                                                                               // [rsp+A88h] [rbp-360h] BYREF
//     SPAposition v350;                                                                                               // [rsp+AA0h] [rbp-348h] BYREF
//     SPAposition v351;                                                                                               // [rsp+AB8h] [rbp-330h] BYREF
//     SPAposition v352;                                                                                               // [rsp+AD0h] [rbp-318h] BYREF
//     SPAposition v353;                                                                                               // [rsp+AE8h] [rbp-300h] BYREF
//     SPAposition v354;                                                                                               // [rsp+B00h] [rbp-2E8h] BYREF
//     SPAposition v355;                                                                                               // [rsp+B18h] [rbp-2D0h] BYREF
//     SPAposition v356;                                                                                               // [rsp+B30h] [rbp-2B8h] BYREF
//     SPAposition v357;                                                                                               // [rsp+B48h] [rbp-2A0h] BYREF
//     SPAvector v358;                                                                                                 // [rsp+B60h] [rbp-288h] BYREF
//     SPAposition v359;                                                                                               // [rsp+B78h] [rbp-270h] BYREF
//     SPAposition v360;                                                                                               // [rsp+B90h] [rbp-258h] BYREF
//     SPAinterval result;                                                                                             // [rsp+BA8h] [rbp-240h] BYREF
//     SPAvector v362;                                                                                                 // [rsp+BC0h] [rbp-228h] BYREF
//     SPAvector v363;                                                                                                 // [rsp+BD8h] [rbp-210h] BYREF
//     SPAposition v364;                                                                                               // [rsp+BF0h] [rbp-1F8h] BYREF
//     SPAposition v365;                                                                                               // [rsp+C08h] [rbp-1E0h] BYREF
//     SPAinterval v366;                                                                                               // [rsp+C20h] [rbp-1C8h] BYREF
//     SPAunit_vector v367;                                                                                            // [rsp+C38h] [rbp-1B0h] BYREF
//     SPAvector v368;                                                                                                 // [rsp+C50h] [rbp-198h] BYREF
//     SPAunit_vector v369;                                                                                            // [rsp+C68h] [rbp-180h] BYREF
//     SPAunit_vector v370;                                                                                            // [rsp+C80h] [rbp-168h] BYREF
//     SPAunit_vector v371;                                                                                            // [rsp+C98h] [rbp-150h] BYREF
//     SPApar_box sf_range;                                                                                            // [rsp+CB0h] [rbp-138h] BYREF
//     SPAbox edge_box1;                                                                                               // [rsp+CE0h] [rbp-108h] BYREF
//     SPAbox edge_box2;                                                                                               // [rsp+D30h] [rbp-B8h] BYREF
//     SPAbox face_box;                                                                                                // [rsp+D80h] [rbp-68h] BYREF
//
//     ccrel_data 交点 cur1 cur2 ed_cur1 curve ed_cur2 curve ed1_range ed2_range cur1_range cur2_range
//
//       double dist_tol = SPArasabs;
//     angle_tol = SPAresnor;
//     curve_curve_int* ccrel_data = nullptr;
//     curve* ed_cur1 = nullptr;
//     curve* ed_cur2 = nullptr;
//     error_no = 0;
//     CURVE* cur1 = edge1->geometry();
//     CURVE* cur2 = edge2->geometry();
//     if(cur1) {
//         const curve& v152 = cur1->equation();
//         ed_cur1 = v152.make_copy();
//     }
//     if(cur2) {
//         const curve& v153 = cur2->equation();
//         ed_cur2 = v153.make_copy();
//     }
//     ed1_range = edge1->param_range();
//     ed2_range = edge2->param_range();
//     SPAinterval cur1_range;
//
//     if(edge1->sense() == 1) {
//         cur1_range = -ed1_range;
//     } else {
//         cur1_range = ed1_range;
//     }
//
//     // if(EDGE::sense(edge2) == 1) {
//     //     v155 = (char*)operator-(&v366, &ed2_range);
//     // } else {
//     //     qmemcpy(v336, &ed2_range, sizeof(v336));
//     //     v155 = v336;
//     // }
//     // v189 = v155;
//     // qmemcpy(&cur2_range, v155, sizeof(cur2_range));
//
//     eq1 eq2 SPAposition start1 SPAposition start2 SPAposition end1 SPAposition end2 curve_dir1 dir1 curve_dir2 dir2 cross_prod = dir1 * dir2
//
//                                                                                                                                  if(cur1 && cur2) {
//         const curve& v156 = cur1->equation();
//         if(v156.type() == 1 && (v157 = cur2->equation(cur2), v157->type((curve*)v157) == 1)) {
//             eq1 = (const straight*)ed_cur1;
//             eq2 = (const straight*)ed_cur2;
//             v4 = EDGE::start(edge1);
//             v5 = VERTEX::geometry(v4);
//             v6 = APOINT::coords(v5);
//             SPAposition::SPAposition(&start1, v6);
//             v7 = EDGE::start(edge2);
//             v8 = VERTEX::geometry(v7);
//             v9 = APOINT::coords(v8);
//             SPAposition::SPAposition(&start2, v9);
//             v10 = EDGE::end(edge1);
//             v11 = VERTEX::geometry(v10);
//             v12 = APOINT::coords(v11);
//             SPAposition::SPAposition(&end1, v12);
//             v13 = EDGE::end(edge2);
//             v14 = VERTEX::geometry(v13);
//             v15 = APOINT::coords(v14);
//             SPAposition::SPAposition(&end2, v15);
//             SPAunit_vector::SPAunit_vector(&curve_dir1, (const SPAunit_vector*)&ed_cur1[1]);
//             SPAunit_vector::SPAunit_vector(&curve_dir2, (const SPAunit_vector*)&ed_cur2[1]);
//             if(EDGE::sense(edge1) == 1) {
//                 v158 = operator-(&v367, &curve_dir1);
//             } else {
//                 SPAunit_vector::SPAunit_vector(&v369, &curve_dir1);
//                 v158 = v16;
//             }
//             __that = v158;
//             SPAunit_vector::SPAunit_vector(&dir1, v158);
//             if(EDGE::sense(edge2) == 1) {
//                 v159 = operator-(&v370, &curve_dir2);
//             } else {
//                 SPAunit_vector::SPAunit_vector(&v371, &curve_dir2);
//                 v159 = v17;
//             }
//             v191 = v159;
//             SPAunit_vector::SPAunit_vector(&dir2, v159);
//
//             SPAposition::SPAposition(&foot);
//             param_actual = SpaAcis::NullObj::get_parameter();
//             param_guess = SpaAcis::NullObj::get_parameter();
//             straight::point_perp((straight*)eq1, &start1, &foot, param_guess, param_actual, 0);
//             qmemcpy(&start1, &foot, sizeof(start1));
//             parameter = SpaAcis::NullObj::get_parameter();
//             v195 = SpaAcis::NullObj::get_parameter();
//             straight::point_perp((straight*)eq2, &start2, &foot, v195, parameter, 0);
//             qmemcpy(&start2, &foot, sizeof(start2));
//             operator*(&cross_prod, &dir1, &dir2);
//             cross_pr_len = SPAvector::len(&cross_prod);
//             AcisVersion::AcisVersion(&v312, 22, 0, 0);
//             vt2 = v18;
//             vt1 = GET_ALGORITHMIC_VERSION(&v313);
//             R22_onwards = operator>=(vt1, vt2);
//             if(angle_tol < cross_pr_len) {
//                 check_vertices = 1;
//                 SPAposition::SPAposition(&pt1);
//                 SPAposition::SPAposition(&pt2);
//                 closest_points(&start1, &dir1, &start2, &dir2, &t1, &pt1, t2, &pt2);
//                 t1 = t1 / eq1->param_scale;
//                 t2[0] = t2[0] / eq2->param_scale;
//
//                 v25 = operator-(&v343, &pt1, &pt2);
//                 min_dist = SPAvector::len(v25);
//                 if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Minimum distance between lines is %g\n", (double)min_dist);
//
//                 if(dist_tol >= min_dist) {
//                     dist_to_inter = t2[0] * eq2->param_scale;
//                     if(dist_to_inter >= COERCE_DOUBLE(*(_QWORD*)&dist_tol ^ _xmm)) {
//                         v26 = operator-(&v344, &end2, &start2);
//                         v27 = SPAvector::len(v26);
//                         if(v27 + dist_tol >= dist_to_inter) {
//                             if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Intersecting Lines \n");
//                             v28 = operator-(&v345, &start1, &end1);
//                             line_len = SPAvector::len(v28);  // 第一条线的长度
//                             v29 = operator-(&v346, &start1, &pt2);
//                             ptst_len = SPAvector::len(v29);
//                             v30 = operator-(&v347, &end1, &pt2);
//                             pten_len = SPAvector::len(v30);
//                             v121 = dist_tol >= fabs_0(ptst_len) || dist_tol >= fabs_0(pten_len) || dist_tol >= fabs_0(ptst_len + pten_len - line_len);
//                             intersects = v121;
//                             if(v121) {
//                                 check_vertices = 0;
//                                 v171 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 2537, &alloc_file_index_2127);
//                                 if(v171) {
//                                     par_pos = SpaAcis::NullObj::get_par_pos();
//                                     v226 = eq2->param;
//                                     v225 = SpaAcis::NullObj::get_parameter();
//                                     v230 = v226((struct straight*)eq2, &pt2, v225);
//                                     v228 = eq1->param;
//                                     v227 = SpaAcis::NullObj::get_parameter();
//                                     v231 = v228((struct straight*)eq1, &pt2, v227);
//                                     curve_curve_int::curve_curve_int(v171, 0i64, &pt2, v231, v230, par_pos);
//                                     v179 = v31;
//                                 } else {
//                                     v179 = 0i64;
//                                 }
//                                 v232 = v179;
//                                 ccrel_data = v179;  // 存交点参数值
//
//                                 v32 = operator%(&dir2, &dir1);
//                                 if(angle_tol < fabs_0(v32)) {
//                                     ccrel_data->low_rel = cur_cur_unknown;
//                                     ccrel_data->high_rel = cur_cur_unknown;
//                                 } else {
//                                     ccrel_data->low_rel = cur_cur_normal;
//                                     ccrel_data->high_rel = cur_cur_normal;
//                                 }
//                                 if(R22_onwards) {
//                                     ccrel_data->low_rel = cur_cur_normal;
//                                     ccrel_data->high_rel = cur_cur_normal;
//                                 }
//                                 if(sg_inter_module_header.debug_level >= 0x1E) {
//                                     acis_fprintf(debug_file_ptr, "Internal Intersection \n");
//                                     sg_debug_cci(ccrel_data, debug_file_ptr, "\t");
//                                 }
//                             }
//                         }
//                     }
//                 }
//
//                 if(R22_onwards) {
//                     if(check_vertices) {
//                         SPAposition_array::SPAposition_array(&prospective_sols, 0, 2);
//                         test_point_tol = ed_cur2->test_point_tol;
//                         v234 = SpaAcis::NullObj::get_parameter();
//                         v235 = SpaAcis::NullObj::get_parameter();
//                         start_pos = edge1->start_pos;
//                         v236 = (__int64)start_pos(edge1, &v348);
//                         if(((unsigned int(__fastcall*)(curve*, __int64, __int64, SPAparameter*, SPAparameter*))test_point_tol)(ed_cur2, v236, v33, v235, v234)) {
//                             v238 = edge1->start_pos;
//                             v34 = v238(edge1, &v349);
//                             SPAposition_array::Push(&prospective_sols, v34);
//                         }
//                         v243 = ed_cur2->test_point_tol;
//                         v240 = SpaAcis::NullObj::get_parameter();
//                         v241 = SpaAcis::NullObj::get_parameter();
//                         end_pos = edge1->end_pos;
//                         v242 = (__int64)end_pos(edge1, &v350);
//                         if(((unsigned int(__fastcall*)(curve*, __int64, __int64, SPAparameter*, SPAparameter*))v243)(ed_cur2, v242, v35, v241, v240)) {
//                             v244 = edge1->end_pos;
//                             v36 = v244(edge1, &v351);
//                             SPAposition_array::Push(&prospective_sols, v36);
//                         }
//                         v249 = ed_cur1->test_point_tol;
//                         v246 = SpaAcis::NullObj::get_parameter();
//                         v247 = SpaAcis::NullObj::get_parameter();
//                         v245 = edge2->start_pos;
//                         v248 = (__int64)v245(edge2, &v352);
//                         if(((unsigned int(__fastcall*)(curve*, __int64, __int64, SPAparameter*, SPAparameter*))v249)(ed_cur1, v248, v37, v247, v246)) {
//                             v250 = edge2->start_pos;
//                             v38 = v250(edge2, &v353);
//                             SPAposition_array::Push(&prospective_sols, v38);
//                         }
//                         v255 = ed_cur1->test_point_tol;
//                         v252 = SpaAcis::NullObj::get_parameter();
//                         v253 = SpaAcis::NullObj::get_parameter();
//                         v251 = edge2->end_pos;
//                         v254 = (__int64)v251(edge2, &v354);
//                         if(((unsigned int(__fastcall*)(curve*, __int64, __int64, SPAparameter*, SPAparameter*))v255)(ed_cur1, v254, v39, v253, v252)) {
//                             v256 = edge2->end_pos;
//                             v40 = v256(edge2, &v355);
//                             SPAposition_array::Push(&prospective_sols, v40);
//                         }
//                         for(i = 0;; ++i) {
//                             v41 = SPAposition_array::Size(&prospective_sols);
//                             if(i >= v41) break;
//                             v134 = cur_cur_normal;
//                             v135 = cur_cur_normal;
//                             v42 = SPAposition_array::operator[](&prospective_sols, i);
//                             if(test_intersection_point(v42, edge1, edge2, &ccrel_data, &v135, &v134, dist_tol, angle_tol)) break;
//                         }
//                         SPAposition_array::~SPAposition_array(&prospective_sols);
//                     }
//                     param = 0.0;
//                     if(ccrel_data) {
//                         get_edge_box(&edge_box1, edge1, 0i64, 0, 0i64);
//                         get_edge_box(&edge_box2, edge2, 0i64, 0, 0i64);
//                         p2 = SPAbox::high(&edge_box1, &v356);
//                         p1 = SPAbox::low(&edge_box1, &v357);
//                         v43 = operator-(&v358, p1, p2);
//                         diag1_len = 0.1 * SPAvector::len(v43);
//                         v259 = SPAbox::high(&edge_box2, &v359);
//                         v260 = SPAbox::low(&edge_box2, &v360);
//                         v44 = operator-(&v368, v260, v259);
//                         diag2_len = 0.1 * SPAvector::len(v44);
//                         smaller_len = D3_min(diag1_len, diag2_len);
//                         smaller_len = D3_min(smaller_len, 10.0 * dist_tol) / 2.0;
//                         v = operator*(&v362, &dir1, smaller_len);
//                         p_int_point = &ccrel_data->int_point;
//                         operator+(&pt1_plus_smaller_len, &ccrel_data->int_point, v);
//                         v265 = operator*(&v363, &dir1, smaller_len);
//                         v266 = &ccrel_data->int_point;
//                         operator-(&pt1_minus_smaller_len, &ccrel_data->int_point, v265);
//                         if(sg_pt_on_in_edge(&pt1_plus_smaller_len, edge1, &param, dist_tol, 0i64)) {
//                             v269 = ed_cur2->test_point_tol;
//                             v267 = SpaAcis::NullObj::get_parameter();
//                             v268 = SpaAcis::NullObj::get_parameter();
//                             if(((unsigned int(__fastcall*)(curve*, SPAposition*, __int64, SPAparameter*, SPAparameter*))v269)(ed_cur2, &pt1_plus_smaller_len, v45, v268, v267)) {
//                                 goto LABEL_106;
//                             }
//                         }
//                         if(sg_pt_on_in_edge(&pt1_minus_smaller_len, edge1, &param, dist_tol, 0i64)) {
//                             v272 = ed_cur2->test_point_tol;
//                             v270 = SpaAcis::NullObj::get_parameter();
//                             v271 = SpaAcis::NullObj::get_parameter();
//                             if(((unsigned int(__fastcall*)(curve*, SPAposition*, __int64, SPAparameter*, SPAparameter*))v272)(ed_cur2, &pt1_minus_smaller_len, v46, v271, v270)) {
//                             LABEL_106:
//                                 sg_delete_cci_list(ccrel_data);
//                                 ccrel_data = d3_cu_cu_int(ed_cur1, &cur1_range, ed_cur2, &cur2_range, dist_tol);
//                             }
//                         }
//                     }
//                 }
//             } else {
//                 if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Parallel Lines \n");
//                 v19 = operator-(&v340, &start1, &start2);
//                 v20 = operator%(v19, &dir2);
//                 v21 = operator*(&v341, v20, &dir2);
//                 operator+(&proj_pt, &start2, v21);
//                 v22 = operator-(&v342, &start1, &proj_pt);
//                 proj_dist = SPAvector::len(v22);
//                 if(dist_tol >= proj_dist) {
//                     start_pt = 0i64;
//                     end_pt = 0i64;
//                     if(!line_line_overlap(&start1, &end1, &start2, &end2, dist_tol, &start_pt, &end_pt)) {
//                         if(line_line_overlap(&start2, &end2, &start1, &end1, dist_tol, &start_pt, &end_pt)) {
//                             if(operator%(&dir1, &dir2) < 0.0 && end_pt) {
//                                 temp = start_pt;
//                                 start_pt = end_pt;
//                                 end_pt = temp;
//                             }
//                         } else {
//                             if(same_point(&start1, &start2, dist_tol) || same_point(&start1, &end2, dist_tol)) {
//                                 p_start1 = &start1;
//                             } else {
//                                 if(same_point(&end1, &start2, dist_tol) || same_point(&end1, &end2, dist_tol))
//                                     p_end1 = &end1;
//                                 else
//                                     p_end1 = 0i64;
//                                 p_start1 = p_end1;
//                             }
//                             start_pt = p_start1;
//                         }
//                     }
//                     if(end_pt) {
//                         v162 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 2306, &alloc_file_index_2127);
//                         if(v162) {
//                             _uv = SpaAcis::NullObj::get_par_pos();
//                             v203 = eq2->param;
//                             v201 = SpaAcis::NullObj::get_parameter();
//                             v202 = end_pt;
//                             d2 = v203((struct straight*)eq2, end_pt, v201);
//                             v206 = eq1->param;
//                             v204 = SpaAcis::NullObj::get_parameter();
//                             v205 = end_pt;
//                             d1 = v206((struct straight*)eq1, end_pt, v204);
//                             p = end_pt;
//                             curve_curve_int::curve_curve_int(v162, 0i64, end_pt, d1, d2, _uv);
//                             v163 = v23;
//                         } else {
//                             v163 = 0i64;
//                         }
//                         v211 = v163;
//                         ccrel_data = (curve_curve_int*)v163;
//                         *(_DWORD*)(v163 + 32) = 3;
//                         ccrel_data->high_rel = cur_cur_coin;
//                     }
//                     if(start_pt) {
//                         v164 = (curve_curve_int*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 2316, &alloc_file_index_2127);
//                         if(v164) {
//                             v218 = SpaAcis::NullObj::get_par_pos();
//                             v214 = eq2->param;
//                             v212 = SpaAcis::NullObj::get_parameter();
//                             v213 = start_pt;
//                             v219 = v214((struct straight*)eq2, start_pt, v212);
//                             v217 = eq1->param;
//                             v215 = SpaAcis::NullObj::get_parameter();
//                             v216 = start_pt;
//                             v220 = v217((struct straight*)eq1, start_pt, v215);
//                             v221 = start_pt;
//                             n = ccrel_data;
//                             curve_curve_int::curve_curve_int(v164, ccrel_data, start_pt, v220, v219, v218);
//                             v165 = v24;
//                         } else {
//                             v165 = 0i64;
//                         }
//                         v223 = v165;
//                         ccrel_data = v165;
//                         if(end_pt) {
//                             ccrel_data->low_rel = cur_cur_coin;
//                             ccrel_data->high_rel = cur_cur_coin;
//                         } else {
//                             ccrel_data->low_rel = cur_cur_tangent;
//                             ccrel_data->high_rel = cur_cur_tangent;
//                         }
//                     }
//                 }
//             }
//             if(ccrel_data && ccrel_data->next) sg_sort_cci(&ccrel_data, 0, 0);
//         } else {
//             v174 = cur1->equation(cur1);
//             if(v174->type((curve*)v174) == 1 && (v175 = cur2->equation(cur2), v175->type((curve*)v175) == 2)) {
//                 difficult_config = 0;
//                 ccrel_data = line_ell_int(edge1, edge2, dist_tol, angle_tol, &difficult_config);
//                 if(difficult_config) goto LABEL_117;
//                 AcisVersion::AcisVersion(&v314, 14, 0, 0);
//                 v273 = v47;
//                 v274 = GET_ALGORITHMIC_VERSION(&v315);
//                 if(operator>=(v274, v273)) {
//                     if(str_ell_intersect_at_ends(ccrel_data, &ed1_range, &ed2_range)) {
//                         v176 = cur1->equation(cur1);
//                         v279 = v176->test_point_tol;
//                         v276 = SpaAcis::NullObj::get_parameter();
//                         v277 = SpaAcis::NullObj::get_parameter();
//                         mid_pos = edge2->mid_pos;
//                         v278 = (__int64)mid_pos(edge2, &v364, 1);
//                         if(((unsigned int(__fastcall*)(const curve*, __int64, __int64, SPAparameter*, SPAparameter*))v279)(v176, v278, v48, v277, v276)) {
//                         LABEL_117:
//                             sg_delete_cci_list(ccrel_data);
//                             ccrel_data = d3_cu_cu_int(ed_cur1, &cur1_range, ed_cur2, &cur2_range, dist_tol);
//                         }
//                     }
//                 }
//                 if(sg_inter_module_header.debug_level >= 0x1E) {
//                     acis_fprintf(debug_file_ptr, "Line-Ellipse Intersection:\n");
//                     sg_debug_cci(ccrel_data, debug_file_ptr, "\t");
//                 }
//             } else {
//                 v177 = cur2->equation(cur2);
//                 if(v177->type((curve*)v177) == 1 && (v172 = cur1->equation(cur1), v172->type((curve*)v172) == 2)) {
//                     v103 = 0;
//                     ccrel_data = line_ell_int(edge2, edge1, dist_tol, angle_tol, &v103);
//                     reverse_param_values(&ccrel_data);
//                     if(v103) goto LABEL_127;
//                     AcisVersion::AcisVersion(&v316, 14, 0, 0);
//                     v280 = v49;
//                     v281 = GET_ALGORITHMIC_VERSION(&v317);
//                     if(operator>=(v281, v280)) {
//                         if(str_ell_intersect_at_ends(ccrel_data, &ed2_range, &ed1_range)) {
//                             v178 = cur2->equation(cur2);
//                             v286 = v178->test_point_tol;
//                             v283 = SpaAcis::NullObj::get_parameter();
//                             v284 = SpaAcis::NullObj::get_parameter();
//                             v282 = edge1->mid_pos;
//                             v285 = (__int64)v282(edge1, &v365, 1);
//                             if(((unsigned int(__fastcall*)(const curve*, __int64, __int64, SPAparameter*, SPAparameter*))v286)(v178, v285, v50, v284, v283)) {
//                             LABEL_127:
//                                 sg_delete_cci_list(ccrel_data);
//                                 ccrel_data = d3_cu_cu_int(ed_cur1, &cur1_range, ed_cur2, &cur2_range, dist_tol);
//                             }
//                         }
//                     }
//                     if(sg_inter_module_header.debug_level >= 0x1E) {
//                         acis_fprintf(debug_file_ptr, "Line-Ellipse Intersection:\n");
//                         sg_debug_cci(ccrel_data, debug_file_ptr, "\t");
//                     }
//                 } else if(special_case_ellipses(edge1, edge2, dist_tol, angle_tol)) {
//                     if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Intersecting two circles \n");
//                     ccrel_data = int_ellipses((ellipse*)ed_cur1, &cur1_range, (ellipse*)ed_cur2, &cur2_range, dist_tol);
//                 } else if(special_case_ellipses2(edge1, edge2, dist_tol, angle_tol, &same_sense, &full_ellipse)) {
//                     if(sg_inter_module_header.debug_level >= 0x1E) acis_fprintf(debug_file_ptr, "Intersecting two concentric, coplanar elliptical arcs \n");
//                     ccrel_data = int_ellipses2((ellipse*)ed_cur1, &cur1_range, (ellipse*)ed_cur2, &cur2_range, same_sense, full_ellipse, dist_tol);
//                 } else {
//                     v144 = cur1->equation(cur1);
//                     if(v144->type((curve*)v144) == 40 || (v145 = cur2->equation(cur2), v145->type((curve*)v145) == 40)) {
//                         region_of_interest = SpaAcis::NullObj::get_box();
//                         v288 = (curve*)cur2->equation(cur2);
//                         v289 = (curve*)cur1->equation(cur1);
//                         ccrel_data = int_cur_cur((ellipse*)v289, (ellipse*)v288, region_of_interest, dist_tol);
//                     } else {
//                         common_face = 0i64;
//                         coed1 = 0i64;
//                         coed2 = 0i64;
//                         if(EDGE::coedge(edge1) && (v51 = EDGE::coedge(edge1), COEDGE::loop(v51)) && EDGE::coedge(edge2) && (v52 = EDGE::coedge(edge1), v290 = COEDGE::loop(v52), v53 = EDGE::coedge(edge2), v54 = COEDGE::loop(v53), v290 == v54)) {
//                             v55 = EDGE::coedge(edge1);
//                             v56 = COEDGE::loop(v55);
//                             common_face = LOOP::face(v56);
//                             coed1 = EDGE::coedge(edge1);
//                             coed2 = EDGE::coedge(edge2);
//                         } else if(EDGE::coedge(edge1) && (v57 = EDGE::coedge(edge1), COEDGE::loop(v57)) && EDGE::coedge(edge2) && (v58 = EDGE::coedge(edge2), COEDGE::partner(v58)) &&
//                                   (v59 = EDGE::coedge(edge1), v291 = COEDGE::loop(v59), v60 = EDGE::coedge(edge2), v61 = COEDGE::partner(v60), v62 = COEDGE::loop(v61), v291 == v62)) {
//                             v63 = EDGE::coedge(edge1);
//                             v64 = COEDGE::loop(v63);
//                             common_face = LOOP::face(v64);
//                             coed1 = EDGE::coedge(edge1);
//                             v65 = EDGE::coedge(edge2);
//                             coed2 = COEDGE::partner(v65);
//                         } else {
//                             if(!EDGE::coedge(edge1)) goto LABEL_223;
//                             v66 = EDGE::coedge(edge1);
//                             if(!COEDGE::partner(v66)) goto LABEL_223;
//                             v67 = EDGE::coedge(edge1);
//                             v68 = COEDGE::partner(v67);
//                             if(COEDGE::loop(v68) && EDGE::coedge(edge2) && (v69 = EDGE::coedge(edge1), v70 = COEDGE::partner(v69), v292 = COEDGE::loop(v70), v71 = EDGE::coedge(edge2), v72 = COEDGE::loop(v71), v292 == v72)) {
//                                 v73 = EDGE::coedge(edge1);
//                                 v74 = COEDGE::partner(v73);
//                                 v75 = COEDGE::loop(v74);
//                                 common_face = LOOP::face(v75);
//                                 v76 = EDGE::coedge(edge1);
//                                 coed1 = COEDGE::partner(v76);
//                                 coed2 = EDGE::coedge(edge2);
//                             } else {
//                             LABEL_223:
//                                 if(EDGE::coedge(edge1)) {
//                                     v77 = EDGE::coedge(edge1);
//                                     if(COEDGE::partner(v77)) {
//                                         v78 = EDGE::coedge(edge1);
//                                         v79 = COEDGE::partner(v78);
//                                         if(COEDGE::loop(v79)) {
//                                             if(EDGE::coedge(edge2)) {
//                                                 v80 = EDGE::coedge(edge2);
//                                                 if(COEDGE::partner(v80)) {
//                                                     v81 = EDGE::coedge(edge1);
//                                                     v82 = COEDGE::partner(v81);
//                                                     v293 = COEDGE::loop(v82);
//                                                     v83 = EDGE::coedge(edge2);
//                                                     v84 = COEDGE::partner(v83);
//                                                     v85 = COEDGE::loop(v84);
//                                                     if(v293 == v85) {
//                                                         v86 = EDGE::coedge(edge1);
//                                                         v87 = COEDGE::partner(v86);
//                                                         v88 = COEDGE::loop(v87);
//                                                         common_face = LOOP::face(v88);
//                                                         v89 = EDGE::coedge(edge1);
//                                                         coed1 = COEDGE::partner(v89);
//                                                         v90 = EDGE::coedge(edge2);
//                                                         coed2 = COEDGE::partner(v90);
//                                                     }
//                                                 }
//                                             }
//                                         }
//                                     }
//                                 }
//                             }
//                         }
//                         if(common_face)
//                             v146 = FACE::geometry(common_face);
//                         else
//                             v146 = 0i64;
//                         common_SF = v146;
//                         if(v146 && coed1 && coed2 && COEDGE::geometry(coed1) && COEDGE::geometry(coed2)) {
//                             common_sf = common_SF->equation(common_SF);
//                             if(common_sf && !SpaAcis::NullObj::check_surface(common_sf)) {
//                                 get_face_box(&face_box, common_face, 0i64, 0, 0i64);
//                                 param_range = common_sf->param_range;
//                                 param_range((surface*)common_sf, &sf_range, &face_box);
//                                 v295 = COEDGE::geometry(coed1);
//                                 PCURVE::equation(v295, &pcu1, 0);
//                                 v138 = COEDGE::sense(coed1);
//                                 v91 = EDGE::sense(edge1);
//                                 v123 = v138 != v91;
//                                 negate_pcu1 = v123;
//                                 if(v138 != v91) pcurve::negate(&pcu1);
//                                 v296 = COEDGE::geometry(coed2);
//                                 PCURVE::equation(v296, &pcu2, 0);
//                                 v139 = COEDGE::sense(coed2);
//                                 v92 = EDGE::sense(edge2);
//                                 v124 = v139 != v92;
//                                 negate_pcu2 = v124;
//                                 if(v139 != v92) pcurve::negate(&pcu2);
//                                 v93 = common_SF->equation(common_SF);
//                                 ccrel_data = d3_cu_cu_on_sf_int(ed_cur1, &cur1_range, &pcu1, ed_cur2, &cur2_range, &pcu2, v93, &sf_range, dist_tol, 0);
//                                 if(negate_pcu1) pcurve::negate(&pcu1);
//                                 new_pcu = 0i64;
//                                 AcisVersion::AcisVersion(&v318, 21, 0, 0);
//                                 v297 = v94;
//                                 v298 = GET_ALGORITHMIC_VERSION(&v319);
//                                 if(operator<=(v298, v297)) {
//                                     v147 = (PCURVE*)ACIS_OBJECT::operator new(0x90ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 2835, &alloc_file_index_2127);
//                                     if(v147) {
//                                         PCURVE::PCURVE(v147, &pcu1);
//                                         v148 = v95;
//                                     } else {
//                                         v148 = 0i64;
//                                     }
//                                     v299 = v148;
//                                     new_pcu = v148;
//                                     set_geometry = coed1->set_geometry;
//                                     set_geometry(coed1, v148, 1);
//                                 }
//                                 if(negate_pcu2) pcurve::negate(&pcu2);
//                                 AcisVersion::AcisVersion(&v320, 21, 0, 0);
//                                 v301 = v96;
//                                 v302 = GET_ALGORITHMIC_VERSION(&v187);
//                                 if(operator<=(v302, v301)) {
//                                     v149 = (PCURVE*)ACIS_OBJECT::operator new(0x90ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 2846, &alloc_file_index_2127);
//                                     if(v149) {
//                                         PCURVE::PCURVE(v149, &pcu2);
//                                         v150 = v97;
//                                     } else {
//                                         v150 = 0i64;
//                                     }
//                                     v303 = v150;
//                                     new_pcu = v150;
//                                     v304 = coed2->set_geometry;
//                                     v304(coed2, v150, 1);
//                                 }
//                                 for(j = 0; j < 2; ++j) {
//                                     if(j)
//                                         v151 = coed2;
//                                     else
//                                         v151 = coed1;
//                                     coed = v151;
//                                     if(is_TCOEDGE(v151)) {
//                                         v305 = COEDGE::geometry(coed);
//                                         PCURVE::equation(v305, &temp_pcurve, 1);
//                                         pcurve::param_range(&temp_pcurve, &pcurve_range);
//                                         v306 = coed->param_range;
//                                         v306(coed, &coedge_range);
//                                         v126 = pcurve::param_period(&temp_pcurve) > 0.0;
//                                         pc_periodic = v126;
//                                         if(!v126 && !SPAinterval::operator>>(&pcurve_range, &coedge_range)) TCOEDGE::set_param_range((TCOEDGE*)coed, &pcurve_range);
//                                         pcurve::~pcurve(&temp_pcurve);
//                                     }
//                                 }
//                                 if(sg_inter_module_header.debug_level >= 0x1E) {
//                                     acis_fprintf(debug_file_ptr, "Curve-Curve-On-Surface Intersection: \n");
//                                     sg_debug_cci(ccrel_data, debug_file_ptr, "\t");
//                                     debug_newline(debug_file_ptr);
//                                 }
//                                 pcurve::~pcurve(&pcu2);
//                                 pcurve::~pcurve(&pcu1);
//                             }
//                         } else {
//                             ccrel_data = d3_cu_cu_int(ed_cur1, &cur1_range, ed_cur2, &cur2_range, dist_tol);
//                             if(sg_inter_module_header.debug_level >= 0x1E) {
//                                 acis_fprintf(debug_file_ptr, "General Curve-Curve Intersection: \n");
//                                 sg_debug_cci(ccrel_data, debug_file_ptr, "\t");
//                                 debug_newline(debug_file_ptr);
//                             }
//                         }
//                     }
//                 }
//             }
//         }
//     }
//     if(ed_cur1) {
//         *(_QWORD*)&t2[1] = ed_cur1;
//         v307 = ed_cur1->~curve;
//         v308 = ((__int64(__fastcall*)(curve*, __int64))v307)(ed_cur1, 1i64);
//     }
//     if(ed_cur2) {
//         *(_QWORD*)&t2[2] = ed_cur2;
//         v309 = ed_cur2->~curve;
//         v310 = ((__int64(__fastcall*)(curve*, __int64))v309)(ed_cur2, 1i64);
//     }
//     if(error_no) sg_delete_cci_list(ccrel_data);
//     exception_save::~exception_save(&exception_save_mark);
//     if(resignal_no || acis_interrupted()) sys_error(resignal_no, error_info_base_ptr);
//     acis_exception::~acis_exception(&error_info_holder);
//     return ccrel_data;
// }

// int sg_inter_ed_ed(EDGE* e1, EDGE* e2, curve_curve_int*& inters, double dist_tol, double angle_tol) {
//     const curve* v6;                                                                  // rax
//     const curve* v7;                                                                  // rax
//     double v8;                                                                        // xmm0_8
//     long double v9;                                                                   // xmm0_8
//     long double v10;                                                                  // xmm0_8
//     long double v11;                                                                  // xmm0_8
//     long double v12;                                                                  // xmm0_8
//     long double v13;                                                                  // xmm0_8
//     long double v14;                                                                  // xmm0_8
//     double v15;                                                                       // xmm0_8
//     long double v16;                                                                  // xmm0_8
//     long double v17;                                                                  // xmm0_8
//     long double v18;                                                                  // xmm0_8
//     long double v19;                                                                  // xmm0_8
//     long double v20;                                                                  // xmm0_8
//     long double v21;                                                                  // xmm0_8
//     const curve* v22;                                                                 // rax
//     const curve* v23;                                                                 // rax
//     curve_curve_int* this_int;                                                        // [rsp+30h] [rbp-2C8h]
//     curve_curve_int* next;                                                            // [rsp+38h] [rbp-2C0h]
//     double diff;                                                                      // [rsp+40h] [rbp-2B8h]
//     double curve_period;                                                              // [rsp+48h] [rbp-2B0h]
//     double X;                                                                         // [rsp+50h] [rbp-2A8h]
//     double v29;                                                                       // [rsp+58h] [rbp-2A0h]
//     unsigned int count;                                                               // [rsp+60h] [rbp-298h]
//     curve_curve_int* temp_int;                                                        // [rsp+78h] [rbp-280h]
//     breakpoint_callback* breakpoint_callback;                                         // [rsp+80h] [rbp-278h]
//     RenderingObject* v33;                                                             // [rsp+88h] [rbp-270h]
//     CURVE* v34;                                                                       // [rsp+90h] [rbp-268h]
//     __int64 v35;                                                                      // [rsp+98h] [rbp-260h]
//     CURVE* v36;                                                                       // [rsp+A0h] [rbp-258h]
//     __int64 v37;                                                                      // [rsp+A8h] [rbp-250h]
//     CURVE* v38;                                                                       // [rsp+B0h] [rbp-248h]
//     CURVE* v39;                                                                       // [rsp+B8h] [rbp-240h]
//     breakpoint_callback* v40;                                                         // [rsp+C0h] [rbp-238h]
//     CURVE* v41;                                                                       // [rsp+C8h] [rbp-230h]
//     __int64 v42;                                                                      // [rsp+D0h] [rbp-228h]
//     CURVE* v43;                                                                       // [rsp+D8h] [rbp-220h]
//     __int64 v44;                                                                      // [rsp+E0h] [rbp-218h]
//     CURVE* v45;                                                                       // [rsp+E8h] [rbp-210h]
//     __int64 v46;                                                                      // [rsp+F0h] [rbp-208h]
//     CURVE* v47;                                                                       // [rsp+F8h] [rbp-200h]
//     __int64 v48;                                                                      // [rsp+100h] [rbp-1F8h]
//     breakpoint_callback* v49;                                                         // [rsp+108h] [rbp-1F0h]
//     RenderingObject* v50;                                                             // [rsp+110h] [rbp-1E8h]
//     CURVE* v51;                                                                       // [rsp+118h] [rbp-1E0h]
//     CURVE* v52;                                                                       // [rsp+120h] [rbp-1D8h]
//     breakpoint_callback* v53;                                                         // [rsp+128h] [rbp-1D0h]
//     SPAinterval edge_range;                                                           // [rsp+130h] [rbp-1C8h] BYREF
//     SPAinterval i;                                                                    // [rsp+148h] [rbp-1B0h] BYREF
//     const char* v56;                                                                  // [rsp+160h] [rbp-198h]
//     _iobuf* fp;                                                                       // [rsp+168h] [rbp-190h]
//     void(__fastcall * delete_render_object)(breakpoint_callback*, RenderingObject*);  // [rsp+170h] [rbp-188h]
//     double v59;                                                                       // [rsp+178h] [rbp-180h]
//     double started;                                                                   // [rsp+180h] [rbp-178h]
//     double v61;                                                                       // [rsp+188h] [rbp-170h]
//     double v62;                                                                       // [rsp+190h] [rbp-168h]
//     double v63;                                                                       // [rsp+198h] [rbp-160h]
//     double v64;                                                                       // [rsp+1A0h] [rbp-158h]
//     double v65;                                                                       // [rsp+1A8h] [rbp-150h]
//     double v66;                                                                       // [rsp+1B0h] [rbp-148h]
//     double v67;                                                                       // [rsp+1B8h] [rbp-140h]
//     double v68;                                                                       // [rsp+1C0h] [rbp-138h]
//     double v69;                                                                       // [rsp+1C8h] [rbp-130h]
//     double v70;                                                                       // [rsp+1D0h] [rbp-128h]
//     double v71;                                                                       // [rsp+1D8h] [rbp-120h]
//     double v72;                                                                       // [rsp+1E0h] [rbp-118h]
//     RenderingObject*(__fastcall * v73)(breakpoint_callback*, int);                    // [rsp+1E8h] [rbp-110h]
//     void(__fastcall * v74)(breakpoint_callback*, RenderingObject*);                   // [rsp+1F0h] [rbp-108h]
//     RenderingObject*(__fastcall * new_render_object)(breakpoint_callback*, int);      // [rsp+1F8h] [rbp-100h]
//     const char* v76;                                                                  // [rsp+200h] [rbp-F8h]
//
//     if(!e1 || !e2 || !e1->geometry() || !e2->geometry()) return 1;
//     SPAbox b1 = get_edge_box(e1, 0, 0, 0);
//     SPAbox b2 = get_edge_box(e2, 0, 0, 0);
//     if(&b1 && &b2) {
//         *inters = eded_int(e1, e2, dist_tol, angle_tol);
//         if(sg_inter_module_header.debug_level >= 40) {
//             acis_fprintf(debug_file_ptr, "\tThe intersection points found are : \n");
//             sg_debug_cci(*inters, debug_file_ptr, "\t");
//             debug_newline(debug_file_ptr);
//         }
//         v41 = EDGE::geometry(e1);
//         v42 = (__int64)v41->equation(v41);
//         if((*(unsigned int(__fastcall**)(__int64))(*(_QWORD*)v42 + 312i64))(v42) && *inters) {
//             EDGE::param_range(e1, &edge_range);
//             if(EDGE::sense(e1) == 1) qmemcpy(&edge_range, operator-(&result, &edge_range), sizeof(edge_range));
//             v43 = EDGE::geometry(e1);
//             v44 = (__int64)v43->equation(v43);
//             curve_period = (*(double(__fastcall**)(__int64))(*(_QWORD*)v44 + 328i64))(v44);
//             this_int = *inters;
//             while(this_int) {
//                 if(this_int->high_rel == cur_cur_coin) {
//                     if(this_int->next) {
//                         for(diff = this_int->next->param1 - this_int->param1; COERCE_DOUBLE(*(_QWORD*)&curve_period ^ _xmm) > diff; diff = diff + curve_period) {
//                             ;
//                         }
//                         while(diff > curve_period) diff = diff - curve_period;
//                         v59 = fabs_0(diff);
//                         v8 = safe_function_type<double>::operator double(&SPAresnor);
//                         if(v8 > v59) diff = curve_period;
//                         while(1) {
//                             started = SPAinterval::start_pt(&edge_range);
//                             v9 = safe_function_type<double>::operator double(&SPAresnor);
//                             if(started - v9 <= this_int->param1) break;
//                             this_int->param1 = this_int->param1 + curve_period;
//                         }
//                         while(1) {
//                             v61 = SPAinterval::end_pt(&edge_range);
//                             v10 = safe_function_type<double>::operator double(&SPAresnor);
//                             if(this_int->param1 <= v61 + v10) break;
//                             this_int->param1 = this_int->param1 - curve_period;
//                         }
//                         if(COERCE_DOUBLE(COERCE_UNSIGNED_INT64(safe_function_type<double>::operator double(&SPAresnor)) ^ _xmm) <= diff) {
//                             if(diff > safe_function_type<double>::operator double(&SPAresnor)) {
//                                 while(1) {
//                                     v63 = SPAinterval::end_pt(&edge_range);
//                                     v12 = safe_function_type<double>::operator double(&SPAresnor);
//                                     if(this_int->param1 <= v63 - v12) break;
//                                     this_int->param1 = this_int->param1 - curve_period;
//                                 }
//                             }
//                         } else {
//                             while(1) {
//                                 v62 = SPAinterval::start_pt(&edge_range);
//                                 v11 = safe_function_type<double>::operator double(&SPAresnor);
//                                 if(v62 + v11 <= this_int->param1) break;
//                                 this_int->param1 = this_int->param1 + curve_period;
//                             }
//                         }
//                         this_int->next->param1 = this_int->param1 + diff;
//                         this_int = this_int->next->next;
//                     } else {
//                         this_int = this_int->next;
//                     }
//                 } else {
//                     while(1) {
//                         v64 = SPAinterval::start_pt(&edge_range);
//                         v13 = safe_function_type<double>::operator double(&SPAresnor);
//                         if(v64 - v13 <= this_int->param1) break;
//                         this_int->param1 = this_int->param1 + curve_period;
//                     }
//                     while(1) {
//                         v65 = SPAinterval::end_pt(&edge_range);
//                         v14 = safe_function_type<double>::operator double(&SPAresnor);
//                         if(this_int->param1 <= v65 + v14) break;
//                         this_int->param1 = this_int->param1 - curve_period;
//                     }
//                     this_int = this_int->next;
//                 }
//             }
//         }
//         v45 = EDGE::geometry(e2);
//         v46 = (__int64)v45->equation(v45);
//         if((*(unsigned int(__fastcall**)(__int64))(*(_QWORD*)v46 + 312i64))(v46) && *inters) {
//             EDGE::param_range(e2, &i);
//             if(EDGE::sense(e2) == 1) qmemcpy(&i, operator-(&v80, &i), sizeof(i));
//             v47 = EDGE::geometry(e2);
//             v48 = (__int64)v47->equation(v47);
//             v29 = (*(double(__fastcall**)(__int64))(*(_QWORD*)v48 + 328i64))(v48);
//             next = *inters;
//             while(next) {
//                 if(next->high_rel == cur_cur_coin) {
//                     if(next->next) {
//                         for(X = next->next->param2 - next->param2; COERCE_DOUBLE(*(_QWORD*)&v29 ^ _xmm) > X; X = X + v29)
//                             ;
//                         while(X > v29) X = X - v29;
//                         v66 = fabs_0(X);
//                         v15 = safe_function_type<double>::operator double(&SPAresnor);
//                         if(v15 > v66) X = v29;
//                         while(1) {
//                             v67 = SPAinterval::start_pt(&i);
//                             v16 = safe_function_type<double>::operator double(&SPAresnor);
//                             if(v67 - v16 <= next->param2) break;
//                             next->param2 = next->param2 + v29;
//                         }
//                         while(1) {
//                             v68 = SPAinterval::end_pt(&i);
//                             v17 = safe_function_type<double>::operator double(&SPAresnor);
//                             if(next->param2 <= v68 + v17) break;
//                             next->param2 = next->param2 - v29;
//                         }
//                         if(COERCE_DOUBLE(COERCE_UNSIGNED_INT64(safe_function_type<double>::operator double(&SPAresnor)) ^ _xmm) <= X) {
//                             if(X > safe_function_type<double>::operator double(&SPAresnor)) {
//                                 while(1) {
//                                     v70 = SPAinterval::end_pt(&i);
//                                     v19 = safe_function_type<double>::operator double(&SPAresnor);
//                                     if(next->param2 <= v70 - v19) break;
//                                     next->param2 = next->param2 - v29;
//                                 }
//                             }
//                         } else {
//                             while(1) {
//                                 v69 = SPAinterval::start_pt(&i);
//                                 v18 = safe_function_type<double>::operator double(&SPAresnor);
//                                 if(v69 + v18 <= next->param2) break;
//                                 next->param2 = next->param2 + v29;
//                             }
//                         }
//                         next->next->param2 = next->param2 + X;
//                         next = next->next->next;
//                     } else {
//                         next = next->next;
//                     }
//                 } else {
//                     while(1) {
//                         v71 = SPAinterval::start_pt(&i);
//                         v20 = safe_function_type<double>::operator double(&SPAresnor);
//                         if(v71 - v20 <= next->param2) break;
//                         next->param2 = next->param2 + v29;
//                     }
//                     while(1) {
//                         v72 = SPAinterval::end_pt(&i);
//                         v21 = safe_function_type<double>::operator double(&SPAresnor);
//                         if(next->param2 <= v72 + v21) break;
//                         next->param2 = next->param2 - v29;
//                     }
//                     next = next->next;
//                 }
//             }
//         }
//         if(sg_inter_module_header.debug_level >= 0x28) {
//             acis_fprintf(debug_file_ptr, "\tAfter range adjustment the intersection points are : \n");
//             sg_debug_cci(*inters, debug_file_ptr, "\t");
//             debug_newline(debug_file_ptr);
//         }
//     } else {
//         *inters = 0i64;
//     }
//     if(Debug_Break_Active("sg_inter_ed_ed", "Intr")) {
//         if(get_breakpoint_callback()) {
//             v49 = get_breakpoint_callback();
//             v73 = v49->new_render_object;
//             v50 = v73(v49, 1);
//         } else {
//             v50 = 0i64;
//         }
//         if(v50) {
//             v51 = EDGE::geometry(e1);
//             v22 = v51->equation(v51);
//             show_crv(v22, RED, 0i64, v50, &b1);
//             v52 = EDGE::geometry(e2);
//             v23 = v52->equation(v52);
//             show_crv(v23, ORANGE, 0i64, v50, &b2);
//             if(*inters) {
//                 count = 0;
//                 for(temp_int = *inters; temp_int; temp_int = temp_int->next) {
//                     show_position(&temp_int->int_point, v50);
//                     ++count;
//                 }
//                 acis_fprintf(debug_file_ptr, "%d intersections found\n", count);
//             } else {
//                 acis_fprintf(debug_file_ptr, "No intersections found\n");
//             }
//             Debug_Break("sg_inter_ed_ed", "Intr", "E:\\build\\acis\\NTSwin_b64_debug\\SPAintr\\intersct_sg_husk_inter.m\\src\\ededint.cpp", 606);
//             if(get_breakpoint_callback()) {
//                 v53 = get_breakpoint_callback();
//                 v74 = v53->delete_render_object;
//                 v74(v53, v50);
//             }
//         }
//     }
//     return 0i64;
// }

int get_connectionId_at_intersection(offset_segment* iSeg1, offset_segment* iSeg2, SPAposition& iIntsctPositon, int& ioConnectionId) {
    int connectionId = 0;
    if(iSeg1) {
        if(iSeg2) {
            COEDGE* c1 = iSeg1->coedge();
            COEDGE* c2 = iSeg2->coedge();
            if(c1) {
                if(c2) {
                    int atStart = 1;
                    const SPAposition& v4 = c1->start_pos();
                    if(v4 == iIntsctPositon) {
                        connectionId = iSeg1->get_start_connectionId();
                    } else {
                        const SPAposition& v5 = c1->end_pos();  // 不确定
                        if(v5 == iIntsctPositon) {
                            connectionId = iSeg1->get_end_connectionId();
                            atStart = 0;
                        }
                    }
                    const SPAposition& v6 = c2->start_pos();
                    if(operator==(v6, iIntsctPositon)) {
                        if(connectionId) {
                            int start_connectionId = iSeg2->get_start_connectionId();
                            if(abs(connectionId - start_connectionId) > 1) {
                                iSeg2->set_start_branch_connectionId(connectionId);
                                if(!atStart) {
                                    int v8 = iSeg2->get_start_connectionId();
                                    iSeg1->set_end_branch_connectionId(v8);
                                }
                            }
                        } else {
                            connectionId = iSeg2->get_start_connectionId();
                        }
                    } else {
                        const SPAposition& v9 = c2->end_pos();
                        if(v9 == iIntsctPositon) {
                            if(connectionId) {
                                int end_connectionId = iSeg2->get_end_connectionId();
                                if(abs(connectionId - end_connectionId) > 1) {
                                    iSeg2->set_end_branch_connectionId(connectionId);
                                    if(atStart) {
                                        iSeg1->set_start_branch_connectionId(iSeg2->get_end_connectionId());
                                    }
                                }
                            } else {
                                connectionId = iSeg2->get_end_connectionId();
                            }
                        }
                    }
                    if(!connectionId) return ++ioConnectionId;
                }
            }
        }
    }
    return connectionId;
}
int is_overlap(curve_curve_int* iIntersectionInfo) {
    int isOverLap = 0;
    if(iIntersectionInfo && iIntersectionInfo->next && !iIntersectionInfo->next->next && iIntersectionInfo->high_rel == cur_cur_coin && iIntersectionInfo->low_rel == cur_cur_coin && iIntersectionInfo->next->high_rel == cur_cur_coin) {
        return iIntersectionInfo->next->low_rel == cur_cur_coin;
    }
    return isOverLap;
}
int correct_seg_overlap(offset_segment_list* iSegList, offset_segment* ioSeg) {
    int correctionDone = 0;
    int connectId = -1;
    if(!ioSeg->start_out() || ioSeg->end_out()) {
        if(!ioSeg->start_out() && (unsigned int)ioSeg->end_out()) connectId = ioSeg->get_start_connectionId();
    } else {
        connectId = ioSeg->get_end_connectionId();
    }
    if(connectId > -1) {
        offset_segment* thisSeg = iSegList->first_segment();
        offset_segment* segment = iSegList->last_segment();
        offset_segment* lastSeg = segment->next();
        correctionDone = 1;
        while(thisSeg != lastSeg && correctionDone) {
            if(thisSeg->get_start_connectionId() == connectId) {
                correctionDone = thisSeg->end_out();
            } else if(thisSeg->get_end_connectionId() == connectId) {
                correctionDone = thisSeg->start_out();
            }
            thisSeg = thisSeg->next();
        }
        if(correctionDone) {
            ioSeg->set_start_out();
            ioSeg->set_end_out();
        }
    }
    return correctionDone;
}
int overlap_correction(offset_segment_list* iSegList, offset_segment* ioSeg1, offset_segment* ioSeg2) {
    int correctionDone = 0;
    if(ioSeg1->outside()) {
        return correct_seg_overlap(iSegList, ioSeg2);
    } else if(ioSeg2->outside()) {
        return correct_seg_overlap(iSegList, ioSeg1);
    }
    return correctionDone;
}
int valid_intersection(offset_segment* s1, offset_segment* s2, sg_seg_rating* rating) {
    if(rating->low_rating == seg_tan) return 0;
    if(!s1->original_coedge() || !s2->original_coedge()) return 1;
    curve_curve_int* cci = rating->cci;
    SPAinterval intv = s1->coedge()->param_range();
    double param = rating->cci->param1;
    SPAinterval orig_intv = s1->original_coedge()->param_range();
    double started = intv.start_pt();
    double v15 = fabs(param - started);
    if(SPAresabs <= v15 || (orig_intv.start_pt() - SPAresabs <= param)) {
        double v19 = fabs(param - intv.end_pt());
        if(SPAresabs <= v19 || param <= orig_intv.end_pt() + SPAresabs) {
            intv = s2->coedge()->param_range();
            COEDGE* v14 = s2->original_coedge();
            orig_intv = v14->param_range();
            double parama = rating->cci->param2;
            double v23 = fabs(parama - intv.start_pt());
            if(SPAresabs <= v23 || orig_intv.start_pt() - SPAresabs <= parama) {
                double v27 = fabs(parama - intv.end_pt());
                if(SPAresabs <= v27) return 1;
                double v28 = orig_intv.end_pt();
                if(parama <= v28 + SPAresabs) {
                    return 1;
                } else {
                    double param1 = cci->param1;
                    add_generic_named_attribute(s1->coedge(), "tee_intersection", param1, SplitCopy, MergeKeepKept, TransIgnore, CopyCopy);
                    return 0;
                }
            } else {
                add_generic_named_attribute(s1->coedge(), "tee_intersection", cci->param1, SplitCopy, MergeKeepKept, TransIgnore, CopyCopy);
                return 0;
            }
        } else {
            add_generic_named_attribute(s2->coedge(), "tee_intersection", cci->param2, SplitCopy, MergeKeepKept, TransIgnore, CopyCopy);
            return 0;
        }
    } else {
        add_generic_named_attribute(s2->coedge(), "tee_intersection", cci->param2, SplitCopy, MergeKeepKept, TransIgnore, CopyCopy);
        return 0;
    }
}
int isOverlap_arc(EDGE* iEdge1, EDGE* iEdge2) {
    int isOverlap = 0;
    if(is_circular_edge(iEdge1) && is_circular_edge(iEdge2)) {
        const curve* acrv1 = &iEdge1->geometry()->equation();
        const curve* acrv2 = &iEdge2->geometry()->equation();  // 不确定
        return acrv1[2].subset_range.end_pt() == acrv2[2].subset_range.end_pt() && (const SPAposition*)&acrv1[1] == (const SPAposition*)&acrv2[1];
    }
    return isOverlap;
}

int is_overlap_0(COEDGE* iCEdge1, COEDGE* iCEdge2, const sg_seg_rating* irating, SPAinterval& iParamRang1, SPAinterval& iParamRang2) {
    int isOverlap = 0;
    if(iCEdge1) {
        if(iCEdge2) {
            if(irating) {
                if(irating->cci) {
                    SPAposition intsctPos(irating->cci->int_point);
                    if(iParamRang1 >> irating->cci->param1) {
                        if(iParamRang2 >> irating->cci->param2) {
                            if(intsctPos != iCEdge1->start_pos() && intsctPos != iCEdge1->end_pos() || intsctPos != iCEdge2->start_pos() && intsctPos != iCEdge2->end_pos()) {
                                SPAposition p2 = iCEdge2->end_pos();
                                SPAposition p1 = iCEdge2->start_pos();
                                SPAvector v2 = p1 - p2;
                                SPAposition v18 = iCEdge1->end_pos();
                                SPAposition v19 = iCEdge1->start_pos();
                                SPAvector v1 = v19 - v18;
                                if(v1 % v2 > 0.0) {
                                    BOOL v15 = 0;
                                    if(is_STRAIGHT(iCEdge1->edge()->geometry())) {
                                        if(is_STRAIGHT(iCEdge2->edge()->geometry())) v15 = 1;
                                    }
                                    isOverlap = v15;
                                }
                                if(!isOverlap) {
                                    return isOverlap_arc(iCEdge1->edge(), iCEdge2->edge());
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return isOverlap;
}
int move_split_pos_to_center(curve_curve_int* ioIntersection1, curve_curve_int* ioIntersection2, EDGE* iEdge1, EDGE* iEdge2) {
    int moveDone = 0;
    if(ioIntersection1) {
        if(ioIntersection2) {
            if(iEdge1) {
                if(iEdge2) {
                    double midParam1 = 0.5 * (ioIntersection1->param1 + ioIntersection2->param1);
                    const curve& iCurve1 = iEdge1->geometry()->equation();
                    SPAposition midPosition1 = iCurve1.eval_position(midParam1);
                    const curve& iCurve12 = iEdge2->geometry()->equation();
                    SPAparameter guessParam(0.5 * (ioIntersection1->param2 + ioIntersection2->param2));
                    SPAposition midPosition2;
                    SPAparameter midParam2;
                    ((curve&)iCurve12).point_perp(midPosition1, midPosition2, guessParam, midParam2, 0);
                    if(midPosition1 == midPosition2) {
                        ioIntersection1->param1 = midParam1;
                        ioIntersection2->param1 = midParam1;
                        ioIntersection1->param2 = midParam2.operator double();
                        ioIntersection2->param2 = midParam2.operator double();
                        ioIntersection1->int_point = midPosition1;
                        ioIntersection2->int_point = midPosition2;
                        int moveDone = 1;
                        display_segment_intersection_points(iEdge1, ioIntersection1, iEdge2);
                    }
                }
            }
        }
    }
    return moveDone;
}

int sg_intersect_rate_segments(offset_segment_list* iSegList, offset_segment* s1, offset_segment* s2, law* dist_law, SPAunit_vector& normal, sg_gap_type use_type, int& ioConnectionId) {
    SPAbox b2 = s2->get_box();
    SPAbox b1 = s1->get_box();
    if(!(b1 && b2)) {
        if(trim_offset_wire_module_header.debug_level >= 40) acis_fprintf(debug_file_ptr, "sg_intersect_rate_segments() return: box\n");
        return 0;
    }
    COEDGE* c1;
    COEDGE* c2;
    EDGE* e1;
    EDGE* e2;
    c1 = s1->coedge();
    c2 = s2->coedge();
    e1 = c1->edge();
    e2 = c2->edge();
    RenderingObject* pRO = nullptr;
    int c1_sense = c1->sense() != e1->sense();
    int c2_sense = c2->sense() != e2->sense();
    int resignal_no = 0;
    acis_exception error_info_holder(0);
    error_info_base* error_info_base_ptr = nullptr;
    exception_save exception_save_mark;
    curve_curve_int* cci = nullptr;
    int* valid_intr = nullptr;
    int* index = nullptr;
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    int error_no = 0;
    sg_inter_ed_ed(e1, e2, cci, SPAresabs, SPAresnor);  // 求交
    if(!cci) {
        if(trim_offset_wire_module_header.debug_level >= 40) acis_fprintf(debug_file_ptr, "sg_intersect_rate_segments() return: no ints\n");
        return 0;
    }
    curve_curve_int* this_cci = cci;
    int cci_count = 0;
    while(this_cci)  //
    {
        ++cci_count;
        if(c1_sense == 1) this_cci->param1 = -this_cci->param1;
        if(c2_sense == 1) this_cci->param2 = -this_cci->param2;
        this_cci = this_cci->next;
    }
    sg_sort_cci(cci, 0, 0);
    if(trim_offset_wire_module_header.debug_level >= 40) acis_fprintf(debug_file_ptr, "sg_intersect_rate_segments() %d ints\n", cci_count);
    curve_curve_int* this_ccia = cci;
    curve_curve_int* next_cci = cci->next;
    if(c1->end() == c2->start()) {
        if(!next_cci) {
            if(trim_offset_wire_module_header.debug_level >= 40) acis_fprintf(debug_file_ptr, "sg_intersect_rate_segments() return: only junctions\n");

            if(cci) cci = nullptr;  // 不确定

            return 0;
        }
        while(next_cci->next) {
            this_ccia = next_cci;
            next_cci = next_cci->next;
        }
        if(next_cci) next_cci = nullptr;
        --cci_count;
        this_ccia->next = nullptr;
    }
    curve_curve_int* this_ccic;
    // curve_curve_int* this_ccic = cci;  // 保存当前节点
    // if(this_ccic != nullptr) {
    //     ACIS_DELETE this_ccic;  // 销毁当前节点对象
    // }
    if(c1->start() != c2->end() || (this_ccic = cci, cci = cci->next, ((this_ccic == nullptr) ? this_ccic = nullptr : NULL, --cci_count), cci)) {
        if(trim_offset_wire_module_header.debug_level >= 40) acis_fprintf(debug_file_ptr, "sg_intersect_rate_segments(): %d ints left\n");  // 不确定， 有v10
        if(!is_INTCURVE(e1->geometry())) goto LABEL_184;
        if(!is_INTCURVE(e2->geometry())) goto LABEL_184;
        if(trim_offset_wire_module_header.debug_level >= 40) acis_fprintf(debug_file_ptr, "sg_intersect_rate_segments(): refining ints\n");
        if(!cci) {
            if(trim_offset_wire_module_header.debug_level >= 40) acis_fprintf(debug_file_ptr, "sg_intersect_rate_segments() return: refined all out\n");
            return 0;
        } else {
        LABEL_184:
            if(trim_offset_wire_module_header.debug_level >= 30) sg_debug_cci(cci, debug_file_ptr, "cci:  ");
            sg_seg_rating* rating_list = ACIS_NEW sg_seg_rating[cci_count];
            curve_curve_int* this_ccib = cci;
            for(int i = 0; i < cci_count; ++i) {
                SPAunit_vector tan1 = coedge_param_dir(c1, this_ccib->param1);
                SPAunit_vector tan2 = coedge_param_dir(c2, this_ccib->param2);
                double dist = dist_law->eval(this_ccib->param1);
                /*              double dot = ((tan1 * tan2) % normal) * dist;*/
                double dot = (tan1 * tan2).len();
                // 关键部分，不理解
                if(dot <= SPAresabs) {
                    if(-SPAresabs <= dot) {
                        rating_list[i].low_rating = seg_tan;
                        rating_list[i].high_rating = seg_tan;
                    } else {
                        rating_list[i].low_rating = seg_out;
                        rating_list[i].high_rating = seg_in;
                    }
                } else {
                    rating_list[i].low_rating = seg_in;
                    rating_list[i].high_rating = seg_out;
                }
                rating_list[i].cci = this_ccib;
                rating_list[i].mIntersectionId = get_connectionId_at_intersection(s1, s2, this_ccib->int_point, ioConnectionId);  // 疑问：不理解连接ID时做什么的？
                if(trim_offset_wire_module_header.debug_level >= 40) {
                    acis_fprintf(debug_file_ptr, "sg_intersect_rate_segments(): set ratings %d: ", i);
                    if(rating_list[i].low_rating) {
                        if(rating_list[i].low_rating == seg_out) {
                            acis_fprintf(debug_file_ptr, "low out");
                        } else if(rating_list[i].low_rating == seg_tan) {
                            acis_fprintf(debug_file_ptr, "low tan");
                        }
                    } else {
                        acis_fprintf(debug_file_ptr, "low  in");
                    }
                    if(rating_list[i].high_rating) {
                        if(rating_list[i].high_rating == seg_out) {
                            acis_fprintf(debug_file_ptr, ", high out\n");
                        } else if(rating_list[i].high_rating == seg_tan) {
                            acis_fprintf(debug_file_ptr, ", high tan\n");
                        }
                    } else {
                        acis_fprintf(debug_file_ptr, ", high  in\n");
                    }
                }
                this_ccib = this_ccib->next;
            }
            int* index = ACIS_NEW int[cci_count];
            for(int ia = 0; ia < cci_count; ++ia) index[ia] = ia;
            for(int ib = 0; ib < cci_count - 1; ++ib) {
                for(int j = ib + 1; j < cci_count; ++j) {
                    int v50 = index[ib];
                    int v49 = index[j];
                    if(rating_list[v50].cci->param2 > rating_list[v49].cci->param2) {
                        index[ib] = v49;
                        index[j] = v50;
                    }
                }
            }
            if(trim_offset_wire_module_header.debug_level >= 30) {
                acis_fprintf(debug_file_ptr, "Rating list along coedge 1:\n");
                for(int ic = 0; ic < cci_count; ++ic) {
                    const char* v76;
                    const char* v77;
                    const char* v78;
                    const char* v79;
                    if(rating_list[ic].high_rating) {
                        if(rating_list[ic].high_rating == seg_out)
                            v76 = "outside";
                        else
                            v76 = "tangent";
                        v78 = v76;
                    } else {
                        v78 = "inside";
                    }
                    if(rating_list[ic].low_rating) {
                        if(rating_list[ic].low_rating == seg_out)
                            v77 = "outside";
                        else
                            v77 = "tangent";
                        v79 = v77;
                    } else {
                        v79 = "inside";
                    }
                    acis_fprintf(debug_file_ptr, "\t%s-%s\n", v79, v78);
                }
                acis_fprintf(debug_file_ptr, "Rating list along coedge 2:\n");
                for(int id = 0; id < cci_count; ++id) {
                    const char* v80;
                    const char* v82;
                    if(rating_list[index[id]].high_rating) {
                        if(rating_list[index[id]].high_rating == seg_out)
                            v80 = "inside";
                        else
                            v80 = "tangent";
                        v82 = v80;
                    } else {
                        v82 = "outside";
                    }
                    const char* v83;
                    const char* v87;
                    if(rating_list[index[id]].low_rating) {
                        if(rating_list[index[id]].low_rating == seg_out)
                            v87 = "inside";
                        else
                            v87 = "tangent";
                        v83 = v87;
                    } else {
                        v83 = "outside";
                    }
                    acis_fprintf(debug_file_ptr, "\t%s-%s\n", v83, v82);
                }
                acis_fprintf(debug_file_ptr, "\n");
            }
            offset_segment* this_seg = s1;
            if(cci_count) {
                valid_intr = ACIS_NEW int[cci_count];
            }

            SPAinterval paramRange1 = s1->coedge()->param_range();
            SPAinterval paramRange2 = s2->coedge()->param_range();
            if(is_overlap(cci)) {
                overlap_correction(iSegList, s1, s2);
            }
            for(int ie = 0; ie < cci_count; ++ie) {
                sg_seg_rating* crrRating = &rating_list[ie];
                int valid = valid_intersection(s1, s2, crrRating);
                valid_intr[ie] = valid;
                if(!valid_intr[ie] && cci_count == 2) {
                    COEDGE* iCEdge2 = s2->coedge();
                    COEDGE* iCEdge1 = s1->coedge();
                    int v23 = is_overlap_0(iCEdge1, iCEdge2, crrRating, paramRange1, paramRange2);
                    valid_intr[ie] = v23;
                    if(valid_intr[ie]) {
                        EDGE* iEdge2 = s2->coedge()->edge();
                        EDGE* iEdge1 = s1->coedge()->edge();
                        curve_curve_int* ioIntersection2 = rating_list[1].cci;
                        curve_curve_int* ioIntersection1 = rating_list->cci;
                        if(move_split_pos_to_center(ioIntersection1, ioIntersection2, iEdge1, iEdge2)) {
                            cci_count = 1;
                            rating_list[ie].mIntersectionId = get_connectionId_at_intersection(s1, s2, rating_list->cci->int_point, ioConnectionId);
                        }
                    }
                }
                if((use_type == arc || valid_intr[ie]) && this_seg->split_and_rate(rating_list[ie], 0)) {
                    if(trim_offset_wire_module_header.debug_level >= 40) acis_fprintf(debug_file_ptr, "sg_intersect_rate_segments(): split-and-rated, 1st\n");
                    this_seg = this_seg->next();
                }
            }
            this_seg = s2;
            for(int ig = 0; ig < cci_count; ++ig) {
                if((use_type == arc || valid_intr[ig]) && this_seg->split_and_rate(rating_list[index[ig]], 1)) {
                    if(trim_offset_wire_module_header.debug_level >= 40) acis_fprintf(debug_file_ptr, "sg_intersect_rate_segments(): split-and-rated, 2nd\n");
                    this_seg = this_seg->next();
                }
            }
            if(valid_intr) {
                ACIS_DELETE[] STD_CAST valid_intr;
            }
            sg_delete_cci(cci);
            ACIS_DELETE[] STD_CAST rating_list;
            ACIS_DELETE[] STD_CAST index;
            if(resignal_no || acis_interrupted()) sys_error(resignal_no, error_info_base_ptr);

            return 1;
        }
    }

    else {
        if(trim_offset_wire_module_header.debug_level >= 40) acis_fprintf(debug_file_ptr, "sg_intersect_rate_segments() return: culled all junctions\n");
        return 0;
    }
}
double find_max_curv_param(curve* cu, SPAinterval& search_int, SPAposition& max_curv_pos) {
    double param1 = search_int.start_pt();
    double param2 = search_int.end_pt();
    double curv1_sq = cu->eval_curvature(param1).len_sq();  // 不确param1
    double curv2_sq = cu->eval_curvature(param2).len_sq();  // 不确param1
    SPAposition pos1 = cu->eval_position(param1);           // 不确param1
    SPAposition pos2 = cu->eval_position(param2);           // 不确param1

    double param_mid = curv1_sq / (curv1_sq + curv2_sq) * param1 + curv2_sq / (curv1_sq + curv2_sq) * param2;
    double curv_mid_sq = cu->eval_curvature(param_mid).len_sq();

    while(param2 - param1 > SPAresnor) {
        if(curv2_sq <= curv1_sq) {
            double v29 = curv_mid_sq / (curv_mid_sq + curv2_sq) * param_mid + curv2_sq / (curv_mid_sq + curv2_sq) * param2;
            if(SPAresnor > v29 - param_mid || SPAresnor > param2 - v29) {
                v29 = 0.5 * (param_mid + param2);
            }
            double v31 = cu->eval_curvature(v29).len_sq();  // 不确定
            if(curv_mid_sq <= v31) {
                param1 = param_mid;
                curv1_sq = curv_mid_sq;
                pos1 = cu->eval_position(param1);
                param_mid = v29;
                curv_mid_sq = v31;
            } else {
                param2 = v29;
                curv2_sq = v31;
                pos2 = cu->eval_position(param2);
                param_mid = curv1_sq / (curv1_sq + curv2_sq) * param1 + curv2_sq / (curv1_sq + curv2_sq) * v29;
                curv_mid_sq = cu->eval_curvature(param_mid).len_sq();
            }
        } else {
            double param_new = curv1_sq / (curv1_sq + curv_mid_sq) * param1 + curv_mid_sq / (curv1_sq + curv_mid_sq) * param_mid;
            if(SPAresnor > param_mid - param_new || SPAresnor > param_new - param1) {
                param_new = 0.5 * (param1 + param_mid);
            }
            double curv_new_sq = cu->eval_curvature(param_new).len_sq();
            if(curv_mid_sq <= curv_new_sq) {
                param2 = param_mid;
                curv2_sq = curv_mid_sq;
                pos2 = cu->eval_position(param2);
                param_mid = param_new;
                curv_mid_sq = curv_new_sq;
            } else {
                param1 = param_new;
                curv1_sq = curv_new_sq;
                pos1 = cu->eval_position(param1);
                param_mid = curv1_sq / (curv1_sq + curv2_sq) * param_new + curv2_sq / (curv1_sq + curv2_sq) * param2;
                curv_mid_sq = cu->eval_curvature(param_mid).len_sq();
            }
        }
    }
    max_curv_pos = pos1;
    return param_mid;
}
void set_curve_to_coedge(COEDGE* ioCoedge, curve* iCurve) {
    if(ioCoedge && iCurve) {
        if(ioCoedge->edge()) {
            CURVE* curve = make_curve(*iCurve);
            ioCoedge->edge()->set_geometry(curve, 1);
        }
    }
}
void correct_param_0(EDGE* segEdge, double& sp, double& ep) {
    sp = segEdge->start_param().operator double();
    ep = segEdge->end_param().operator double();
    if(segEdge->sense() == 1) {
        int temp = sp;
        sp = -ep;
        ep = -temp;
    }
}
int is_first_intersect_contains_all(curve_curve_int* ioIntersections, double iHalfParamRange) {
    int containsAll = 0;
    if(ioIntersections) {
        curve_curve_int* crrIntersect = ioIntersections->next;
        containsAll = 1;
        while(crrIntersect && containsAll) {
            if(crrIntersect->param2 <= ioIntersections->param2)
                crrIntersect = crrIntersect->next;
            else
                containsAll = 0;
        }
        if(containsAll && iHalfParamRange > ioIntersections->param2 - ioIntersections->param1) return 0;
    }
    return containsAll;
}
void remove_intersections_containing_exactly_one_cusp(curve_curve_int*& intersectionsList, curve_curve_int* cuspsList, COEDGE* coed) {
    int which;
    curve_curve_int* next_inter;
    if(intersectionsList && cuspsList) {
        const curve_curve_int* itr = cuspsList;
        SPAdouble_array cusps(0, 2);
        while(itr) {
            cusps.Push(itr->param1);
            itr = itr->next;
        }
        curve_curve_int* inter = intersectionsList;
        for(inter = intersectionsList; inter; inter = next_inter) {
            next_inter = inter->next;
            which = 0;
            if(contains_exactly_one_cusp(inter, cusps, which)) intersectionsList = remove_intersection(intersectionsList, inter);
        }
    }
}
int contains_exactly_one_cusp(curve_curve_int* the_inter, SPAdouble_array& cusps, int& which) {
    int cont_count = 0;
    for(int ii = 0; ii < cusps.Size(); ++ii) {
        if(cci_contains_par(the_inter, cusps[ii])) {
            which = ii;
            ++cont_count;
        }
        if(cont_count > 1) break;
    }
    return cont_count == 1;
}
int cci_contains_par(curve_curve_int* the_inter, double par) {
    if(!the_inter) return NULL;
    double t_start = the_inter->param1;
    double t_end = the_inter->param2;
    if(t_start > t_end) {
        t_start = the_inter->param2;
        t_end = the_inter->param1;
    }
    return par >= t_start - double(SPAresnor) && t_end + double(SPAresnor) >= par;
}

offset_segment::offset_segment(COEDGE* c_offset, VERTEX* v_original, COEDGE* c_original, double iSParam, double iEParam) {
    this->bx = SPAbox();
    this->prev_segment = nullptr;
    this->next_segment = nullptr;
    this->end_is_out = 1;
    this->start_is_out = 1;
    this->seg_reversed = 0;
    if(v_original) {
        this->is_singular = 0;
    } else {
        this->is_singular = c_offset->edge()->geometry() == nullptr;
    }
    this->is_extension = 0;
    this->seg_coedge = c_offset;
    this->mStartConnectId = 0;
    this->mEndConnectId = 0;
    this->mStartBranchConnectId = 0;
    this->mEndBranchConnectId = 0;
    this->mUserFlag = 0;
    this->v_underlying = v_original;
    this->c_underlying = c_original;
    this->bx_set = 0;
    this->mMayHaveSelfIntersections = 0;
    this->mParams[0] = iSParam;
    this->mParams[1] = iEParam;
}
offset_segment::offset_segment(COEDGE* c_offset, COEDGE* c_original) {
    this->bx = SPAbox();
    this->prev_segment = nullptr;
    this->next_segment = nullptr;
    this->end_is_out = 1;
    this->start_is_out = 1;
    this->seg_reversed = 0;
    this->is_singular = c_offset->edge()->geometry() == nullptr;
    this->is_extension = 0;
    this->seg_coedge = c_offset;
    this->mStartConnectId = 0;
    this->mEndConnectId = 0;
    this->mStartBranchConnectId = 0;
    this->mEndBranchConnectId = 0;
    this->mUserFlag = 0;
    this->v_underlying = nullptr;
    this->c_underlying = c_original;
    this->bx_set = 0;
    this->mMayHaveSelfIntersections = 0;
    this->mParams[0] = 0.0;
    this->mParams[1] = 0.0;
}
offset_segment::offset_segment(COEDGE* c_offset, VERTEX* v_original) {
    this->bx = SPAbox();
    this->prev_segment = nullptr;
    this->next_segment = nullptr;
    this->end_is_out = 1;
    this->start_is_out = 1;
    this->seg_reversed = 0;
    this->is_singular = 0;
    this->is_extension = 0;
    this->seg_coedge = c_offset;
    this->mStartConnectId = 0;
    this->mEndConnectId = 0;
    this->mStartBranchConnectId = 0;
    this->mEndBranchConnectId = 0;
    this->mUserFlag = 0;
    this->v_underlying = v_original;
    this->c_underlying = nullptr;
    this->bx_set = 0;
    this->mMayHaveSelfIntersections = 0;
    this->mParams[0] = 0.0;
    this->mParams[1] = 0.0;
}
offset_segment::offset_segment() {
    this->bx = SPAbox();
    this->prev_segment = nullptr;
    this->next_segment = nullptr;
    this->end_is_out = 1;
    this->start_is_out = 1;
    this->seg_reversed = 0;
    this->is_singular = 0;
    this->is_extension = 0;
    this->seg_coedge = nullptr;
    this->mStartConnectId = 0;
    this->mEndConnectId = 0;
    this->mStartBranchConnectId = 0;
    this->mEndBranchConnectId = 0;
    this->mUserFlag = 0;
    this->v_underlying = nullptr;
    this->c_underlying = nullptr;
    this->bx_set = 0;
    this->mMayHaveSelfIntersections = 0;
    this->mParams[0] = 0.0;
    this->mParams[1] = 0.0;
}
offset_segment* offset_segment::next() {
    return this->next_segment;
}
offset_segment* offset_segment::previous() {
    return this->prev_segment;
}
COEDGE* offset_segment::coedge() {
    return this->seg_coedge;
}
int offset_segment::start_out() {
    return this->start_is_out;
}
int offset_segment::end_out() {
    return this->end_is_out;
}
int offset_segment::inside() {
    return !this->start_is_out && !this->end_is_out;
}
int offset_segment::outside() {
    return this->start_is_out && this->end_is_out;  // 不确定
}
int offset_segment::reversed() {
    return this->seg_reversed;
}
int offset_segment::singular_offset() {
    return this->is_singular;
}
int offset_segment::simple_offset() {
    BOOL v4 = 1;
    if(!this->singular_offset()) {
        if(is_INTCURVE(this->seg_coedge->edge()->geometry())) return 0;
    }
    return v4;
}
int offset_segment::extension() {
    return this->is_extension;
}
int offset_segment::vertex_offset() {
    return this->v_underlying != nullptr;
}
int offset_segment::coedge_offset() const {
    return this->c_underlying != nullptr;
}
VERTEX* offset_segment::original_vertex() const {
    return this->v_underlying;
}
COEDGE* offset_segment::original_coedge() const {
    return this->c_underlying;
}
SPAbox offset_segment::get_box() {
    if(!this->bx_set) {
        this->bx = get_edge_box(this->coedge()->edge());
        this->bx_set = 1;
    }
    SPAbox result = SPAbox(this->bx);
    return result;
}
SPAbox offset_segment::reset_box() {
    this->bx = get_edge_box(this->coedge()->edge());
    this->bx_set = 1;
    SPAbox result = SPAbox(this->bx);
    return result;
}
void offset_segment::set_start_out() {
    this->start_is_out = 1;
}
void offset_segment::set_start_in() {
    this->start_is_out = 0;
}
void offset_segment::set_end_out() {
    this->end_is_out = 1;
}
void offset_segment::set_end_in() {
    this->end_is_out = 0;
}
void offset_segment::set_start_connectionId(const int iConnectionId) {
    this->mStartConnectId = iConnectionId;
}
int offset_segment::get_start_connectionId() {
    return this->mStartConnectId;
}
void offset_segment::set_end_connectionId(const int iConnectionId) {
    this->mEndConnectId = iConnectionId;
}
int offset_segment::get_end_connectionId() {
    return this->mEndConnectId;
}
void offset_segment::set_start_branch_connectionId(const int iConnectionId) {
    this->mStartBranchConnectId = iConnectionId;
}
void offset_segment::set_end_branch_connectionId(const int iConnectionId) {
    this->mEndBranchConnectId = iConnectionId;
}
int offset_segment::get_end_branch_connectionId() {
    return this->mEndBranchConnectId;
}
void offset_segment::set_user_flag(const int ivalue) {
    this->mUserFlag = ivalue;
}
int offset_segment::get_user_flag() {
    return this->mUserFlag;
}
void offset_segment::set_reversed() {
    this->seg_reversed = 1;
}
void offset_segment::set_extension() {
    this->is_extension = 1;
}
void offset_segment::set_start(sg_seg_rating& rating, int is_second) {
    if(this->start_is_out && (rating.high_rating == seg_in && !is_second || rating.high_rating == seg_out && is_second)) {
        this->set_start_in();
    }
}
void offset_segment::set_end(sg_seg_rating& rating, int is_second) {
    if(this->end_is_out && (rating.low_rating == seg_in && !is_second || rating.low_rating == seg_out && is_second)) {
        this->set_end_in();
    }
}
void offset_segment::set_next(offset_segment* next) {
    this->next_segment = next;
}
void offset_segment::set_previous(offset_segment* prev) {
    this->prev_segment = prev;
}
void offset_segment::split_and_rate(curve_curve_int*& ioIntersections, law* iDistLaw, SPAunit_vector& iNormal, int& ioStartConnectionId, const double iTanTol, int iSingleSegment, int iMayHaveSelfIntersections) {
    if(ioIntersections) {
        acis_exception error_info_holder(0);
        exception_save exception_save_mark;
        exception_save_mark.begin();
        get_error_mark().buffer_init = 1;
        EDGE* segEdge = this->seg_coedge->edge();
        curve_curve_int* this_cci = ioIntersections;
        int c_sense = this->seg_coedge->sense() != segEdge->sense();
        while(this_cci) {
            if(c_sense == 1) {
                this_cci->param1 = -this_cci->param1;
                this_cci->param2 = -this_cci->param2;
            }
            this_cci = this_cci->next;
        }
        sg_sort_cci(ioIntersections, 0, 0);
        display_segment_intersection_points(segEdge, ioIntersections, nullptr);

        if(segEdge->start() != segEdge->end()) {
            goto LABEL_19;
        LABEL_19:
            if(trim_offset_wire_module_header.debug_level >= 30) {
                sg_debug_cci(ioIntersections, debug_file_ptr, "self-cci:  ");
            }
            sg_reorder_self_cci(ioIntersections);
            int cci_count = 0;
            for(curve_curve_int* this_ccic = ioIntersections; this_ccic; this_ccic = this_ccic->next) {
                ++cci_count;
            }
            sg_seg_rating* rating_list = ACIS_NEW sg_seg_rating[cci_count];
            curve_curve_int* this_ccid = ioIntersections;
            int ratingDone = 0;
            if(iSingleSegment) {
                if(cci_count == 1) {
                    ratingDone = rate_single_seg_case(this, this_ccid, rating_list);
                    if(ratingDone) {
                        rating_list->cci = this_ccid;
                        rating_list->mIntersectionId = ++ioStartConnectionId;
                    }
                }
            }
            if(!ratingDone) {
                for(int i = 0; i < cci_count; ++i) {
                    if(this_ccid->uv_set) {
                        if(this->seg_coedge->sense() == 1) {
                            rating_list[i].low_rating = seg_in;
                            rating_list[i].high_rating = seg_out;
                        } else {
                            rating_list[i].low_rating = seg_out;
                            rating_list[i].high_rating = seg_in;
                        }
                    } else {
                        double coedge_param = this_ccid->param1;
                        COEDGE* coedge = this->seg_coedge;
                        SPAunit_vector tan1 = coedge_param_dir(coedge, coedge_param);
                        COEDGE* seg_coedge = this->seg_coedge;
                        SPAunit_vector tan2 = coedge_param_dir(seg_coedge, this_ccid->param2);
                        long double dist = iDistLaw->eval(this_ccid->param1);
                        double dot = tan1 * tan2 % iNormal * dist;
                        if(GET_ALGORITHMIC_VERSION() >= AcisVersion(28, 0, 1)) {
                            if(c_sense == 1) dot = -dot;
                        } else if(this->seg_coedge->sense() == 1) {
                            dot = -dot;
                        }
                        if(dot <= iTanTol) {
                            if(DOUBLE(-iTanTol) <= dot) {
                                rating_list[i].low_rating = seg_tan;
                                rating_list[i].high_rating = seg_tan;
                            } else {
                                rating_list[i].low_rating = seg_out;
                                rating_list[i].high_rating = seg_in;
                            }
                        } else {
                            rating_list[i].low_rating = seg_in;
                            rating_list[i].high_rating = seg_out;
                        }
                    }
                    rating_list[i].cci = this_ccid;
                    rating_list[i].mIntersectionId = ++ioStartConnectionId;
                    this_ccid = this_ccid->next;
                }
            }
            int* index = ACIS_NEW int[cci_count];
            for(int ia = 0; ia < cci_count; ++ia) index[ia] = ia;
            for(int ib = 0; ib < cci_count - 1; ++ib) {
                for(int j = ib + 1; j < cci_count; ++j) {
                    if(rating_list[index[ib]].cci->param2 > rating_list[index[j]].cci->param2) {
                        index[ib] = index[j];
                        index[j] = index[ib];
                    }
                }
            }
            if(trim_offset_wire_module_header.debug_level >= 30) {
                acis_fprintf(debug_file_ptr, "Rating list along self-intersecting coedge:\n");
                int v30 = 0;
                int v31 = 0;
                while(v30 < cci_count || v31 < cci_count) {
                    curve_curve_int* cci;
                    if(v30 >= cci_count)
                        cci = rating_list[v30 - 1].cci;
                    else
                        cci = rating_list[v30].cci;
                    long double param1 = cci->param1;
                    curve_curve_int* v17;
                    if(v31 >= cci_count)
                        v17 = rating_list[index[v31 - 1]].cci;
                    else
                        v17 = rating_list[index[v31]].cci;
                    long double param2 = v17->param2;
                    if(param2 < param1 || v30 >= cci_count) {
                        const char* v50;
                        if(rating_list[index[v31]].high_rating) {
                            const char* v48;
                            if(rating_list[index[v31]].high_rating == seg_out)
                                v48 = "inside";
                            else
                                v48 = "tangent";
                            v50 = v48;
                        } else {
                            v50 = "outside";
                        }
                        if(rating_list[index[v31]].low_rating) {
                            const char* v49;
                            if(rating_list[index[v31]].low_rating == seg_out)
                                v49 = "inside";
                            else
                                v49 = "tangent";
                            acis_fprintf(debug_file_ptr, "\t%s-%s\n", v49, v50);
                        } else {
                            acis_fprintf(debug_file_ptr, "\t%s-%s\n", "outside", v50);
                        }
                        ++v31;
                    } else {
                        const char* v47;
                        if(rating_list[v30].high_rating) {
                            const char* v45;
                            if(rating_list[v30].high_rating == seg_out)
                                v45 = "outside";
                            else
                                v45 = "tangent";
                            v47 = v45;
                        } else {
                            v47 = "inside";
                        }
                        if(rating_list[v30].low_rating) {
                            const char* v46;
                            if(rating_list[v30].low_rating == seg_out)
                                v46 = "outside";
                            else
                                v46 = "tangent";
                            acis_fprintf(debug_file_ptr, "\t%s-%s\n", v46, v47);
                        } else {
                            acis_fprintf(debug_file_ptr, "\t%s-%s\n", "inside", v47);
                        }
                        ++v30;
                    }
                }
                acis_fprintf(debug_file_ptr, "\n");
            }
            offset_segment* this_seg = this;
            int i1 = 0;
            int i2 = 0;
            while(i1 < cci_count || i2 < cci_count) {
                curve_curve_int* v18;
                curve_curve_int* v19;
                if(i1 >= cci_count)
                    v18 = rating_list[i1 - 1].cci;
                else
                    v18 = rating_list[i1].cci;
                double v75 = v18->param1;
                if(i2 >= cci_count)
                    v19 = rating_list[index[i2 - 1]].cci;
                else
                    v19 = rating_list[index[i2]].cci;
                double v74 = v19->param2;
                if(v74 < v75 || i1 >= cci_count) {
                    if(this_seg->split_and_rate(rating_list[index[i2]], 1)) {
                        this_seg->mMayHaveSelfIntersections = iMayHaveSelfIntersections;  // 不确定
                        this_seg = this_seg->next();
                        if(this_seg) {
                            this_seg->mMayHaveSelfIntersections = iMayHaveSelfIntersections;
                        }
                    }
                    ++i2;
                } else {
                    if(this_seg->split_and_rate(rating_list[i1], 0)) {
                        this_seg->mMayHaveSelfIntersections = iMayHaveSelfIntersections;
                        this_seg = this_seg->next();
                        if(this_seg) this_seg->mMayHaveSelfIntersections = iMayHaveSelfIntersections;
                    }
                    ++i1;
                }
            }
            if(rating_list) {
                ACIS_DELETE[] STD_CAST rating_list;
            }
            if(index) {
                ACIS_DELETE[] STD_CAST index;
            }
            if(acis_interrupted()) sys_error(0, (error_info_base*)0);  // 不確
        }
    }
    // curve_curve_int* this_ccia = ioIntersections;
    // for(curve_curve_int* next_cci = ioIntersections; next_cci->next; next_cci = next_cci->next)
    //{
    //     this_ccia = next_cci;
    // }
    // curve_curve_int* this_ccib = ioIntersections;
    // ioIntersections = ioIntersections->next;
    // if(this_ccib) {
    //     ACIS_DELETE this_ccib;
    // }
}

int offset_segment::split_and_rate(sg_seg_rating& rating, int is_second) {
    EDGE* seg_edge = this->seg_coedge->edge();
    SPAparameter started = seg_edge->start_param();
    double start_param = started.operator double();
    double end_param = seg_edge->end_param().operator double();
    if(this->seg_coedge->sense() == 1) {
        double tmp_param = start_param;
        start_param = -end_param;
        end_param = -tmp_param;
    }
    curve_curve_int* cci = rating.cci;
    double param2;
    if(is_second)
        param2 = cci->param2;
    else
        param2 = cci->param1;
    int did_split = 0;
    const curve& edgeCurve = seg_edge->geometry()->equation();
    SPAposition iEndPos = this->seg_coedge->end_pos();
    SPAposition iStartPos = this->seg_coedge->start_pos();  // 不懂，為什麼ida裡是char[40]

    int split_position = check_split_position(&((curve&)edgeCurve), param2, start_param, end_param, cci->int_point, iStartPos, iEndPos);
    if(split_position) {
        if(split_position == 2) {
            this->set_end(rating, is_second);
        } else if(this->split(param2)) {
            offset_segment* new_seg;
            new_seg = this->next();
            this->set_end(rating, is_second);
            new_seg->set_start(rating, is_second);
            if(rating.mIntersectionId) {
                new_seg->set_start_connectionId(rating.mIntersectionId);
                new_seg->set_end_connectionId(this->mEndConnectId);
                this->mEndConnectId = rating.mIntersectionId;
            }
            return 1;
        }
    } else {
        this->set_start(rating, is_second);
    }
    return did_split;
}
int offset_segment::split(double split_param) {
    double split_parama = split_param;
    SPAinterval prange = this->seg_coedge->edge()->param_range();
    double v35;
    if(this->seg_coedge->sense() == 1)
        v35 = -split_parama;
    else
        v35 = split_parama;
    if(!(prange >> v35)) return 0;
    COEDGE* coedge = this->seg_coedge;
    SPAposition int_point = coedge_param_pos(coedge, split_parama);
    acis_exception error_info_holder(0);
    error_info_base* error_info_base_ptr = nullptr;
    exception_save exception_save_mark;
    ENTITY_LIST coedge_list;
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    split_coedge_at_params(this->seg_coedge, 1, &split_parama, coedge_list, 0);
    SPAinterval range = SPAinterval(prange.start_pt(), split_parama);
    if(this->seg_coedge->sense() == 1) {
        SPAinterval v92 = SPAinterval(prange.start_pt(), -split_parama);
        range = v92;
    }
    this->seg_coedge->edge()->set_param_range(&range);
    COEDGE* second_part;
    if(this->seg_coedge->sense() == 1) {
        second_part = this->seg_coedge;
        this->seg_coedge = (COEDGE*)coedge_list[0];
    } else {
        second_part = (COEDGE*)coedge_list[0];
    }
    if(acis_interrupted()) sys_error(0, error_info_base_ptr);
    COEDGE* coedges[2];
    coedges[0] = this->seg_coedge;
    coedges[1] = second_part;
    if(!this->simple_offset()) {
        EDGE* edges[2];
        edges[0] = coedges[0]->edge();
        edges[1] = coedges[1]->edge();
        for(int i = 0; i < 2; ++i) {
            int same_as_curve = coedges[i]->sense() == edges[i]->sense();
            double v37;
            if(coedges[i]->sense() == edges[i]->sense())
                v37 = split_parama;
            else
                v37 = -split_parama;
            double crv_param = v37;
            acis_exception v89 = acis_exception(0);
            error_info_base* e_info = nullptr;
            exception_save v82 = exception_save();
            /*option_header split_allow_slivers();*/
            if(GET_ALGORITHMIC_VERSION() >= AcisVersion(20, 0, 0)) {
                option_header* allow_slivers = find_option("split_allow_slivers");
                allow_slivers->push(1);
            }
            v82.begin();
            get_error_mark().buffer_init = 1;
            intcurve* right = (intcurve*)(edges[i]->geometry()->equation().make_copy());
            display_segment_split_status(edges[0], edges[1], right, &int_point);
            double start_param;
            if(right->periodic()) {
                start_param = right->param_range().start_pt();
                SPAposition start_pos = right->eval_position(start_param);

                // right->split(start_param)
                // left = right->split(start_param,start_pos);  // 不懂
                /*  v26 = (__int64(__fastcall***)(_QWORD, __int64))((__int64(__fastcall*)(intcurve*, __int64, SPAposition*))split)(right, v10, &start_pos);*/
                /*  if(v26)
                  {
                      v59 = **v26;
                      v60 = v59(v26, 1i64);
                  }
                  else
                  {
                      v60 = 0i64;
                  }*/
            }
            intcurve* left = (intcurve*)right->split(crv_param, int_point);
            if(left && (left->type() != 11 || &(left->get_int_cur())) && right && (right->type() != 11 || &(right->get_int_cur())))  // 不確
            {
                int keep_left = same_as_curve == (i == 0);
                CURVE* curve;
                if(same_as_curve == (i == 0))
                    curve = make_curve(*left);
                else
                    curve = make_curve(*right);
                edges[i]->set_geometry(curve, 1);
            } else {
                ofst_error(ZERO_FIT_INTCURVE, 1, this->original_coedge()->edge());
            }
            if(GET_ALGORITHMIC_VERSION() >= AcisVersion(20, 0, 0)) {
                option_header* allow_slivers2 = find_option("split_allow_slivers");
                allow_slivers2->pop();
            }
            if(acis_interrupted()) sys_error(0, e_info);
        }
    }
    COEDGE* seg_coedge = this->seg_coedge;
    seg_coedge->set_partner(nullptr, 1);
    second_part->set_partner(nullptr, 1);
    offset_segment* v34 = ACIS_NEW offset_segment(second_part, this->v_underlying, this->c_underlying, this->mParams[0], this->mParams[1]);
    offset_segment* new_seg = v34;
    if(this->extension()) new_seg->set_extension();
    //
    new_seg->set_next(this->next());
    new_seg->set_previous(this);
    this->set_next(new_seg);
    new_seg->next()->set_previous(new_seg);
    // 设置新edge的区间
    SPAinterval new_edge_range(split_parama, prange.end_pt());
    if(new_seg->seg_coedge->sense() == 1) {
        SPAinterval v94(prange.start_pt(), -split_parama);
        new_edge_range = v94;
    }
    new_seg->seg_coedge->edge()->set_param_range(&new_edge_range);
    new_seg->end_is_out = this->end_is_out;
    new_seg->set_start_out();
    this->set_end_out();
    new_seg->set_end_connectionId(this->mEndConnectId);
    SPAbox v95 = this->reset_box();
    SPAbox v96 = new_seg->reset_box();
    return 1;
}
int offset_segment::split_at_cusps(law* dist_law) {
    if(!this->coedge_offset() || this->singular_offset()) return 1;
    acis_exception error_info_holder(0);
    error_info_base* error_info_base_ptr = nullptr;
    exception_save exception_save_mark;
    curve_curve_int* cusps = nullptr;
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    SPAunit_vector uvec;
    this->comp_cusp_points(cusps, dist_law, uvec, 1);
    cusps = sort_intersections(cusps);
    EDGE* iOfstEdge2 = this->c_underlying->edge();
    curve_curve_int* iIntersections = cusps;
    EDGE* iOfstEdge = this->coedge()->edge();
    display_segment_intersection_points(iOfstEdge, iIntersections, iOfstEdge2);
    if(!this->coedge()->sense()) cusps = reverse_intersections(cusps);
    for(curve_curve_int* cusp = cusps; cusp; cusp = cusp->next) {
        double split_param = cusp->param1;
        if(this->coedge()->sense() == 1) split_param = -split_param;
        this->split(split_param);
    }
    sg_delete_cci(cusps);
    if(acis_interrupted()) sys_error(0, error_info_base_ptr);
    return 1;
}
int offset_segment::remove_sliver_segment() {
    if((unsigned int)this->singular_offset()) return NULL;
    int remove = 0;
    COEDGE* v2 = this->coedge();
    double ed_len = this->coedge()->edge()->length(1);
    if(SPAresfit > ed_len) {
        offset_segment* prev_seg = this->previous();
        offset_segment* next_seg = this->next();
        VERTEX* vert1 = prev_seg->coedge()->end();
        VERTEX* vert2 = next_seg->coedge()->start();
        SPAposition p2 = vert1->geometry()->coords();
        SPAposition p1 = vert2->geometry()->coords();
        const SPAvector v7 = (p1 - p2);
        SPAvector v = 0.5 * v7;
        SPAposition p = vert1->geometry()->coords();
        SPAposition avg_pos = p + v;
        TVERTEX* new_vert = nullptr;
        replace_vertex_with_tvertex(vert1, new_vert);
        APOINT* point = ACIS_NEW APOINT(avg_pos);
        new_vert->set_geometry(point, 1);

        if(next_seg->coedge()->sense()) {
            next_seg->coedge()->edge()->set_end(new_vert, 1, 1);
        } else {
            next_seg->coedge()->edge()->set_start(new_vert, 1, 1);
        }
        COEDGE* next = next_seg->coedge();
        prev_seg->coedge()->set_next(next, 0, 1);
        next_seg->coedge()->set_previous(prev_seg->coedge(), 0, 1);
        this->coedge()->edge()->lose();
        this->coedge()->lose();
        vert2->lose();
        return 1;
    }
    return remove;
}
int offset_segment::do_self_intersect_filter_bad_intersections(law* dist_law, SPAunit_vector& normal, int iSingleSegment, int& ioStartConnectionId) {
    if(!dist_law || !this->c_underlying || !this->seg_coedge || this->simple_offset()) return NULL;
    int retOk = 1;
    curve_curve_int* mainList = nullptr;
    int resignal_no = 0;
    acis_exception error_info_holder(0);
    error_info_base* error_info_base_ptr = nullptr;
    exception_save exception_save_mark;
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    int error_no = 0;
    EDGE* segEdge = this->seg_coedge->edge();
    /* if(!segEdge) _wassert(L"segEdge", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\trmofwi.cpp", 0xA6Du);*/
    EDGE* baseEdge = this->c_underlying->edge();
    SPAinterval edgeParamRange;
    SPAtransf iTransf;

    curve* offsetCurve = get_curve_from_coedge(this->seg_coedge, edgeParamRange, iTransf);
    int curveClosed = segEdge->start() == segEdge->end();

    curve_curve_int* intersectionsList = get_curve_self_ints(*((intcurve*)offsetCurve), SPAresabs);  // 不确
    curve_curve_int* cuspsList = nullptr;
    this->comp_cusp_points(cuspsList, dist_law, normal, 0);
    int mayHaveSelfIntersections = 0;
    if(intersectionsList) {
        display_segment_intersection_points(segEdge, intersectionsList, baseEdge);
        display_segment_intersection_points(segEdge, cuspsList, baseEdge);
        ofst_bad_intersections_remover badIntsctsRemover;
        double started = edgeParamRange.start_pt();
        double parameRange = edgeParamRange.end_pt() - started;
        curve_curve_int* resultList = nullptr;
        mayHaveSelfIntersections = badIntsctsRemover.process_intersections(intersectionsList, cuspsList, resultList, edgeParamRange.end_pt() - started);
        if(resultList) {
            display_segment_intersection_points(segEdge, resultList, baseEdge);
            add_intersectins(mainList, resultList);
            sg_delete_cci(resultList);
            resultList = 0i64;
        } else {
            add_intersectins(mainList, intersectionsList);
            add_intersectins(mainList, cuspsList);
        }

    } else if(cuspsList) {
        display_segment_intersection_points(segEdge, cuspsList, baseEdge);
        add_intersectins(mainList, cuspsList);
    }
    if(curveClosed) {
        SPAposition pt;
        SPAvector dpt;
        SPAvector ddpt;
        offsetCurve->eval(double(segEdge->end_param()), pt, dpt, ddpt);
        curve_curve_int* v32 = ACIS_NEW curve_curve_int(mainList, pt, double(segEdge->start_param()), double(segEdge->start_param()));
        mainList = v32;
        curve_curve_int* v34 = ACIS_NEW curve_curve_int(mainList, pt, double(segEdge->end_param()), double(segEdge->end_param()));
        mainList = v34;
    }
    if(intersectionsList) {
        sg_delete_cci(intersectionsList);
        intersectionsList = nullptr;
    }
    if(cuspsList) {
        sg_delete_cci(cuspsList);
        cuspsList = nullptr;
    }
    if(retOk && mainList) this->split_and_rate(mainList, dist_law, normal, ioStartConnectionId, SPAresabs, iSingleSegment, mayHaveSelfIntersections);
    if(acis_interrupted()) sys_error(resignal_no, error_info_base_ptr);
    if(mainList) {
        sg_delete_cci(mainList);
        mainList = nullptr;
    }
    return retOk;
}
int offset_segment::do_self_intersect_new(curve* ofstCurve, law* dist_law, SPAunit_vector& normal, int iSingleSegment, int& ioStartConnectionId, int iSplitAtCusps, int iRemoveContainments) {
    if(!ofstCurve || !dist_law || !this->c_underlying || this->simple_offset()) return 0;
    int retOk = 1;
    curve_curve_int* mainList = nullptr;
    int resignal_no = 0;
    acis_exception error_info_holder(0);
    error_info_base* error_info_base_ptr = nullptr;
    exception_save exception_save_mark;
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    int error_no = 0;
    double tanTol = SPAresabs;
    /* if(!this->seg_coedge) _wassert(L"seg_coedge", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\trmofwi.cpp", 0xAE0u);*/
    EDGE* segEdge = this->seg_coedge->edge();
    /* if(!segEdge) _wassert(L"segEdge", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\trmofwi.cpp", 0xAE2u);*/
    double v26;
    double ep;
    correct_param_0(segEdge, v26, ep);
    EDGE* baseEdge = nullptr;
    if(this->c_underlying) baseEdge = this->c_underlying->edge();
    int curveClosed = segEdge->start() == segEdge->end();
    /*  tol = safe_function_type<double>::operator double(&SPAresabs);*/
    curve_curve_int* intersectionsList = get_curve_self_ints(*(intcurve*)ofstCurve, SPAresabs);  // 不确
    if(intersectionsList) {
        BOOL v21 = has_high_curvature(intersectionsList, iSingleSegment) == 0;
        retOk = v21;
    }
    ofst_intersection_containment_remover cRemover;
    int remContainments = 0;
    if(intersectionsList) {
        display_segment_intersection_points(segEdge, intersectionsList, baseEdge);
        if(!retOk && iRemoveContainments) {
            if(!iSingleSegment) {
                double halfParamRange = 0.5 * (ep - v26);
                cRemover.set_curve_info(halfParamRange, curveClosed, 0);
                cRemover.remove_contained_intersections(intersectionsList);
                remContainments = 1;
                if(!intersectionsList->next) tanTol = 0.00001;
            }
            retOk = 1;
        }
        if(retOk) {
            add_intersectins(mainList, intersectionsList);
            if(curveClosed) {
                SPAposition pt;
                SPAvector dpt;
                SPAvector ddpt;
                ofstCurve->eval(v26, pt, dpt, ddpt);  // 不确
                curve_curve_int* v30 = ACIS_NEW curve_curve_int(mainList, pt, v26, v26);
                mainList = v30;
                curve_curve_int* v32 = ACIS_NEW curve_curve_int(mainList, pt, ep, ep);
                mainList = v32;
            }
        }
    }
    int mayHaveSelfIntersections = 0;
    curve_curve_int* cuspsList = nullptr;
    if(retOk) {
        if(iSplitAtCusps) {
            this->comp_cusp_points(cuspsList, dist_law, normal, 0);
            if(cuspsList) {
                display_segment_intersection_points(segEdge, cuspsList, baseEdge);
                cRemover.remove_contained_cusps(intersectionsList, cuspsList, remContainments);
                add_intersectins(mainList, cuspsList);
                if(!intersectionsList) {
                    if(cuspsList) mayHaveSelfIntersections = 1;
                }
            }
        }
    }
    if(intersectionsList) {
        sg_delete_cci(intersectionsList);
        intersectionsList = nullptr;
    }
    if(cuspsList) {
        sg_delete_cci(cuspsList);
        cuspsList = nullptr;
    }
    if(retOk && mainList) this->split_and_rate(mainList, dist_law, normal, ioStartConnectionId, tanTol, iSingleSegment, mayHaveSelfIntersections);

    if(error_no) retOk = 0;

    if(acis_interrupted()) sys_error(resignal_no, error_info_base_ptr);

    if(mainList) {
        sg_delete_cci(mainList);
        mainList = nullptr;
    }
    return retOk;
}
void offset_segment::do_self_intersect(law* dist_law, SPAunit_vector& normal, int& ioStartConnectionId) {
    if(!this->simple_offset()) {
        int resignal_no = 0;
        acis_exception error_info_holder(0);
        error_info_base* error_info_base_ptr = nullptr;
        exception_save exception_save_mark;
        curve_curve_int* list = nullptr;
        int* index = nullptr;
        exception_save_mark.begin();
        get_error_mark().buffer_init = 1;
        int error_no = 0;
        EDGE* seg_edge = this->seg_coedge->edge();

        if(this->c_underlying) {
            curve* offcrv = seg_edge->geometry()->trans_curve();
            curve* given_curve = this->c_underlying->edge()->geometry()->trans_curve();
            curve_law_data* v385 = ACIS_NEW curve_law_data(*given_curve);
            curve_law_data* old_cld = v385;
            curvature_law* v235 = ACIS_NEW curvature_law(old_cld);
            law* cur_law = v235;
            old_cld->remove();
            constant_law* v238 = ACIS_NEW constant_law(1.0);
            law* const_one = v238;
            abs_law* v240 = ACIS_NEW abs_law(dist_law);
            law* fabs_dist = v240;
            times_law* v242 = ACIS_NEW times_law(fabs_dist, cur_law);
            law* cons_law = v242;
            curve_law_data* v244 = ACIS_NEW curve_law_data(*offcrv);
            curve_law_data* cld = v244;
            intersect_lawi* v246 = ACIS_NEW intersect_lawi(cld);
            law* sect_law = v246;
            law* v651[5];
            v651[2] = v246;
            cld->remove();
            identity_law* v249 = ACIS_NEW identity_law(0, 88);
            v651[3] = v249;
            law* sub1 = v249;

            identity_law* v251 = ACIS_NEW identity_law(1, 88);
            v651[4] = v251;
            law* sub2 = v251;

            constant_law* v253 = ACIS_NEW constant_law(SPAresabs);
            law* sub3 = v253;

            plus_law* v255 = ACIS_NEW plus_law(sub2, sub3);
            law* sub4 = v255;

            minus_law* v257 = ACIS_NEW minus_law(sub1, sub4);
            law* domain_law = v257;
            sub1->remove();
            sub2->remove();
            sub3->remove();
            sub4->remove();
            int wsize = 0;
            double maxx = seg_edge->end_param().operator double();
            double minx = seg_edge->start_param().operator double();
            int psize;
            double* cpoints_temp = Nsolve(const_one, cons_law, minx, maxx, &psize);
            const_one->remove();
            cons_law->remove();
            int delete_cpoints_temp = 0;
            SPAposition pt;
            SPAposition pt2;
            SPAposition upt;
            SPAvector dpt;
            SPAvector ddpt;
            SPAvector udpt;
            SPAvector uddpt;
            offcrv->eval(seg_edge->end_param().operator double(), upt);
            offcrv->eval(seg_edge->start_param().operator double(), pt);

            int closed_curve = 0;
            double* cpoints = nullptr;
            if(pt == upt) {
                closed_curve = 1;
                cpoints = ACIS_NEW double[psize + 1];  // 不确定
                /*v416 = (long double*)operator new[](v25, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\trmofwi.cpp", 3359, &alloc_file_index_3519);
                cpoints = v416;*/
                for(int k = 0; k < psize; ++k) cpoints[k] = cpoints_temp[k];
                double rc_end = cur_law->eval(seg_edge->end_param().operator double() - SPAresabs / 100.0);
                double rc_start = cur_law->eval(seg_edge->start_param().operator double());
                double dist = fabs_dist->eval(seg_edge->end_param().operator double());

                if(dist > 0.0) {
                    double rc_test = 1.0 / dist;
                    if(rc_end > rc_start && rc_test >= rc_start && rc_end > rc_test) {
                        cpoints[psize++] = seg_edge->end_param().operator double();
                    }
                    if(rc_start > rc_end && rc_test >= rc_end && rc_start > rc_test) {
                        cpoints[psize++] = seg_edge->start_param().operator double();
                    }
                }
                delete_cpoints_temp = 1;
            } else {
                cpoints = cpoints_temp;
            }
            fabs_dist->remove();
            law* dcur_law = cur_law->derivative(0);
            cur_law->remove();
            double* wpoints = nullptr;
            int err_num = 0;
            acis_exception v701(0);
            error_info_base* e_info = nullptr;
            exception_save v639;
            double old_tol = root_tolerances->ROOT_TOL_X;
            v639.begin();
            get_error_mark().buffer_init = 1;
            root_tolerances->ROOT_TOL_X = 1.0e-7;
            wpoints = Nroot(dcur_law, seg_edge->start_param().operator double(), seg_edge->end_param().operator double(), &wsize);
            dcur_law->remove();
            root_tolerances->ROOT_TOL_X = old_tol;
            if(err_num || acis_interrupted()) sys_error(err_num, e_info);
            double* tpoints = ACIS_NEW double[wsize + psize];
            double* domain_center = ACIS_NEW double[wsize + 2 * psize + 2];
            int dom_size = 0;
            int tp_count;
            for(tp_count = 0; tp_count < wsize; ++tp_count) {
                tpoints[tp_count] = wpoints[tp_count];
                domain_center[dom_size++] = wpoints[tp_count];
            }
            void* alloc_ptr = wpoints;
            ACIS_DELETE[] STD_CAST wpoints;

            for(int m = 0; m < psize; ++m) {
                offcrv->eval(cpoints[m], pt, dpt, ddpt);
                given_curve->eval(cpoints[m], upt, udpt, uddpt);
                double testd = dist_law->eval(cpoints[m]);
                int flipped = 0;
                if(closed_curve) {
                    if(cpoints[m] == seg_edge->end_param().operator double()) {
                        SPAposition ept;
                        SPAposition eupt;
                        SPAvector edpt;
                        SPAvector eudpt;
                        offcrv->eval(cpoints[m] - SPAresabs, ept, edpt);
                        given_curve->eval(cpoints[m] - SPAresabs, eupt, eudpt);
                        if((edpt % eudpt) < 0.0) flipped = 1;
                    }
                }
                // if(closed_curve) {
                //     SPAparameter v41 = seg_edge->start_param();  // 不懂
                //     // v41.operator double();
                // }
                if(!flipped) {
                    if(testd >= 0.0 || (udpt * uddpt) % normal <= 0.0) {
                        if(testd < 0.0) continue;
                        if((udpt * uddpt % normal) >= 0.0) continue;
                    }
                }
                tpoints[tp_count++] = cpoints[m];
                if(!closed_curve) goto LABEL_83;
                if(fabs(cpoints[m] - seg_edge->end_param()) > SPAresabs) {
                    if(fabs(cpoints[m] - seg_edge->start_param()) > SPAresabs) {
                    LABEL_83:
                        curve_curve_int* v268 = ACIS_NEW curve_curve_int(list, pt, cpoints[m], cpoints[m]);
                        list = v268;
                        curve_curve_int* v270 = ACIS_NEW curve_curve_int(list, pt, cpoints[m], cpoints[m]);
                        list = v270;
                    }
                }
                if(m < psize - 1) {
                    double t = (cpoints[m] + cpoints[m + 1]) / 2.0;
                    offcrv->eval(*cpoints, pt, dpt, ddpt);
                    given_curve->eval(t, upt, udpt, uddpt);
                    if(dpt % udpt >= 0.0) {
                        domain_center[dom_size] = cpoints[m] + root_tolerances->ROOT_TOL_SEP * 2.0;
                        int dom_sizeb = dom_size + 1;
                        domain_center[dom_sizeb] = cpoints[m + 1] - root_tolerances->ROOT_TOL_SEP * 2.0;
                        dom_size = dom_sizeb + 1;
                    }

                    else {
                        identity_law* v273 = ACIS_NEW identity_law(0, 88);
                        law* e_left = v273;
                        identity_law* v275 = ACIS_NEW identity_law(1, 88);
                        law* v210 = v275;
                        constant_law* v277 = ACIS_NEW constant_law(cpoints[m + 1]);
                        law* e_right = v277;
                        constant_law* v279 = ACIS_NEW constant_law(cpoints[m]);
                        law* v212 = v279;
                        minus_law* v281 = ACIS_NEW minus_law(e_left, e_right);
                        law* subs_x[2];
                        subs_x[0] = v281;
                        minus_law* v283 = ACIS_NEW minus_law(v210, e_right);
                        law* subs_y[2];
                        subs_y[0] = v283;
                        minus_law* v285 = ACIS_NEW minus_law(v212, e_left);
                        subs_x[1] = v285;
                        minus_law* v287 = ACIS_NEW minus_law(v212, v210);
                        subs_y[1] = v287;
                        e_left->remove();
                        v210->remove();
                        e_right->remove();
                        v212->remove();  // else后肯定没错

                        max_law* v289 = ACIS_NEW max_law(subs_x, 2);
                        law* subs_xy[2];
                        subs_xy[0] = v289;
                        max_law* v291 = ACIS_NEW max_law(subs_y, 2);
                        subs_xy[1] = v291;
                        min_law* v293 = ACIS_NEW min_law(subs_xy, 2);
                        law* subs_sum[2];
                        subs_sum[0] = v293;
                        subs_sum[1] = domain_law;
                        law* temp_law = domain_law;
                        min_law* v295 = ACIS_NEW min_law(subs_sum, 2);
                        domain_law = v295;
                        temp_law->remove();
                        domain_center[dom_size] = cpoints[m] - root_tolerances->ROOT_TOL_SEP * 2.0;
                        int dom_sizea = dom_size + 1;
                        domain_center[dom_sizea] = cpoints[m + 1] + root_tolerances->ROOT_TOL_SEP * 2.0;
                        if(domain_center[dom_sizea] > seg_edge->end_param()) {
                            domain_center[dom_sizea] = seg_edge->start_param() + root_tolerances->ROOT_TOL_SEP * 2.0;
                        }
                        dom_size = dom_sizea + 1;
                        subs_x[0]->remove();
                        subs_y[0]->remove();
                        subs_x[1]->remove();
                        subs_y[1]->remove();
                        subs_xy[0]->remove();
                        subs_xy[1]->remove();
                        subs_sum[0]->remove();
                    }
                }

                if(!m) {
                    offcrv->eval((*cpoints + seg_edge->start_param()) / 2.0, pt, dpt, ddpt);
                    given_curve->eval((*cpoints + seg_edge->start_param()) / 2.0, upt, udpt, uddpt);
                    if((dpt % udpt) >= 0.0) {
                        domain_center[dom_size] = seg_edge->start_param() + root_tolerances->ROOT_TOL_SEP * 2.0;
                        int dom_sized = dom_size + 1;
                        domain_center[dom_sized] = cpoints[0] - root_tolerances->ROOT_TOL_SEP * 2.0;
                        dom_size = dom_sized + 1;
                    }

                    else {
                        identity_law* v298 = ACIS_NEW identity_law(0, 88);
                        law* i0 = v298;
                        identity_law* v300 = ACIS_NEW identity_law(1, 88);
                        law* v214 = v300;
                        constant_law* v302 = ACIS_NEW constant_law(*cpoints);
                        law* ci = v302;
                        constant_law* v304 = ACIS_NEW constant_law(seg_edge->start_param().operator double());
                        law* v216 = v304;
                        minus_law* v306 = ACIS_NEW minus_law(i0, ci);
                        law* subs = v306;
                        minus_law* v308 = ACIS_NEW minus_law(v214, ci);
                        law* v389 = v308;
                        minus_law* v310 = ACIS_NEW minus_law(v216, i0);
                        law* v388 = v310;
                        minus_law* v312 = ACIS_NEW minus_law(v216, v214);
                        law* v390 = v312;
                        i0->remove();
                        v214->remove();
                        ci->remove();
                        v216->remove();  // 肯定没错

                        max_law* v314 = ACIS_NEW max_law(&subs, 2);
                        law* v391 = v314;
                        max_law* v316 = ACIS_NEW max_law(&v389, 2);
                        law* v392 = v316;
                        min_law* v318 = ACIS_NEW min_law(&v391, 2);
                        law* v650[2];
                        v650[0] = v318;
                        v650[1] = domain_law;
                        law* v508 = domain_law;
                        min_law* v320 = ACIS_NEW min_law(v650, 2);
                        domain_law = v320;
                        v508->remove();
                        domain_center[dom_size] = cpoints[0] + root_tolerances->ROOT_TOL_SEP * 2.0;
                        int dom_sizec = dom_size + 1;
                        domain_center[dom_sizec] = seg_edge->end_param().operator double() - root_tolerances->ROOT_TOL_SEP * 2.0;
                        dom_size = dom_sizec + 1;
                        subs->remove();
                        v389->remove();
                        v388->remove();
                        v390->remove();
                        v391->remove();
                        v392->remove();
                        v650[0]->remove();
                    }
                }
                if(m == psize - 1) {
                    offcrv->eval((cpoints[m] + seg_edge->end_param()) / 2.0, pt, dpt, ddpt);
                    given_curve->eval((cpoints[m] + seg_edge->end_param()) / 2.0, upt, udpt, uddpt);  // 不确
                    if(dpt % udpt >= 0.0) {
                        domain_center[dom_size] = cpoints[m] + root_tolerances->ROOT_TOL_SEP * 2.0;
                        if(domain_center[dom_size] > seg_edge->end_param()) {
                            domain_center[dom_size] = seg_edge->start_param() + root_tolerances->ROOT_TOL_SEP * 2.0;
                        }
                        ++dom_size;
                        if(fabs(seg_edge->end_param() - cpoints[m]) > SPAresabs) {
                            domain_center[dom_size++] = seg_edge->end_param() - root_tolerances->ROOT_TOL_SEP * 2.0;
                        }
                    } else {
                        identity_law* v323 = ACIS_NEW identity_law(0, 88);
                        law* v217 = v323;
                        identity_law* v325 = ACIS_NEW identity_law(1, 88);
                        law* v218 = v325;
                        constant_law* v327 = ACIS_NEW constant_law(cpoints[m]);
                        law* v219 = v327;
                        constant_law* v329 = ACIS_NEW constant_law(seg_edge->end_param().operator double());
                        law* ce = v329;
                        minus_law* v331 = ACIS_NEW minus_law(v217, ce);
                        law* v393 = v331;
                        minus_law* v333 = ACIS_NEW minus_law(v218, ce);
                        law* v395 = v333;
                        minus_law* v335 = ACIS_NEW minus_law(v219, v217);
                        law* v394 = v335;
                        minus_law* v337 = ACIS_NEW minus_law(v219, v218);
                        law* v396 = v337;
                        v217->remove();
                        v218->remove();
                        v219->remove();
                        ce->remove();
                        max_law* v339 = ACIS_NEW max_law(&v393, 2);
                        law* v397 = v339;
                        max_law* v341 = ACIS_NEW max_law(&v395, 2);
                        law* v398 = v341;
                        min_law* v343 = ACIS_NEW min_law(&v397, 2);

                        v651[0] = v343;
                        v651[1] = domain_law;
                        law* v532 = domain_law;
                        min_law* v345 = ACIS_NEW min_law(v651, 2);
                        domain_law = v345;
                        v532->remove();
                        domain_center[dom_size] = cpoints[m] - root_tolerances->ROOT_TOL_SEP * 2.0;
                        int dom_sizee = dom_size + 1;
                        domain_center[dom_sizee] = seg_edge->start_param() + root_tolerances->ROOT_TOL_SEP * 2.0;
                        dom_size = dom_sizee + 1;
                        v393->remove();
                        v395->remove();
                        v394->remove();
                        v396->remove();
                        v397->remove();
                        v398->remove();
                        v651[0]->remove();
                    }
                }
            }
            ACIS_DELETE[] STD_CAST tpoints;
            int __n = (dom_size + 1) * dom_size / 2 + 1;
            complex_number* v347 = ACIS_NEW complex_number[__n];
            complex_number* ex_points = v347;
            int excount = 0;

            for(int xi = 0; xi < dom_size; ++xi) {
                for(int yi = xi + 1; yi < dom_size; ++yi) {
                    if(domain_center[xi] <= domain_center[yi]) {
                        complex_number* v549 = &ex_points[excount];
                        v549->set(domain_center[yi], domain_center[xi]);
                    } else {
                        complex_number* v548 = &ex_points[excount];
                        v548->set(domain_center[xi], domain_center[yi]);
                    }
                    if(xi >= wsize && yi >= wsize && yi - xi <= 1) {
                        ++excount;
                    } else {
                        complex_number v690 = ex_points[excount];
                        double svalue = sect_law->evaluateC_R(v690);
                        double dp = domain_center[xi] - domain_center[yi];
                        double dpdp = dp * dp;
                        svalue = svalue * (dp * dp);
                        double odist1 = dist_law->eval(domain_center[xi]);
                        double odist2 = dist_law->eval(domain_center[yi]);
                        double odist = odist1;
                        if(odist2 > odist1) odist = odist2;
                        if(odist * odist > svalue) ++excount;
                    }
                }
            }

            complex_number* v563 = &ex_points[excount];
            v563->set(seg_edge->end_param() - SPAresabs * 10.0, seg_edge->start_param() + SPAresabs * 10.0);
            ++excount;
            rec2d rec(seg_edge->start_param().operator double(), seg_edge->end_param().operator double(), seg_edge->start_param().operator double(), seg_edge->end_param().operator double());
            int rsize;
            complex_number* roots = Nroot2D(sect_law, domain_law, rec, &rsize, ex_points, excount);
            sect_law->remove();
            domain_law->remove();
            for(int ii = 0; ii < rsize; ++ii) {
                if(root_tolerances->ROOT_TOL_SEP <= fabs(roots[ii].re - roots[ii].im)) {
                    if(!closed_curve || root_tolerances->ROOT_TOL_SEP <= fabs(fabs(seg_edge->start_param() - seg_edge->end_param()) - fabs(roots[ii].re - roots[ii].im))) {
                        SPAposition v699;
                        offcrv->eval(roots[ii].re, pt, dpt, ddpt, 0, 0);  // 不确
                        offcrv->eval(roots[ii].im, v699, dpt, ddpt, 0, 0);
                        curve_curve_int* v352 = ACIS_NEW curve_curve_int(list, pt, roots[ii].re, roots[ii].re);
                        list = v352;
                        curve_curve_int* v357 = ACIS_NEW curve_curve_int(list, v699, roots[ii].im, roots[ii].im);
                        list = v357;
                    }
                }
            }

            if(closed_curve) {
                offcrv->eval(seg_edge->end_param().operator double(), pt, dpt, ddpt);
                curve_curve_int* v362 = ACIS_NEW curve_curve_int(list, pt, seg_edge->start_param().operator double(), seg_edge->start_param().operator double());
                list = v362;
                curve_curve_int* v364 = ACIS_NEW curve_curve_int(list, pt, seg_edge->end_param().operator double(), seg_edge->end_param().operator double());
                list = v364;
            }
            if(given_curve) {
                given_curve = nullptr;
            }
            if(offcrv) {
                offcrv = nullptr;
            }
            ACIS_DELETE[] STD_CAST domain_center;
            ACIS_DELETE[] ex_points;
            if(delete_cpoints_temp) {
                ACIS_DELETE[] STD_CAST cpoints_temp;
            }
            ACIS_DELETE[] STD_CAST cpoints;
            ACIS_DELETE[] roots;
        }
        if(list) {
            curve_curve_int* this_cci = list;
            int c_sense = this->seg_coedge->sense() != seg_edge->sense();
            while(this_cci) {
                if(c_sense == 1) {
                    this_cci->param1 = -this_cci->param1;
                    this_cci->param2 = -this_cci->param2;
                }
                this_cci = this_cci->next;
            }
            sg_sort_cci(list, 0, 0);
            sg_seg_rating* v619;
            sg_seg_rating* rating_list;
            int cci_count;
            if(seg_edge->start() != seg_edge->end()) goto LABEL_318;
            this_cci = list;
            curve_curve_int* next_cci;
            for(next_cci = list; next_cci->next; next_cci = next_cci->next) this_cci = next_cci;
            if(next_cci) ACIS_DELETE next_cci;
            this_cci->next = nullptr;
            // curve_curve_int* v368 = next_cci;
            // ACIS_DELETE v368; // 不确定不确定
            /*   v616 = next_cci ? curve_curve_int::`scalar deleting destructor'(v368, 1u) : 0i64; this_cci->next = 0i64;*/
            this_cci = list;
            list = list->next;
            if(this_cci) ACIS_DELETE this_cci;
            v619 = ACIS_NEW sg_seg_rating[cci_count];
            rating_list = v619;
            cci_count = 0;
            if(list) {
            LABEL_318:
                if(trim_offset_wire_module_header.debug_level >= 30) sg_debug_cci(list, debug_file_ptr, "self-cci:  ");
                sg_reorder_self_cci(list);

                for(this_cci = list; this_cci; this_cci = this_cci->next) ++cci_count;

                this_cci = list;
                for(int i = 0; i < cci_count; ++i) {
                    SPAunit_vector tan1 = coedge_param_dir(this->seg_coedge, this_cci->param1);
                    SPAunit_vector tan2 = coedge_param_dir(this->seg_coedge, this_cci->param2);
                    double dot = ((tan1 * tan2) % normal) * dist_law->eval(this_cci->param1);
                    if(this->seg_coedge->sense() == 1) dot = -dot;
                    if(dot <= SPAresabs) {
                        if((-SPAresabs) <= dot)  //
                        {
                            rating_list[i].low_rating = seg_tan;
                            rating_list[i].high_rating = seg_tan;
                        } else {
                            rating_list[i].low_rating = seg_out;
                            rating_list[i].high_rating = seg_in;
                        }
                    } else {
                        rating_list[i].low_rating = seg_in;
                        rating_list[i].high_rating = seg_out;
                    }
                    rating_list[i].cci = this_cci;
                    rating_list[i].mIntersectionId = ++ioStartConnectionId;
                    this_cci = this_cci->next;
                }
            }
            int* v631 = ACIS_NEW int[cci_count];
            index = v631;
            for(int ia = 0; ia < cci_count; ++ia) index[ia] = ia;
            for(int ib = 0; ib < cci_count - 1; ++ib) {
                for(int j = ib + 1; j < cci_count; ++j) {
                    int v191 = index[ib];
                    int v190 = index[j];
                    if(rating_list[v191].cci->param2 > rating_list[v190].cci->param2) {
                        index[ib] = v190;
                        index[j] = v191;
                    }
                }
            }
            if(trim_offset_wire_module_header.debug_level >= 30) {
                acis_fprintf(debug_file_ptr, "Rating list along self-intersecting coedge:\n");
                int v165 = 0;
                int v168 = 0;
                while(v165 < cci_count || v168 < cci_count) {
                    curve_curve_int* cci;
                    if(v165 >= cci_count)
                        cci = rating_list[v165 - 1].cci;
                    else
                        cci = rating_list[v165].cci;
                    double v370 = cci->param1;
                    double param1 = v370;
                    curve_curve_int* v141;
                    if(v168 >= cci_count)
                        v141 = rating_list[index[v168 - 1]].cci;
                    else
                        v141 = rating_list[index[v168]].cci;
                    double v371 = v141->param2;
                    double param2 = v371;
                    if(v371 < param1 || v165 >= cci_count) {
                        const char* v378;
                        if(rating_list[index[v168]].high_rating) {
                            const char* v376;
                            if(rating_list[index[v168]].high_rating == seg_out)
                                v376 = "inside";
                            else
                                v376 = "tangent";
                            v378 = v376;
                        } else {
                            v378 = "outside";
                        }
                        const char* v379;
                        if(rating_list[index[v168]].low_rating) {
                            const char* v377;
                            if(rating_list[index[v168]].low_rating == seg_out)
                                v377 = "inside";
                            else
                                v377 = "tangent";
                            v379 = v377;
                        } else {
                            v379 = "outside";
                        }
                        acis_fprintf(debug_file_ptr, "\t%s-%s\n", v379, v378);
                        ++v168;
                    } else {
                        const char* v374;
                        if(rating_list[v165].high_rating) {
                            const char* v372;
                            if(rating_list[v165].high_rating == seg_out)
                                v372 = "outside";
                            else
                                v372 = "tangent";
                            v374 = v372;
                        } else {
                            v374 = "inside";
                        }
                        const char* v375;
                        if(rating_list[v165].low_rating) {
                            const char* v373;
                            if(rating_list[v165].low_rating == seg_out)
                                v373 = "outside";
                            else
                                v373 = "tangent";
                            v375 = v373;
                        } else {
                            v375 = "inside";
                        }
                        acis_fprintf(debug_file_ptr, "\t%s-%s\n", v375, v374);
                        ++v165;
                    }
                }
                acis_fprintf(debug_file_ptr, "\n");
            }
            offset_segment* this_seg = this;
            int i1 = 0;
            int i2 = 0;
            while(i1 < cci_count || i2 < cci_count) {
                curve_curve_int* v142;
                if(i1 >= cci_count)
                    v142 = rating_list[i1 - 1].cci;
                else
                    v142 = rating_list[i1].cci;
                double v380 = v142->param1;
                double v635 = v380;
                curve_curve_int* v143;
                if(i2 >= cci_count)
                    v143 = rating_list[index[i2 - 1]].cci;
                else
                    v143 = rating_list[index[i2]].cci;
                double v381 = v143->param2;
                double v634 = v381;
                if(v381 < v635 || i1 >= cci_count) {
                    if(this_seg->split_and_rate(rating_list[index[i2]], 1)) this_seg = this_seg->next();
                    ++i2;
                } else {
                    if(this_seg->split_and_rate(rating_list[i1], 0)) this_seg = this_seg->next();
                    ++i1;
                }
            }
            sg_delete_cci(list);
            ACIS_DELETE[] STD_CAST rating_list;
            ACIS_DELETE[] STD_CAST index;
            if(resignal_no || acis_interrupted()) sys_error(resignal_no, error_info_base_ptr);
        }
    }
}

void offset_segment::do_self_intersect(law* dist_law, SPAunit_vector& normal, int iSingleSegment, int& ioStartConnectionId, ofst_edge_smooth_manager* iEdgeSmoothMgr, int& oHasError) {
    oHasError = 0;  // 不懂
    if(dist_law && this->c_underlying && !this->simple_offset()) {
        curve_curve_int* mainList = nullptr;
        int resignal_no = 0;
        acis_exception error_info_holder(0);
        error_info_base* error_info_base_ptr = nullptr;
        exception_save exception_save_mark;
        EDGE* baseEdge = nullptr;
        exception_save_mark.begin();
        get_error_mark().buffer_init = 1;
        int error_no = 0;

        EDGE* segEdge = this->seg_coedge->edge();
        /*if(!segEdge) _wassert(L"segEdge", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\trmofwi.cpp", 0xC69u);*/
        if(this->c_underlying) baseEdge = this->c_underlying->edge();
        SPAinterval edgeParamRange;
        SPAtransf iTransf;
        intcurve* offsetCurve = (intcurve*)get_curve_from_coedge(this->seg_coedge, edgeParamRange, iTransf);

        int curveClosed = segEdge->start() == segEdge->end();
        int removeContainment = 0;
        if(baseEdge) {
            if(iEdgeSmoothMgr) {
                int smoothLevel = iEdgeSmoothMgr->get_edge_smooth_level(baseEdge, removeContainment);
                curve* newOfstCurve = smooth_curve1(offsetCurve, normal, smoothLevel, dist_law);
                if(newOfstCurve) {
                    set_curve_to_coedge(this->seg_coedge, newOfstCurve);
                    if(offsetCurve) {
                        offsetCurve = nullptr;
                    }
                    offsetCurve = (intcurve*)newOfstCurve;
                    newOfstCurve = nullptr;
                }
            }
        }
        curve_curve_int* intersectionsList = get_curve_self_ints(*offsetCurve, SPAresabs);
        if(baseEdge && iEdgeSmoothMgr) iEdgeSmoothMgr->set_edge_intersection_info(baseEdge, intersectionsList);
        if(curveClosed) {
            SPAposition pt;
            SPAvector dpt;
            SPAvector ddpt;
            double d1 = segEdge->start_param().operator double();
            double ep = segEdge->end_param().operator double();
            if(segEdge->sense() == 1) {
                double temp = d1;
                d1 = -ep;
                ep = -temp;
            }
            offsetCurve->eval(d1, pt, dpt, ddpt);  // 不确定
            curve_curve_int* v35 = ACIS_NEW curve_curve_int(mainList, pt, d1, d1);
            mainList = v35;
            curve_curve_int* v37 = ACIS_NEW curve_curve_int(mainList, pt, ep, ep);
            mainList = v37;
        }
        int mayHaveSelfIntersections = 0;
        curve_curve_int* cuspsList = nullptr;
        this->comp_cusp_points(cuspsList, dist_law, normal, 0);
        if(cuspsList) {
            display_segment_intersection_points(segEdge, cuspsList, baseEdge);
            int no_ints = intersectionsList == nullptr;
            COEDGE* coed = this->coedge();
            curve_curve_int* v53 = cuspsList;
            remove_intersections_containing_exactly_one_cusp(intersectionsList, cuspsList, coed);
            add_intersectins(mainList, intersectionsList);
            ofst_intersection_containment_remover cRemover;
            cRemover.remove_contained_cusps(intersectionsList, cuspsList, removeContainment);
            add_intersectins(mainList, cuspsList);
            if(no_ints && cuspsList) mayHaveSelfIntersections = 1;

        } else {
            add_intersectins(mainList, intersectionsList);
        }
        if(intersectionsList) {
            sg_delete_cci(intersectionsList);
            intersectionsList = nullptr;
        }
        if(cuspsList) {
            sg_delete_cci(cuspsList);
            cuspsList = nullptr;
        }
        if(mainList) {
            this->split_and_rate(mainList, dist_law, normal, ioStartConnectionId, SPAresabs, iSingleSegment, mayHaveSelfIntersections);
        }
        if(offsetCurve) {
            offsetCurve = nullptr;
        }
        if(error_no) oHasError = 1;
        if(acis_interrupted()) sys_error(resignal_no, error_info_base_ptr);
        if(mainList) sg_delete_cci(mainList);
    }
}
void offset_segment::print(_iobuf* dfp) {
    acis_fprintf(dfp, "\n");
    acis_fprintf(dfp, "Offset segment %x:\n", this);
    acis_fprintf(dfp, "\tNext segment: %x\n", this->next_segment);
    acis_fprintf(dfp, "\tPrev segment: %x\n", this->prev_segment);
    acis_fprintf(dfp, "\tCoedge: %x ( ", this->seg_coedge);
    ((SPAposition&)(this->seg_coedge->start()->geometry()->coords())).debug(dfp);
    acis_fprintf(dfp, " ) to ( ");
    ((SPAposition&)this->seg_coedge->end()->geometry()->coords()).debug(dfp);  // 不确定
    acis_fprintf(dfp, " )\n");
    if(this->start_is_out)
        acis_fprintf(dfp, "\tStart rating: %s\n", "out");
    else
        acis_fprintf(dfp, "\tStart rating: %s\n", "in");
    if(this->end_is_out)
        acis_fprintf(dfp, "\tEnd rating: %s\n", "out");
    else
        acis_fprintf(dfp, "\tEnd rating: %s\n", "in");
    int is_extension = this->is_extension;

    acis_fprintf(dfp, "\tReversed, Singular, Simple, Extension:\t%d\t%d\t%d\t%d\n", this->seg_reversed, this->is_singular, this->simple_offset(), is_extension);
    if(this->v_underlying || !this->c_underlying) {
        if(!this->v_underlying || this->c_underlying)
            acis_fprintf(dfp, " !!!!  AMBIGUOUS OFFSET  !!!!\n");
        else
            acis_fprintf(dfp, "Offset from a vertex\n");
    } else {
        acis_fprintf(dfp, "Offset from a coedge\n");
    }
}
void offset_segment::get_init_params(double& iSParam, double& iEParam) {
    iSParam = this->mParams[0];
    iEParam = this->mParams[1];
}
void offset_segment::set_init_params(double iSParam, double iEParam) {
    this->mParams[0] = iSParam;
    this->mParams[1] = iEParam;
}
void offset_segment::comp_cusp_points(curve_curve_int*& ioList, law* iDistLaw, SPAunit_vector& iNormal, int use_high_curv_func) {
    int psize;  // [rsp+50h] [rbp-888h] BYREF

    /*if(!this->c_underlying) _wassert(L"this->c_underlying", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\trmofwi.cpp", 0x68Du);*/
    EDGE* segEdge = this->seg_coedge->edge();
    intcurve* offcrv = (intcurve*)segEdge->geometry()->trans_curve();
    curve* given_curve = nullptr;

    if(is_intcurve(offcrv)) {
        const int_cur& intCur = offcrv->get_int_cur();

        // v95 = offset_int_cur::id();  //
        // v133 = intCur.__vftable;
        if(offset_int_cur::id() == intCur.type()) {
            // if(0 == intCur.type()) {
            // ofstIntcur = (offset_int_cur*)intCur;
            // baseCurve = offset_int_cur::get_orig_curve((offset_int_cur*)intCur);
            // given_curve = curve::copy_curve((curve*)baseCurve);
            const curve& baseCurve = ((const offset_int_cur&)intCur).get_orig_curve();
            given_curve = ((curve&)baseCurve).copy_curve();
        } else {
            given_curve = this->c_underlying->edge()->geometry()->trans_curve();
        }
    } else {
        given_curve = this->c_underlying->edge()->geometry()->trans_curve();
    }
    if(given_curve) {
        if(use_high_curv_func) {
            SPAinterval* hc_ints = nullptr;
            double test_curv = 0.1 / (SPAresabs);
            int num_hcs = offcrv->high_curvature(test_curv, hc_ints);
            for(int ii = 0; ii < num_hcs; ++ii) {
                SPAposition pos;
                double max_param = find_max_curv_param(offcrv, hc_ints[ii], pos);
                const SPAposition& v39 = this->coedge()->start()->geometry()->coords();
                if((pos - v39).len() > SPAresabs) {
                    if((pos - this->coedge()->start()->geometry()->coords()).len() > 10.0 * SPAresabs) {
                        curve_curve_int* v128 = ACIS_NEW curve_curve_int(ioList, pos, max_param, max_param);
                        ioList = v128;
                    }
                }
            }
            int num_discs = 0;

            const double* discs = offcrv->discontinuities(num_discs, 1);
            for(int iia = 0; iia < num_discs; ++iia) {
                SPAinterval range = this->coedge()->edge()->param_range();
                double started = range.start_pt();
                if(discs[iia] > started + SPAresnor) {
                    if((range.end_pt() - SPAresnor) > discs[iia]) {
                        int contained = 0;
                        for(int jj = 0; jj < num_hcs; ++jj) {
                            if(discs[iia] << hc_ints[jj]) {
                                contained = 1;
                                break;
                            }
                        }
                        if(!contained) {
                            SPAposition p1;
                            SPAvector deriv_low;
                            SPAvector deriv_high;
                            SPAvector* derivs[1];
                            derivs[0] = &deriv_low;
                            offcrv->evaluate(discs[iia], p1, derivs, 1, evaluate_curve_side::evaluate_curve_below);
                            derivs[0] = &deriv_high;
                            offcrv->evaluate(discs[iia], p1, derivs, 1, evaluate_curve_side::evaluate_curve_above);
                            // 不确定
                            if((deriv_low % deriv_high) < 0.0) {
                                if((p1 - this->coedge()->start()->geometry()->coords()).len_sq() > SPAresabs * SPAresabs) {
                                    if((p1 - this->coedge()->end()->geometry()->coords()).len_sq() > SPAresabs * SPAresabs) {
                                        curve_curve_int* v130 = ACIS_NEW curve_curve_int(ioList, p1, discs[iia], discs[iia]);
                                        ioList = v130;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ACIS_DELETE[] STD_CAST hc_ints;
        } else {
            curve_law_data* v109 = ACIS_NEW curve_law_data(*given_curve, 0.0, 0.0);  // 不确定
            curve_law_data* old_cld = v109;
            curvature_law* v111 = ACIS_NEW curvature_law(old_cld);
            law* cur_law = v111;
            old_cld->remove();
            constant_law* v114 = ACIS_NEW constant_law(1.0);
            law* const_one = v114;
            abs_law* v116 = ACIS_NEW abs_law(iDistLaw);
            law* fabs_dist = v116;
            times_law* v118 = ACIS_NEW times_law(fabs_dist, cur_law);
            law* cons_law = v118;
            int wsize = 0;
            double minx;
            double ep;
            correct_param_0(segEdge, minx, ep);  // 不懂

            double* cpoints_temp = Nsolve(const_one, cons_law, minx, ep, &psize);
            const_one->remove();
            cons_law->remove();
            int delete_cpoints_temp = 0;
            SPAposition pt;
            SPAposition pt2;
            SPAposition upt;
            SPAvector dpt;
            SPAvector ddpt;
            SPAvector udpt;
            SPAvector uddpt;
            offcrv->eval(ep, upt);
            offcrv->eval(minx, pt);
            int closed_curve = 0;
            double* v155 = ACIS_NEW double[psize + 1];
            double* cpoints = v155;
            if(pt == upt) {
                closed_curve = 1;
                for(int index = 0; index < psize; ++index) cpoints[index] = cpoints_temp[index];
                double x = ep - SPAresabs / 100.0;
                double rc_end = cur_law->eval(x);
                double rc_start = cur_law->eval(minx);
                double dist = fabs_dist->eval(ep);
                if(dist > 0.0) {
                    double rc_test = 1.0 / dist;
                    if(rc_end > rc_start && rc_test >= rc_start && rc_end > rc_test) {
                        cpoints[psize++] = segEdge->end_param().operator double();
                    }
                    if(rc_start > rc_end && rc_test >= rc_end && rc_start > rc_test) cpoints[psize++] = ep;
                }
                delete_cpoints_temp = 1;
            } else {
                cpoints = cpoints_temp;
            }
            fabs_dist->remove();
            law* dcur_law = cur_law->derivative(0);
            cur_law->remove();
            double* wpoints = nullptr;
            int resignal_no = 0;
            acis_exception error_info_holder(0);
            error_info_base* error_info_base_ptr = nullptr;
            exception_save exception_save_mark;

            double old_tol = root_tolerances->ROOT_TOL_X;
            exception_save_mark.begin();
            get_error_mark().buffer_init = 1;
            int error_no = 0;
            root_tolerances->ROOT_TOL_X = 1.0e-7;
            wpoints = Nroot(dcur_law, minx, ep, &wsize);
            dcur_law->remove();
            law_root_tolerances* v20 = root_tolerances;
            v20->ROOT_TOL_X = old_tol;
            if(resignal_no || acis_interrupted()) sys_error(resignal_no, error_info_base_ptr);
            double* tpoints = ACIS_NEW double[wsize + psize];
            int tp_count;
            for(tp_count = 0; tp_count < wsize; ++tp_count) tpoints[tp_count] = wpoints[tp_count];
            void* alloc_ptr = wpoints;
            ACIS_DELETE[] STD_CAST wpoints;

            for(int i = 0; i < psize; ++i) {
                offcrv->eval(cpoints[i], pt, dpt, ddpt, 0, 0);          // 不确定
                given_curve->eval(cpoints[i], upt, udpt, uddpt, 0, 0);  // 不确定
                double testd = iDistLaw->eval(cpoints[i]);
                int flipped = 0;
                if(closed_curve) {
                    if(cpoints[i] == segEdge->end_param().operator double()) {
                        SPAposition ept;
                        SPAposition eupt;
                        SPAvector edpt;
                        SPAvector eudpt;
                        offcrv->eval(cpoints[i] - SPAresabs, ept, edpt);
                        given_curve->eval(cpoints[i] - SPAresabs, eupt, eudpt);
                        if((edpt % eudpt) < 0.0) flipped = 1;
                    }
                }
                if(!flipped) {
                    if(testd >= 0.0 || (udpt * uddpt) % iNormal <= 0.0) {
                        if(testd < 0.0) continue;
                        if((udpt * uddpt) % iNormal >= 0.0) continue;
                    }
                }
                tpoints[tp_count++] = cpoints[i];
                if(closed_curve) {
                    if(fabs(cpoints[i] - ep) <= SPAresabs) continue;
                    if(fabs(cpoints[i] - minx) <= SPAresabs) continue;
                }
                curve_curve_int* v125 = ACIS_NEW curve_curve_int(ioList, pt, cpoints[i], cpoints[i]);
                ioList = v125;
            }

            ACIS_DELETE[] STD_CAST tpoints;
            if(delete_cpoints_temp) {
                ACIS_DELETE[] STD_CAST cpoints_temp;
            }
            ACIS_DELETE[] STD_CAST cpoints;
        }
        if(given_curve) {
            given_curve = nullptr;
        }
        if(offcrv) offcrv = nullptr;
    }
}
void offset_segment::split_and_rate_new(curve_curve_int*& ioIntersections, int& ioStartConnectionId, double iHalfParamRange, int iSingleSegment) {
    if(ioIntersections) {
        EDGE* segEdge = this->seg_coedge->edge();
        int c_sense = this->seg_coedge->sense() != segEdge->sense();
        if(c_sense == 1) {
            for(curve_curve_int* i = ioIntersections; i; i = i->next) {
                i->param1 = -i->param1;
                i->param2 = -i->param2;  // 不确定
            }
        }
        sg_sort_cci(ioIntersections, 0, 0);
        int firstIntersectContainsAll = 0;
        if(iSingleSegment) firstIntersectContainsAll = is_first_intersect_contains_all(ioIntersections, iHalfParamRange);
        if(trim_offset_wire_module_header.debug_level >= 30) sg_debug_cci(ioIntersections, debug_file_ptr, "self-cci:  ");
        offset_segment* thisSeg = this;
        for(curve_curve_int* thisIntersect = ioIntersections; thisIntersect; thisIntersect = thisIntersect->next) {
            ++ioStartConnectionId;
            sg_seg_rating intersectionRating;
            intersectionRating.low_rating = seg_out;
            intersectionRating.high_rating = seg_out;
            intersectionRating.cci = thisIntersect;
            intersectionRating.mIntersectionId = ioStartConnectionId;
            if(thisIntersect->param1 == thisIntersect->param2) {
                if(thisSeg->split_and_rate(intersectionRating, 0)) thisSeg = thisSeg->next();
            } else {
                if(firstIntersectContainsAll && thisIntersect == ioIntersections) {
                    intersectionRating.low_rating = seg_in;
                    intersectionRating.high_rating = seg_out;
                } else {
                    intersectionRating.high_rating = seg_in;
                }
                if(thisSeg->split_and_rate(intersectionRating, 0)) thisSeg = thisSeg->next();
                if(firstIntersectContainsAll && thisIntersect == ioIntersections) {
                    intersectionRating.low_rating = seg_out;
                    intersectionRating.high_rating = seg_in;
                } else {
                    intersectionRating.low_rating = seg_in;
                    intersectionRating.high_rating = seg_out;
                }
                if(thisSeg->split_and_rate(intersectionRating, 1) && (!firstIntersectContainsAll || thisIntersect != ioIntersections)) {
                    thisSeg = thisSeg->next();
                }
            }
        }
    }
}

offset_segment_list::offset_segment_list(law* dist, const SPAunit_vector& normal, int add_attribs, sg_gap_type use_type, int iNonPlanar) {
    this->offset_normal = SPAunit_vector();
    this->head_node = offset_segment();
    this->tail_node = offset_segment();
    this->dist_law = dist;
    this->dist_law->add();
    this->offset_normal = normal;
    this->add_attributes = add_attribs;
    if(this->dist_law->constant()) {
        law* dist_law = this->dist_law;
        this->mOffsetDist = dist_law->eval(1.0);
    }
    this->head_node.set_next(&this->tail_node);
    this->tail_node.set_previous(&this->head_node);
    this->count = 0;
    this->close_type = use_type;
    this->mLastSegmentConnectionId = 0;
    this->mStartSegmentConnectionId = 0;
    this->mBaseWire = nullptr;
    this->mTrimInsideWireDone = 0;
    this->mNonPlanar = iNonPlanar;
    this->mEdgeSmoothMgr = nullptr;
    this->mSelfIntesectError = 0;
}
offset_segment_list::offset_segment_list() {
    this->offset_normal = SPAunit_vector();
    this->head_node = offset_segment();
    this->tail_node = offset_segment();
    this->dist_law = nullptr;
    this->add_attributes = 0;
    this->head_node.set_next(&this->tail_node);
    this->tail_node.set_previous(&this->head_node);
    this->count = 0;
    this->close_type = arc;
    this->mLastSegmentConnectionId = 0;
    this->mStartSegmentConnectionId = 0;
    this->mBaseWire = nullptr;
    this->mTrimInsideWireDone = 0;
    this->mNumberOfOffsetCoedges = 0;
    this->mNonPlanar = 0;
    this->mOffsetDist = 0.0;
    this->mEdgeSmoothMgr = nullptr;
    this->mSelfIntesectError = 0;
}
law* offset_segment_list::distance() {
    return this->dist_law;
}
WIRE* offset_segment_list::get_base_wire() {
    return this->mBaseWire;
}
void offset_segment_list::set_base_wire(WIRE* iBaseWire, int iNumCoedges) {
    this->mBaseWire = iBaseWire;
    this->mNumberOfOffsetCoedges = iNumCoedges;
}
int offset_segment_list::get_number_of_offset_coedges() {
    return this->mNumberOfOffsetCoedges;
}
int offset_segment_list::trim_inside_wire_done() {
    return this->mTrimInsideWireDone;
}
void offset_segment_list::set_trim_inside_wire_done(const int iDone) {
    this->mTrimInsideWireDone = iDone;
}
int offset_segment_list::get_start_segment_connectionId() {
    return this->mStartSegmentConnectionId;
}
void offset_segment_list::set_start_segment_connectionId(const int iId) {
    this->mStartSegmentConnectionId = iId;
}
SPAunit_vector offset_segment_list::normal() {
    SPAunit_vector result(this->offset_normal);
    return result;
}
int offset_segment_list::add_attribs() {
    return this->add_attributes;
}
offset_segment* offset_segment_list::first_segment() {
    return this->head_node.next();
}
offset_segment* offset_segment_list::last_segment() {
    return this->tail_node.previous();
}
void offset_segment_list::set_distance(law* d) {
    if(this->dist_law) this->dist_law->remove();
    this->dist_law = d;
    if(this->dist_law) this->dist_law->add();
}
void offset_segment_list::set_normal(const SPAunit_vector& v) {
    this->offset_normal = v;
}
void offset_segment_list::set_add_attribs(int l) {
    this->add_attributes = l;
}
void offset_segment_list::close_segments() {
    offset_segment* segment = this->last_segment();
    COEDGE* last_coedge = segment->coedge();
    COEDGE* first_coedge = this->first_segment()->coedge();
    if(last_coedge->next() == first_coedge) {
        if(this->last_segment()->reversed()) {
            if(this->last_segment()->simple_offset()) {
                this->first_segment()->set_start_in();
            }
        }
    }
}

int offset_segment_list::add_segment(offset_segment* new_segment, RenderingObject* pRO) {
    int intersection_found = 0;
    if(new_segment) {
        COEDGE* new_coedge = new_segment->coedge();
        if(new_coedge) {
            EDGE* pNewEdge = new_coedge->edge();
            if(pNewEdge) {
                int bContinue = 1;
                if(is_intcurve_edge(pNewEdge)) {
                    const intcurve& rIntcurve = (const intcurve&)(pNewEdge->geometry()->equation());
                    bContinue = (((intcurve&)rIntcurve)).cur(-1.0, 0) != NULL;  // 不确定
                }
                if(bContinue) {
                    offset_segment* last_node = this->tail_node.previous();
                    last_node->set_next(new_segment);
                    new_segment->set_previous(last_node);
                    new_segment->set_next(&this->tail_node);
                    offset_segment* p_tail_node = &this->tail_node;
                    this->tail_node.set_previous(new_segment);
                    ++this->count;
                    SPAunit_vector orig_tan;
                    SPAunit_vector new_tan;
                    SPAposition origPos(0.0, 0.0, 0.0);
                    SPAposition newPos(0.0, 0.0, 0.0);
                    if(new_segment->singular_offset())  // 疑问：singular_offset在什么时候初始化的
                    {
                        new_tan = normalise(SPAvector(0.0, 0.0, 0.0));
                        new_segment->set_start_in();
                        new_segment->set_end_in();
                    } else {
                        if(new_segment->vertex_offset()) {
                            new_tan = coedge_start_dir(new_coedge);
                            newPos = coedge_start_pos(new_coedge);
                        } else {
                            new_tan = coedge_mid_dir(new_coedge);
                            newPos = coedge_mid_pos(new_coedge);
                        }
                    }
                    if(new_segment->vertex_offset())  // 疑问：不知道作用
                    {
                        COEDGE* last_coedge = last_node->coedge();
                        COEDGE* last_orig_coedge = nullptr;
                        if(last_coedge) last_orig_coedge = last_node->original_coedge();
                        int from_corner_gap = is_STRAIGHT(new_coedge->edge()->geometry());
                        if(!last_orig_coedge || from_corner_gap) {
                            orig_tan = new_tan;
                        } else {
                            orig_tan = normalise(coedge_end_dir(last_orig_coedge));
                            origPos = coedge_end_pos(last_orig_coedge);
                        }
                    } else {
                        COEDGE* pOriginalCoed = new_segment->original_coedge();
                        if(pOriginalCoed) {
                            SPAparameter origParam;
                            int paramComputed = 0;
                            if(this->mNonPlanar) {
                                double initEParam;
                                double initSParam;
                                new_segment->get_init_params(initSParam, initEParam);
                                if(initSParam != initEParam) {
                                    double sParam = new_coedge->param_range().start_pt();
                                    double eParam = new_coedge->param_range().end_pt();
                                    if(sParam != initSParam || eParam != initEParam) {
                                        double origSParam = pOriginalCoed->param_range().start_pt();
                                        double origEParam = pOriginalCoed->param_range().end_pt();
                                        double pmRatio = (0.5 * (eParam + sParam) - initSParam) / (initEParam - initSParam);
                                        origParam = SPAparameter(origSParam + pmRatio * (origEParam - origSParam));  // 不确定
                                        paramComputed = 1;
                                    }
                                }
                            }
                            if(!paramComputed) {
                                SPAparameter offsParam(new_coedge->edge()->param_range().mid_pt());
                                int do_point_perp = 1;
                                if(new_coedge->edge() && new_coedge->edge()->geometry()) {
                                    if(is_intcurve(&(new_coedge->edge()->geometry()->equation())))  //&不确定
                                    {
                                        const int_cur& int_cur = ((intcurve&)new_coedge->edge()->geometry()->equation()).get_int_cur();
                                        if(((struct int_cur&)int_cur).type() == offset_int_cur::id()) do_point_perp = 0;  // 不确定
                                    }
                                } else {
                                    do_point_perp = 0;
                                }
                                if(do_point_perp) {
                                    SPAposition posOnOffset = new_coedge->edge()->mid_pos();
                                    SPAposition posOnOriginal;
                                    const curve& origCurve = pOriginalCoed->edge()->geometry()->equation();
                                    SPAparameter guessParam(pOriginalCoed->edge()->param_range().mid_pt());
                                    if(pOriginalCoed->edge()->sense() == 1) {
                                        offsParam = -offsParam;
                                        guessParam = -guessParam;
                                    }

                                    ((curve&)origCurve).point_perp(posOnOffset, posOnOriginal, offsParam, origParam, 0);
                                    SPAposition tmpPosOnOriginal;
                                    SPAparameter tmpOrigParam;
                                    ((curve&)origCurve).point_perp(posOnOffset, tmpPosOnOriginal, guessParam, tmpOrigParam, 0);
                                    if((posOnOffset - posOnOriginal).len_sq() > (posOnOffset - tmpPosOnOriginal).len_sq()) origParam = tmpOrigParam;
                                    if(pOriginalCoed->edge()->sense() == 1) origParam = -origParam;
                                } else {
                                    origParam = offsParam;
                                }
                                if(pOriginalCoed->sense() == 1) origParam = -origParam;
                            }
                            double coedge_param = origParam.operator double();
                            orig_tan = normalise(coedge_param_dir(pOriginalCoed, coedge_param));
                            origPos = coedge_param_pos(pOriginalCoed, origParam.operator double());
                        }
                    }
                    int numOf_org_edges = this->get_number_of_offset_coedges();
                    int set_prev_next_flag = GET_ALGORITHMIC_VERSION() <= AcisVersion(32, 0, 1) || numOf_org_edges == 1;
                    int showTans = 0;
                    if(orig_tan % new_tan < 0.0) {
                        if(!this->mNonPlanar || 2.0 * this->mOffsetDist > (origPos - newPos).len()) {
                            if(set_prev_next_flag && new_segment->simple_offset()) last_node->set_end_in();
                            new_segment->set_start_in();
                            new_segment->set_end_in();
                            new_segment->set_reversed();
                            showTans = 1;
                        }
                    }
                    if(set_prev_next_flag && last_node->reversed() && last_node->simple_offset()) {
                        new_segment->set_start_in();
                    }
                    if(new_segment->reversed() || new_segment->singular_offset()) {
                        return 0;
                    }
                    offset_segment* this_segment = this->head_node.next();
                    offset_segment* next_segment = this_segment->next();
                    offset_segment* tail_ptr = &this->tail_node;
                    while(this_segment != new_segment) {
                        if(!this_segment->reversed() && !this_segment->singular_offset()) {
                            offset_segment* sub_segment = new_segment;
                            offset_segment* next_sub = new_segment->next();
                            while(sub_segment != tail_ptr) {
                                if(sg_intersect_rate_segments(this, this_segment, sub_segment, this->dist_law, this->offset_normal, this->close_type, this->mLastSegmentConnectionId) == 1) intersection_found = 1;
                                sub_segment = next_sub;
                                next_sub = next_sub->next();
                            }
                        }
                        this_segment = next_segment;
                        next_segment = next_segment->next();
                    }
                }
            }
        }
    }
    return intersection_found;
}
void offset_segment_list::insert_segment(offset_segment* seg_to_insert, offset_segment* key_seg) {
    seg_to_insert->set_next(key_seg->next());
    seg_to_insert->set_previous(key_seg);
    key_seg->set_next(seg_to_insert);
    seg_to_insert->next()->set_previous(seg_to_insert);
    ++this->count;
}
void offset_segment_list::remove_segment(offset_segment* seg) {
    seg->previous()->set_next(seg->next());
    seg->next()->set_previous(seg->previous());
    --this->count;
}
void offset_segment_list::print(_iobuf* dfp) {
    acis_fprintf(dfp, "Offset segment list: %d segments\n", this->count);
    acis_fprintf(dfp, "\tHead node at %x\n", this->head_node);
    acis_fprintf(dfp, "\tTail node at %x\n", this->tail_node);
    for(offset_segment* this_seg = this->head_node.next(); this_seg != &this->tail_node; this_seg = this_seg->next()) this_seg->print(dfp);  // 不确定
    acis_fprintf(dfp, "\n===========================\n");
}
void offset_segment_list::empty_list() {
    this->head_node.set_next(&this->tail_node);  // 不确定
    this->tail_node.set_previous(&this->head_node);
    this->count = 0;
}
void offset_segment_list::set_gap_type(sg_gap_type in_type) {
    this->close_type = in_type;
}
sg_gap_type offset_segment_list::get_gap_type() {
    return this->close_type;
}
void offset_segment_list::set_last_connectionId(const int iConnectionId) {
    this->mLastSegmentConnectionId = iConnectionId;
}
int offset_segment_list::get_last_connectionId() {
    return this->mLastSegmentConnectionId;
}
void offset_segment_list::init_connection() {
    offset_segment* thisSeg = this->first_segment();
    offset_segment* lastSeg = this->last_segment()->next();
    if(thisSeg && lastSeg) {
        int startId = thisSeg->get_start_connectionId();
        if(startId < 1) {
            startId = ++this->mLastSegmentConnectionId;
            thisSeg->set_start_connectionId(startId);
        }
        this->mStartSegmentConnectionId = startId;
        while(thisSeg != lastSeg) {
            if(thisSeg->get_end_connectionId() < 1) {
                int segId;
                if(thisSeg->singular_offset())
                    segId = thisSeg->get_start_connectionId();
                else
                    segId = ++this->mLastSegmentConnectionId;
                thisSeg->set_end_connectionId(segId);
                offset_segment* nextSeg = thisSeg->next();
                nextSeg->set_start_connectionId(segId);
            }
            thisSeg = thisSeg->next();
        }
        this->first_segment();
        offset_segment* lastSega = this->last_segment();
        lastSega->set_end_connectionId(startId);
        if(lastSega->singular_offset()) {
            lastSega->set_start_connectionId(startId);
            offset_segment* prevSeg = lastSega->previous();
            if(prevSeg) prevSeg->set_end_connectionId(startId);
        }
    }
}
void offset_segment_list::set_edge_smooth_manager(ofst_edge_smooth_manager* iEdgeSmoothMgr) {
    this->mEdgeSmoothMgr = iEdgeSmoothMgr;
}
ofst_edge_smooth_manager* offset_segment_list::get_edge_smooth_manager() {
    return this->mEdgeSmoothMgr;
}
offset_segment_list::~offset_segment_list() {
    if(this->dist_law) this->dist_law->remove();
    offset_segment* this_seg = this->first_segment();

    while(this_seg != this->last_segment()->next()) {
        offset_segment* next_seg = this_seg->next();
        ACIS_DELETE this_seg;
        this_seg = next_seg;
    }
    this->empty_list();
    this->mEdgeSmoothMgr = nullptr;
}
int offset_segment_list::segments_connected() {
    int segsConnected = 1;
    offset_segment* dummy_seg = this->first_segment();
    COEDGE* preCoedge = nullptr;
    while(dummy_seg != this->last_segment()->next() && segsConnected) {
        COEDGE* crrCoedge = dummy_seg->coedge();
        segsConnected = coedge_connected(preCoedge, crrCoedge);
        preCoedge = crrCoedge;
        dummy_seg = dummy_seg->next();
    }
    return segsConnected;
}
void offset_segment_list::add_connection_after_split(offset_segment* iStartSegment1, offset_segment* iStartSegment2) {
    while(iStartSegment1 && iStartSegment2 && (!iStartSegment1->get_end_connectionId() || !iStartSegment2->get_end_connectionId())) {
        int connectionId = iStartSegment1->get_end_connectionId();
        if(!connectionId) connectionId = iStartSegment2->get_end_connectionId();
        if(!connectionId) connectionId = ++this->mLastSegmentConnectionId;
        iStartSegment1->set_end_connectionId(connectionId);
        iStartSegment2->set_end_connectionId(connectionId);
        iStartSegment1 = iStartSegment1->next();
        iStartSegment2 = iStartSegment2->next();
        iStartSegment1->set_start_connectionId(connectionId);
        iStartSegment2->set_start_connectionId(connectionId);
    }
}