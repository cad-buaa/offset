#pragma once
#include "acis/alltop.hxx"
#include "acis/thmgr.hxx"
#include "acis/generic_graph.hxx"
#include "acis/model_state.hxx"
#include "acis/off_wire.hxx"
#include "PrivateInterfaces/ofst_edge_smooth_manager.hxx"


int check_failed_full_surface(FACE* face_copy, surface* offset_geom, SPAbox& face_box, SPApar_box& face_pb);
bool check_if_face_is_full_surface(FACE* original_face, SURFACE* original_SUR, surface* offset_geom);
void ofst_init();
int ofst_error_set(int* err_no);
void ofst_term();
void ofst_error(int err_num, int hopeless, ENTITY* e_ent);
SPApar_vec invert_vec(SPAvector& off, SPAvector* dpos, int* well_conditioned);
int spline_isoparam(surface* sf, pcurve* pcur, int i, double* iso_val, double tol);
int on_isoparam_curve(surface* sf, SPAposition& ctrlpt, int i, const double const_val, SPApar_vec& off, double tol);

class iop_modeler_state_sentry
{
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

int offset_success(BODY*& iOffsetBody);
BODY* sg_offset_planar_wire_internal(WIRE* wire, TRANSFORM* wire_transf, law* dist_law, law* twist_law, SPAunit_vector& wire_normal, sg_gap_type close_type, int add_attribs, int trim, int zero_length, int overlap, int iKeepMiniTopo,
                                     ofst_edge_smooth_manager* iEdgeSmoothMgr);

int is_planar_wire(WIRE* this_wire, SPAposition& centroid, SPAunit_vector& normal, int apply_transf, int use_winding_number);
void display_wire_offset_io(WIRE* in_wire, law* dist_law, BODY* out_wire);
int is_open_wire(WIRE* w);
COEDGE* start_of_wire_chain(WIRE* w);