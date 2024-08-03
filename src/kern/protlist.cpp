#include "ProtectedInterfaces/protlist.hxx"

#include "acis/at_macro.hxx"
#include "acis/fileio.hxx"

#define THIS() LOP_PROTECTED_LIST
#define THIS_LIB NONE
#define PARENT() SPACOLLECTION
#define PARENT_LIB KERN

#define LOP_PROTECTED_LIST_NAME "lop_prot_list"

// 以下内容可以替换为以下宏, 为方便调试, 暂时全部展开
ENTITY_DEF(LOP_PROTECTED_LIST_NAME)
SAVE_DEF
RESTORE_DEF
LOSE_DEF
DTOR_DEF
COPY_WITH_DEEP_COPY_DEF
SCAN_DEF
FIX_POINTER_DEF
FIXUP_COPY_DEF
TERMINATE_DEF

// 宏替换结束

void LOP_PROTECTED_LIST::set_default() {
    SPACOLLECTION::set_copyable(FALSE);
    SPACOLLECTION::set_replace_member_action(ReplaceMemberIgnore);
    SPACOLLECTION::set_merge_member_action(MergeMemberIgnore);
    SPACOLLECTION::set_split_member_action(SplitMemberIgnore);  // 逆向结果为 3u
    SPACOLLECTION::set_copy_member_action(CopyMemberIgnore);
    SPACOLLECTION::set_geomchanged_member_action(GeomChangedMemberIgnore);
    SPACOLLECTION::set_trans_member_action(TransMemberIgnore);
}

LOP_PROTECTED_LIST::LOP_PROTECTED_LIST(): SPACOLLECTION() {
    set_default();
}

LOP_PROTECTED_LIST::LOP_PROTECTED_LIST(ENTITY* e): SPACOLLECTION(e) {
    set_default();
}

LOP_PROTECTED_LIST::LOP_PROTECTED_LIST(ENTITY_LIST& ents): SPACOLLECTION(ents) {
    set_default();
}
