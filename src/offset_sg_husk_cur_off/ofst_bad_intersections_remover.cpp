#include "PrivateInterfaces/ofst_bad_intersections_remover.hxx"

ofst_bad_intersections_remover::ofst_bad_intersections_remover() {
    this->mStatus = nullptr;
    this->mParams = nullptr;
    this->mIntersections = nullptr;
    this->mNumParameters = 0;
    this->mCurveParamRange = 0.0;
    this->mMaxIntersectionRange = 0.0;
}
ofst_bad_intersections_remover::~ofst_bad_intersections_remover() {
    this->release_allocation();
    this->mIntersections = nullptr;
}
void ofst_bad_intersections_remover::release_allocation() {
    this->mNumParameters = 0;
    if(this->mStatus) {
        ACIS_DELETE this->mStatus;
    }
    if(this->mParams) {
        ACIS_DELETE this->mParams;
    }
    if(this->mIntersections) {
        ACIS_DELETE this->mIntersections;
    }
}

int ofst_bad_intersections_remover::process_intersections(curve_curve_int* iIntersections, curve_curve_int* iCusps, curve_curve_int*& oResultIntersections, double iParamRange) {
    return NULL;
    // int retOk;                              // [rsp+30h] [rbp-18h]
    //     unsigned int badIntersectionExtracted;  // [rsp+34h] [rbp-14h]
    //
    //     if(good_intersections(iIntersections, 0i64) && good_intersections(iCusps, 0i64)) return 0i64;
    //     this->mCurveParamRange = iParamRange;
    //     this->mMaxIntersectionRange = 0.2 * iParamRange;
    //     ofst_bad_intersections_remover::release_allocation(this);
    //     retOk = ofst_bad_intersections_remover::make_allocation(this, iIntersections, iCusps);
    //     if(retOk) retOk = ofst_bad_intersections_remover::build_array(this, iIntersections, iCusps);
    //     badIntersectionExtracted = 0;
    //     if(retOk) return (unsigned int)ofst_bad_intersections_remover::create_result_intersections(this, oResultIntersections);
    //     return badIntersectionExtracted;
}
