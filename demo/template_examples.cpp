/*********************************************************************
 * @file    template_examples.cpp
 * @brief
 * @details
 * @author  CastaneaG
 * @date    2023.6.17
 *********************************************************************/

#include "template_examples.hxx"

#include "access.hpp"
#include "acis/acis.hxx"
#include "acis/alltop.hxx"
#include "acis/ckoutcom.hxx"
#include "template_simple_api.hxx"

// **********************************************************************************
// C++ Example:
//
// Title:
//	  cuboid_demo:
//
// APIs:
//    gme_api_make_cuboid
//
// Description:
//	  create cuboid by call gme_api_make_cuboid.
//    Save the cuboid entity to .sat file, then restore it.
// **********************************************************************************

outcome aei_CUBOID_DEMO(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt) {
    /* Initialization Block */

    BODY* body = NULL;  // Pointer to tool body

    /* API Call Block */
    API_BEGIN
    check_outcome(gme_api_make_cuboid(2.0, 2.0, 2.0, body));
    acis_api_save_entity("C:\\Users\\Lenovo\\Desktop\\cuboid.sat", body);
    API_END

    if(result.ok()) {
        ENTITY* ent = nullptr;
        acis_api_restore_entity("C:\\Users\\Lenovo\\Desktop\\cuboid.sat", ent);
        output_ents.add(ent);
    }
    return result;
}