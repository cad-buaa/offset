#include "PrivateInterfaces/off_trim.hxx"
#include "ProtectedInterfaces/at_rel.hxx"
#include "acis/calctol.hxx"
#include "acis/curdef.hxx"
#include "acis/curve.hxx"
#include "acis/geometry.hxx"
#include "acis/point.hxx"
#include "acis/sgquery.hxx"
#include "acis/module.hxx"


#define MODULE() ofstwireextractor
module_debug ofstwireextractor_module_header("ofstwireextractor");

void add_wire_0(WIRE* iWire, WIRE*& ioWireList) {
    WIRE* i;
    WIRE* lastWire;
    // if(!iWire) _wassert(L"iWire", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_wires_extractor.cpp", 0x384u);
    if(ioWireList) {
        lastWire = ioWireList;
        for(i = ioWireList->next(PAT_CAN_CREATE); i; i = lastWire->next(PAT_CAN_CREATE)) 
            lastWire = lastWire->next(PAT_CAN_CREATE); 
        lastWire->set_next(iWire, 1);
    } 
    else {
        ioWireList = iWire;
    }
}
int add_coedge_to_wire(COEDGE* iCoedge, WIRE*& ioWire) {
    /*if(!iCoedge) _wassert(L"iCoedge", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_wires_extractor.cpp", 0x353u);*/
    if(ioWire) {
        COEDGE* currCoedge = ioWire->coedge();
        COEDGE* lastCoedge = currCoedge->previous();
        iCoedge->set_wire(ioWire, 1);
        currCoedge->set_previous(iCoedge, 0, 1);
        lastCoedge->set_next(iCoedge, 0, 1);
        iCoedge->set_previous(lastCoedge, 0, 1);
        iCoedge->set_next(currCoedge, 0, 1);
        if(iCoedge->start() != lastCoedge->end()) {
            if(iCoedge->sense()) {
                iCoedge->edge()->set_end(lastCoedge->end(), 1, 1);
            } else {
                iCoedge->edge()->set_start(lastCoedge->end(), 1, 1);
            }
        }
        iCoedge->end()->set_edge(iCoedge->edge(), 1);
    } else {
        iCoedge->set_next(iCoedge, 0, 1);
        iCoedge->set_previous(iCoedge, 0, 1);
        WIRE* v7 = ACIS_NEW WIRE(iCoedge, nullptr);
        ioWire = v7;
        iCoedge->set_wire(ioWire, 1);
        VERTEX* v9 = iCoedge->start();
        EDGE* edge = iCoedge->edge();
        v9->set_edge(edge, 1);
        VERTEX* v11 = iCoedge->end();
        EDGE* v10 = iCoedge->edge();
        v11->set_edge(v10, 1);
    }
    return 1;
}
int is_point_close_to_coedge(SPAposition& iCheckPoint, SPAposition& oClosePointOnEdge, COEDGE* iCheckCoedge, const double iCheckDist, int iSmallTol) {
    int closeToEdge = 0;
    if(iCheckCoedge) {
        EDGE* edge = iCheckCoedge->edge();
        SPAtransf face_trans;
        find_cls_pt_on_edge(iCheckPoint, edge, oClosePointOnEdge, face_trans);
        SPAvector v6 = oClosePointOnEdge - iCheckPoint;
        double testdist = v6.len();
        double tol = 0.0;
        find_max_tolerance(iCheckCoedge, tol);
        double offsetDistTol;
        if(iSmallTol)
            offsetDistTol = 0.000001;
        else
            offsetDistTol = 0.0001;

        if(offsetDistTol > tol) tol = offsetDistTol;
        if(tol > 0.2 * iCheckDist) tol = 0.2 * iCheckDist;
        return iCheckDist > testdist + tol;
    }
    return closeToEdge;
}
int wire_dist_too_close(SPAposition& iCheckPoint, WIRE* iBaseWire, const double iCheckDist, SPAposition& oBasePos, int iSmallTol) {
    int tooClose = 0;
    const COEDGE* prevCoedge;
    const COEDGE* startCoedge;
    if(iBaseWire && iCheckDist > 0.0) {
        startCoedge = start_of_wire_chain(iBaseWire);
        COEDGE* thisCoedge = (COEDGE*)startCoedge;
        do {
            if(is_point_close_to_coedge(iCheckPoint, oBasePos, thisCoedge, iCheckDist, iSmallTol)) tooClose = 1;
            prevCoedge = thisCoedge;
            thisCoedge = thisCoedge->next();
        } while(thisCoedge != prevCoedge && thisCoedge != startCoedge && !tooClose);
    }
    return tooClose;
}

