#include "PrivateInterfaces/ofst_jour.hxx"

#include "acis/acis_options.hxx"
#include "acis/api.hxx"
#include "acis/cstrapi.hxx"
#include "acis/offset_opts.hxx"
#include "acis/wire_offset_options.hxx"

// class OfstJournal : public LoptJournal {
//     void OfstJournal(OfstJournal& __that);
//     void OfstJournal(AcisJournal& aj);
//     void OfstJournal();
//     virtual void ~OfstJournal();
//     const char* componentName();
//     void write_offset_face(FACE* fa, SPAbox& region_of_interest, double offset_distance, int& part_inv, int* remake_face, int* adaptive_flag, offset_options* pOffOpts);
//     void write_edm_offset_face(FACE* face, edm_tool_motion* tool_motion, AcisOptions* ao);
//     void write_offset_planar_wire(WIRE* given_wire, TRANSFORM* trans, double offset_distance, SPAunit_vector& wire_normal, AcisOptions* ao);
//     void write_offset_planar_wire(BODY* given_wire, double offset_dist, SPAunit_vector& wire_normal, AcisOptions* ao);
//     void write_offset_planar_wire(BODY* given_wire, law* offset_law, law* twist_law, SPAunit_vector& wire_normal, int gap_type, int trim, int overlap, AcisOptions* ao);
//     void write_offset_planar_wire(BODY* iGivenWire, wire_offset_options* iOptions, AcisOptions* ao);
//     void write_api_offset_face(FACE* given_face, double offset, offset_options* pOffOpts, AcisOptions* ao);
//     void write_api_offset_face_edge(COEDGE* given_coedge, double offset, AcisOptions* ao);
//     void write_api_offset_face_loops(FACE* iFace, ENTITY_LIST& iEdges, double iOffsetDist, AcisOptions* iAcisOpts);
//     void write_api_offset_face_loops(FACE* iFace, double iOffsetDist, AcisOptions* iAcisOpts);
//     void write_edm_tool_motion(edm_tool_motion* tool_motion);
//     void write_api_offset_edges_on_faces(BODY* wire_body, BODY* body_with_faces, offset_on_faces which_faces, double offset_dist, offset_edges_options* offset_edges_opts, AcisOptions* ao);
//     void write_api_offset_edges_on_faces(ENTITY_LIST& coedge_chain, offset_on_faces which_faces, double offset_dist, offset_edges_options* offset_edges_opts, AcisOptions* ao);
//     void write_api_offset_edges_on_faces(EDGE* edge, double offset_dist, offset_edges_options* offset_edges_opts, AcisOptions* ao);
//     void write_api_offset_edges_on_faces(LOOP* inp_loop, offset_on_faces which_faces, double offset_dist, offset_edges_options* offset_edges_opts, AcisOptions* ao);
//     void write_api_offset_edges_on_faces(FACE* inp_face, offset_on_faces which_faces, double offset_dist, offset_edges_options* offset_edges_opts, AcisOptions* ao);
//     void write_ipi_offset_surface(FACE* given_face, double offset, offset_options* pOffOpts, AcisOptions* ao);
//     void write_ipi_offset_surface_map_bs2curves(surface* given_surface, surface* offset_surface, double offset_distance, int num_curves, bs2_curve_def** given_bs2_curves, AcisOptions* ao);
//     void write_wire_offset_options(wire_offset_options* iOpts);
//     OfstJournal& operator=(OfstJournal& __that);
//     virtual void* __vecDelDtor();
// };

void J_api_offset_face(FACE* given_face, double offset_distance, offset_options* pOffOpts, AcisOptions* ao) {
    // AcisJournal* p_dummy;  // [rsp+30h] [rbp-248h]
    //// OfstJournal ofst_journal;  // [rsp+40h] [rbp-238h] BYREF
    // AcisJournal dummy;  // [rsp+160h] [rbp-118h] BYREF

    //// AcisJournal::AcisJournal(&dummy);
    // if(ao)
    //     // p_dummy = ao->get_journal(ao);
    //     p_dummy = &(ao->get_journal());
    // else
    //     p_dummy = &dummy;
    //// OfstJournal::OfstJournal(&ofst_journal, p_dummy);
    // OfstJournal ofst_journal(p_dummy);  // yhy add
    // AcisJournal::start_api_journal(&ofst_journal, "api_offset_face", 1);
    // OfstJournal::write_api_offset_face(&ofst_journal, given_face, offset_distance, pOffOpts, ao);
    // AcisJournal::end_api_journal(&ofst_journal);
    // OfstJournal::~OfstJournal(&ofst_journal);
    // AcisJournal::~AcisJournal(&dummy);
}