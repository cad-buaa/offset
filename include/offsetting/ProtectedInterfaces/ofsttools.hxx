#ifndef _OFSTTOOLS_H
#define _OFSTTOOLS_H
#include "PrivateInterfaces/ofst_edge_smooth_manager.hxx"
#include "ProtectedInterfaces/off_cu.hxx"
#include "acis/alltop.hxx"
#include "acis/boolapi.hxx"
#include "acis/bs3c_def.hxx"
#include "acis/generic_graph.hxx"
#include "acis/intcucu.hxx"
#include "acis/model_state.hxx"
#include "acis/module.hxx"
#include "acis/off_wire.hxx"
#include "acis/thmgr.hxx"
#include "acis/tolerize_ent_opts.hxx"
#include "acis/fit.hxx"

extern module_debug woffset_module_header;
extern module_debug trim_offset_wire_module_header;

class offset_segment;
class offset_segment_list;
struct sg_seg_rating;


int check_failed_full_surface(FACE* face_copy, surface* offset_geom, SPAbox& face_box, SPApar_box& face_pb);//已检查
void sg_close_with_arc(offset_segment* this_seg, law* dist_law, SPAposition& p, SPAposition& p1, SPAposition& p2, COEDGE* c1, COEDGE* c2, VERTEX* vert);  // 已检查
int comp_arc_center(SPAposition& iPoint1, SPAposition& iPoint2, const double iRadius, SPAposition& iRefPoint, SPAposition& oCenter);                      // 已检查
void reduce_to_principal_param_range(SPAparameter& t, const SPAinterval& range, double period, double tol);//别的模块
int validate_intersection_as_extension(SPAposition& intersect_pos, COEDGE* c, curve* c_geom, int fixed_start);                                                   // 已检查，有警告
int extend_coedges_to_intersection(curve_curve_int* iIntersections, COEDGE* c1, COEDGE* c2, curve* c1_geom, curve* c2_geom, curve* master_crv, SPAposition& p);  // 已检查
bool check_if_face_is_full_surface(FACE* original_face, SURFACE* original_SUR, surface* offset_geom);                                                            // 已检查
void ofst_init();
int ofst_error_set(int* err_no);
void ofst_term();
void ofst_error(int err_num, int hopeless, ENTITY* e_ent);
SPApar_vec invert_vec(SPAvector& off, SPAvector* dpos, int* well_conditioned);
int spline_isoparam(surface* sf, pcurve* pcur, int i, double* iso_val, double tol);
int on_isoparam_curve(surface* sf, SPAposition& ctrlpt, int i, const double const_val, SPApar_vec& off, double tol);

class iop_modeler_state_sentry {
  public:
    iop_modeler_state_sentry();
    ~iop_modeler_state_sentry();
    option_header* ag_opt;
    modeler_state* mdlr_state;
};
class parallel_get_tolerance : public thread_work_base 
{
public:
    typedef class std::pair<TEDGE*, double> value_t;
    typedef class SpaStdVector<std::pair<TEDGE*, double>> vector_t;
    parallel_get_tolerance();
    virtual bool is_threadsafe_tol_update(EDGE* ed) = 0;
    void get_tolerances(SpaStdVector<TEDGE*>& tedges, SpaStdVector<std::pair<TEDGE*, double>>* tedge_tols);
    virtual void process(void* arg);
    iop_modeler_state_sentry* _pState;
};



int offset_success(BODY*& iOffsetBody);//已检查
BODY* sg_offset_planar_wire_internal(WIRE* wire,const TRANSFORM* wire_transf, law* dist_law, law* twist_law,const SPAunit_vector& wire_normal, sg_gap_type close_type, int add_attribs, int trim, int zero_length, int overlap, int iKeepMiniTopo,
                                      ofst_edge_smooth_manager* iEdgeSmoothMgr);

