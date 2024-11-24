#ifndef _OFST_EDGE_SMOOTH_MANAGER_H
#define _OFST_EDGE_SMOOTH_MANAGER_H
#include "acis/alltop.hxx"
#include "acis/bs3c_def.hxx"
#include "acis/intcucu.hxx"
#include "acis/law_base.hxx"
#include "acis/lists.hxx"
#include "acis/off_wire.hxx"
#include "acis/strdef.hxx"

// 已检查
class ofst_edge_smooth_manager : ACIS_OBJECT 
{
public:
    ofst_edge_smooth_manager();//已检查
    ~ofst_edge_smooth_manager();
    int init(ENTITY_LIST* iEdges, int iMaxSmoothLevel, int iMaxIteration);
    int get_edge_smooth_level(EDGE* iEdge, int oRemoceContainment);
    void set_edge_intersection_info(EDGE* iEdge, curve_curve_int* intersectionsList);
    int apply_more_smooth(int iNumIteration);
    int is_last_iteration();
    int is_first_iteration();
    int is_intersection_tangential();
    void set_edge_self_intersection_fail(EDGE* iEdge);
    void relese_allocations();
    int edge_index(EDGE* iEdge);
    void adjust_smoothlevel();

    ENTITY_LIST mBaseEdges;
    int mNumEdges;
    int mIteration;
    int mMaxIteration;
    int mMaxSmoothLevel;
    int mTangential;
    int* mSmoothLevel;
    int* mLastSmoothLevel;
    int* mNumIntersections;
    int* mNumOverlaps;
};
//
//BODY* sg_offset_planar_wire_internal(WIRE* wire, TRANSFORM* wire_transf, law* dist_law, law* twist_law, SPAunit_vector& wire_normal, sg_gap_type close_type, int add_attribs, int trim, int zero_length, int overlap, int iKeepMiniTopo,
//                                     ofst_edge_smooth_manager* iEdgeSmoothMgr);  // 暂未实现

bs3_curve_def* smooth_bs3_curve(bs3_curve_def*& iBs3Curve, SPAinterval& iParamRange, double iFitTolerance, int iLevel);

curve* smooth_curve1(intcurve* iCurve, SPAunit_vector& iNormal, int iLevel, law* iDstLaw);                                 // 暂未实现
void get_intersection_info(curve_curve_int* iIntersections, int& oNumIntersections, int& oNumOverlaps, int& oNumUnknown);  // 暂未实现

#endif