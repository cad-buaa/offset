#pragma once
#include "ProtectedInterfaces/off_cu.hxx"
#include "acis/alltop.hxx"
#include "acis/module.hxx"

extern module_debug ofstwireextractor_module_header;



void add_wire_0(WIRE* iWire, WIRE*& ioWireList);  // 已检查
int add_coedge_to_wire(COEDGE* iCoedge, WIRE*& ioWire);
int is_point_close_to_coedge(SPAposition& iCheckPoint, SPAposition& oClosePointOnEdge, COEDGE* iCheckCoedge, const double iCheckDist, int iSmallTol);  // 已检查
int wire_dist_too_close(SPAposition& iCheckPoint, WIRE* iBaseWire, const double iCheckDist, SPAposition& oBasePos, int iSmallTol);                     // 已检查

class ofst_wires_extractor {
    int mNumSegmens;
    offset_segment** mSegments;
    offset_segment_list* mSegsList;
    int mNumSegmentsInWorkWire;
    int* mWorkingWire;
    int mSegListAddAttribs;
    int mNumMainSegmentsConnectionId;
    int* mMainSegmentsConnectionId;
    int mRemoveOverlap;

  public:
    enum { eOverlapAllowed = 0, eOverlapNotAllowed = 1, eOverlapRemoved = 2 };
    enum { eSegment_not_processed = 0, eSegment_in_processing = 1, eSegment_end_dangling = 2, eSegment_overlapping = 3, eSegment_extracted = 4 };
    // 段尚未处理  段正在处理  段末端悬空（未连接）  段重叠   段已提取
    ofst_wires_extractor();
    ~ofst_wires_extractor();
    int extract(offset_segment_list* iSeglist, WIRE*& ioWireList, const int iExtractLoopOnly, const int iRemoveOverlap);
    int get_overlap_status();
    void clear();
    int init(offset_segment_list* iSegList);
    void init_segment_marks();                      // 已检查
    void init_wire_creation();                      // 已检查
    int get_seed_segment(int iSeedType);            // 已检查
    int get_next_connected_seg();                   // 已检查
    WIRE* extract_closed_wire(const int iSeedInd);  // 已检查
    WIRE* extract_open_wire(const int iSeedInd);    // 已检查
    WIRE* extract_wire(const int iStartIndex);      // 暂未实现
    int search_loop();
    offset_segment* get_segment(const int iSegIndex);  // 已检查
    COEDGE* get_coedge(const int iSegIndex);
    void mark_segment(const int iSegIndex, const int iFlag);  // 已检查
    void add_last_segment(const int iSegIndex);               // 已检查
    void store_main_segments_connectionId(const int iStartIndex);
    int is_coonectionId_in_main_loop(const int iCheckId);
    int is_wire_connect_to_main_segments(int iStartIndex);  // 暂未实现
    int remove_last_segment();
    int degenrated_cases();
    int wire_is_too_close(const int iStartIndex, int iNumEdges);                                                                                                    // 已检查
    int remove_overlap(const int iStartIndex, const int ioNumEdges);                                                                                                // 已检查
    int is_self_loop(const int iStartIndex);                                                                                                                        // 已检查
    void dump_wire(const int iStartIndex);                                                                                                                          // 已检查
    void debug_display_trim_close_wire(const int iStartIndex, SPAposition& iCheckPos, SPAposition& iBasePos, const double offsetDist, SPAunit_vector& wireNormal);  // 已检查
};