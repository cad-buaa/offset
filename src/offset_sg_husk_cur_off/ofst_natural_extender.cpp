#include "PrivateInterfaces/ofst_natural_extender.hxx"

#include "ProtectedInterfaces/ofsttools.hxx"
#include "acis/api.hxx"
#include "acis/cnstruct.hxx"
#include "acis/cucuint.hxx"
#include "acis/curdef.hxx"
#include "acis/curextnd.hxx"
#include "acis/curve.hxx"
#include "acis/geom_utl.hxx"
#include "acis/geometry.hxx"
#include "acis/intdef.hxx"
#include "acis/vrtx_utl.hxx"
#include "acis//allcurve.hxx"

void release_intersection_data(curve_curve_int* mIntsectData) {
    curve_curve_int* nextIntsectData;
    curve_curve_int* mIntsectDataa = mIntsectData;
    if(mIntsectData) {
        while(mIntsectDataa) {
            nextIntsectData = mIntsectDataa->next;
            mIntsectDataa->~curve_curve_int();
            mIntsectDataa = nextIntsectData;  // 不确定
        }
    }
}
int extend_offset_int_cur(curve* mCrvGeom, SPAinterval& newInterval) {

     int extOk = 0;

     offset_int_cur& ofstIntcur = (offset_int_cur&)((intcurve*)mCrvGeom)->get_int_cur();  
     if(&ofstIntcur) {
         SPAinterval retIntval = ofstIntcur.extend(newInterval);
         if(retIntval == newInterval) {
             mCrvGeom->change_event();
             return 1;
         }
     }
     return extOk;
}
int extend_curve_geom(curve* mCrvGeom, const double iExtParamLen, const int mCurve1AtStart) {
    const void* v4;        // rax
    extension_info* info;  // rax

    if(iExtParamLen <= 0.0) return 1;
    int extendOk = 1;
    SPAinterval newInterval = mCrvGeom->param_range();
    SPAinterval deltaInterval;
    SPAinterval v12;
    if(mCurve1AtStart) {
        v12 = SPAinterval(-iExtParamLen, 0.0);
    } else {
        v12 = SPAinterval(0.0, iExtParamLen);
    }
    deltaInterval = v12;
    newInterval += deltaInterval;
    SPAinterval checkInterval = extend_curve(*mCrvGeom, newInterval, 1);
    if((checkInterval != newInterval) && !mCrvGeom->periodic()) extendOk = 0;
    if(!extendOk) return extend_offset_int_cur(mCrvGeom, newInterval);
    return extendOk;
}

void debug_display_natural_extender(COEDGE* iCoedge1, COEDGE* iCoedge2, curve_curve_int* iIntersections, curve* iCurve1, curve* iCurve2) {
    return;
}

