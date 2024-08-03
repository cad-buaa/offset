#pragma once

#include "acis/alltop.hxx"
#include "acis/api.hxx"
#include "acis/offset_opts.hxx"

outcome offset_face_internal(FACE* given_face, double offset_distance, FACE*& offset_face, offset_options* pOffOpts);
