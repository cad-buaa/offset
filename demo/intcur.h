#pragma once
#include "acis/acis_options.hxx"
#include "acis/alltop.hxx"
#include "acis/api.hxx"
#include "acis/ellipse.hxx"
#include "acis/intcucu.hxx"
#include "acis/intdef.hxx"
#include "acis/lists.hxx"
#include "intcur.h"

outcome aei_INTCUR_1(ENTITY_LIST& output_ents, AcisOptions* ptrAcisOpt);

class ofstintcur_linear_extender : public ACIS_OBJECT {
  public:
    ofstintcur_linear_extender(ofstintcur_linear_extender& iSource);
    ofstintcur_linear_extender();
    ~ofstintcur_linear_extender();
    bool operator==(ofstintcur_linear_extender& iSrc);
    SPAinterval get_extended_range();
    double get_start_param();
    double get_end_param();
    void set_extender_point(SPAinterval& iExtendedRange, SPAposition& iExtendedPosition, SPAvector& iExtendedTangent, const int iReverse);
    SPAunit_vector get_direction();
    void get_extender_point(SPAinterval& oExtendedRange, SPAposition& oExtendedPosition, SPAvector& oExtendedTangent, int& oReverse);
    int inside_extension(const double iParam);
    int eval(const double iParam, SPAposition& oPos, SPAvector& oTan);
    void reparam(const double iNewEndparam, const double iScale);
    double param(SPAposition& iPoint);
    void operator*=(SPAtransf& t);
    int split(double iParam, SPAposition& iPos, ofstintcur_linear_extender** oExtPieces);
    void save_data();
    void restore_data();
    void* __vecDelDtor();

    int mReverse;
    SPAinterval mExtendedRange;
    SPAposition mStartPosition;
    SPAunit_vector mStartDirection;
    double mScale;
};

struct ag_spline {
    ag_spline* next;
    ag_spline* prev;
    int ctype;
    int dim;
    int m;
    int n;
    int rat;
    int form;
    ag_cnode* node0;
    ag_cnode* noden;
    ag_cnode* node;
    ag_mmbox* bsbox;
    ag_substr* agonly;
};
struct ag_cnode {
    ag_cnode* next;
    ag_cnode* prev;
    double* Pw;  // 带权重的点
    double* t;   // 点所在的参数
};
struct ag_substr {
    ag_substr* next;
    int type;
    int size;
    char* ptr;
};
struct ag_mmbox {
    double* min;
    double* max;
};
class gme_offset_int_cur : public int_cur {
  public:
    SPAunit_vector point_direction(SPAposition& point, SPAparameter& param_guess);

    curve* orig_curve;
    SPAunit_vector offset_plane;
    SPAinterval curve_range;
    ofstintcur_linear_extender* mStartExtender;
    ofstintcur_linear_extender* mEndExtender;
    law* dist_law;
    law* twist_law;
};
struct ag_cpoint {
    ag_cpoint* next;
    ag_cpoint* prev;
    double* P;
};
class ag_vec : public ACIS_OBJECT {
  public:
    ag_vec(SPApar_pos& pos);
    ag_vec(SPAvector& vec);
    ag_vec(SPAposition& pos);
    double P[3];
};

class agspline_thread_ctx;

int bs3_curve_evaluate(double param, bs3_curve_def* cur, SPAposition& pos, const SPAvector** deriv, int nd, int index);
SPAinterval bs3_curve_range(bs3_curve_def* cur);
int ag_cls_pt_bs(ag_spline* bs, double* P, double* tmin, int* ERR);
int ag_cls_bs(ag_spline* bs, int (*f)(ag_spline*, char*, double**), char* par, double* P, double* d2min, double* tmin, int* ERR);
int ag_n_sp_in_bs(ag_spline* bs);  // 返回    的数量
int ag_min_dist_f(ag_spline* __formal);
int ag_cls_sp(ag_spline* bs, int (*f)(ag_spline*, char*, double**), char* par, double* P, double* d2min, double* tmin, int* ERR, ag_cnode* curr_node);