void display_wire_offset_io(WIRE* in_wire, law* dist_law, BODY* out_wire);  // print相关
int is_open_wire(WIRE* w);                                                  // 已检查
COEDGE* start_of_wire_chain(WIRE* w);                                       // 已检查  trm

law* edge_dist_law_to_coedge_dist_law(COEDGE* c, EDGE* e, law* dist_law);                                                            // 已检查
int coedge_endpoint_localized_offset_curvature_problem(COEDGE* ce, int at_start, double dist,const SPAunit_vector& norm);                 // 已检查
int curve_point_offset_curvature_problem(curve& cu, double param, double dist, int rev,const SPAunit_vector& norm);                       // 已检查
int make_tvertex(COEDGE* c, int start);                                                                                              // 已检查
void make_same_vertex_pointers(VERTEX* vert1, VERTEX* vert2, COEDGE* this_coedge, COEDGE* next_coedge, bool reset_this_edge_range);  // 已检查

outcome sg_split_edge_at_disc(EDGE* theEdge, ENTITY_LIST& new_edges, int cont_type);  // 别的模块
void merge_vertex(VERTEX* vertex, int (*edge_sel)(EDGE*, EDGE*));                     // 别的模块
int all_offset_degenerated(ENTITY_LIST& iCoedges);                                    // 已检查
void sg_sort_cci(curve_curve_int*& ccx_dat, int rev, int second);                     // 别的模块
void sg_debug_cci(curve_curve_int* rel_data, _iobuf* fptr, const char* __formal);     // 别的模块
int rate_single_seg_case(offset_segment* iSeg, curve_curve_int* iIntsect, sg_seg_rating* ioRating);//已检查
int is_seg_reversed(offset_segment* iSeg, double iCheckParam);//已检查

curve_curve_int* sort_intersections(curve_curve_int* cci);                                    // 别的模块
curve_curve_int* reverse_intersections(curve_curve_int* head_cci);                            // 别的模块
void sg_delete_cci(curve_curve_int*& ccx_dat);                                                // 别的模块
curve* get_curve_from_coedge(COEDGE* iCoedge, SPAinterval& oParamRange, SPAtransf& iTransf);  // 已检查
curve_curve_int* get_curve_self_ints(const intcurve& intcur, double given_tol);               // 别的模块

int has_high_curvature(curve_curve_int* iIntersections, int iSingleSegment);                          // 已检查trm
int ofst_params_over_lapping(double iParam1, double iParam2, double iParam3, double iParam4);         // 已检查trm
int bs3_curve_unique_knots(bs3_curve_def* bs3, int& nknots, double*& knots, int*& cont, double tol);  // 别的模块

double find_max_curv_param(curve* cu, SPAinterval& search_int, SPAposition& max_curv_pos);                        // 已检查
void set_curve_to_coedge(COEDGE* ioCoedge, curve* iCurve);                                                        // 已检查
void correct_param_0(EDGE* segEdge, double& sp, double& ep);                                                      // 不懂，因为VS code里搜不到correct_param_0//已检查
int check_gap_type(COEDGE* this_orig_coedge, COEDGE* prev_orig_coedge,const SPAvector& plane_nv, double offset_dist);  // 已检查
int is_circle_offset_by_radius(curve* geometry,const SPAunit_vector& normal, law* dist_law, law* twist_law);           // 已检查
int comp_tangent_at_coedge_commom_vertex(COEDGE* iCoedge1, COEDGE* iCoedge2, COEDGE* iOfstCoedge1, COEDGE* iOfstCoedge2, SPAunit_vector& oTangent, SPAunit_vector& oStartOfstDir, SPAunit_vector& oEndfstDir);  // 已检查
int get_circle_offset_radius(curve* geometry,const SPAunit_vector& normal, law* dist_law, law* twist_law, double& off_radius);                                                                                       // 暂有不懂
int ofst_coedge_reversed(COEDGE* iOfstCoedge, COEDGE* iorigCoedge, int iAtStart);                                                                                                                               // 已检查
int process_overlapping(COEDGE*& ioCoedge1, COEDGE*& ioCoedge2);                                                                                                                                                // 已检查

