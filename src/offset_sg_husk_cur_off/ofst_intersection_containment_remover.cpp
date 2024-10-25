#include "PrivateInterfaces/ofst_intersection_containment_remover.hxx"

#include "ProtectedInterfaces/ofsttools.hxx"
// #include "acis/container_utils.hxx"

ofst_intersection_containment_remover::ofst_intersection_containment_remover() {
    this->mHalfParaRange = 0.0;
    this->mCurveClosed = 0;
    this->mNoRemoveNormalIntersections = 0;
}
ofst_intersection_containment_remover::~ofst_intersection_containment_remover() {
    return;
}
void ofst_intersection_containment_remover::set_curve_info(const double iHalfParamRange, const int iCurveClosed, const int iNotRemoveNormalIntersections) {
    this->mHalfParaRange = iHalfParamRange;
    this->mCurveClosed = iCurveClosed;
    this->mNoRemoveNormalIntersections = iNotRemoveNormalIntersections;
}
int ofst_intersection_containment_remover::remove_contained_intersections(curve_curve_int*& ioIntersections) {
    this->reset_mark(ioIntersections);
    this->mark_contained_intersections(ioIntersections);
    return this->remove_marked_intersections(ioIntersections);
}
int ofst_intersection_containment_remover::remove_contained_cusps(curve_curve_int* iIntersections, curve_curve_int*& ioCusps, int iRemoveContained) {
    this->reset_mark(ioCusps);
    this->mark_coincident_cusps(ioCusps);
    this->mark_contained_cusps(iIntersections, ioCusps, iRemoveContained);
    return this->remove_marked_intersections(ioCusps);
}
int ofst_intersection_containment_remover::intersection_contained(curve_curve_int* iIntersect1, curve_curve_int* iIntersect2) {
    int contained = 0;
    if(iIntersect1 && iIntersect2) {
        if(iIntersect2->param1 <= iIntersect1->param1 || iIntersect1->param2 <= iIntersect2->param2) {
            if(this->mCurveClosed && iIntersect1->param2 - iIntersect1->param1 > 1.6 * this->mHalfParaRange) contained = 1;
        } else {
            contained = this->mHalfParaRange > iIntersect1->param2 - iIntersect1->param1;
        }
    }
    if(contained && this->mNoRemoveNormalIntersections && iIntersect2->high_rel == cur_cur_normal && iIntersect2->low_rel == cur_cur_normal) {
        return 0;
    }
    return contained;
}
void ofst_intersection_containment_remover::reset_mark(curve_curve_int* ioIntersections) {
    while(ioIntersections) {
        if(ioIntersections->param1 > ioIntersections->param2) {
            double tmp = ioIntersections->param1;
            ioIntersections->param1 = ioIntersections->param2;
            ioIntersections->param2 = tmp;
        }
        ioIntersections->uv_set = 0;
        ioIntersections = ioIntersections->next;
    }
}
void ofst_intersection_containment_remover::mark_contained_intersections(curve_curve_int* ioIntersections) {
    while(ioIntersections) {
        if(!ioIntersections->uv_set) {
            for(curve_curve_int* checkIntersect = ioIntersections->next; checkIntersect; checkIntersect = checkIntersect->next) {
                if(!checkIntersect->uv_set) {
                    if((unsigned int)this->intersection_contained(ioIntersections, checkIntersect)) {
                        checkIntersect->uv_set = 1;
                    } else if((unsigned int)this->intersection_contained(checkIntersect, ioIntersections)) {
                        ioIntersections->uv_set = 1;
                    }
                }
            }
        }
        ioIntersections = ioIntersections->next;
    }
}
void ofst_intersection_containment_remover::mark_contained_cusps(curve_curve_int* iIntersections, curve_curve_int* ioCusps, int iRemoveContained) {
    if(iIntersections && ioCusps) {
        for(curve_curve_int* crrCusp = ioCusps; crrCusp; crrCusp = crrCusp->next) {
            for(const curve_curve_int* crrIntesect = iIntersections; crrIntesect && !crrCusp->uv_set; crrIntesect = crrIntesect->next)  // 不确定
            {
                if(iRemoveContained) {
                    crrCusp->uv_set = this->intersection_contained((curve_curve_int*)crrIntesect, crrCusp);  // 不确定
                } else {
                    crrCusp->uv_set = same_point(crrCusp->int_point, crrIntesect->int_point, SPAresfit);
                }
            }
        }
    }
}
void ofst_intersection_containment_remover::mark_coincident_cusps(curve_curve_int* ioCusps) {
    while(ioCusps) {
        if(!ioCusps->uv_set) {
            for(curve_curve_int* nxtCusp = ioCusps->next; nxtCusp && !nxtCusp->uv_set; nxtCusp = nxtCusp->next) {
                double v4 = abs(nxtCusp->param1 - ioCusps->param1);
                if(SPAresabs > v4 && nxtCusp->int_point == ioCusps->int_point) {
                    nxtCusp->uv_set = 1;
                }
            }
        }
        ioCusps = ioCusps->next;
    }
}
int ofst_intersection_containment_remover::remove_marked_intersections(curve_curve_int*& ioIntersectiions) {
    BOOL didRemove;
    curve_curve_int* removedIntersections = nullptr;
    curve_curve_int* lastRemovedIntersect = nullptr;
    curve_curve_int* preIntersect = nullptr;
    curve_curve_int* crrIntersect = ioIntersectiions;

    while(crrIntersect) {
        if(crrIntersect->uv_set) {
            curve_curve_int* removedIntersect = crrIntersect;
            crrIntersect = crrIntersect->next;
            if(preIntersect) {
                preIntersect->next = crrIntersect;
            } else {
                ioIntersectiions = crrIntersect;
            }
            if(removedIntersections) {
                lastRemovedIntersect->next = removedIntersect;
            } else {
                removedIntersections = removedIntersect;
                lastRemovedIntersect = removedIntersect;
            }
            lastRemovedIntersect = removedIntersect;
            removedIntersect->next = nullptr;
        } else {
            preIntersect = crrIntersect;
            crrIntersect = crrIntersect->next;
        }
    }
    didRemove = removedIntersections != nullptr;
    sg_delete_cci(removedIntersections);
    return didRemove;
}
