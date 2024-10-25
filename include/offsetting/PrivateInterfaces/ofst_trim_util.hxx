#pragma once
#include "acis/alltop.hxx"

int check_split_positoin(curve* iCurveGeom, const double iSplitParam, const double iStartParam, const double iEndParam, SPAposition& iSplitPos, SPAposition& iStartPos, SPAposition& iEndPos);

void display_segment_split_status(EDGE* iEdge1, EDGE* iEdge2, curve* iCurve, SPAposition* iPoint);

int tolerate_wire_edges(BODY* ioWireBody);  // 暂未实现