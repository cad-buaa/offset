#pragma once
#include "acis/alltop.hxx"
#include "acis/collection.hxx"
#include "acis/collection_util.hxx"

// class LOP_PROTECTED_LIST : public SPACOLLECTION {
//   public:
//     void set_default();
//     virtual ENTITY* make_copy();
//     ~LOP_PROTECTED_LIST();
//     void fixup_copy(LOP_PROTECTED_LIST* rollback);
//     virtual int identity(int level);
//     virtual const char* type_name();
//     virtual unsigned int size();
//     virtual void debug_ent(_iobuf* fp);
//     virtual void save(ENTITY_LIST& list);
//     virtual void copy_scan(ENTITY_LIST& list, SCAN_TYPE reason, int dpcpy_skip);
//     virtual int is_deepcopyable();
//     virtual ENTITY* copy_data(ENTITY_LIST& list, pointer_map* pm, int dpcpy_skip, SCAN_TYPE reason);
//     virtual void fix_pointers(ENTITY** array, SCAN_TYPE reason);
//     void restore_common();
//     void save_common(ENTITY_LIST& list);
//     void copy_common(ENTITY_LIST& list, LOP_PROTECTED_LIST* from, pointer_map* pm, int dpcpy_skip, SCAN_TYPE reason);
//     void fix_common(ENTITY** array, SCAN_TYPE reason);
//
// };

extern int LOP_PROTECTED_LIST_TYPE;

#define LOP_PROTECTED_LIST_LEVEL (SPACOLLECTION_LEVEL + 1)

// 以下内容可替换为宏
ENTITY_IS_PROTOTYPE(LOP_PROTECTED_LIST, NONE)
/////// 结束

class LOP_PROTECTED_LIST : public SPACOLLECTION {
    // 以下内容可替换为宏
    ENTITY_FUNCTIONS(LOP_PROTECTED_LIST, NONE)

    ///// 结束宏替换

    void set_default();

  public:
    LOP_PROTECTED_LIST();
    LOP_PROTECTED_LIST(ENTITY* e);
    LOP_PROTECTED_LIST(ENTITY_LIST& ents);
};
