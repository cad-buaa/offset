#ifndef _OFF_CU_H
#define _OFF_CU_H
#include "PrivateInterfaces/ofst_edge_smooth_manager.hxx"
#include "ProtectedInterfaces/ofsttools.hxx"
#include "acis/SPA_approx_options.hxx"
#include "acis/alltop.hxx"
#include "acis/gen_rendobj.hxx"
#include "acis/intdef.hxx"
#include "acis/law.hxx"
#include "acis/mmgr.hxx"
#include "acis/off_wire.hxx"
#include "acis/sgcofrtn.hxx"
#include "acis/sp3crtn.hxx"

// const struct sg_seg_rating;

enum sg_seg_seg_rating { seg_in = 0, seg_out = 1, seg_tan = 2 };
typedef struct sg_seg_rating {
    sg_seg_seg_rating low_rating;
    sg_seg_seg_rating high_rating;
    curve_curve_int* cci;
    int mIntersectionId;
} sg_seg_rating;

class offset_segment : public ACIS_OBJECT {
  public:
    offset_segment(COEDGE* c_offset, VERTEX* v_original, COEDGE* c_original, double iSParam, double iEParam);
    offset_segment(COEDGE* c_offset, COEDGE* c_original);
    offset_segment(COEDGE* c_offset, VERTEX* v_original);
    offset_segment();
    offset_segment* next();
    offset_segment* previous();
    COEDGE* coedge();
    int start_out();
    int end_out();
    int inside();
    int outside();
    int reversed();
    int singular_offset();
    int simple_offset();
    int extension();
    int vertex_offset();
    int coedge_offset() const;
    VERTEX* original_vertex() const;
    COEDGE* original_coedge() const;
    SPAbox get_box();
    SPAbox reset_box();
    void set_start_out();
    void set_start_in();
    void set_end_out();
    void set_end_in();
    void set_start_connectionId(const int iConnectionId);
    int get_start_connectionId();
    void set_end_connectionId(const int iConnectionId);
    int get_end_connectionId();
    void set_start_branch_connectionId(const int iConnectionId);
    void set_end_branch_connectionId(const int iConnectionId);
    int get_end_branch_connectionId();
    void set_user_flag(const int ivalue);
    int get_user_flag();  // 已检查
    void set_reversed();
    void set_extension();
    void set_start(sg_seg_rating& rating, int is_second);
    void set_end(sg_seg_rating& rating, int is_second);
    void set_next(offset_segment* next);
    void set_previous(offset_segment* prev);

    void split_and_rate(curve_curve_int*& ioIntersections, law* iDistLaw, SPAunit_vector& iNormal, int& ioStartConnectionId, const double iTanTol, int iSingleSegment, int iMayHaveSelfIntersections);
    int split_and_rate(sg_seg_rating& rating, int is_second);
    int split(double split_param);
    int split_at_cusps(law* dist_law);
    int remove_sliver_segment();
    int do_self_intersect_filter_bad_intersections(law* dist_law, SPAunit_vector& normal, int iSingleSegment, int& ioStartConnectionId);
    int do_self_intersect_new(curve* ofstCurve, law* dist_law, SPAunit_vector& normal, int iSingleSegment, int& ioStartConnectionId, int iSplitAtCusps, int iRemoveContainments);
    void do_self_intersect(law* dist_law, SPAunit_vector& normal, int& ioStartConnectionId);  // 暂未实现
    void do_self_intersect(law* dist_law, SPAunit_vector& normal, int iSingleSegment, int& ioStartConnectionId, ofst_edge_smooth_manager* iEdgeSmoothMgr, int& oHasError);
    void print(_iobuf* dfp);
    void get_init_params(double& iSParam, double& iEParam);
    void set_init_params(double iSParam, double iEParam);
    void comp_cusp_points(curve_curve_int*& ioList, law* iDistLaw, SPAunit_vector& iNormal, int use_high_curv_func);  // 暂未实现
    void split_and_rate_new(curve_curve_int*& ioIntersections, int& ioStartConnectionId, double iHalfParamRange, int iSingleSegment);

