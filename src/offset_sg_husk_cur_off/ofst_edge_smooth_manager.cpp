#include "PrivateInterfaces/ofst_edge_smooth_manager.hxx"

#include "PrivateInterfaces/apiFinder.hxx"
#include "ProtectedInterfaces/ofsttools.hxx"
#include "acis/cur_off.err"
#include "acis/sp3crtn.hxx"
#include "acis/testwire.hxx"
#include "acis/unitvec.hxx"

extern APIFINDER apiFinderACIS;
extern PROCSTATE prostate;

// int is_planar_wire(WIRE* this_wire, SPAposition& centroid, SPAunit_vector& normal, int apply_transf, int use_winding_number) {
//     typedef int (*is_planar_wire_type)(WIRE* this_wire, SPAposition& centroid, SPAunit_vector& normal, int apply_transf, int use_winding_number);
//     is_planar_wire_type f = (is_planar_wire_type)apiFinderACIS.GetAddress("?is_planar_wire@@YAHPEAVWIRE@@AEAVSPAposition@@AEAVSPAunit_vector@@HH@Z", prostate);
//     if(f) {
//         return f(this_wire, centroid, normal, apply_transf, use_winding_number);
//     } else {
//         return NULL;
//     }
// }
// bs3_curve_def* bs3_curve_snap_end_curvatures(bs3_curve_def* crv, SPAvector& start_crv, SPAvector& end_crv, double smoothing) {
//    typedef bs3_curve_def* (*bs3_curve_snap_end_curvatures)(bs3_curve_def* crv, SPAvector& start_crv, SPAvector& end_crv, double smoothing);
//    bs3_curve_snap_end_curvatures f = (bs3_curve_snap_end_curvatures)apiFinderACIS.GetAddress("?bs3_curve_snap_end_curvatures@@YAPEAVbs3_curve_def@@PEAV1@AEBVSPAvector@@1N@Z", prostate);
//    if(f) {
//        return f(crv, start_crv, end_crv, smoothing);
//    } else {
//        return nullptr;
//    }
//}
// double bs3_cpt_dist(bs3_curve_def* bs3_0, bs3_curve_def* bs3_1) {
//    typedef double (*bs3_cpt_dist)(bs3_curve_def* bs3_0, bs3_curve_def* bs3_1);
//    bs3_cpt_dist f = (bs3_cpt_dist)apiFinderACIS.GetAddress("?bs3_cpt_dist@@YANPEAVbs3_curve_def@@0@Z", prostate);
//    if(f) {
//        return f(bs3_0, bs3_1);
//    } else {
//        return NULL;
//    }  // 陆
//}
// outcome sg_split_edge_at_disc(EDGE* theEdge, ENTITY_LIST& new_edges, int cont_type) {
//     typedef outcome (*sg_split_edge_at_disc)(EDGE* theEdge, ENTITY_LIST& new_edges, int cont_type);
//     sg_split_edge_at_disc f = (sg_split_edge_at_disc)apiFinderACIS.GetAddress("?sg_split_edge_at_disc@@YA?AVoutcome@@PEAVEDGE@@AEAVENTITY_LIST@@H@Z", prostate);
//     if(f) {
//         return f(theEdge, new_edges, cont_type);
//     }
//     else {
//         return NULL;  // 不确定
//     }
// }

// 陆
// bool Debug_Break_Active(const char* name, const char* component) {
//     typedef bool (*Debug_Break_Active)(const char* name, const char* component);
//     Debug_Break_Active f = (Debug_Break_Active)apiFinderACIS.GetAddress("? Debug_Break_Active @ @YA_NPEBD0 @Z", prostate);
//     if(f) {
//         return f(name, component);
//     } else {
//         return NULL;  // 不确定
//     }
// }

