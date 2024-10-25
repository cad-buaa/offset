#pragma once
#include"acis/alltop.hxx"
#include"acis/annotation.hxx"
#include"acis/cur_off_anno.hxx"
#include"acis/eelists.hxx"


//class OFST_ANNOTATION : public ANNOTATION
//{
//  public:
//    void member_lost_internal(ENTITY* entity);
//    virtual const char* member_name(ENTITY* entity);
//    virtual void member_lost(ENTITY* entity);
//    virtual void inputs(ENTITY_LIST& list, int no_tags);
//    virtual void outputs(ENTITY_LIST& list);
//    virtual void unhook_members();
//    virtual void hook_members();
//    virtual void lose_input_tags();
//    virtual void lose_lists();
//    virtual ENTITY*& find_entity_ref_by_name(const char* name, int& isInput);
//    virtual ENTITY* get_entity_by_name(const char* name);
//    virtual void set_entity_by_name(const char* name, ENTITY* value);
//    virtual int is_entity_by_name(const char* name, ENTITY* entity);
//    virtual ENTITY* make_copy();
//    ~OFST_ANNOTATION();
//    void fixup_copy(OFST_ANNOTATION* rollback);
//    virtual int identity(int level);
//    virtual const char* type_name();
//    virtual unsigned int size();
//    virtual void debug_ent(_iobuf* fp);
//    virtual void save(ENTITY_LIST& list);
//    virtual void copy_scan(ENTITY_LIST& list, SCAN_TYPE reason, int dpcpy_skip);
//    virtual int is_deepcopyable();
//    virtual ENTITY* copy_data(ENTITY_LIST& list, pointer_map* pm, int dpcpy_skip, SCAN_TYPE reason);
//    virtual void fix_pointers(ENTITY** array, SCAN_TYPE reason);
//    void restore_common();
//    void save_common(ENTITY_LIST& list);
//    void copy_common(ENTITY_LIST& list, OFST_ANNOTATION* from, pointer_map* pm, int dpcpy_skip, SCAN_TYPE reason);
//    void fix_common(ENTITY** array, SCAN_TYPE reason);
//   /* virtual int(void*, METHOD_ARGS&) add_method(METHOD_ID& id, int (__cdecl *)(void*, METHOD_ARGS&) func) =*/
//    virtual int call_method(METHOD_ID& id, METHOD_ARGS& args);
//    virtual void lose();
//    OFST_ANNOTATION(OFST_ANNOTATION& __that);
//    OFST_ANNOTATION();
//    OFST_ANNOTATION& operator=(OFST_ANNOTATION& __that);
//    virtual void* __vecDelDtor();
//
//
//};
//
//
//class WIRE_OFFSET_ANNO : public OFST_ANNOTATION 
//{
// public:
//    enum e_original_ents { e_original_ents = 0, e_offset_ent = 1, e_num_datums = 2 };  // ²»È·¶¨
//    static const int num_ents;
//    static annotation_descriptor descriptors[0];
//    ENTITY* ents[2];
//    int unhooked_out_is_ee[2];
//    void member_lost_internal(ENTITY* entity);
//    virtual const char* member_name(ENTITY* entity);
//    virtual void member_lost(ENTITY* entity);
//    virtual void inputs(ENTITY_LIST& list, int no_tags);
//    virtual void outputs(ENTITY_LIST& list);
//    virtual void unhook_members();
//    virtual void hook_members();
//    virtual void lose_input_tags();
//    virtual void lose_lists();
//    virtual ENTITY*& find_entity_ref_by_name(const char* name, int& isInput);
//    virtual ENTITY* get_entity_by_name(const char* name);
//    virtual void set_entity_by_name(const char* name, ENTITY* value);
//    virtual int is_entity_by_name(const char* name, ENTITY* entity);
//    virtual ENTITY* make_copy();
//    ~WIRE_OFFSET_ANNO();
//    void fixup_copy(WIRE_OFFSET_ANNO* rollback);
//    virtual int identity(int level);
//    virtual const char* type_name();
//    virtual unsigned int size();
//    virtual void debug_ent(_iobuf* fp);
//    virtual void save(ENTITY_LIST& list);
//    virtual void copy_scan(ENTITY_LIST& list, SCAN_TYPE reason, int dpcpy_skip);
//    virtual int is_deepcopyable();
//    virtual ENTITY* copy_data(ENTITY_LIST& list, pointer_map* pm, int dpcpy_skip, SCAN_TYPE reason);
//    virtual void fix_pointers(ENTITY** array, SCAN_TYPE reason);
//    void __cdecl restore_common();
//    void __cdecl save_common(ENTITY_LIST& list);
//    void __cdecl copy_common(ENTITY_LIST& list, WIRE_OFFSET_ANNO* from, pointer_map* pm, int dpcpy_skip, SCAN_TYPE reason);
//    void __cdecl fix_common(ENTITY** array, SCAN_TYPE reason);
//    //virtual int (__cdecl *)(void*, METHOD_ARGS&) add_method(METHOD_ID& id, int (__cdecl *)(void*, METHOD_ARGS&) func) ;
//    virtual int call_method(METHOD_ID& id, METHOD_ARGS& args);
//    virtual void lose();
//    WIRE_OFFSET_ANNO(WIRE_OFFSET_ANNO& __that);
//    WIRE_OFFSET_ANNO(ENTITY* original_ent, ENTITY* offset_ent, ENTITY* second_original_ent);
//    ENTITY* original_ents();
//    ENTITY* offset_ent();
//    virtual void split_member(ENTITY* member_entity, ENTITY* new_split_entity);
//    WIRE_OFFSET_ANNO& operator=(WIRE_OFFSET_ANNO& __that);
//    virtual void* __vecDelDtor();
//
//};