int get_coedge_data(COEDGE* iCoedge, VERTEX*& oVertex, SPAposition& oVrtPos, curve*& oCurGeom, int iNeedStartData);//已检查
int trim_coedge(COEDGE*& ioCoedge, VERTEX*& ioSplitVertex, SPAposition& iSplitPos, SPAparameter iParam, curve* ioCurve, int iKeepFirstPart);
int check_gap_intersection(double iGapDist, double iOfstDist, EDGE* iEdge1, EDGE* iEdge2);  // 已检查
// int is_COPY_ANNOTATION(ENTITY* e);
void display_null_offset_coedge_to_be_deleted(COEDGE* this_off_coedge, COEDGE* this_orig_coedge);
void del_entity(ENTITY* ent);                                             // 别的模块
SPAvector conv_curvature(const SPAvector& dpos, const SPAvector& ddpos);  // 别的模型
void comp_vdist(SPAposition& p, SPAposition& p1, SPAposition& p2, SPAvector& oVDist1, SPAvector& oVDist2);
int sg_close_with_natural2(offset_segment* this_seg, law* dist_law, SPAposition& p, SPAposition& p1, SPAposition& p2, COEDGE* c1, COEDGE* c2, VERTEX* vert);
int sg_close_offset_gap(offset_segment_list& seg_list, offset_segment*& this_seg, COEDGE* this_coedge, VERTEX* common_vertex, SPAunit_vector& iOffsetDir, SPAunit_vector& iStartOfstDir, SPAunit_vector& iEndOfstDir, law* dist_law, sg_gap_type gap_type,
                        int iKeepMiniTopo, int skipbackup);  // 已检查

logical sel_ed(EDGE* e1, EDGE* e2);                                                                                                                                            // 已检查
int sg_close_with_corner(offset_segment* this_seg, law* dist_law, SPAposition& p, SPAposition& p1, SPAposition& p2, COEDGE* c1, COEDGE* c2, VERTEX* vert, int iKeepMiniTopo);  // 已检查
void display_gap_after_close(COEDGE* c1, COEDGE* c2, char* type);                                          //异常处理暂未实现
int sg_extend_coedges(law* dist_law, SPAposition& in_pos, COEDGE* in_coedge, curve* crv_geom, SPAvector& v_dist);                                                              // 已检查

int is_overlap(curve_curve_int* iIntersectionInfo);
int correct_seg_overlap(offset_segment_list* iSegList, offset_segment* ioSeg);
int overlap_correction(offset_segment_list* iSegList, offset_segment* ioSeg1, offset_segment* ioSeg2);
int valid_intersection(offset_segment* s1, offset_segment* s2, sg_seg_rating* rating);
int isOverlap_arc(EDGE* iEdge1, EDGE* iEdge2);
int is_overlap_0(COEDGE* iCEdge1, COEDGE* iCEdge2, const sg_seg_rating* irating, SPAinterval& iParamRang1, SPAinterval& iParamRang2);//已检查！
int move_split_pos_to_center(curve_curve_int* ioIntersection1, curve_curve_int* ioIntersection2, EDGE* iEdge1, EDGE* iEdge2);//已检查！