ofst_wires_extractor::ofst_wires_extractor() {
    this->mNumSegmens = 0;
    this->mSegments = nullptr;
    this->mSegsList = nullptr;
    this->mNumSegmentsInWorkWire = 0;
    this->mWorkingWire = nullptr;
    this->mSegListAddAttribs = 0;
    this->mNumMainSegmentsConnectionId = 0;
    this->mMainSegmentsConnectionId = nullptr;
    this->mRemoveOverlap = 1;
}
ofst_wires_extractor::~ofst_wires_extractor() {
    this->clear();
}

int ofst_wires_extractor::extract(offset_segment_list* iSeglist, WIRE*& ioWireList, const int iExtractLoopOnly, const int iRemoveOverlap) {
    int retVal = 0;

    // if(ioWireList) _wassert(L"ioWireList == NULL", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_wires_extractor.cpp", 0x31Fu);
    if(!this->init(iSeglist)) return 0;

    if(this->degenrated_cases()) return 1;
    this->mRemoveOverlap = iRemoveOverlap != 0;

    for(int seedInd = this->get_seed_segment(0); seedInd >= 0; seedInd = this->get_seed_segment(0)) //找到0状态（未处理状态）的第一个Segments，并返回对应的索引
    {
        WIRE* extractedWire = this->extract_closed_wire(seedInd);//将未处理的segment设置成处理过后
        if(extractedWire) add_wire_0(extractedWire, ioWireList);//ioWireList set_next(extractedWire)
    }

    if(!iExtractLoopOnly) {
        this->init_segment_marks();
        for(int seedInda = this->get_seed_segment(0); seedInda >= 0; seedInda = this->get_seed_segment(0)) {
            WIRE* iWire = this->extract_open_wire(seedInda);
            if(iWire) add_wire_0(iWire, ioWireList);
        }
    }
    if(ioWireList) {
        int retVal = 1;
        iSeglist->set_trim_inside_wire_done(1);
    }
    return retVal;
}
int ofst_wires_extractor::get_overlap_status() {
    return this->mRemoveOverlap;
}
void ofst_wires_extractor::clear() {
    this->mNumSegmens = 0;
    if(this->mSegments) ACIS_DELETE[] this->mSegments;
    this->mSegments = nullptr;
    this->mNumSegmentsInWorkWire = 0;
    if(this->mWorkingWire) ACIS_DELETE[] this->mWorkingWire;
    this->mWorkingWire = nullptr;
    this->mNumMainSegmentsConnectionId = 0;
    if(this->mMainSegmentsConnectionId) ACIS_DELETE[](this->mMainSegmentsConnectionId);
    this->mMainSegmentsConnectionId = nullptr;
    this->mSegsList = nullptr;
}
int ofst_wires_extractor::init(offset_segment_list* iSegList) {
    if(!iSegList) return 0;
    int retVal = 1;
    this->clear();
    this->mSegsList = iSegList;
    offset_segment* thisSeg = iSegList->first_segment();
    offset_segment* segment = iSegList->last_segment();
    offset_segment* lastSeg = segment->next();
    while(thisSeg != lastSeg) {
        if(!thisSeg) {
            retVal = 0;
            break;
        }
        if(thisSeg->outside()) {
            ++this->mNumSegmens;
        }
        thisSeg->set_user_flag(0);
        thisSeg = thisSeg->next();
    }
    if(retVal) {
        int mNumSegmens = this->mNumSegmens;

        this->mSegments = ACIS_NEW offset_segment* [mNumSegmens];
        this->mWorkingWire = ACIS_NEW int[this->mNumSegmens];
        if(!this->mSegments || !this->mWorkingWire) retVal = 0;
    }
    if(retVal) {
        offset_segment* thisSega = iSegList->first_segment();
        offset_segment* lastSega = iSegList->last_segment()->next();
        int i = 0;
        while(thisSega != lastSega) {
            if(thisSega->outside()) this->mSegments[i++] = thisSega;
            thisSega = thisSega->next();
        }
        this->mSegListAddAttribs = iSegList->add_attribs();
    }
    return retVal;
}
void ofst_wires_extractor::init_segment_marks() {
    int i;
    for(i = 0; i < this->mNumSegmens; ++i) {
        if(this->mSegments[i]->get_user_flag() != 4) this->mark_segment(i, 0);
    }
}
void ofst_wires_extractor::init_wire_creation() {
    int i;

    /* if(!this->mWorkingWire) _wassert(L"mWorkingWire", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_wires_extractor.cpp", 0x2CDu);*/
    this->mNumSegmentsInWorkWire = 0;
    for(i = 0; i < this->mNumSegmens; ++i) this->mWorkingWire[i] = -1;
}
int ofst_wires_extractor::get_seed_segment(int iSeedType) {
    int seed = -1;
    if(this->mSegments && this->mWorkingWire) {
        for(int i = 0; i < this->mNumSegmens && seed < 0; ++i) {
            if(this->mSegments[i]->get_user_flag() == iSeedType) seed = i;
        }
    }
    return seed;
}
int ofst_wires_extractor::get_next_connected_seg() {
    int nextSegInd = -1;
    if(this->mNumSegmentsInWorkWire > 0) {
        offset_segment* thisSegment = this->get_segment(this->mWorkingWire[this->mNumSegmentsInWorkWire - 1]);
        if(thisSegment) {
            int lastConnectionId = thisSegment->get_end_connectionId();
            for(int i = 0; i < this->mNumSegmens && nextSegInd < 0; ++i) {
                offset_segment* tmpSeg = this->get_segment(i);
                if(!tmpSeg->get_user_flag() && tmpSeg->get_start_connectionId() == lastConnectionId) {
                    nextSegInd = i;
                }
            }
            int branchId = thisSegment->get_end_branch_connectionId();
            if(nextSegInd < 0 && branchId > 0) {
                for(int iSegIndex = 0; iSegIndex < this->mNumSegmens && nextSegInd < 0; ++iSegIndex) {
                    offset_segment* segment = this->get_segment(iSegIndex);
                    if(!segment->get_user_flag() && segment->get_start_connectionId() == branchId) {
                        nextSegInd = iSegIndex;
                    }
                }
            }
        }
    }
    return nextSegInd;
}
WIRE* ofst_wires_extractor::extract_closed_wire(const int iSeedInd) {
    WIRE* retWire = nullptr;
    if(iSeedInd >= 0 && iSeedInd < this->mNumSegmens) 
    {
        this->init_wire_creation();//将this->mWorkingWire[i] = -1, this->mNumSegmentsInWorkWire = 0;
        int nextSeg = iSeedInd;


        while(nextSeg >= 0) 
        {
            this->add_last_segment(nextSeg);//将segment_list[nextSeg]的user_flag设置成正在处理状态(1),将this->mWorkingWire[this->mNumSegmentsInWorkWire++] = nextSeg

            int loopStart = this->search_loop();//找到环开始的索引,但是对于第一个segment，返回-1，
            if(loopStart < 0) {
                do nextSeg = this->get_next_connected_seg();//找到下一个segment的索引
                while(nextSeg < 0 && this->remove_last_segment());
            } 
            else {
                retWire = this->extract_wire(loopStart);
                nextSeg = -1;
            }
        }
    }
    return retWire;
}
WIRE* ofst_wires_extractor::extract_open_wire(const int iSeedInd) {
    int nextSeg;
    WIRE* retWire = nullptr;
    if(iSeedInd >= 0 && iSeedInd < this->mNumSegmens) {
        this->init_wire_creation();
        for(nextSeg = iSeedInd; nextSeg >= 0; nextSeg = this->get_next_connected_seg()) {
            this->add_last_segment(nextSeg);
        }
        return this->extract_wire(0);
    }
    return retWire;
}
WIRE* ofst_wires_extractor::extract_wire(const int iStartIndex) {

     ATTRIB_OFFREL* v9;  // [rsp+48h] [rbp-30h]
     WIRE* v10;          // [rsp+50h] [rbp-28h]

     WIRE* retWire[4];  // [rsp+58h] [rbp-20h] BYREF

    /*if(!this->mWorkingWire || iStartIndex < 0 || iStartIndex >= this->mNumSegmentsInWorkWire)
     _wassert(L"mWorkingWire && iStartIndex >= 0 && iStartIndex < mNumSegmentsInWorkWire", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_wires_extractor.cpp", 0x20Cu);*/
     retWire[0] = nullptr;
     for(int i = 0; i < this->mNumSegmentsInWorkWire; ++i) 
     {
        if(i >= iStartIndex)
            this->mark_segment(this->mWorkingWire[i], 4);
        else
            this->mark_segment(this->mWorkingWire[i], 0);
     }
     if(this->mRemoveOverlap == 1 && this->remove_overlap(iStartIndex, this->mNumSegmentsInWorkWire - iStartIndex)) 
     {
        this->mNumSegmentsInWorkWire -= (this->mNumSegmentsInWorkWire - iStartIndex) / 2;
        this->mRemoveOverlap = 2;
     }
     if(!this->is_wire_connect_to_main_segments(iStartIndex) && !this->is_self_loop(iStartIndex) && !this->wire_is_too_close(iStartIndex, this->mNumSegmentsInWorkWire - iStartIndex)) {
        int startConnectionId = this->mSegsList->get_start_segment_connectionId();
        for(int ia = iStartIndex; ia < this->mNumSegmentsInWorkWire; ++ia) {
            offset_segment* crrSeg = this->get_segment(this->mWorkingWire[ia]);
            /* if(!crrSeg) _wassert(L"crrSeg", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_wires_extractor.cpp", 0x228u);*/
            if(!this->mMainSegmentsConnectionId && startConnectionId == crrSeg->get_start_connectionId()) {
                this->store_main_segments_connectionId(iStartIndex);
            }
            COEDGE* crrCoedge = crrSeg->coedge();
            /*if(!crrCoedge) _wassert(L"crrCoedge", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_wires_extractor.cpp", 0x231u);*/
            if(this->mSegListAddAttribs) {
                ATTRIB_OFFREL* v10 = ACIS_NEW ATTRIB_OFFREL(crrCoedge, *crrSeg);  // 不确定
                retWire[1] = (WIRE*)v10;
                crrCoedge->set_attrib(v10);
            }
            add_coedge_to_wire(crrCoedge, *retWire);
        }
     }
     DEBUG_LEVEL(DEBUG_ALL) {
         this->dump_wire(iStartIndex);
     }
     return retWire[0];

}

