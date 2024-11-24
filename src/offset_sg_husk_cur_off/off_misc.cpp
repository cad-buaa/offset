#include "PublicInterfaces/off_misc.hxx"

#include "acis/ckoutcom.hxx"
#include "acis/kernapi.hxx"
#include "acis/wire.hxx"


void order_wire_coedges(WIRE*& wire) {



    ENTITY_LIST coedge_list;
    outcome coedges = api_get_coedges((ENTITY*)wire, coedge_list, PAT_CAN_CREATE);
    check_outcome(coedges);
    if(coedge_list.count() > 2) {
        COEDGE* start_coedge = wire->coedge();
        COEDGE* current_coedge = start_coedge;
        for(COEDGE* next_coedge = start_coedge->next(); next_coedge && next_coedge != current_coedge && next_coedge != start_coedge; next_coedge = next_coedge->next()) {
            if(next_coedge->previous() != current_coedge) {
                COEDGE* prev = next_coedge->previous();
                next_coedge->set_previous(current_coedge, 0, 1);
                next_coedge->set_next(prev, 0, 1);
                if(next_coedge->sense())
                    next_coedge->set_sense(0, 1);
                else
                    next_coedge->set_sense(1, 1);
            }
            current_coedge = next_coedge;
        }
        COEDGE* current_coedgea = start_coedge;
        for(COEDGE* previous_coedge = start_coedge->previous(); previous_coedge && previous_coedge != current_coedgea && previous_coedge != start_coedge; previous_coedge = previous_coedge->previous()) {
            if(previous_coedge->next() != current_coedgea) {
                COEDGE* next = previous_coedge->next();
                previous_coedge->set_next(current_coedgea, 0, 1);
                previous_coedge->set_previous(next, 0, 1);
                if(previous_coedge->sense())
                    previous_coedge->set_sense(0, 1);
                else
                    previous_coedge->set_sense(1, 1);
            }
            current_coedgea = previous_coedge;
        }
    }

}
void order_wire_coedges(ENTITY* wire_body) {
    bool postR22Sp1 = GET_ALGORITHMIC_VERSION() > AcisVersion(22, 0, 1);
    BODY* wirebody = (BODY*)wire_body;
    WIRE* wire = ((BODY*)wire_body)->wire();  
    if(wire) {
        do {
            order_wire_coedges(wire);
            wire = wire->next(PAT_CAN_CREATE);
        } while(postR22Sp1 && wire);
    } 
    else 
    {
        ENTITY_LIST wire_shells;
        outcome shells = api_get_shells((BODY*)wire_body, wire_shells, PAT_CAN_CREATE);
        check_outcome(shells);
        wire_shells.init();
        SHELL* wire_shell = (SHELL*)wire_shells.next();
        if(wire_shell) {
            do {
                wire = wire_shell->wire();
                if(wire) {
                    do {
                        order_wire_coedges(wire);
                        wire = wire->next(PAT_CAN_CREATE);
                    } 
                    while(postR22Sp1 && wire);
                }
                wire_shell = (SHELL*)wire_shells.next();
            } while(postR22Sp1 && wire_shell);
        }
    }
}