void display_segment_in_colors(offset_segment_list& iSegList, int iNumColors);                                                           // 已检查
curve* filter_high_curvature(law* iDistLaw, SPAunit_vector& iNormal, curve* offsetCurve, SPAinterval& iParamRange, int iSingleSegment);  // 已检查
bs3_curve_def* smooth_curve(curve* iCurve, SPAinterval& iParamRange, double iFitTolerance, int iMoreSamples);                            // 已检查
void display_segment_trim_status(offset_segment_list& iSegList);
int remove_consecutive_overlaps(BODY* wire_body);                                                         // 已检查
int consecutive_coedge_overlap(COEDGE* iCoedge, SPAposition& iCheckPosition);                             // 已检查
int coedge_overlap_with_next_coedge(COEDGE* iCoedge, SPAposition& iCheckPosition);                        // 已检查
void remove_overlap_coedges(COEDGE* overLapCoedge1);                                                      // 已检查
void delete_overlap_coedges(COEDGE*& ioCoedge1, COEDGE*& ioCoedge2);                                      // 已检查
void sg_collect_segments(BODY* wire_body, offset_segment_list& seg_list, int zero_length, int iOverlap);  // 已检查
void repair_wire_at_branches(BODY* wire_body);                                                            // 已检查
void sg_add_offset_coedge(WIRE*& current_wire, WIRE* wire_list, COEDGE* offset_coedge);                   // 已检查，暂有不懂
void trim_the_wire(offset_segment_list* seg_list, int wire_closed, WIRE*& wire_list);                     // 已检查
int trim_edge(offset_segment_list* seg_list, bool start_too_close, int iNumEdges, EDGE*& edgeToTrim);
offset_segment* get_prev_out_seg(offset_segment*& curr_seg, offset_segment_list* seg_list);                                            // 已检查
void remove_edge_from_wire(WIRE*& wire_list, EDGE* edge_to_remove);                                                                    // 已检查
int trim_coedge_0(VERTEX*& ioSplitVertex, const SPAposition& iSplitPos, SPAparameter& iParam, int iKeepFirstPart, COEDGE*& ioCoedge);  // 已检查
offset_segment* get_next_out_seg(offset_segment*& curr_seg, offset_segment_list* seg_list);                                            // 已检查
void sg_remove_attributes(WIRE* this_wire);                                                                                            // 暂未实现
void close_coedges_at_vertex(generic_graph* base_graph, VERTEX* gvent);                                                                // 已检查
void sg_clean_and_close_wires(WIRE*& wire_list, ENTITY_LIST& delete_list, int orig_wire_closed);                                       // 已检查，remove未实现

void check_edge_error(const ENTITY_LIST& edges, ENTITY_LIST& bad_entities, ENTITY*& worst_entity, double& worst_error, double tol, int stop_immediately, ENTITY_LIST& new_entities, int careful_on, teo_data* tedata);  // trim_util.cpp
void check_vertex_error(const ENTITY_LIST& vertices, ENTITY_LIST& bad_vertices, ENTITY*& worst_entity, double& worst_error, double tol, int stop_immediately, ENTITY_LIST& new_vertices, int careful_on, int& proceeded_after_errors, error_info_list* eil,
                        teo_data* tedata);  // trim_util.cpp

int do_boolean(BODY* tool_body, BODY* blank_body, BOOL_TYPE type, BODY*& outside_body, BODY*& leftovers, NDBOOL_KEEP ndbool_keep, BODY*& ndbool_result,const glue_options* glue_opts, int quick_failure_preferred);  



void sg_point_on_offset(curve& orig_curve, SPAinterval& __formal, SPAunit_vector& n, double param, SPAposition& pos, SPAvector& dpos, SPAvector& ddpos, law* dist_law, law* twist_law, evaluate_curve_side side);//kernal模块
curve* sg_offset_planar_curve_internal(curve& geom, SPAinterval& range, double fit_data, law* dist_law, law* twist_law, SPAunit_vector& off_nor, double tol);//kernal模块
class curve_interp;

class offset_int_interp : curve_interp 
{
    SPAinterval curve_range;
    law* dist_law;
    law* twist_law;
    SPAunit_vector offset_plane;
    curve* orig_curve;
    double d[400];

  public:
    offset_int_interp(curve& orig_spline, SPAinterval& range, SPAunit_vector& curve_normal, int npt, SPAposition* pos_arr, SPAvector* tan_arr, double* par_arr, double fit_data, law* law_dist, law* law_twist);
};


void amalgamate_bodies(BODY* from_body, BODY* to_body);


#endif