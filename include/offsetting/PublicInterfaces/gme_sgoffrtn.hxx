#pragma once
#include "acis/logical.h"
#include "gme_dcl_ofst.h"

DECL_OFST logical gme_sg_at_apex(  // Return TRUE
  const SPAposition& test_pos,     // if test_pos is at the apex of
  const surface* surf              // this surface
);

DECL_OFST SPAposition gme_sg_offset_pos(  // Return SPAposition of offset point
  const SPAposition&,                     // Point to offset
  const surface*,                         // Surface to offset from
  double);
