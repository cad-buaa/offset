#include "PublicInterfaces/off_misc.hxx"

#include "acis/ckoutcom.hxx"
#include "acis/kernapi.hxx"
#include "acis/wire.hxx"

void order_wire_coedges(ENTITY* wire_body) {
    bool postR22Sp1 = GET_ALGORITHMIC_VERSION() > AcisVersion(22, 0, 1);
    WIRE* wire = ((BODY*)wire_body)->lump()->shell()->wire();  // 不确定，加了lump->>shell
    if(wire) {
        do {
            order_wire_coedges(wire);
            wire = wire->next(PAT_CAN_CREATE);
        } while(postR22Sp1 && wire);
    } else {
        ENTITY_LIST wire_shells;
        outcome shells = api_get_shells(wire_body, wire_shells, PAT_CAN_CREATE);
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
                    } while(postR22Sp1 && wire);
                }
                wire_shell = (SHELL*)wire_shells.next();
            } while(postR22Sp1 && wire_shell);
        }
    }
}
