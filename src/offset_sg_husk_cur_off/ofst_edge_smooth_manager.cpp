
#include "PrivateInterfaces/ofst_edge_smooth_manager.hxx"
#include "acis/cur_off.err"
#include "acis/testwire.hxx"
#include "acis/unitvec.hxx"
#include "ProtectedInterfaces/ofsttools.hxx"
#include "PrivateInterfaces/apiFinder.hxx"

extern APIFINDER apiFinderACIS;
extern PROCSTATE prostate;

int is_planar_wire(WIRE* this_wire, SPAposition& centroid, SPAunit_vector& normal, int apply_transf, int use_winding_number) {
    typedef int(*is_planar_wire_type)(WIRE* this_wire, SPAposition& centroid, SPAunit_vector& normal, int apply_transf, int use_winding_number);
    is_planar_wire_type f = (is_planar_wire_type)apiFinderACIS.GetAddress("?is_planar_wire@@YAHPEAVWIRE@@AEAVSPAposition@@AEAVSPAunit_vector@@HH@Z",prostate);
    if(f) {
        return f(this_wire,centroid,normal,apply_transf,use_winding_number);
    } else {
        return NULL;
    }
}

ofst_edge_smooth_manager::ofst_edge_smooth_manager() {

    this->mBaseEdges = ENTITY_LIST();
    this->mNumEdges = 0;
    this->mIteration = 0;
    this->mMaxIteration = 5;
    this->mMaxSmoothLevel = 0;
    this->mTangential = 0;
    this->mSmoothLevel = nullptr;
    this->mLastSmoothLevel = nullptr;
    this->mNumIntersections = nullptr;
    this->mNumOverlaps = nullptr;

}

ofst_edge_smooth_manager::~ofst_edge_smooth_manager() {
    this->relese_allocations();
    this->mBaseEdges .~ENTITY_LIST();
}

void ofst_edge_smooth_manager::relese_allocations() {
    if(this->mSmoothLevel) {
        delete[](this->mSmoothLevel);
        this->mSmoothLevel = nullptr;
    }
    if(this->mLastSmoothLevel) {
        delete[](this->mLastSmoothLevel);
        this->mLastSmoothLevel = nullptr;
    }
    if(this->mNumIntersections) {
        delete[](this->mNumIntersections);
        this->mNumIntersections = nullptr;
    }
    if(this->mNumOverlaps) {
        delete[](this->mNumOverlaps);
        this->mNumOverlaps = nullptr;
    }
}

int ofst_edge_smooth_manager::init(ENTITY_LIST* iEdges, int iMaxSmoothLevel, int iMaxIteration) {
    return NULL;
}

int ofst_edge_smooth_manager::apply_more_smooth(int iNumIteration) {
    return NULL;
}