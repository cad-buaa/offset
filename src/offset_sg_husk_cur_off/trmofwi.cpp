#include "ProtectedInterfaces/ofsttools.hxx"

COEDGE* start_of_wire_chain(WIRE* w) {
    COEDGE* wire_coedge = w->coedge();
    COEDGE* this_coedge = wire_coedge;
    COEDGE* prev_coedge=nullptr;
    for(prev_coedge = wire_coedge->previous(); prev_coedge != this_coedge && prev_coedge != wire_coedge && prev_coedge; prev_coedge = prev_coedge->previous()) {
        this_coedge = prev_coedge;
    }
    if(prev_coedge) 
    {
        return prev_coedge;
    }
    return this_coedge;
}