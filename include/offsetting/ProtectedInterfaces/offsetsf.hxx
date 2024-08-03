#pragma once

#include "acis/alltop.hxx"
#include "acis/err_info.hxx"
#include "acis/offset_opts.hxx"
#include "acis/sphdef.hxx"
#include "acis/pladef.hxx"
#include "acis/add_pcu.hxx"

class offset_surface_options : public ACIS_OBJECT {
  public:
    offset_surface_options();

    int exact_offset_var_blend;
    int make_approx_sf;
    int simplify;
    int do_adaptive;
    int tolerize;
    int replace_edges_with_parintcurve;
    int replace_vertex_blend;
    int allow_split;
    int need_par_int_curs;
    int trim_bad_geometry;
};

class standard_offsetter : public ACIS_OBJECT {
  public:
    offset_surface_options m_offset_surface_options;
    int* m_remake_face;
    int* m_adaptive_flag;
    int m_tolerize;
    int m_replace_vertex_blend;
    int m_allow_split;
    int& m_part_inv;
    offset_options* m_off_opts;
    int m_reparam_exact_offset_surf;
    int m_natural_boundary_for_closed_spline;
    int m_need_par_int_curs;

    standard_offsetter(standard_offsetter& __that);
    standard_offsetter(int& part_inv, offset_options* pOffOpts, int* remake_face, int* adaptive_flag, int tolerize, int replace_vertex_blend, int allow_split);
    ~standard_offsetter();
    virtual surface* offset_surface_tweak(FACE* fa, SPAbox& region_of_interest, double offset_distance, error_info*& err);  //目前暂未填充
    virtual surface* offset_surface_single(FACE* original_face, FACE* face_copy, SPApar_box* face_pb, SPAbox& face_box, double offset_distance, error_info*& err);
    void make_face_with_natural_boundaries(FACE*& face_copy, FACE* original_face, surface* offset_geom, SPApar_box& face_pb, int failed_full_surface);
    virtual void make_face_with_offset_boundaries(FACE* face_copy, FACE* original_face, SURFACE* offset_surf, double offset_distance, SPApar_box& face_pb, int did_adaptive);
    void offset_loops(FACE* orig_face, FACE* this_face, SURFACE* offset_surf, double offset_distance, SPApar_box& pb);
    virtual SPAposition offset_pos(SPAposition& original_pos, surface* surf, surface* offset_surface, SPApar_pos& uv, double offset_distance);
    virtual SPAposition offset_pos_at_sing(VERTEX* vert, EDGE* edge, double offset_distance, surface* orig_surf, SPApar_box& pb);
    virtual void offset_curve(COEDGE* this_coedge, pcurve& pcu_geom, curve*& offset_geom, double offset_distance, FACE* orig_face, SURFACE* original_surf, SURFACE* offset_surf, int add_edge, EDGE* new_edge, int u_sing, int v_sing, int& adjust_coedge_dir,
                              SPApar_pos& uv, int* approx_offset);
    void offset_vertex_pos(VERTEX* vert, SPAposition& sing_pos, double offset_distance, SPAposition& off_pos, EDGE* new_edge, SURFACE* original_surf, SURFACE* offset_surf, SPApar_box& pb, SPApar_pos& uv, int& add_edge, const int& u_sing_diff_par);
    void make_face(FACE*& face_copy, FACE* original_face, SURFACE* original_surf, surface* offset_geom, SPAbox& face_box, int& failed_full_surface, SPApar_box& face_pb, int did_adaptive, double offset_distance);
    virtual int check_collapsing_face(FACE* face, double face_offset_dist, ENTITY_LIST& collapsedFaceList, int dummy);  //目前暂未填充
    int is_reparameterized();  //目前暂未填充
    int did_adaptive();
    int did_remake_face();  //目前暂未填充
    int is_part_inv();  //目前暂未填充
    int need_par_int_curs();  //目前暂未填充
};

class update_face_geom_options : public ACIS_OBJECT {
  public:
    update_face_geom_options(int use_lops_settings);
    int replace_surf;
    int replace_pcurves;
    int tolerize;
    int replace_edges_with_parintcurve;
    int replace_all_edges;
    int g1_disc;
    int prefer_c2_pcur;
    int allow_extension;
    int fixup_vertex;
};

