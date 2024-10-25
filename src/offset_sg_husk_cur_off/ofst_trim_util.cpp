#include "PrivateInterfaces/ofst_trim_util.hxx"
#include "ProtectedInterfaces/ofsttools.hxx"
#include "acis/alltop.hxx"
#include "acis/get_top.hxx"


int tolerate_wire_edges(BODY* ioWireBody) 
{
    if(ioWireBody) {
        ENTITY_LIST edges;
        get_edges(ioWireBody, edges, PAT_CAN_CREATE);
        ENTITY_LIST bad_edges;
        ENTITY_LIST new_edges;
        ENTITY* worst_entity = 0;
        double worst_error = 0.0;
        check_edge_error(edges, bad_edges, worst_entity, worst_error, SPAresabs, 0, new_edges, 1, nullptr);
        ENTITY_LIST vertices;
        ENTITY_LIST bad_vertices;
        ENTITY_LIST new_vertices;
        get_vertices(ioWireBody, vertices, PAT_CAN_CREATE);
        double worst_vert_error = 0.0;
        check_vertex_error(vertices, bad_vertices, worst_entity, worst_vert_error, SPAresabs, 0, new_vertices, 1, SpaAcis::NullObj::get_logical(), nullptr, nullptr);
        if(worst_vert_error > worst_error) worst_error = worst_vert_error;

    }
    return 1;
}