// void merge_vertex(VERTEX* vertex, int (*edge_sel)(EDGE*, EDGE*)) {
//     typedef void (*merge_vertex)(VERTEX* vertex, int (*edge_sel)(EDGE*, EDGE*));
//     merge_vertex f = (merge_vertex)apiFinderACIS.GetAddress("?merge_vertex@@YAXPEAVVERTEX@@P6AHPEAVEDGE@@1@Z@Z", prostate);
//     if(f) {
//         return f(vertex, edge_sel);
//     } else {
//         return;  // 不确定
//     }
// }
// void sg_sort_cci(curve_curve_int*& ccx_dat, int rev, int second) {
//     typedef void (*sg_sort_cci)(curve_curve_int*& ccx_dat, int rev, int second);
//     sg_sort_cci f = (sg_sort_cci)apiFinderACIS.GetAddress("?sg_sort_cci@@YAXAEAPEAVcurve_curve_int@@HH@Z", prostate);
//     if(f) {
//         return f(ccx_dat, rev, second);
//     } else {
//         return;
//     }
// }
// void sg_debug_cci(curve_curve_int* rel_data, _iobuf* fptr, const char* __formal) {
//     typedef void (*sg_debug_cci)(curve_curve_int* rel_data, _iobuf* fptr, const char* __formal);
//     sg_debug_cci f = (sg_debug_cci)apiFinderACIS.GetAddress("?sg_debug_cci@@YAXPEAVcurve_curve_int@@PEAU_iobuf@@PEBD@Z", prostate);
//     if(f) {
//         return f(rel_data, fptr, __formal);
//     } else {
//         return;
//     }
// }
// curve_curve_int* sort_intersections(curve_curve_int* cci) {
//     typedef curve_curve_int* (*sort_intersections)(curve_curve_int* cci);
//     sort_intersections f = (sort_intersections)apiFinderACIS.GetAddress("?sort_intersections@@YAPEAVcurve_curve_int@@PEAV1@@Z", prostate);
//     if(f) {
//         return f(cci);
//     } else {
//         return nullptr;
//     }
// }
// curve_curve_int* reverse_intersections(curve_curve_int* head_cci) {
//     typedef curve_curve_int* (*reverse_intersections)(curve_curve_int* cci);
//     reverse_intersections f = (reverse_intersections)apiFinderACIS.GetAddress("?reverse_intersections@@YAPEAVcurve_curve_int@@PEAV1@@Z", prostate);
//     if(f) {
//         return f(head_cci);
//     } else {
//         return nullptr;
//     }
// }
// void sg_delete_cci(curve_curve_int*& ccx_dat) {
//    typedef void (*sg_delete_cci)(curve_curve_int*& cci);
//    sg_delete_cci f = (sg_delete_cci)apiFinderACIS.GetAddress("?sg_delete_cci@@YAXAEAPEAVcurve_curve_int@@@Z", prostate);
//    if(f) {
//        return f(ccx_dat);
//    } else {
//        return;
//    }
//}
//
// curve_curve_int* get_curve_self_ints(intcurve& intcur, double given_tol) {
//    typedef curve_curve_int* (*get_curve_self_ints)(intcurve& intcur, double given_tol);
//    get_curve_self_ints f = (get_curve_self_ints)apiFinderACIS.GetAddress("?get_curve_self_ints@@YAPEAVcurve_curve_int@@AEBVintcurve@@N@Z", prostate);
//    if(f) {
//        return f(intcur, given_tol);
//    } else {
//        return nullptr;
//    }
//}

