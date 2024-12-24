#include"PublicInterfaces/gme_ofstapi.hxx"
#include "PublicInterfaces/gme_sgcofrtn.hxx"
#include "acis/module.hxx"
#include "acis/check.hxx"
#include "acis/off_utl.hxx"
#include "acis/api.err"
#include "acis/acistype.hxx"
#include "acis/straight.hxx"
#include "acis/point.hxx"
#include "acis/cstrapi.hxx"
#include "acis/wire_utl.hxx"
#include "acis/main_law.hxx"


#define MODULE() api
extern DECL_BASE module_debug api_module_header;


outcome gme_api_offset_planar_wire(BODY* given_wire, law* offset_law, law* twist_law, SPAunit_vector& wire_normal, BODY*& offset_wire, int gap_type, int trim, int overlap, AcisOptions* ao)
{
  
    DEBUG_LEVEL(DEBUG_CALLS) {
       acis_fprintf(debug_file_ptr, "calling api_offset_planar_wire\n");
    }
    API_BEGIN
    int made_twist_law_here = 0;
    ACISExceptionCheck("API");
    call_update_from_bb = 1;
    if(api_check_on()) {
        check_wire_body(given_wire, 0, 0);
        check_pos_length(wire_normal.len(), "normal");
    }
    if(ao && ao->journal_on()) {
        // J_api_offset_planar_wire(iGivenWire, iOptions, ao);
    }
    order_wire_coedges(given_wire);
    if(!given_wire->lump()) goto LABEL_28;
    if(!given_wire->lump()->shell()) goto LABEL_28;
    if(!given_wire->lump()->shell()->wire_list()) goto LABEL_28;
    if(given_wire->lump()->shell()->wire_list() ->next_in_list(PAT_CAN_CREATE)) goto LABEL_28;
    if(is_STRAIGHT((ENTITY*)given_wire->lump()->shell()->wire_list()->coedge()->edge()->geometry()) && offset_law && offset_law->constant() && twist_law && twist_law->linear() && !twist_law->constant()) 
    {
         SPAposition v28 = ((STRAIGHT*)given_wire->lump()->shell()->wire_list()->coedge()->edge()->geometry())->root_point();
         SPAposition axis_start(v28);
         SPAposition axis_end(given_wire->lump()->shell()->wire_list()->coedge()->start()->geometry()->coords());
         if(axis_start == axis_end) 
         {
             axis_end = given_wire->lump()->shell()->wire_list()->coedge()->end()->geometry()->coords();
         }
         SPAvector axis_dir(((STRAIGHT*)given_wire->lump()->shell()->wire_list()->coedge()->edge()->geometry())->direction());
         SPAvector start_dir = axis_dir * wire_normal;
         double radius = 1.0;
         offset_law->evaluate(nullptr,&radius);
  
         SPAparameter start_par = given_wire->lump()->shell()->wire_list()->coedge()->start_param();
         SPAparameter end_par = given_wire->lump()->shell()->wire_list()->coedge()->end_param();
         double start_rad = twist_law->eval(start_par.operator double());
         double end_rad = twist_law->eval(end_par.operator double());
         double turns = (end_rad - start_rad) / 6.283185307179586;
         int handiness = 1;
         if(SPAresabs > turns) {
             handiness = 0;
             turns = -turns;
         }
         SPAvector v37 = axis_end-axis_start;
         double dist = v37.len();
         double thread_distance = dist / turns;
         EDGE*offset_edge = NULL;
         result = api_edge_helix(axis_start, axis_end, start_dir, radius, dist / turns, handiness, offset_edge);
         ENTITY_LIST elist;
         elist.add(offset_edge, 1);
         create_wire_from_edge_list(elist, offset_wire);
    } 
    else 
    {
       LABEL_28:
           sg_gap_type gap = (sg_gap_type)(gap_type == arc);
           if(gap_type == 2) gap = natural;
           if(!twist_law) 
           {
               constant_law* v59 = ACIS_NEW constant_law(0.0);
               twist_law = v59;
               made_twist_law_here = 1;
           }
           offset_wire = gme_sg_offset_planar_wire(given_wire, offset_law, twist_law, wire_normal, gap, 0, trim, 0, overlap, 0);
    }
    int bad_result = 0;
    if(!offset_wire || !offset_wire->lump() && !offset_wire->wire()) bad_result = 1;
    if(bad_result) {
        result = outcome(API_FAILED);
    } 
    else {
        result=outcome(0);
    }
    API_END
    DEBUG_LEVEL(DEBUG_FLOW)
    {
        const char* err_ident = find_err_ident(result.error_number());
        acis_fprintf(debug_file_ptr, "leaving api_offset_planar_wire: %s\n", err_ident);
    } 
    return result;
}
    


outcome gme_api_offset_planar_wire(BODY* iGivenWire, wire_offset_options* iOptions, BODY*& oOffsetWire, AcisOptions* ao)
{
    DEBUG_LEVEL(DEBUG_CALLS) {
        acis_fprintf(debug_file_ptr, "calling api_offset_planar_wire\n");
    }
    API_BEGIN
    if(api_check_on()) {
        check_wire_body(iGivenWire, 0, 0);
    }
    if(ao && ao->journal_on()) {
        //J_api_offset_planar_wire(iGivenWire, iOptions, ao);
    }
    order_wire_coedges((ENTITY*)iGivenWire);
    oOffsetWire = gme_sg_offset_planar_wire(iGivenWire, iOptions);
    int bad_result = 0;
    if(!oOffsetWire || !oOffsetWire->lump() && !oOffsetWire->wire()) bad_result = 1;
    if(bad_result) {
        result = outcome(API_FAILED, (error_info*)0);
    } else {
        result = outcome(0, (error_info*)0);
    }
    API_END
    DEBUG_LEVEL(DEBUG_FLOW) {
        acis_fprintf(debug_file_ptr, "leaving api_offset_planar_wire: %s\n", find_err_ident(result.error_number()));
    }
    return result;
    
}