int ofst_wires_extractor::search_loop() {
    int startIndex = -1;
    if(this->mNumSegmentsInWorkWire > 0 && this->mSegments && this->mWorkingWire) {
        if(this->mNumSegmens == 1) {
            int numOf_org_edges = this->mSegsList->get_number_of_offset_coedges();
            if(GET_ALGORITHMIC_VERSION() > AcisVersion(32, 0, 1) && numOf_org_edges > 1) {
                COEDGE* pCoedge = (*this->mSegments)->coedge();
                if(same_point(pCoedge->start()->geometry()->coords(), pCoedge->end()->geometry()->coords(), SPAresabs)) return 0;
            } else {
                return 0;
            }
        } 
        else {
            int lastConnectionId = this->mSegments[this->mWorkingWire[this->mNumSegmentsInWorkWire - 1]]->get_end_connectionId();
            for(int i = 0; i < this->mNumSegmentsInWorkWire && startIndex < 0; ++i) 
            {
                if(this->mSegments[this->mWorkingWire[i]]->get_start_connectionId() == lastConnectionId) startIndex = i;
            }
        }
    }
    return startIndex;
}
offset_segment* ofst_wires_extractor::get_segment(const int iSegIndex) {
    offset_segment* retSegment = nullptr;
    if(iSegIndex >= 0 && iSegIndex < this->mNumSegmens && this->mSegments) return this->mSegments[iSegIndex];
    return retSegment;
}
COEDGE* ofst_wires_extractor::get_coedge(const int iSegIndex) {
    COEDGE* retCoedge = nullptr;
    offset_segment* thisSegment = this->get_segment(iSegIndex);
    if(thisSegment) return thisSegment->coedge();
    return retCoedge;
}
void ofst_wires_extractor::mark_segment(const int iSegIndex, const int iFlag) {
    offset_segment* seg = this->get_segment(iSegIndex);
    if(seg) seg->set_user_flag(iFlag);
}
void ofst_wires_extractor::add_last_segment(const int iSegIndex) {
    if(iSegIndex >= 0) 
    {
        this->mark_segment(iSegIndex, 1);//将segment_list[iSegIndex]的user_flag设置成1，表示正在处理
        this->mWorkingWire[this->mNumSegmentsInWorkWire++] = iSegIndex;//将正在处理的segment设置为索引index，this->mNumSegmentsInWorkWire+1，
    }
}
void ofst_wires_extractor::store_main_segments_connectionId(const int iStartIndex) {
    /*  if(this->mMainSegmentsConnectionId) _wassert(L"mMainSegmentsConnectionId == 0", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_wires_extractor.cpp", 0x1D4u);*/
    this->mNumMainSegmentsConnectionId = this->mNumSegmentsInWorkWire - iStartIndex;
    int mNumMainSegmentsConnectionId = this->mNumMainSegmentsConnectionId;
    this->mMainSegmentsConnectionId = ACIS_NEW int[mNumMainSegmentsConnectionId];
    if(this->mMainSegmentsConnectionId) {
        for(int i = iStartIndex; i < this->mNumSegmentsInWorkWire; ++i) {
            offset_segment* crrSeg = this->get_segment(this->mWorkingWire[i]);
            /* if(!crrSeg) _wassert(L"crrSeg", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_wires_extractor.cpp", 0x1DCu);*/
            this->mMainSegmentsConnectionId[i - iStartIndex] = crrSeg->get_start_connectionId();
        }
    }
}
int ofst_wires_extractor::is_coonectionId_in_main_loop(const int iCheckId) {
    int connectionIdFound = 0;
    if(this->mMainSegmentsConnectionId) {
        for(int i = 0; i < this->mNumMainSegmentsConnectionId && !connectionIdFound; ++i) connectionIdFound = iCheckId == this->mMainSegmentsConnectionId[i];
    }
    return connectionIdFound;
}
int ofst_wires_extractor::is_wire_connect_to_main_segments(int iStartIndex) {
    int connected = 0;
    if(this->mMainSegmentsConnectionId) {
        offset_segment* crrSeg = nullptr;
        while(iStartIndex < this->mNumSegmentsInWorkWire && !connected) {
            crrSeg = this->get_segment(this->mWorkingWire[iStartIndex]);
            /*if(!crrSeg) _wassert(L"crrSeg", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_wires_extractor.cpp", 0x1C5u);*/
            int start_connectionId = crrSeg->get_start_connectionId();
            connected = this->is_coonectionId_in_main_loop(start_connectionId);
            ++iStartIndex;
        }
        if(!connected && crrSeg) {
            int end_connectionId = crrSeg->get_end_connectionId();
            return this->is_coonectionId_in_main_loop(end_connectionId);
        }
    }
    return connected;
}
int ofst_wires_extractor::remove_last_segment() {
    int removeDone = 0;
    if(this->mNumSegmentsInWorkWire > 0) {
        this->mark_segment(this->mWorkingWire[--this->mNumSegmentsInWorkWire], 2);
        return 1;
    }
    return removeDone;
}
int ofst_wires_extractor::degenrated_cases() {
    int isDegnerated = this->mNumSegmens == 0;
    if(this->mNumSegmens && this->mSegments) {
        int isDegnerated = 1;
        int connectionId = (*this->mSegments)->get_start_connectionId();
        for(int i = 0; i < this->mNumSegmens && isDegnerated; ++i) {
            if(connectionId == this->mSegments[i]->get_start_connectionId() && connectionId == this->mSegments[i]->get_end_connectionId()) {
                COEDGE* thisCoedge = this->mSegments[i]->coedge();
                if(thisCoedge) {
                    if(thisCoedge->edge()) {
                        if(thisCoedge->edge()->geometry()) {
                            const curve& thisCurve = thisCoedge->edge()->geometry()->equation();
                            if(((curve&)thisCurve).type() != 1) isDegnerated = 0;
                        }
                    }
                }
            } else {
                isDegnerated = 0;
            }
        }
    }
    return isDegnerated;
}
int ofst_wires_extractor::wire_is_too_close(const int iStartIndex, int iNumEdges) {
    int too_close = 0;
    /*if(!this->mSegsList) _wassert(L"mSegsList", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_wires_extractor.cpp", 0x277u);*/
    if(iNumEdges != this->mNumSegmens) {
        const WIRE* baseWire = this->mSegsList->get_base_wire();
        const law* dstLaw = this->mSegsList->distance();
        if(baseWire) {
            if(dstLaw && ((law*)dstLaw)->constant()) {
                double offsetDist = ((law*)dstLaw)->eval(1.0);
                double checkDist = fabs(offsetDist);
                const COEDGE* checkCoedge = this->get_coedge(this->mWorkingWire[iStartIndex]);
                SPAposition checkPos;
                SPAposition basePos;
                if(checkCoedge) {
                    checkPos = checkCoedge->start_pos();
                    too_close = wire_dist_too_close(checkPos, (WIRE*)baseWire, checkDist, basePos, iNumEdges == 1);  // 不确定
                    if(too_close && iNumEdges == 1) {
                        checkPos = checkCoedge->end_pos();
                        too_close = wire_dist_too_close(checkPos, (WIRE*)baseWire, checkDist, basePos, 1);
                    }
                    int numOf_org_edges = this->mSegsList->get_number_of_offset_coedges();
                    if(GET_ALGORITHMIC_VERSION() > AcisVersion(32, 0, 1) && too_close) {
                        checkPos = coedge_mid_pos(checkCoedge);
                        int start_mid_too_close = wire_dist_too_close(checkPos, (WIRE*)baseWire, checkDist, basePos, 0);
                        const COEDGE* checkCoedgea = this->get_coedge(this->mWorkingWire[iNumEdges - 1]);
                        checkPos = coedge_mid_pos(checkCoedgea);
                        int end_mid_too_close = wire_dist_too_close(checkPos, (WIRE*)baseWire, checkDist, basePos, 0);
                        if((start_mid_too_close == 1 || end_mid_too_close == 1) && iNumEdges > 2) {
                            const COEDGE* checkCoedgeb = this->get_coedge(this->mWorkingWire[iNumEdges / 2]);
                            checkPos = coedge_mid_pos(checkCoedgeb);
                            too_close = wire_dist_too_close(checkPos, (WIRE*)baseWire, checkDist, basePos, 0);
                        } else if(!start_mid_too_close || !end_mid_too_close) {
                            too_close = 0;
                        }
                    } else {
                        if((GET_ALGORITHMIC_VERSION() <= AcisVersion(32, 0, 1) || numOf_org_edges == 1) && !too_close && iNumEdges == 2) {
                            const COEDGE* checkCoedgec = this->get_coedge(this->mWorkingWire[iStartIndex + 1]);
                            checkPos = coedge_mid_pos(checkCoedgec);
                            too_close = wire_dist_too_close(checkPos, (WIRE*)baseWire, checkDist, basePos, 0);
                        }
                    }
                }
                if(too_close) {
                    const SPAunit_vector& wireNormal = this->mSegsList->normal();
                    this->debug_display_trim_close_wire(iStartIndex, checkPos, basePos, offsetDist, (SPAunit_vector&)wireNormal);  // 不确定
                }
            }
        }
    }
    return too_close;
}
int ofst_wires_extractor::remove_overlap(const int iStartIndex, const int ioNumEdges) {
    int overlapRemoved = 0;
    if(!(ioNumEdges % 2)) {
        int midIndex = ioNumEdges / 2;
        int lastIndex = iStartIndex + ioNumEdges - 1;
        offset_segment* crrSeg = this->get_segment(this->mWorkingWire[iStartIndex]);
        offset_segment* comSeg = this->get_segment(this->mWorkingWire[lastIndex]);
        /* if(!crrSeg || !comSeg) _wassert(L"crrSeg && comSeg", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_wires_extractor.cpp", 0x24Du);*/
        int start_connectionId = crrSeg->get_start_connectionId();
        overlapRemoved = start_connectionId == comSeg->get_end_connectionId();
        for(int i = 0; i < midIndex && overlapRemoved; ++i) {
            crrSeg = this->get_segment(this->mWorkingWire[i + iStartIndex]);
            comSeg = this->get_segment(this->mWorkingWire[lastIndex - i]);
            /* if(!crrSeg || !comSeg) _wassert(L"crrSeg && comSeg", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_wires_extractor.cpp", 0x254u);*/
            int end_connectionId = crrSeg->get_end_connectionId();
            overlapRemoved = end_connectionId == comSeg->get_start_connectionId();
        }
        if(overlapRemoved && ioNumEdges == 2) {
            COEDGE* crrCoedge = crrSeg->coedge();
            SPAposition midPt = coedge_mid_pos(crrCoedge);
            /*if(!offset_segment::coedge(comSeg)) _wassert(L"comSeg->coedge ()", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_wires_extractor.cpp", 0x25Du);*/
            EDGE* comEdge = comSeg->coedge()->edge();
            /*  if(!comEdge || !EDGE::geometry(comEdge)) _wassert(L"comEdge && comEdge->geometry ()", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_wires_extractor.cpp", 0x25Fu);*/
            const curve& comCurve = comEdge->geometry()->equation();
            SPAinterval curveRange = comEdge->param_range();
            if(comEdge->sense() == 1) curveRange = -curveRange;
            double started = curveRange.start_pt();
            double v6 = curveRange.end_pt();
            SPAparameter guessParam = 0.5 * (started + v6);
            SPAposition midPt2;
            SPAparameter midParam2;
            ((curve&)comCurve).point_perp(midPt, midPt2, guessParam, midParam2, 0);
            if(!same_point(midPt, midPt2, SPAresfit)) return 0;
        }
    }
    return overlapRemoved;
}
int ofst_wires_extractor::is_self_loop(const int iStartIndex) {
    int selfLoop = 0;
    if(this->mNumSegmentsInWorkWire - iStartIndex == 1 && this->mNumSegmens > 1 && iStartIndex != this->mNumSegmens - 1) {
        /* if(!this->mWorkingWire) _wassert(L"mWorkingWire != NULL", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_wires_extractor.cpp", 0x1EDu);*/
        offset_segment* mySeg = this->get_segment(this->mWorkingWire[iStartIndex]);
        /*  if(!mySeg) _wassert(L"mySeg != NULL", L"E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\ofst_wires_extractor.cpp", 0x1EFu);*/
        int start_connectionId = mySeg->get_start_connectionId();
        return start_connectionId == mySeg->get_end_connectionId();
    }
    return selfLoop;
}
void ofst_wires_extractor::dump_wire(const int iStartIndex) {
    acis_fprintf(debug_file_ptr, "Wire Segments:\n");
    for(int i = iStartIndex; i < this->mNumSegmentsInWorkWire; ++i) {
        offset_segment* crrSeg = this->mSegments[this->mWorkingWire[i]];
        const COEDGE* crrCoedge = crrSeg->coedge();
        SPAposition v15 = ((COEDGE*)crrCoedge)->start_pos();
        SPAposition ep = ((COEDGE*)crrCoedge)->end_pos();
        double v8 = ep.z();
        double v9 = ep.y();
        double v10 = ep.x();
        double v11 = v15.z();
        double v12 = v15.y();
        double v13 = v15.x();
        int end_connectionId = crrSeg->get_end_connectionId();
        int start_connectionId = crrSeg->get_start_connectionId();
        /*       int v2;*/
        //*(reinterpret_cast<uint32_t*>(&v2)) = end_connectionId;
        /*       acis_fprintf(debug_file_ptr, "%d: StartId %d   Pos: %f  %f  %f  %f  %f  %f\n", i, start_connectionId,v13, v12, v11, v10, v9, v8);*/
    }
}
void ofst_wires_extractor::debug_display_trim_close_wire(const int iStartIndex, SPAposition& iCheckPos, SPAposition& iBasePos, const double offsetDist, SPAunit_vector& wireNormal) {
    return;
}
