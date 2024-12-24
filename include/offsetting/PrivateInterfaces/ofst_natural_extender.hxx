#pragma once
// #include "ProtectedInterfaces/off_cu.hxx"
#include "acis/alltop.hxx"

class curve_curve_int;
class offset_segment;
class offset_segment_list;
class law;

void release_intersection_data(curve_curve_int* mIntsectData);
int extend_offset_int_cur(curve* mCrvGeom, SPAinterval& newInterval);  // 暂有不懂，已检查
int extend_curve_geom(curve* mCrvGeom, const double iExtParamLen, const int mCurve1AtStart);
void debug_display_natural_extender(COEDGE* iCoedge1, COEDGE* iCoedge2, curve_curve_int* iIntersections, curve* iCurve1, curve* iCurve2);
int comp_curve_end_info(curve* iCurve, SPAposition& iEndPos, SPAvector& oEndTangent);
int beyond_intesect(SPAposition& iRefPos, SPAposition& iIntsectPos, SPAposition& iCheckPos);  // 暂未实现
void sg_add_extensions(offset_segment_list& l);//已检查 trm

class ofst_natural_extender {
  public:
    enum ofst_natural_extension_types { eExtensionNone = 0, eExtensionArc = 1, eExtensionNatural = 2 };
    ofst_natural_extender();
    ~ofst_natural_extender();
    int extend(offset_segment* iOfstSegment, law* iDistLaw, COEDGE* iFstOfstCoedge, COEDGE* iSndOfstCoedge, VERTEX* iCommonVeertex, int iNinimumTopo, int skipbackup);  // 暂未实现
    int get_extension_type();
    int try_old_extension();                                                                  
    int try_new_extension(const double iCurve1ExtParamLen, const double iCurve2ExtParamLen);  
    int extend_with_arc();                                                                    
    int get_coedge_geomtry();
    int init(offset_segment* iOfstSegment, law* iDistLaw, COEDGE* iFstOfstCoedge, COEDGE* iSndOfstCoedge, VERTEX* iCommonVeertex);
    curve_curve_int* comp_intersections();
    int do_extension(curve_curve_int* iIntersections);
    int comp_init_extension_length(double& oCurve1ExtParamLen, double& oCurve2ExtParamLen);
    void filter_intersections(curve_curve_int*& ioIntersections);
    int check_spacial_caes();

    int mExtensionType;
    int mFailCase;
    double mOfstDistance;
    SPAposition mCommonVrtPos;
    double coord[3];
    SPAposition mC1End;
    SPAposition mC2Start;
    SPAposition mIntersectPoint;
    offset_segment* mOfstSegment;
    law* mDistLaw;
    COEDGE* mC1;
    COEDGE* mC2;
    VERTEX* mCommVertex;
    curve* mMainCurve;
    curve* mC1Geom;
    curve* mC2Geom;
    int mCurve1AtStart;
    int mCurve2AtStart;
};
