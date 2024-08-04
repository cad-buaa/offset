#pragma once
#include "acis/lists.hxx"
#include "acis/alltop.hxx"
#include "acis/intcucu.hxx"
#include "acis/law_base.hxx"
#include "acis/off_wire.hxx"


class ofst_edge_smooth_manager {
public:

	ofst_edge_smooth_manager();
	~ofst_edge_smooth_manager();
	int init(ENTITY_LIST* iEdges, int iMaxSmoothLevel, int iMaxIteration);  //暂未实现
	int get_edge_smooth_level(EDGE* iEdge, int oRemoceContainment);
	void set_edge_intersection_info(EDGE* iEdge, curve_curve_int* intersectionsList);
	int apply_more_smooth(int iNumIteration);  //暂未实现
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

 BODY* sg_offset_planar_wire_internal(WIRE* wire, TRANSFORM* wire_transf, law* dist_law, law* twist_law, SPAunit_vector& wire_normal, sg_gap_type close_type, int add_attribs, int trim, int zero_length, int overlap, int iKeepMiniTopo,
                                     ofst_edge_smooth_manager* iEdgeSmoothMgr);