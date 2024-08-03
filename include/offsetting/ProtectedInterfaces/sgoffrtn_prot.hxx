#pragma once
#include "acis/alltop.hxx"
#include "acis/offset_opts.hxx"

FACE* sg_offset_face(FACE* original_face, double offset_distance, int& failed_full_surface, offset_options* pOffsetOptions);
