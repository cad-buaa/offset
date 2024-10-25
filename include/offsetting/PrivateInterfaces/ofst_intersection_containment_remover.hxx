#pragma once
#include "ProtectedInterfaces/off_cu.hxx"
#include "acis/alltop.hxx"
#include "acis/container_utils.hxx"
#include "acis/intcucu.hxx"
#include "acis/sgcofrtn.hxx"

void display_segment_intersection_points(EDGE* iOfstEdge, curve_curve_int* iIntersections, EDGE* iOfstEdge2);    //已检查  调试                                                                         // 暂未实现
int sg_intersect_rate_segments(offset_segment_list* iSegList, offset_segment* s1, offset_segment* s2, law* dist_law, SPAunit_vector& normal, sg_gap_type use_type, int& ioConnectionId);  // 暂未实现
int coedge_connected(COEDGE* iCoedge1, COEDGE* iCoedge2);
int is_first_intersect_contains_all(curve_curve_int* ioIntersections, double iHalfParamRange);
void remove_intersections_containing_exactly_one_cusp(curve_curve_int*& intersectionsList, curve_curve_int* cuspsList, COEDGE* coed);  // 暂未实现
int contains_exactly_one_cusp(curve_curve_int* the_inter, SPAdouble_array& cusps, int& which);
int cci_contains_par(curve_curve_int* the_inter, double par);
int get_connectionId_at_intersection(offset_segment* iSeg1, offset_segment* iSeg2, SPAposition& iIntsctPositon, int& ioConnectionId);

class ofst_intersection_containment_remover {
  public:
    ofst_intersection_containment_remover();
    ~ofst_intersection_containment_remover();
    void set_curve_info(const double iHalfParamRange, const int iCurveClosed, const int iNotRemoveNormalIntersections);
    int remove_contained_intersections(curve_curve_int*& ioIntersections);
    int remove_contained_cusps(curve_curve_int* iIntersections, curve_curve_int*& ioCusps, int iRemoveContained);
    int intersection_contained(curve_curve_int* iIntersect1, curve_curve_int* iIntersect2);
    void reset_mark(curve_curve_int* ioIntersections);
    void mark_contained_intersections(curve_curve_int* ioIntersections);
    void mark_contained_cusps(curve_curve_int* iIntersections, curve_curve_int* ioCusps, int iRemoveContained);
    void mark_coincident_cusps(curve_curve_int* ioCusps);
    int remove_marked_intersections(curve_curve_int*& ioIntersectiions);

    double mHalfParaRange;
    int mCurveClosed;
    int mNoRemoveNormalIntersections;
};