// void check_edge_error(ENTITY_LIST& edges, ENTITY_LIST& bad_entities, ENTITY*& worst_entity, double& worst_error, double tol, int stop_immediately, ENTITY_LIST& new_entities, int careful_on, teo_data* tedata)
//{
//     typedef void (*check_edge_error)(ENTITY_LIST& edges, ENTITY_LIST& bad_entities, ENTITY*& worst_entity, double& worst_error, double tol, int stop_immediately, ENTITY_LIST& new_entities, int careful_on, teo_data* tedata);
//     check_edge_error f = (check_edge_error)apiFinderACIS.GetAddress("?check_edge_error@@YAXAEBVENTITY_LIST@@AEAV1@AEAPEAVENTITY@@AEANNH1HPEAVteo_data@@@Z", prostate);
//     if(f) {
//         return f(edges, bad_entities, worst_entity , worst_error,tol,stop_immediately, new_entities, careful_on, tedata);
//     }
//     else {
//         return;
//     }
// }
//
// void check_vertex_error(ENTITY_LIST& vertices, ENTITY_LIST& bad_vertices, ENTITY*& worst_entity, double& worst_error, double tol, int stop_immediately, ENTITY_LIST& new_vertices, int careful_on, int& proceeded_after_errors, error_info_list* eil,
//                         teo_data* tedata)
//{
//     typedef void (*check_vertex_error)(ENTITY_LIST& vertices, ENTITY_LIST& bad_vertices, ENTITY*& worst_entity, double& worst_error, double tol, int stop_immediately, ENTITY_LIST& new_vertices, int careful_on, int& proceeded_after_errors,
//                                        error_info_list* eil, teo_data* tedata);
//     check_vertex_error f = (check_vertex_error)apiFinderACIS.GetAddress("?check_vertex_error@@YAXAEBVENTITY_LIST@@AEAV1@AEAPEAVENTITY@@AEANNH1HAEAHPEAVerror_info_list@@PEAVteo_data@@@Z", prostate);
//     if(f) {
//         return f(vertices, bad_vertices, worst_entity,worst_error, tol, stop_immediately,new_vertices,careful_on,proceeded_after_errors,eil,tedata);
//     }
//     else {
//         return;
//     }
//
// }

// outcome sg_split_edge_at_disc(EDGE* theEdge, ENTITY_LIST& new_edges, int cont_type) {
// typedef outcome(*sg_split_edge_at_disc)(EDGE* theEdge, ENTITY_LIST& new_edges, int cont_type);
// sg_split_edge_at_disc f = (sg_split_edge_at_disc)apiFinderACIS.GetAddress("?sg_split_edge_at_disc@@YA?AVoutcome@@PEAVEDGE@@AEAVENTITY_LIST@@H@Z", prostate);
// if(f) {
//     return f(theEdge, new_edges, cont_type);
// } else {
//     return NULL;
// }
// }

// int bs3_curve_unique_knots(bs3_curve_def* bs3, int& nknots, double*& knots, int*& cont, double tol) {
//     typedef int (*bs3_curve_unique_knots)(bs3_curve_def* bs3, int& nknots, double*& knots, int*& cont, double tol);
//     bs3_curve_unique_knots f = (bs3_curve_unique_knots)apiFinderACIS.GetAddress("?bs3_curve_unique_knots@@YAHPEAVbs3_curve_def@@AEAHAEAPEANAEAPEAHN@Z", prostate);
//     if(f) {
//         return f(bs3, nknots, knots, cont, tol);
//     } else {
//         return NULL;
//     }
// }
// curve_curve_int* remove_intersection(curve_curve_int* head_cci, curve_curve_int* remove_cci) {
//     typedef curve_curve_int* (*remove_intersection)(curve_curve_int* head_cci, curve_curve_int* remove_cci);
//     remove_intersection f = (remove_intersection)apiFinderACIS.GetAddress("?remove_intersection@@YAPEAVcurve_curve_int@@PEAV1@AEBVSPAposition@@@Z", prostate);
//     if(f) {
//         return f(head_cci, remove_cci);
//     } else {
//         return nullptr;
//     }
// }
// void del_entity(ENTITY* ent) {
//     typedef void (*del_entity)(ENTITY* ent);
//     del_entity f = (del_entity)apiFinderACIS.GetAddress("?del_entity@@YAXPEAVENTITY@@@Z", prostate);
//     if(f) {
//         return f(ent);
//     } else {
//         return;
//     }
// }
// SPAvector conv_curvature(SPAvector& dpos, SPAvector& ddpos) {
//     typedef SPAvector (*conv_curvature)(SPAvector& dpos, SPAvector& ddpos);
//     conv_curvature f = (conv_curvature)apiFinderACIS.GetAddress("?conv_curvature@@YA?AVSPAvector@@AEBV1@0@Z", prostate);
//     if(f) {
//         return f(dpos, ddpos);
//     } else {
//         return NULL;
//     }
// }

