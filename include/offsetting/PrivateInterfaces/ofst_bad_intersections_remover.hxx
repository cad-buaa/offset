#pragma once
#include "acis/alltop.hxx"
#include "acis/intcucu.hxx"

void add_intersectins(curve_curve_int*& ioMainList, curve_curve_int* iIntersections);//已检查
int sg_inter_ed_ed(EDGE* e1, EDGE* e2, curve_curve_int*& inters, double dist_tol, double angle_tol);//别的模块

class ofst_bad_intersections_remover {
  public:
    enum { eIntersection_not_processed = 0, eIntersection_processed = 1, eIntersection_main_loop = 2 };
    ofst_bad_intersections_remover();
    ~ofst_bad_intersections_remover();
    int process_intersections(curve_curve_int* iIntersections, curve_curve_int* iCusps, curve_curve_int*& oResultIntersections, double iParamRange);
    int create_result_intersections(curve_curve_int*& oIntersections);
    int extract_next_loop_intersection(int& ioStartIndex, int& ioEndIndex, int& oNormalLoopIntersection);
    int extract_next_loop_intersection1(int& ioStartIndex, int& ioEndIndex, int& oNormalLoopIntersection);
    int extract_next_intersection(int& ioStartIndex, int& ioEndIndex);
    int find_next_max_param_loop(int& ioStartIndex, int& ioEndIndex, double& oMaxParamRange);
    int extractLoops(curve_curve_int*& oIntersections);
    int extract_non_loops(curve_curve_int*& oIntersections);
    void insert_intersection(curve_curve_int* iIntersection, int iUseSecondParam);
    int build_array(curve_curve_int* iIntersectionList, curve_curve_int* iCuspList);
    int make_allocation(curve_curve_int* iIntersectionList, curve_curve_int* iCuspList);
    void release_allocation();

    int* mStatus;
    double* mParams;
    curve_curve_int** mIntersections;
    int mNumParameters;
    double mCurveParamRange;
    double mMaxIntersectionRange;
};