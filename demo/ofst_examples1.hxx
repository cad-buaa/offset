#pragma once

#include "acis/acis_options.hxx"
#include "acis/alltop.hxx"
#include "acis/api.hxx"
#include "acis/lists.hxx"





#include "acis/rnd_api.hxx"
#include "acis/straight.hxx"
#include "acis/point.hxx"
#include "sweep_examples.hxx"
#include "acis/wire_offset_options.hxx"
#include "acis/main_law.hxx"


outcome aei_OFFSET_FACE_1(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt);
outcome aei_OFFSET_FACE_2(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt);
outcome aei_OFFSET_FACE_3(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt);
outcome aei_OFFSET_FACE_4(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt);
outcome aei_OFFSET_FACE_5(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt);

outcome aei_OFFSET_WIRE_1(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt);

//void point_perp(straight* b, const SPAposition& point, SPAposition& foot, SPAunit_vector& tangent, SPAvector& curv, SPAparameter& param_actual);