int do_boolean(BODY* tool_body, BODY* blank_body, BOOL_TYPE type, BODY*& outside_body, BODY*& leftovers, NDBOOL_KEEP ndbool_keep, BODY*& ndbool_result, glue_options* glue_opts, int quick_failure_preferred) {
    typedef int (*do_boolean)(BODY* tool_body, BODY* blank_body, BOOL_TYPE type, BODY*& outside_body, BODY*& leftovers, NDBOOL_KEEP ndbool_keep, BODY*& ndbool_result, glue_options* glue_opts, int quick_failure_preferred);
    do_boolean f = (do_boolean)apiFinderACIS.GetAddress("?do_boolean@@YAHPEAVBODY@@0W4BOOL_TYPE@@AEAPEAV1@2W4NDBOOL_KEEP@@2PEBVglue_options@@H@Z", prostate);
    if(f) {
        return f(tool_body, blank_body, type, outside_body, leftovers, ndbool_keep, ndbool_result, glue_opts, quick_failure_preferred);
    } else {
        return NULL;
    }
}

void get_intersection_info(curve_curve_int* iIntersections, int& oNumIntersections, int& oNumOverlaps, int& oNumUnknown) {
    oNumIntersections = 0;
    oNumOverlaps = 0;
    oNumUnknown = 0;
    for(curve_curve_int* intsect1 = iIntersections; intsect1; intsect1 = intsect1->next) ++oNumIntersections;
    if(oNumIntersections >= 1) {
        for(curve_curve_int* intsect1a = iIntersections; intsect1a; intsect1a = intsect1a->next) {
            if(intsect1a->low_rel == cur_cur_normal || intsect1a->high_rel == cur_cur_normal) {
                for(curve_curve_int* intsect2 = intsect1a->next; intsect2; intsect2 = intsect2->next) {
                    if(&intsect1a->int_point == &intsect2->int_point) {  // 不确定
                        ++oNumOverlaps;
                    } else if(ofst_params_over_lapping(intsect1a->param1, intsect1a->param2, intsect2->param1, intsect2->param2)) {
                        ++oNumOverlaps;
                    }
                }
            } else {
                ++oNumUnknown;
            }
        }
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
    this->mBaseEdges.~ENTITY_LIST();
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
    int retOk = 0;
    int mNumEdges;
    this->mNumEdges = iEdges->header->count();
    if(this->mNumEdges > 0) {
        this->relese_allocations();
        mNumEdges = this->mNumEdges;
        this->mSmoothLevel = ACIS_NEW int[mNumEdges];
        this->mLastSmoothLevel = ACIS_NEW int[mNumEdges];
        this->mNumIntersections = ACIS_NEW int[mNumEdges];
        this->mNumOverlaps = ACIS_NEW int[mNumEdges];

        if(this->mSmoothLevel && this->mNumIntersections && this->mNumOverlaps) {
            for(int i = 0; i < this->mNumEdges; ++i) {
                this->mSmoothLevel[i] = 0;
                this->mLastSmoothLevel[i] = 0;
                this->mNumIntersections[i] = 0;
                this->mNumOverlaps[i] = 0;
            }
            retOk = 1;
        }
    }
    if(retOk) {
        if(iMaxSmoothLevel <= 0)
            this->mMaxSmoothLevel = 10;
        else
            this->mMaxSmoothLevel = iMaxSmoothLevel;
        this->mMaxIteration = iMaxIteration;
    }
    if(retOk) this->mBaseEdges = *(const ENTITY_LIST*)(iEdges->header);  // 不确定
    return retOk;
}

int ofst_edge_smooth_manager::apply_more_smooth(int iNumIteration) {
    int applyMore = 0;
    if(this->mNumEdges > 0 && this->mSmoothLevel) {
        for(int i = 0; i < this->mNumEdges && !applyMore; ++i) {
            applyMore = this->mSmoothLevel[i] > 0 && this->mSmoothLevel[i] < this->mMaxSmoothLevel;
        }
    }
    if(applyMore) this->adjust_smoothlevel();
    this->mIteration = iNumIteration;
    return applyMore;
}
int ofst_edge_smooth_manager::get_edge_smooth_level(EDGE* iEdge, int oRemoceContainment) {
    int smoothLevel = 0;
    int edgeIndex = this->edge_index(iEdge);
    if(this->mSmoothLevel && edgeIndex >= 0) {
        if(this->mSmoothLevel[edgeIndex] < this->mMaxSmoothLevel) smoothLevel = this->mSmoothLevel[edgeIndex];
        this->mLastSmoothLevel[edgeIndex] = smoothLevel;
    }
    return smoothLevel;
}
void ofst_edge_smooth_manager::set_edge_intersection_info(EDGE* iEdge, curve_curve_int* intersectionsList) {
    int max_intersection_number = 15;
    int edgeIndex = this->edge_index(iEdge);
    if(intersectionsList && edgeIndex >= 0) {
        int numIntersections = 0;
        int numOverlaps = 0;
        int numUnknowns = 0;
        get_intersection_info(intersectionsList, numIntersections, numOverlaps, numUnknowns);
        if(numUnknowns <= 0) {
            if(numOverlaps > 0 || numIntersections > 15) {
                this->mNumIntersections[edgeIndex] = numIntersections;
                this->mNumOverlaps[edgeIndex] = numOverlaps;
                ++this->mSmoothLevel[edgeIndex];
            }
        } else {
            this->mNumIntersections[edgeIndex] = numIntersections;
            this->mNumOverlaps[edgeIndex] = numOverlaps;
            ++this->mSmoothLevel[edgeIndex];
        }
        this->mTangential = numUnknowns > 0;
    }
}
int ofst_edge_smooth_manager::is_last_iteration() {
    return this->mIteration == this->mMaxIteration;
}
int ofst_edge_smooth_manager::is_first_iteration() {
    return this->mIteration == 0;
}
int ofst_edge_smooth_manager::is_intersection_tangential() {
    return this->mTangential;
}
void ofst_edge_smooth_manager::set_edge_self_intersection_fail(EDGE* iEdge) {
    int edgeIndex = this->edge_index(iEdge);
    if(edgeIndex >= 0) {
        if(this->mSmoothLevel) ++this->mSmoothLevel[edgeIndex];
    }
}
int ofst_edge_smooth_manager::edge_index(EDGE* iEdge) {
    int ind = -1;
    if(this->mNumEdges > 0) return this->mBaseEdges.lookup(iEdge);
    return ind;
}
void ofst_edge_smooth_manager::adjust_smoothlevel() {
    if(this->mSmoothLevel && this->mLastSmoothLevel) {
        int noLevelChange = 1;
        for(int i = 0; i < this->mNumEdges && noLevelChange; ++i) {
            if(this->mLastSmoothLevel[i] > this->mSmoothLevel[i]) noLevelChange = 0;
        }
        if(noLevelChange) {
            for(int j = 0; j < this->mNumEdges && noLevelChange; ++j) {
                if(this->mSmoothLevel[j] > 0) {
                    ++this->mSmoothLevel[j];
                    noLevelChange = 0;
                }
            }
        }
    }
}