//class SPA_curve_fit_options : public ACIS_OBJECT {
//  public:
//    enum failure_mode { NOT_SET = 0, EVAL_FAILED = 1, UNCERT_CHECK_FAILED = 2, COIN_CPTS = 3, FD_DERIV_TEST_FAILED = 4, HUGE_FITOL = 5 };
//    SPA_curve_fit_options(SPA_curve_fit_options& __that);
//    SPA_curve_fit_options();
//    ~SPA_curve_fit_options();
//    int check_ctrl_pts();
//    void set_check_ctrl_pts(int do_check);
//    void set_fit_to_xyz_errors(int do_xyz);
//    int get_fit_to_xyz_errors();
//    void set_check_uncertainty(int do_check);
//    int check_uncertainty();
//    void set_failure_mode(SPA_curve_fit_options::failure_mode new_failure_mode);
//    SPA_curve_fit_options::failure_mode check_failure_mode();
//    void set_fail_param(double new_param);
//    double get_fail_param();
//    void set_check_coin_cpts(int do_check);
//    int check_coin_cpts();
//    void set_refine(int do_refine);
//    int get_do_refine();
//    void set_known_tol(double tol);
//    double get_known_tol();
//    void set_hermite_knot_rem(int do_rem);
//    int get_hermite_knot_rem();
//    void set_new_knot_rem(int do_new_rem);
//    int get_new_knot_rem();
//    virtual double tol_to_ctrl_pt_units(double tol);
//    virtual double ctrl_pt_units_to_tol(double ctrl_pt_size);
//    void set_singularity(SPAposition& pos);
//    void set_vertex_tol(const double tol_value);
//    SPAposition get_singularity();
//    double get_vertex_tol();
//    SPA_curve_fit_options& operator=(SPA_curve_fit_options& __that);
//    virtual void* __vecDelDtor();
//
//    SPA_curve_fit_options::failure_mode how_failed;
//    int ctrl_pt_checking;
//    int fit_to_xyz_errors;
//    int uncertainty_checking;
//    int coin_cpt_checking;
//    double fail_param;
//    int refine;
//    double known_tol;
//    int hermite_knot_rem;
//    int use_new_knot_rem;
//    double vert_tolerance;
//    SPAposition sing_vertex;
//};

//class SPA_pcurve_fit_options : public SPA_curve_fit_options {
//  public:
//    void set_surf_deriv_max(SPAvector& SuMax, SPAvector& SvMax, int check_first);
//    SPA_pcurve_fit_options(SPA_pcurve_fit_options& __that);
//    SPA_pcurve_fit_options();
//    ~SPA_pcurve_fit_options();
//    int extend_if_off_surf();
//    void set_extend_if_off_surf(int do_extend);
//    int fail_if_off_surf();
//    void set_fail_if_off_surf(int do_fail);
//    int add_bs2_to_pbox();
//    void set_add_bs2_to_pbox(int do_add);
//    double get_surf_deriv_max();
//    void get_surf_deriv_max(SPAvector& SuMax, SPAvector& SvMax);
//    int get_fail_early();
//    void set_fail_early(int do_fail_early);
//    bs2_curve_def* get_guess_curve();
//    void set_guess_curve(bs2_curve_def* bs2, int rev, SPApar_vec& offset);
//    double get_par_tol();
//    void set_par_tol(double new_par_tol);
//    virtual double tol_to_ctrl_pt_units(double tol);
//    virtual double ctrl_pt_units_to_tol(double ctrl_pt_size);
//    SPA_pcurve_fit_options& operator=(SPA_pcurve_fit_options& __that);
//    virtual void* __vecDelDtor();
//
//    int fail_early;
//    int off_surface_fail;
//    int off_surface_extend;
//    int pbox_add_bs2;
//    SPAvector Su_max;
//    SPAvector Sv_max;
//    double Su_max_len_sq;
//    double Sv_max_len_sq;
//    bs2_curve_def* guess_curve;
//    double par_tol;
//};

void sg_update_face_geometry(FACE* ref_face, surface* surf, update_face_geom_options* ufg_opts);
//int add_c2_pcurves(COEDGE* ce, double tol, SPApar_box* suggested_pb, SPA_pcurve_fit_options* opts);
int get_pcurve_edge_overlap_interval(COEDGE* coedge, double& start, double& end);
//void get_parintcur_edges_wSameSurf(ENTITY_LIST& el, FACE* face);
//int par_int_to_edge(COEDGE* coed, int fixup_vertex);
//pcurve* generate_c2_pcurve(COEDGE* ce, double tol, SPApar_box* suggested_pb, SPA_pcurve_fit_options* opts);
int add_pcurve_to_coedge_using_intcurve_pcurve(COEDGE* coedge, int exact, bndy_type bndy_info, int reset_pattern, surface* surf);
curve* make_offset_curve_internal(surface* base_sf1, curve* base_cu, const double& offset, pcurve* pcur1, surface* base_sf2, const double& sf2_offset, pcurve* pcur2, surface* offs_sf1, surface* offs_sf2, COEDGE* coed, int& pcurve_invalid,
                                          int thicken, int& approx_offset, int approx_offset_sf1, int approx_offset_sf2);
surface* offset_surface(surface* original_surface, SPAbox& region_of_interest, double offset_distance, int& part_inv, offset_surface_options* off_sur_opts, error_info*& err, int* remake_face, SPApar_box& in_par_box, FACE* in_face, int* did_adaptive,
                        FACE* orig_face);
surface* offset_sphere(sphere* original_sphere, double offset_distance, error_info*& err);
surface* offset_plane(plane* original_plane, double offset_distance);