    offset_segment* next_segment;
    offset_segment* prev_segment;
    int start_is_out;
    int end_is_out;
    int seg_reversed;
    int is_singular;
    int mMayHaveSelfIntersections;
    int is_extension;
    int mStartConnectId;
    int mEndConnectId;
    int mStartBranchConnectId;
    int mEndBranchConnectId;
    int mUserFlag;
    double mParams[2];
    COEDGE* seg_coedge;
    VERTEX* v_underlying;
    COEDGE* c_underlying;
    SPAbox bx;
    int bx_set;
};
class offset_segment_list : public ACIS_OBJECT {
  public:
    offset_segment_list(law* dist, SPAunit_vector& normal, int add_attribs, sg_gap_type use_type, int iNonPlanar);
    offset_segment_list();
    law* distance();
    WIRE* get_base_wire();
    void set_base_wire(WIRE* iBaseWire, int iNumCoedges);
    int get_number_of_offset_coedges();
    int trim_inside_wire_done();
    void set_trim_inside_wire_done(const int iDone);
    int get_start_segment_connectionId();
    void set_start_segment_connectionId(const int iId);
    SPAunit_vector normal();
    int add_attribs();
    offset_segment* first_segment();
    offset_segment* last_segment();
    void set_distance(law* d);           // 已检查
    void set_normal(SPAunit_vector& v);  // 已检查
    void set_add_attribs(int l);         // 已检查
    void close_segments();

    int add_segment(offset_segment* new_segment, RenderingObject* pRO);  // 暂未实现
    void insert_segment(offset_segment* seg_to_insert, offset_segment* key_seg);
    void remove_segment(offset_segment* seg);
    void print(_iobuf* dfp);
    void empty_list();
    void set_gap_type(sg_gap_type in_type);  // 已检查
    sg_gap_type get_gap_type();
    void set_last_connectionId(const int iConnectionId);
    int get_last_connectionId();
    void init_connection();
    void set_edge_smooth_manager(ofst_edge_smooth_manager* iEdgeSmoothMgr);
    ofst_edge_smooth_manager* get_edge_smooth_manager();
    ~offset_segment_list();
    int segments_connected();
    void add_connection_after_split(offset_segment* iStartSegment1, offset_segment* iStartSegment2);

    int mSelfIntesectError;
    law* dist_law;
    SPAunit_vector offset_normal;
    double mOffsetDist;
    int add_attributes;
    sg_gap_type close_type;
    WIRE* mBaseWire;
    int mNumberOfOffsetCoedges;
    int mTrimInsideWireDone;
    int mNonPlanar;
    offset_segment head_node;
    offset_segment tail_node;
    int count;
    int mLastSegmentConnectionId;
    int mStartSegmentConnectionId;
    ofst_edge_smooth_manager* mEdgeSmoothMgr;
};

class offset_int_cur : public int_cur {
    curve* orig_curve;
    double p[200];

  public:
    offset_int_cur(offset_int_cur& old);
    offset_int_cur(curve& orig_spline, SPAinterval& c_range, bs3_curve_def* c, double fit_tol, SPAunit_vector& curve_normal, law* inlaw_dist, law* inlaw_twist);
    offset_int_cur(curve& orig_spline, SPAinterval& c_range, bs3_curve_def* c, double fit_tol, SPAunit_vector& off_dir, SPAunit_vector& curve_normal, law* inlaw_dist, law* inlaw_twist);
    offset_int_cur();
    const curve& get_orig_curve() const;
    SPAinterval extend(SPAinterval& iNewRange);
    static int id();
};

int use_pcurve_for_curve_offset(COEDGE* coed, surface* offsurf);
int check_analytical_curve_offset_on_cone(COEDGE* coed, surface* offsurf);
curve* make_offset_curve_from_pcurve(COEDGE* pCoedge, surface& rOffsetSurface);
void sg_approx(COEDGE* in_coedge, bs2_curve_def*& coedge_approx_result, SPA_internal_approx_options& approxOpts);

COEDGE* sg_offset_pl_coedge(COEDGE* c, law* dist_law, law* twist_law, SPAunit_vector& n);                                                  // 已检查
bs3_curve_def* bs3_curve_snap_end_curvatures(bs3_curve_def* crv, const SPAvector& start_crv, const SPAvector& end_crv, double smoothing);  // 别的模块
// void  bs3_curve_compat(bs3_curve_def* b1, bs3_curve_def* b2);//陆
double bs3_cpt_dist(bs3_curve_def* bs3_0, bs3_curve_def* bs3_1);                               // 别的模块
int make_tvertex(COEDGE* c, int start);                                                        // 陆
curve_curve_int* remove_intersection(curve_curve_int* head_cci, curve_curve_int* remove_cci);  // 别的模块

#endif