int comp_curve_end_info(curve* iCurve, SPAposition& iEndPos, SPAvector& oEndTangent) {
    /* if(!iCurve) _wassert(L"iCurve", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_natural_extender.cpp", 0x33Fu);*/

    double endParam = iCurve->param(iEndPos);
    SPAposition tmpPos;  // 不懂
    iCurve->eval(endParam, tmpPos, oEndTangent);
    double v18 = abs(endParam - iCurve->param_range().end_pt());
    double started = iCurve->param_range().start_pt();
    return v18 > abs(endParam - started);
}
int beyond_intesect(SPAposition& iRefPos, SPAposition& iIntsectPos, SPAposition& iCheckPos) {
    SPAvector v2 = iIntsectPos - iRefPos;
    SPAvector v1 = iCheckPos - iRefPos;
    double v6 = -double(SPAresabs);
    return v6 > (v1 % v2);
}
ofst_natural_extender::ofst_natural_extender() {
    this->mExtensionType = NULL;
    this->mFailCase = NULL;
    this->mCommonVrtPos = SPAposition();
    this->mC1End = SPAposition();
    this->mC2Start = SPAposition();
    this->mIntersectPoint = SPAposition(0.0, 0.0, 0.0);
    this->mOfstSegment = nullptr;
    this->mDistLaw = nullptr;
    this->mC1 = nullptr;
    this->mC2 = nullptr;
    this->mCommVertex = nullptr;
    this->mMainCurve = nullptr;
    this->mC1Geom = nullptr;
    this->mC2Geom = nullptr;
}
ofst_natural_extender::~ofst_natural_extender() {
    curve* mMainCurve;  // [rsp+20h] [rbp-58h]
    curve* mC1Geom;     // [rsp+28h] [rbp-50h]
    curve* mC2Geom;     // [rsp+30h] [rbp-48h]

    this->mDistLaw = nullptr;
    this->mOfstSegment = nullptr;
    this->mC1 = nullptr;
    this->mC2 = nullptr;
    this->mCommVertex = nullptr;
    if(this->mMainCurve) {
        ACIS_DELETE this->mMainCurve;  // 不确定
    }
    if(this->mC1Geom) {
        ACIS_DELETE this->mC1Geom;
    }
    if(this->mC2Geom) {
        ACIS_DELETE this->mC2Geom;
    }
}
int ofst_natural_extender::extend(offset_segment* iOfstSegment, law* iDistLaw, COEDGE* iFstOfstCoedge, COEDGE* iSndOfstCoedge, VERTEX* iCommonVeertex, int iNinimumTopo, int skipbackup) {
 
     double Curve2ExtParamLen;  
     double Curve1ExtParamLen;  
     long double minParamLength;

     int retOk = 0;
     if(this->init(iOfstSegment, iDistLaw, iFstOfstCoedge, iSndOfstCoedge, iCommonVeertex))
    {
         retOk = this->try_old_extension();
         if(!retOk && this->comp_init_extension_length(Curve1ExtParamLen, Curve2ExtParamLen))
         {
             retOk = this->try_new_extension(Curve1ExtParamLen, Curve2ExtParamLen);
             if(!retOk && this->mFailCase == 4)
             {
                 Curve1ExtParamLen = Curve1ExtParamLen * 2.0;
                 Curve2ExtParamLen = Curve2ExtParamLen * 2.0;
                 retOk = this->try_new_extension(Curve1ExtParamLen, Curve2ExtParamLen);
             }
             //if(!retOk && this->mFailCase == 4)
             //{
             //    minParamLength = ; 
             //    if(Curve1ExtParamLen <= 0.0 || Curve1ExtParamLen >= 0.01) Curve1ExtParamLen = Curve1ExtParamLen * 2.0;
             //    else Curve1ExtParamLen = UNDEFC::`vftable'[45]; 
             //    if(Curve2ExtParamLen <= 0.0 || Curve2ExtParamLen >= 0.01) Curve2ExtParamLen = Curve2ExtParamLen * 2.0;
             //    else Curve2ExtParamLen = *(double*)& UNDEFC::`vftable'[45]; 
             //    retOk = this->try_new_extension(Curve1ExtParamLen, Curve2ExtParamLen);
             //}
         }
         if(retOk) this->mExtensionType = 2;
     }
     if(!retOk && !iNinimumTopo && !skipbackup) {
         retOk = this->extend_with_arc();
         if(retOk) this->mExtensionType = 1;
     }
     return retOk;
}
int ofst_natural_extender::get_extension_type() {
    return this->mExtensionType;
}
int ofst_natural_extender::try_old_extension() {
    int retOk = 0;
    SPAvector vDst1;
    SPAvector vDst2;
    SPAposition p = this->mCommonVrtPos;
    comp_vdist(this->mCommonVrtPos, this->mC1End, this->mC2Start, vDst1, vDst2);  // 不懂
    intcurve* mC1GeomCopy = nullptr;
    intcurve* mC2GeomCopy = nullptr;
    int copied_curves = 0;
    bool v2 = GET_ALGORITHMIC_VERSION() > AcisVersion(32, 0, 0);
    if(v2) {
        if(this->mC1Geom->type() == 11) {
            mC1GeomCopy = (intcurve*)this->mC1Geom->copy_curve();
            mC1GeomCopy->make_single_ref();
        }
        if(this->mC2Geom->type() == 11) {
            mC2GeomCopy = (intcurve*)this->mC2Geom->copy_curve();
            mC2GeomCopy->make_single_ref();
        }
    } else if(this->mC1Geom->type() == 11 && this->mC2Geom->type() == 11) {
        mC1GeomCopy = (intcurve*)this->mC1Geom->copy_curve();
        mC2GeomCopy = (intcurve*)this->mC2Geom->copy_curve();
        copied_curves = 1;
    }
    set_global_error_info();
    outcome result(0);
    problems_list_prop problems_prop;
    acis_exception error_info_holder(0);
    error_info_base* error_info_base_ptr = nullptr;
    exception_save exception_save_mark;
    api_bb_save make_bulletin_board(result, api_bb_save::e_bb_type::trial);  //
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    ACISExceptionCheck("API");
    int call_update_from_bb = 1;
    if(sg_extend_coedges(this->mDistLaw, this->mC1End, this->mC1, this->mC1Geom, vDst1) && sg_extend_coedges(this->mDistLaw, this->mC2Start, this->mC2, this->mC2Geom, vDst2)) {
        curve_curve_int* intersections = this->comp_intersections();
        if(intersections) {
            retOk = this->do_extension(intersections);
            release_intersection_data(intersections);
        } else {
            this->mFailCase = 4;
        }
    } else {
        this->mFailCase = 1;
    }
    if(!retOk) {
        outcome v(42, (error_info*)0);
        result = v;  // 不懂
    }
    if(result.ok() && call_update_from_bb) update_from_bb();
    if(acis_interrupted()) sys_error(0, error_info_base_ptr);
    result.error_number();
    problems_prop.process_result(result, PROBLEMS_LIST_PROP_OR_IGNORE, 0);
    if(v2) {
        if(mC1GeomCopy) {
            if(result.ok()) {
                mC1GeomCopy=nullptr;
            } else {
                const int_cur& ic1 = mC1GeomCopy->get_int_cur();
                intcurve* mC1Geom = (intcurve*)this->mC1Geom;
                mC1Geom->replace_int_cur((int_cur&)ic1);
                this->mC1Geom->change_event();

            }
        }
        if(mC2GeomCopy) {
            if(result.ok()) {
                mC2GeomCopy = nullptr;
            } else {
                const int_cur& ic2 = mC2GeomCopy->get_int_cur();
                intcurve* mC2Geom = (intcurve*)this->mC2Geom;
                mC2Geom->replace_int_cur((int_cur&)ic2);
                this->mC2Geom->change_event();
                mC2GeomCopy = nullptr;
            }
        }
    } else if(copied_curves) {
        if(result.ok()) {
        } else {
            const int_cur& new_fit = mC1GeomCopy->get_int_cur();
            const int_cur& intcur = mC2GeomCopy->get_int_cur();
            intcurve* v46 = (intcurve*)this->mC1Geom;
            v46->replace_int_cur((int_cur&)new_fit);
            intcurve* v48 = (intcurve*)this->mC2Geom;
            v48->replace_int_cur((int_cur&)intcur);
            this->mC1Geom->change_event();
            this->mC2Geom->change_event();
            intcurve* v15 = mC1GeomCopy;
        }
    }
    return retOk;
}
int ofst_natural_extender::try_new_extension(const double iCurve1ExtParamLen, const double iCurve2ExtParamLen) {
    /* if(!this->mC1 || !this->mC1Geom || !this->mC2 || !this->mC2Geom) _wassert(L"mC1 && mC1Geom != NULL && mC2 && mC2Geom != NULL", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_natural_extender.cpp", 0x22Eu);*/
    int retOk = 0;
    intcurve* mC1GeomCopy = nullptr;
    intcurve* mC2GeomCopy = nullptr;
    if(GET_ALGORITHMIC_VERSION() > AcisVersion(32, 0, 0)) {
        if(this->mC1Geom->type() == 11) {
            mC1GeomCopy = (intcurve*)this->mC1Geom->copy_curve();
            mC1GeomCopy->make_single_ref();
        }
        if(this->mC2Geom->type() == 11) {
            mC2GeomCopy = (intcurve*)this->mC2Geom->copy_curve();
            mC2GeomCopy->make_single_ref();
        }
    }
    if(extend_curve_geom(this->mC1Geom, iCurve1ExtParamLen, this->mCurve1AtStart) && extend_curve_geom(this->mC2Geom, iCurve2ExtParamLen, this->mCurve2AtStart)) {
        debug_display_natural_extender(this->mC1, this->mC2, nullptr, this->mC1Geom, this->mC2Geom);
        set_global_error_info();
        outcome result(0, (error_info*)0);
        problems_list_prop problems_prop;
        acis_exception error_info_holder(0);
        error_info_base* error_info_base_ptr = nullptr;
        exception_save exception_save_mark;
        api_bb_save make_bulletin_board(result, api_bb_save::e_bb_type::trial);
        exception_save_mark.begin();
        get_error_mark().buffer_init = 1;
        ACISExceptionCheck("API");
        int call_update_from_bb = 1;

        this->mC1->edge()->set_geometry(make_curve(*this->mC1Geom), 1);
        this->mC2->edge()->set_geometry(make_curve(*this->mC2Geom), 1);

        curve_curve_int* intersections = this->comp_intersections();
        if(intersections) {
            debug_display_natural_extender(this->mC1, this->mC2, intersections, nullptr, nullptr);
            this->filter_intersections(intersections);
            if(intersections) {
                retOk = this->do_extension(intersections);
                release_intersection_data(intersections);
                intersections = nullptr;
            } else {
                this->mFailCase = 2;
            }
        } else {
            this->mFailCase = 4;
        }
        if(!retOk) {
            outcome v53(42);
            result = v53;
        }
        if(result.ok() && call_update_from_bb) update_from_bb();
        if(acis_interrupted()) sys_error(0, error_info_base_ptr);
        result.error_number();
        problems_prop.process_result(result, PROBLEMS_LIST_PROP_OR_IGNORE, 0);
    } else {
        this->mFailCase = 1;
    }
    if(mC1GeomCopy) {
        if(retOk) {
            mC1GeomCopy = nullptr;
        } 
        else {
            const int_cur& ic1 = mC1GeomCopy->get_int_cur();
            intcurve* mC1Geom = (intcurve*)this->mC1Geom;
            mC1Geom->replace_int_cur((int_cur&)ic1);
            this->mC1Geom->change_event();
            this->mC1->edge()->set_geometry(make_curve(*this->mC1Geom), 1);
            if(mC1GeomCopy)
            {
               mC1GeomCopy = nullptr;
            }
        }
    }
    if(mC2GeomCopy) {
        if(retOk) {
            mC2GeomCopy = nullptr;
        } 
        else {
            const int_cur& ic2 = mC2GeomCopy->get_int_cur();
            intcurve* mC2Geom = (intcurve*)this->mC2Geom;
            mC2Geom->replace_int_cur((int_cur&)ic2);
            this->mC2Geom->change_event();
            this->mC2->edge()->set_geometry(make_curve(*this->mC2Geom), 1);
            if(mC2GeomCopy)
            {
                mC2GeomCopy = nullptr;
            }
        }
    }
    return retOk;
}
int ofst_natural_extender::extend_with_arc() {
    int retOk = 0;

    if((this->mC1End - this->mC2Start).len() > SPAresfit) {
        sg_close_with_arc(this->mOfstSegment, this->mDistLaw, this->mCommonVrtPos, this->mC1End, this->mC2Start, this->mC1, this->mC2, this->mCommVertex);
        return 1;
    }
    return retOk;
}
int ofst_natural_extender::get_coedge_geomtry() {
    int retOk = 0;
    if(this->mC1) {
        if(this->mC2) {
            this->mC1End = get_vertex_position(this->mC1->end());
            this->mC2Start = get_vertex_position(this->mC2->start());
            this->mCommonVrtPos = get_vertex_position(this->mCommVertex);
            this->mCurve1AtStart = this->mC1->sense() == 1;
            this->mCurve2AtStart = this->mC1->sense() != 1;
            if(this->mC1->edge()) {
                if(this->mC2->edge()) {
                    CURVE* c1 = this->mC1->edge()->geometry();
                    CURVE* c2 = this->mC2->edge()->geometry();
                    if(c1) {
                        if(c2) {
                            curve* v6;
                            if(this->mC1 == this->mC2) {
                                this->mMainCurve = c1->trans_curve();
                                double midpt = this->mC1->edge()->param_range().mid_pt();  // 不确定
                                this->mC1Geom = this->mMainCurve->copy_curve();
                                v6 = this->mC1Geom->split(midpt);
                            } else {
                                this->mC1Geom = c1->trans_curve();
                                v6 = c2->trans_curve();
                            }
                            this->mC2Geom = v6;
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return retOk;
}
int ofst_natural_extender::init(offset_segment* iOfstSegment, law* iDistLaw, COEDGE* iFstOfstCoedge, COEDGE* iSndOfstCoedge, VERTEX* iCommonVeertex) {
    int retOk = 0;
    if(iOfstSegment) {
        if(iDistLaw) {
            if(iFstOfstCoedge) {
                if(iSndOfstCoedge) {
                    if(iCommonVeertex) {
                        this->mOfstSegment = iOfstSegment;
                        this->mDistLaw = iDistLaw;
                        this->mC1 = iFstOfstCoedge;
                        this->mC2 = iSndOfstCoedge;
                        this->mCommVertex = iCommonVeertex;
                        debug_display_natural_extender(iFstOfstCoedge, iSndOfstCoedge, nullptr, nullptr, nullptr);
                        retOk = this->get_coedge_geomtry();
                        if(retOk) return this->check_spacial_caes() == 0;
                    }
                }
            }
        }
    }
    return retOk;
}
curve_curve_int* ofst_natural_extender::comp_intersections() {
    curve_curve_int* intersections = nullptr;
    /*  if(!this->mOfstSegment || !this->mC1 || !this->mC2) _wassert(L"mOfstSegment && mC1 && mC2", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_natural_extender.cpp", 0x128u);*/
    if(this->mC1Geom && this->mC2Geom) {
        SPAbox box_cur;
        SPAinterval v1 = this->mC1Geom->param_range();
        SPAinterval v2 = this->mC2Geom->param_range();
        if(!v1.infinite()) goto LABEL_10;
        if(v2.infinite()) {
            SPAinterval tmp_int(-15000.0, 15000.0);
            box_cur= SPAbox(tmp_int, tmp_int, tmp_int);
        } 
        else {
        LABEL_10:
            SPAbox b2 = this->mC2Geom->bound((const SPAinterval)this->mC2Geom->param_range());
            SPAbox b1 = this->mC1Geom->bound((const SPAinterval)this->mC1Geom->param_range());
            box_cur = b1 | b2;
        }
        intersections = int_cur_cur(*this->mC1Geom, *this->mC2Geom, box_cur, SPAresabs * 0.1);
        /*if(Debug_Break_Active("sg_close_with_natural2", "WIRE-OFFSET"))
        {
            if(get_breakpoint_callback()) {
                breakpoint_callback = get_breakpoint_callback();
                v16 = breakpoint_callback->new_render_object(breakpoint_callback, 1i64);
            } else {
                v16 = 0i64;
            }
            if(v16) {
                orgCodge = offset_segment::original_coedge(this->mOfstSegment);
                if(orgCodge) {
                    v6 = COEDGE::edge(orgCodge);
                    show_entity(v6, BROWN, v16);
                }
                v7 = COEDGE::edge(this->mC1);
                show_entity(v7, YELLOW, v16);
                v8 = COEDGE::edge(this->mC2);
                show_entity(v8, ORANGE, v16);
                tmpInts = intersections;
                counter = 0;
                while(tmpInts) {
                    show_position(&tmpInts->int_point, WHITE, v16, 1);
                    tmpInts = tmpInts->next;
                    ++counter;
                }
                v9 = __acrt_iob_func(2u);
                acis_fprintf(v9, "sg_close_with_natural: There are %d intersection\n\n", counter);
                Debug_Break("sg_close_with_natural2", "WIRE-OFFSET", "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_natural_extender.cpp", 335);
                if(get_breakpoint_callback()) {
                    v17 = get_breakpoint_callback();
                    v17->delete_render_object(v17, v16);
                }
            }
        }*/
    }
    return intersections;
}
int ofst_natural_extender::do_extension(curve_curve_int* iIntersections) {
    return extend_coedges_to_intersection(iIntersections, this->mC1, this->mC2, this->mC1Geom, this->mC2Geom, this->mMainCurve, this->mCommonVrtPos);
}
int ofst_natural_extender::comp_init_extension_length(double& oCurve1ExtParamLen, double& oCurve2ExtParamLen) {
    /*if(!this->mC1Geom || !this->mC2Geom) _wassert(L"mC1Geom != NULL && mC2Geom != NULL", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_natural_extender.cpp", 0x1EDu);*/
    int retVal = 1;
    SPAinterval v3 = this->mC1Geom->param_range();
    if(v3.infinite() || this->mC1Geom->closed()) oCurve1ExtParamLen = 0.0;
    SPAinterval v4 = this->mC2Geom->param_range();
    if(v4.infinite() || this->mC2Geom->closed()) oCurve2ExtParamLen = 0.0;
    SPAvector c1Tangent;
    comp_curve_end_info(this->mC1Geom, this->mC1End, c1Tangent);
    if(this->mCurve1AtStart) c1Tangent = -c1Tangent;
    double c1TangentLen = c1Tangent.len();
    if(SPAresabs > c1TangentLen) {
        retVal = 0;
        this->mFailCase = 3;
    }
    if(retVal) {
        SPAvector c2Tangent;
        comp_curve_end_info(this->mC2Geom, this->mC2Start, c2Tangent);
        if(this->mCurve2AtStart) c2Tangent = -c2Tangent;
        double c2TangentLen = c2Tangent.len();
        if(SPAresabs > c2TangentLen) {
            retVal = 0;
            this->mFailCase = 3;
        }
        if(retVal) {
            SPAvector v2 = normalise(c2Tangent);
            SPAvector v1 = normalise(c1Tangent);
            double dist1;
            double dist2;
            if(int_2_lines_3d(this->mC1End, v1, this->mC2Start, v2, dist1, dist2)) {
                SPAvector v = normalise(c1Tangent);
                const SPAvector v6 = dist1 * v;
                this->mIntersectPoint = this->mC1End + v6;
            } else {
                dist1 = (this->mC2Start - this->mC1End).len();
                dist2 = dist1;
            }
            oCurve1ExtParamLen = dist1 / c1TangentLen;
            oCurve2ExtParamLen = dist2 / c2TangentLen;
        }
    }
    return retVal;
}
void ofst_natural_extender::filter_intersections(curve_curve_int*& ioIntersections) {
    curve_curve_int* crrIntect = ioIntersections;
    curve_curve_int* preIntect = ioIntersections;
    while(crrIntect) {
        curve_curve_int* preIntect;
        curve_curve_int* tmpIntsect;
        if(beyond_intesect(this->mC1End, this->mIntersectPoint, crrIntect->int_point) || beyond_intesect(this->mC2Start, this->mIntersectPoint, crrIntect->int_point)) {
            tmpIntsect = crrIntect;
            crrIntect = crrIntect->next;
            if(tmpIntsect == ioIntersections) {
                ioIntersections = crrIntect;
                preIntect = crrIntect;
            } else if(preIntect) {
                preIntect->next = crrIntect;
            }
            tmpIntsect->next = 0i64;
            release_intersection_data(tmpIntsect);
        } else {
            preIntect = crrIntect;
            crrIntect = crrIntect->next;
        }
    }
}
int ofst_natural_extender::check_spacial_caes() {
    int spCase = 0;
    /* if(!this->mOfstSegment || !this->mC1Geom || !this->mC2Geom) _wassert(L"mOfstSegment && mC1Geom && mC2Geom", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_natural_extender.cpp", 0x10Fu);*/
    if(this->mC1Geom->type() == 2 && this->mC2Geom->type() == 2) {
        COEDGE* c1 = this->mOfstSegment->coedge();
        const COEDGE* v10;
        if(c1)
            v10 = c1->next();
        else
            v10 = nullptr;
        if(c1) {
            if(v10) {
                SPAvector t1(coedge_end_dir(c1));
                SPAvector t2(coedge_start_dir(v10));
                SPAunit_vector dir1 = normalise(t1 + t2);
                SPAunit_vector dir2 = normalise(this->mC2Start - this->mC1End);
                if(dir1 % dir2 < -0.999) return 1;
            }
        }
    }
    return spCase;
}
