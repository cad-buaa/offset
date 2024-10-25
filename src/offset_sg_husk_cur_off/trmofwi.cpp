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
            COEDGE* cur_coedge = cur_coedge->next();
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
        wireExtractor.extract(&seg_list, wire_list, wire_closed, iRemoveOverlap);  // 不确定
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
    /*if(Debug_Break_Active("sg_collect_segments", "WIRE-OFFSET")) {
        if(get_breakpoint_callback()) {
            v110 = get_breakpoint_callback();
            v161 = v110->new_render_object;
            v111 = v161(v110, 1);
        } else {
            v111 = 0i64;
        }
        v94 = v111;
        if(v111) {
            ENTITY_LIST::ENTITY_LIST(&v78);
            get_edges(wire_list, &v78, PAT_CAN_CREATE);
            ENTITY_LIST::init(&v78);
            while(1) {
                ed = (EDGE*)ENTITY_LIST::next(&v78);
                if(!ed) break;
                show_entity(ed, GREEN, v94);
            }
            Debug_Break("sg_collect_segments", "WIRE-OFFSET", "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\trmofwi.cpp", 6051);
            if(get_breakpoint_callback()) {
                v113 = get_breakpoint_callback();
                v162 = v113->delete_render_object;
                v162(v113, v94);
            }
            ENTITY_LIST::~ENTITY_LIST(&v78);
        }
    }*/
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
        ofst_edge_smooth_manager* edgeSmoothMgr = orig_list.get_edge_smooth_manager();
        ENTITY* coedge_owner = orig_list.first_segment()->coedge()->owner();
        if(!is_WIRE(coedge_owner)) return 0;
        WIRE* this_wire = (WIRE*)coedge_owner;
        BODY* wire_body = this_wire->shell()->lump()->body();
        // 不确定
        int resignal_no = 0;
        acis_exception error_info_holder(0);
        error_info_base* error_info_base_ptr = nullptr;
        exception_save exception_save_mark;
        law* dist = orig_list.distance();
        offset_segment_list segment_list(dist, (SPAunit_vector&)orig_list.normal(), orig_list.add_attribs(), orig_list.get_gap_type(), 0);  // 不懂
        law* local_dist_law = nullptr;
        option_header* cur_check = find_option("careful_curve_self_int_check");
        if(GET_ALGORITHMIC_VERSION() >= AcisVersion(20, 0, 0)) cur_check->push(0);
        exception_save_mark.begin();
        get_error_mark().buffer_init = 1;
        int error_no = 0;
        sg_add_extensions(orig_list);
        display_segment_in_colors(orig_list, 5);
        int origConnected = orig_list.segments_connected();
        offset_segment* this_seg = orig_list.first_segment();
        offset_segment* last_seg = orig_list.last_segment()->next();
        int segmentConnectionId = 0;
        int singleSegment = orig_list.get_number_of_offset_coedges() == 1;
        offset_segment* next_seg;
        if(check_offst_self_intersection.on()) {
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
                    law* local_dist_law = v119;
                }
                if(GET_ALGORITHMIC_VERSION() > AcisVersion(17, 0, 0)) {
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

        if(GET_ALGORITHMIC_VERSION() > AcisVersion(20, 0, 0) && check_offst_self_intersection.on()) {
            while(this_sega != last_seg) {
                next_seg = this_sega->next();
                this_sega->split_at_cusps(orig_list.distance());
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
            if(pEdge) {
                trim_edge_geom(this_segb->coedge()->edge());
                int bContinue = 1;
                if(is_intcurve_edge(pEdge)) {
                    const intcurve& rIntcurve = (const intcurve&)pEdge->geometry()->equation();
                    if(!((intcurve&)rIntcurve).cur(-1.0, 0)) bContinue = 0;
                }
                if(bContinue) {
                    if(segment_list.add_segment(this_segb, in_segment_pRO) == 1) intersection_found = 1;
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
        do {
            if(!inside_rating) inside_rating = this_segc->start_out() == 0;
            if(this_segc->end_out()) inside_rating = 0;
            if(inside_rating) {
                this_segc->set_start_in();
                this_segc->set_end_in();
            }
            this_segc = this_segc->next();
        } while(this_segc != last_seg);
        offset_segment* this_segd = segment_list.last_segment();
        offset_segment* first_seg = segment_list.first_segment()->previous();
        int inside_ratinga = 0;
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
        offset_segment* v78 = segment_list.first_segment();
        while(segment_list.first_segment() != segment_list.last_segment()->next()) {
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

        option_header* care_cur_chk = find_option("careful_curve_self_int_check");  // 不确定
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
                                SPAparameter ofstParam = SPAparameter(iCheckParam);
                                SPAparameter baseParam = SPAparameter(iCheckParam);
                                if(ofstCoed->edge()->sense() == 1) ofstParam = baseParam.operator double();  // 不确定
                                if(baseCoed->edge()->sense() == 1) baseParam = baseParam.operator double();
                                double coedge_param = ofstParam.operator double();
                                SPAposition ofstPos = coedge_param_pos(ofstCoed, coedge_param);
                                SPAtransf v22;
                                SPAunit_vector ofstTan = normalise(coedge_param_dir(ofstCoed, double(ofstParam), v22, 1));
                                const curve& baseCurve = baseCoed->edge()->geometry()->equation();
                                SPAparameter adjustedBaseParam;
                                SPAposition basePos;
                                baseCurve.point_perp(ofstPos, basePos, baseParam, adjustedBaseParam, 0);  // 不確定
                                SPAtransf v25;
                                SPAunit_vector baseTan = normalise(coedge_param_dir(baseCoed, double(adjustedBaseParam), v25, 1));
                                return baseTan % ofstTan < 0.0;
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
int check_split_positoin(curve* iCurveGeom, const double iSplitParam, const double iStartParam, const double iEndParam, SPAposition& iSplitPos, SPAposition& iStartPos, SPAposition& iEndPos) {
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
// int sg_inter_ed_ed(EDGE* e1, EDGE* e2, curve_curve_int*& inters, double dist_tol, double angle_tol) {
//     typedef int (*sg_inter_ed_ed)(EDGE* e1, EDGE* e2, curve_curve_int*& inters, double dist_tol, double angle_tol);
//     sg_inter_ed_ed f = (sg_inter_ed_ed)apiFinderACIS.GetAddress("?sg_inter_ed_ed@@YAHPEAVEDGE@@0AEAPEAVcurve_curve_int@@NN@Z", prostate);
//     if(f) {
//         return f(e1, e2, inters, dist_tol, angle_tol);
//     } else {
//         return NULL;
//     }
//
// }  // 不兼容
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
                    int connectionId;
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
                    if(!connectionId) return (unsigned int)++ioConnectionId;
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
    sg_inter_ed_ed(e1, e2, cci, SPAresabs, SPAresnor);
    if(!cci) {
        if(trim_offset_wire_module_header.debug_level >= 40) acis_fprintf(debug_file_ptr, "sg_intersect_rate_segments() return: no ints\n");
        return 0;
    }
    curve_curve_int* this_cci = cci;
    int cci_count = 0;
    while(this_cci) {
        ++cci_count;
        if(c1_sense == 1) this_cci->param1 = -this_cci->param1;
        if(c2_sense == 1) this_cci->param2 = -this_cci->param2;
        this_cci = this_cci->next;
    }
    sg_sort_cci(cci, 0, 0);
    if(trim_offset_wire_module_header.debug_level >= 40) acis_fprintf(debug_file_ptr, "sg_intersect_rate_segments() %d ints\n", (unsigned int)cci_count);
    curve_curve_int* this_ccia = cci;
    curve_curve_int* next_cci = cci->next;
    if(c1->end() == c2->start()) {
        if(!next_cci) {
            if(trim_offset_wire_module_header.debug_level >= 40) acis_fprintf(debug_file_ptr, "sg_intersect_rate_segments() return: only junctions\n");

            // if(cci)
            //     cci-> ~curve_curve_int();
            // else
            //     cci = nullptr;
            return 0;
        }
        while(next_cci->next) {
            this_ccia = next_cci;
            next_cci = next_cci->next;
        }
        --cci_count;
        this_ccia->next = nullptr;
    }
    curve_curve_int* this_ccic = cci;  // 保存当前节点
    cci = cci->next;                   // 移动到下一个节点
    if(this_ccic != nullptr) {
        ACIS_DELETE this_ccic;  // 销毁当前节点对象
    }
    if(c1->start() != c2->end() || --cci_count, cci) {
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
                double dot = ((tan1 * tan2) % normal) * dist;
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
                rating_list[i].mIntersectionId = get_connectionId_at_intersection(s1, s2, this_ccib->int_point, ioConnectionId);
                if(trim_offset_wire_module_header.debug_level >= 40) {
                    acis_fprintf(debug_file_ptr, "sg_intersect_rate_segments(): set ratings %d: ", (unsigned int)i);
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
                int* valid_intr = ACIS_NEW int[cci_count];
            }

            SPAinterval paramRange1 = s1->coedge()->param_range();
            SPAinterval paramRange2 = s2->coedge()->param_range();
            if(is_overlap(cci)) overlap_correction(iSegList, s1, s2);
            for(int ie = 0; ie < cci_count; ++ie) {
                sg_seg_rating* crrRating = &rating_list[ie];
                int valid = valid_intersection(s1, s2, crrRating);
                valid_intr[ie] = valid;
                if(!valid_intr[ie] && cci_count == 2) {
                    COEDGE* iCEdge2 = s2->coedge();
                    COEDGE* iCEdge1 = s1->coedge();
                    int v23 = is_overlap_0(iCEdge1, iCEdge2, crrRating, paramRange1, paramRange2);  // 不懂
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
                    if(trim_offset_wire_module_header.debug_level >= 0x28) acis_fprintf(debug_file_ptr, "sg_intersect_rate_segments(): split-and-rated, 2nd\n");
                    this_seg = this_seg->next();
                }
            }
            if(valid_intr) {
                void* alloc_ptr = valid_intr;
                ACIS_STD_TYPE_OBJECT::operator delete[](valid_intr);
            }
            sg_delete_cci(cci);
            ACIS_DELETE[] rating_list;
            ACIS_DELETE[] index;
            if(resignal_no || acis_interrupted()) sys_error(resignal_no, error_info_base_ptr);

            return 1;
        }
    }

    else {
        if(trim_offset_wire_module_header.debug_level >= 40) acis_fprintf(debug_file_ptr, "sg_intersect_rate_segments() return: culled all junctions\n");
        int v59 = 0;
        return v59;
    }
}
double find_max_curv_param(curve* cu, SPAinterval& search_int, SPAposition& max_curv_pos) {
    double param1 = search_int.start_pt();
    double param2 = search_int.end_pt();
    double curv1_sq = cu->eval_curvature(param1).len_sq();  // 不确定param1
    double curv2_sq = cu->eval_curvature(param2).len_sq();  // 不确定param1
    SPAposition pos1 = cu->eval_position(param1);           // 不确定param1
    SPAposition pos2 = cu->eval_position(param2);           // 不确定param1

    double param_mid = curv1_sq / (curv1_sq + curv2_sq) * param1 + curv2_sq / (curv1_sq + curv2_sq) * param2;
    double curv_mid_sq = cu->eval_curvature(param_mid).len_sq();

    while(param2 - param1 > double(SPAresnor)) {
        if(curv2_sq <= curv1_sq) {
            double v29 = curv_mid_sq / (curv_mid_sq + curv2_sq) * param_mid + curv2_sq / (curv_mid_sq + curv2_sq) * param2;
            if(double(SPAresnor) > v29 - param_mid || double(SPAresnor) > param2 - v29) {
                v29 = 0.5 * (param_mid + param2);
            }
            double v31 = cu->eval_curvature(v29).len();
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
                pos2 = cu->eval_position(param2);  // 不确定，还有上面的也不太确定
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
    return (unsigned int)this->start_is_out;
}
int offset_segment::end_out() {
    return (unsigned int)this->end_is_out;
}
int offset_segment::inside() {
    return !this->start_is_out && !this->end_is_out;
}
int offset_segment::outside() {
    return this->start_is_out && this->end_is_out;  // 不确定
}
int offset_segment::reversed() {
    return (unsigned int)this->seg_reversed;
}
int offset_segment::singular_offset() {
    return (unsigned int)this->is_singular;
}
int offset_segment::simple_offset() {
    BOOL v4 = 1;  // [rsp+20h] [rbp-18h]
    if(!this->singular_offset()) {
        if(is_INTCURVE(this->seg_coedge->edge()->geometry())) return 0;
    }
    return v4;
}
int offset_segment::extension() {
    return (unsigned int)this->is_extension;
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
    long double start_param = double(started);
    long double end_param = double(seg_edge->end_param());
    if(this->seg_coedge->sense() == 1) {
        long double tmp_param = start_param;
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

    int split_position = check_split_positoin(&((curve&)edgeCurve), param2, start_param, end_param, cci->int_point, iStartPos, iEndPos);
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
    if((unsigned int)this->extension()) new_seg->set_extension();
    new_seg->set_next(this->next());
    new_seg->set_previous(this);
    this->set_next(new_seg);
    new_seg->next()->set_previous(new_seg);
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
    if(!this->coedge_offset() || (unsigned int)this->singular_offset()) return 1;
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
                    } else {
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
                        v212->remove();
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
                    offcrv->eval((*cpoints + double(seg_edge->start_param())) / 2.0, pt, dpt, ddpt);
                    given_curve->eval((*cpoints + double(seg_edge->start_param())) / 2.0, upt, udpt, uddpt);
                    if((dpt % udpt) >= 0.0) {
                        domain_center[dom_size] = double(seg_edge->start_param()) + root_tolerances->ROOT_TOL_SEP * 2.0;
                        int dom_sized = dom_size + 1;
                        domain_center[dom_sized] = cpoints[0] - root_tolerances->ROOT_TOL_SEP * 2.0;
                        dom_size = dom_sized + 1;
                    } else {
                        identity_law* v298 = ACIS_NEW identity_law(0, 88);
                        law* i0 = v298;
                        identity_law* v300 = ACIS_NEW identity_law(1, 88);
                        law* v214 = v300;
                        constant_law* v302 = ACIS_NEW constant_law(*cpoints);
                        law* ci = v302;
                        constant_law* v304 = ACIS_NEW constant_law(double(seg_edge->start_param()));
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
                        v216->remove();
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
                        domain_center[dom_sizec] = double(seg_edge->end_param()) - root_tolerances->ROOT_TOL_SEP * 2.0;
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
                        if(fabs(seg_edge->end_param().operator double() - cpoints[m]) > double(SPAresabs)) {
                            domain_center[dom_size++] = seg_edge->end_param() - root_tolerances->ROOT_TOL_SEP * 2.0;
                        }
                    } else {
                        identity_law* v323 = ACIS_NEW identity_law(0, 88);
                        law* v217 = v323;
                        identity_law* v325 = ACIS_NEW identity_law(1, 88);
                        law* v218 = v325;
                        constant_law* v327 = ACIS_NEW constant_law(cpoints[m]);
                        law* v219 = v327;
                        constant_law* v329 = ACIS_NEW constant_law(double(seg_edge->end_param()));
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
                        min_law* v345 = ACIS_NEW min_law(v651, 2);
                        domain_law = v345;
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
            v563->set(seg_edge->end_param() - SPAresabs * 10.0, seg_edge->start_param() + double(SPAresabs) * 10.0);
            ++excount;
            rec2d rec(seg_edge->start_param(), seg_edge->end_param().operator double(), seg_edge->start_param().operator double(), seg_edge->end_param().operator double());
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
                curve_curve_int* v362 = ACIS_NEW curve_curve_int(list, pt, double(seg_edge->start_param()), double(seg_edge->start_param()));
                list = v362;
                curve_curve_int* v364 = ACIS_NEW curve_curve_int(list, pt, seg_edge->end_param(), seg_edge->end_param());
                list = v364;
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
            // curve_curve_int* v368 = next_cci;
            // ACIS_DELETE v368; // 不确定不确定
            /*   v616 = next_cci ? curve_curve_int::`scalar deleting destructor'(v368, 1u) : 0i64; this_cci->next = 0i64;*/
            this_cci = list;
            list = list->next;
            // curve_curve_int* v369 = this_cci;
            // ACIS_DELETE v369;       // 不确定不确定
            // v617 = this_cci ? curve_curve_int::`scalar deleting destructor'(v369, 1u) : 0i64;
            v619 = ACIS_NEW sg_seg_rating;  // 不确定
            rating_list = v619;
            cci_count = 0;
            if(list) {
            LABEL_318:
                DEBUG_LEVEL(DEBUG_CALLS) sg_debug_cci(list, debug_file_ptr, "self-cci:  ");
                sg_reorder_self_cci(list);

                for(this_cci = list; this_cci; this_cci = this_cci->next) ++cci_count;

                this_cci = list;
                for(int i = 0; i < cci_count; ++i) {
                    SPAunit_vector tan1 = coedge_param_dir(this->seg_coedge, this_cci->param1);
                    SPAunit_vector tan2 = coedge_param_dir(this->seg_coedge, this_cci->param2);
                    double dot = (tan1 * tan2) % normal * (dist_law->eval(this_cci->param1));
                    if(this->seg_coedge->sense() == 1) dot = -dot;
                    if(dot <= double(SPAresabs)) {
                        if((-double(SPAresabs)) <= dot)  //
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
            DEBUG_LEVEL(DEBUG_CALLS) {
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
        SPAtransf* iTransf;
        intcurve* offsetCurve = (intcurve*)get_curve_from_coedge(this->seg_coedge, edgeParamRange, *iTransf);

        int curveClosed = segEdge->start() == segEdge->end();
        int removeContainment = 0;
        if(baseEdge) {
            if(iEdgeSmoothMgr) {
                int smoothLevel = iEdgeSmoothMgr->get_edge_smooth_level(baseEdge, removeContainment);
                curve* newOfstCurve = smooth_curve1(offsetCurve, normal, smoothLevel, dist_law);
                if(newOfstCurve) {
                    set_curve_to_coedge(this->seg_coedge, newOfstCurve);
                    offsetCurve = (intcurve*)newOfstCurve;
                    newOfstCurve = nullptr;
                }
            }
        }

        curve_curve_int* intersectionsList = get_curve_self_ints(*offsetCurve, double(SPAresabs));
        if(baseEdge && iEdgeSmoothMgr) iEdgeSmoothMgr->set_edge_intersection_info(baseEdge, intersectionsList);
        if(curveClosed) {
            SPAposition pt;
            SPAvector dpt;
            SPAvector ddpt;
            double d1 = double(segEdge->start_param());
            double ep = double(segEdge->end_param());
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
            this->split_and_rate(mainList, dist_law, normal, ioStartConnectionId, double(SPAresabs), iSingleSegment, mayHaveSelfIntersections);
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
    ((SPAposition)(this->seg_coedge->start()->geometry()->coords())).debug(dfp);
    acis_fprintf(dfp, " ) to ( ");
    ((SPAposition)(this->seg_coedge->end()->geometry()->coords())).debug(dfp);  // 不确定
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
            given_curve = baseCurve.copy_curve();
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
                const SPAposition v39 = this->coedge()->start()->geometry()->coords();
                if((pos - v39).len() > double(SPAresabs)) {
                    if((pos - this->coedge()->start()->geometry()->coords()).len() > 10.0 * double(SPAresabs)) {
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
                if(discs[iia] > started + double(SPAresnor)) {
                    if((range.end_pt() - double(SPAresnor)) > discs[iia]) {
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
                            offcrv->evaluate(started, p1, derivs, 1, evaluate_curve_side::evaluate_curve_below);
                            derivs[0] = &deriv_high;
                            offcrv->evaluate(range.end_pt(), p1, derivs, 1, evaluate_curve_side::evaluate_curve_above);

                            if((deriv_low % deriv_high) < 0.0) {
                                if(SPAvector(p1 - this->coedge()->start()->geometry()->coords()).len_sq() > double(SPAresabs) * double(SPAresabs)) {
                                    if(SPAvector(p1 - this->coedge()->end()->geometry()->coords()).len_sq() > double(SPAresabs) * double(SPAresabs)) {
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
            offcrv->eval(ep, upt);  // 不确定
            offcrv->eval(minx, pt);
            int closed_curve = 0;
            double* v155 = ACIS_NEW double[psize + 1];
            double* cpoints = v155;
            if(pt == upt) {
                closed_curve = 1;
                for(int index = 0; index < psize; ++index) cpoints[index] = cpoints_temp[index];
                double x = ep - double(SPAresabs) / 100.0;
                double rc_end = cur_law->eval(x);
                double rc_start = cur_law->eval(minx);
                double dist = fabs_dist->eval(ep);
                if(dist > 0.0) {
                    double rc_test = 1.0 / dist;
                    if(rc_end > rc_start && rc_test >= rc_start && rc_end > rc_test) {
                        cpoints[psize++] = double(segEdge->end_param());
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
            exception_save(exception_save_mark);

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
            double* v169 = ACIS_NEW double[wsize + psize];
            double* tpoints = v169;
            int tp_count;
            for(tp_count = 0; tp_count < wsize; ++tp_count) tpoints[tp_count] = wpoints[tp_count];
            void* alloc_ptr = wpoints;
            ACIS_DELETE[] STD_CAST wpoints;

            for(int i = 0; i < psize; ++i) {
                offcrv->eval(tpoints[wsize + i], pt, dpt, ddpt, 0, 0);          // 不确定不确定
                given_curve->eval(tpoints[wsize + i], upt, udpt, uddpt, 0, 0);  // 不确定不确定
                double testd = iDistLaw->eval(cpoints[i]);
                int flipped = 0;
                if(closed_curve) {
                    if(cpoints[i] == double(segEdge->end_param())) {
                        SPAposition ept;
                        SPAposition eupt;
                        SPAvector edpt;
                        SPAvector eudpt;
                        offcrv->eval(cpoints[i], ept, edpt);
                        given_curve->eval(cpoints[i] - double(SPAresabs), eupt, eudpt);
                        given_curve->eval(cpoints[i] - double(SPAresabs), eupt, eudpt);
                        if((edpt % eudpt) < 0.0) flipped = 1;
                    }
                }
                if(!flipped) {
                    if(testd >= 0.0 || (udpt * uddpt % iNormal <= 0.0)) {
                        if(testd < 0.0) continue;
                        if(udpt * uddpt % iNormal >= 0.0) continue;
                    }
                }
                tpoints[tp_count++] = cpoints[i];
                if(closed_curve) {
                    if(fabs(cpoints[i] - ep) <= double(SPAresabs)) continue;
                    if(fabs(cpoints[i] - minx) <= double(SPAresabs)) continue;
                }
                curve_curve_int* v125 = ACIS_NEW curve_curve_int(ioList, pt, cpoints[i], cpoints[i]);
                ioList = v125;
            }

            ACIS_DELETE[] STD_CAST tpoints;  // 不确定
            if(delete_cpoints_temp) {
                ACIS_DELETE[] STD_CAST cpoints_temp;
            }
            ACIS_DELETE[] STD_CAST cpoints;
        }
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
        DEBUG_LEVEL(DEBUG_DETAILS)
        sg_debug_cci(ioIntersections, debug_file_ptr, "self-cci:  ");
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

offset_segment_list::offset_segment_list(law* dist, SPAunit_vector& normal, int add_attribs, sg_gap_type use_type, int iNonPlanar) {
    this->offset_normal = SPAunit_vector();
    this->head_node = offset_segment();
    this->tail_node = offset_segment();
    this->dist_law = dist;
    this->dist_law->add();
    this->offset_normal = normal;
    this->add_attributes = add_attribs;
    if(this->dist_law->constant()) {
        dist_law = this->dist_law;
        this->mOffsetDist = dist_law->eval(1.0);
    }
    this->head_node.set_next(&this->tail_node);
    this->tail_node.set_previous(&this->head_node);
    this->count = 0;
    this->close_type = use_type;
    this->mLastSegmentConnectionId = 0;
    this->mStartSegmentConnectionId = 0;
    this->mBaseWire = nullptr;
    this->mTrimInsideWireDone = NULL;
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
void offset_segment_list::set_normal(SPAunit_vector& v) {
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
    return 0;
    // int intersection_found = 0;
    // if(new_segment) {
    //     COEDGE* new_coedge = new_segment->coedge();
    //     if(new_coedge) {
    //         EDGE* pNewEdge = new_coedge->edge();
    //         if(pNewEdge) {
    //             int bContinue = 1;
    //             if(is_intcurve_edge(pNewEdge)) {
    //                 const intcurve& rIntcurve = (const intcurve&)(pNewEdge->geometry()->equation());
    //                 bContinue = (((intcurve&)rIntcurve)).cur(-1.0, 0) != NULL;
    //             }
    //             if(bContinue) {
    //                 offset_segment* last_node = this->tail_node.previous();
    //                 last_node->set_next(new_segment);
    //                 new_segment->set_previous(last_node);
    //                 new_segment->set_next(&this->tail_node);
    //                 offset_segment* p_tail_node = &this->tail_node;
    //                 this->tail_node.set_previous(new_segment);
    //                 ++this->count;
    //                 SPAunit_vector orig_tan;
    //                 SPAunit_vector new_tan;
    //                 SPAposition origPos(0.0, 0.0, 0.0);
    //                 SPAposition newPos(0.0, 0.0, 0.0);
    //                 if(new_segment->singular_offset()) {
    //                     new_tan = normalise(SPAvector(0.0, 0.0, 0.0));
    //                     new_segment->set_start_in();
    //                     new_segment->set_end_in();
    //                 } else {
    //                     if(new_segment->vertex_offset()) {
    //                         new_tan = coedge_start_dir(new_coedge);
    //                         newPos = coedge_start_pos(new_coedge);
    //                     } else {
    //                         new_tan = coedge_mid_dir(new_coedge);
    //                         newPos = coedge_mid_pos(new_coedge);
    //                     }
    //                 }
    //                 if(new_segment->vertex_offset()) {
    //                     COEDGE* last_coedge = last_node->coedge();
    //                     COEDGE* last_orig_coedge = nullptr;
    //                     if(last_coedge) last_orig_coedge = last_node->original_coedge();
    //                     int from_corner_gap = is_STRAIGHT(new_coedge->edge()->geometry());
    //                     if(!last_orig_coedge || from_corner_gap) {
    //                         orig_tan = new_tan;
    //                     } else {
    //                         orig_tan = normalise(coedge_end_dir(last_orig_coedge));
    //                         origPos = coedge_end_pos(last_orig_coedge);
    //                     }
    //                 } else {
    //                     COEDGE* pOriginalCoed = new_segment->original_coedge();
    //                     if(pOriginalCoed) {
    //                         SPAparameter origParam;
    //                         int paramComputed = 0;
    //                         if(this->mNonPlanar) {
    //                             double initEParam;
    //                             double initSParam;
    //                             new_segment->get_init_params(initSParam, initEParam);
    //                             if(initSParam != initEParam) {
    //                                 double sParam = new_coedge->param_range().start_pt();
    //                                 double eParam = new_coedge->param_range().end_pt();
    //                                 if(sParam != initSParam || eParam != initEParam) {
    //                                     double origSParam = pOriginalCoed->param_range().start_pt();
    //                                     double origEParam = pOriginalCoed->param_range().end_pt();
    //                                     double pmRatio = (0.5 * (eParam + sParam) - initSParam) / (initEParam - initSParam);
    //                                     origParam = SPAparameter(origSParam + pmRatio * (origEParam - origSParam));
    //                                     paramComputed = 1;
    //                                 }
    //                             }
    //                         }
    //                         if(!paramComputed) {
    //                             SPAparameter offsParam(new_coedge->edge()->param_range().mid_pt());
    //                             int do_point_perp = 1;
    //                             if(new_coedge->edge() && new_coedge->edge()->geometry()) {
    //                                 if(is_intcurve(&(new_coedge->edge()->geometry()->equation())))  //&不确定
    //                                 {
    //                                     const int_cur& int_cur = ((intcurve&)new_coedge->edge()->geometry()->equation()).get_int_cur();
    //                                     if(int_cur.type() == offset_int_cur::id()) do_point_perp = 0;  // 不确定
    //                                 }
    //                             } else {
    //                                 do_point_perp = 0;
    //                             }
    //                             if(do_point_perp) {
    //                                 SPAposition posOnOffset = new_coedge->edge()->mid_pos();
    //                                 SPAposition posOnOriginal;
    //                                 const curve& origCurve = pOriginalCoed->edge()->geometry()->equation();
    //                                 SPAparameter guessParam(pOriginalCoed->edge()->param_range().mid_pt());
    //                                 if(pOriginalCoed->edge()->sense() == 1) {
    //                                     offsParam = -offsParam;
    //                                     guessParam = -guessParam;
    //                                 }
    //                                 origCurve.point_perp(posOnOffset, posOnOriginal, offsParam, origParam, 0);
    //                                 SPAposition tmpPosOnOriginal;
    //                                 SPAparameter tmpOrigParam;
    //                                 origCurve.point_perp(posOnOffset, tmpPosOnOriginal, guessParam, tmpOrigParam, 0);
    //                                 if((posOnOffset - posOnOriginal).len_sq() > (posOnOffset - tmpPosOnOriginal).len_sq()) origParam = tmpOrigParam;
    //                                 if(pOriginalCoed->edge()->sense() == 1) origParam = -origParam;
    //                             } else {
    //                                 origParam = offsParam;
    //                             }
    //                             if(pOriginalCoed->sense() == 1) origParam = -origParam;
    //                         }
    //                         double coedge_param = origParam.operator double();
    //                         orig_tan = normalise(coedge_param_dir(pOriginalCoed, coedge_param));
    //                         SPAtransf v106;
    //                         origPos = coedge_param_pos(pOriginalCoed, double(origParam), v106, 1);  // 完善
    //                     }
    //                 }
    //                 int numOf_org_edges = this->get_number_of_offset_coedges();
    //                 int set_prev_next_flag = GET_ALGORITHMIC_VERSION() <= AcisVersion(32, 0, 1) || numOf_org_edges == 1;
    //                 int showTans = 0;
    //                 if(orig_tan % new_tan < 0.0) {
    //                     if(!this->mNonPlanar || 2.0 * this->mOffsetDist > (origPos - newPos).len()) {
    //                         if(set_prev_next_flag && new_segment->simple_offset()) last_node->set_end_in();
    //                         new_segment->set_start_in();
    //                         new_segment->set_end_in();
    //                         new_segment->set_reversed();
    //                         showTans = 1;
    //                     }
    //                 }
    //                 if(set_prev_next_flag && last_node->reversed() && last_node->simple_offset()) {
    //                     new_segment->set_start_in();
    //                 }
    //                 if(new_segment->reversed() || new_segment->singular_offset()) {
    //                     return NULL;
    //                 }
    //                 offset_segment* this_segment = this->head_node.next();
    //                 offset_segment* next_segment = this_segment->next();
    //                 offset_segment* tail_ptr = &this->tail_node;
    //                 while(this_segment != new_segment) {
    //                     if(!this_segment->reversed() && !this_segment->singular_offset()) {
    //                         offset_segment* sub_segment = new_segment;
    //                         offset_segment* next_sub = new_segment->next();
    //                         while(sub_segment != tail_ptr) {
    //                             if(sg_intersect_rate_segments(this, this_segment, sub_segment, this->dist_law, this->offset_normal, this->close_type, this->mLastSegmentConnectionId) == 1) intersection_found = 1;
    //                             sub_segment = next_sub;
    //                             next_sub = next_sub->next();
    //                         }
    //                     }
    //                     this_segment = next_segment;
    //                     next_segment = next_segment->next();
    //                 }
    //             }
    //         }
    //     }
    // }
    // return intersection_found;
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
            if((int)thisSeg->get_end_connectionId() < 1) {
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