#include"PublicInterfaces/gme_cur_off_anno.hxx"








//void OFST_ANNOTATION::member_lost_internal(ENTITY* entity) 
//{
//    this->member_lost_internal(entity);
//}
//const char* OFST_ANNOTATION::member_name(ENTITY* entity) //不确定
//{
//    return this->member_name(entity);
//
//}
//void OFST_ANNOTATION::member_lost(ENTITY* entity) 
//{
//    this->member_lost_internal(entity);
//    if(this->empty()) this->lose();
//
//}
//virtual void OFST_ANNOTATION::inputs(ENTITY_LIST& list, int no_tags) 
//{
//
//}
//virtual void OFST_ANNOTATION::outputs(ENTITY_LIST& list) {
//}
//virtual void OFST_ANNOTATION::unhook_members();
//virtual void OFST_ANNOTATION::hook_members();
//virtual void OFST_ANNOTATION::lose_input_tags();
//virtual void OFST_ANNOTATION::lose_lists();
//virtual ENTITY*& find_entity_ref_by_name(const char* name, int& isInput);
//virtual ENTITY* get_entity_by_name(const char* name);
//virtual void set_entity_by_name(const char* name, ENTITY* value);
//virtual int is_entity_by_name(const char* name, ENTITY* entity);
//virtual ENTITY* make_copy();
//virtual void ~OFST_ANNOTATION();
//void fixup_copy(OFST_ANNOTATION* rollback);
//virtual int identity(int level);
//virtual const char* type_name();
//virtual unsigned int size();
//virtual void debug_ent(_iobuf* fp);
//virtual void save(ENTITY_LIST& list);
//virtual void copy_scan(ENTITY_LIST& list, SCAN_TYPE reason, int dpcpy_skip);
//virtual int is_deepcopyable();
//virtual ENTITY* copy_data(ENTITY_LIST& list, pointer_map* pm, int dpcpy_skip, SCAN_TYPE reason);
//virtual void fix_pointers(ENTITY** array, SCAN_TYPE reason);
//void restore_common();
//void save_common(ENTITY_LIST& list);
//void copy_common(ENTITY_LIST& list, OFST_ANNOTATION* from, pointer_map* pm, int dpcpy_skip, SCAN_TYPE reason);
//void fix_common(ENTITY** array, SCAN_TYPE reason);
//virtual int(__cdecl*)(void*, METHOD_ARGS&) __cdecl OFST_ANNOTATION::add_method(METHOD_ID& id, int(__cdecl*)(void*, METHOD_ARGS&) func) = virtual int call_method(METHOD_ID & id, METHOD_ARGS& args);
//virtual void lose();
//void __cdecl OFST_ANNOTATION(OFST_ANNOTATION& __that);
//void __cdecl OFST_ANNOTATION();
//OFST_ANNOTATION& operator=(OFST_ANNOTATION& __that);
//virtual void* __vecDelDtor();








//void WIRE_OFFSET_ANNO::member_lost_internal(ENTITY* entity) 
//{
//    this->member_lost_internal(entity);
//    for(int i = 2; i--; this->member_lost_helper(this->ents[i], entity));
//}
//const char* WIRE_OFFSET_ANNO::member_name(ENTITY* entity) 
//{
//
//    const char* answer = nullptr;
//    int i = 2;
//    while(i--) 
//    {
//        if(this->contains_this_entity(this->ents[i], entity, 0)) 
//        {
//            answer = WIRE_OFFSET_ANNO::descriptors[i].display_name;
//            break;
//        }
//    }
//    if(answer)
//        return answer;
//    else
//        return this->member_name(entity);
//}
//void WIRE_OFFSET_ANNO::member_lost(ENTITY* entity) 
//{
//    this->member_lost_hook(entity);
//    this->member_lost_internal(entity);
//    if(this->empty()) this->lose();
//}
//void WIRE_OFFSET_ANNO::inputs(ENTITY_LIST& list, int no_tags)
//{
//    this->inputs(list, no_tags);
//    int i = 2;
//    while(i--) 
//    {
//        if(descriptors[i].datum_type == in) this->inputs_helper(this->ents[i], list, no_tags);
//    }
//}
//
////不确定virtual
//void WIRE_OFFSET_ANNO::outputs(ENTITY_LIST& list)
//{
//    int i;           // [rsp+20h] [rbp-28h]
//    EE_LIST* elist;  // [rsp+28h] [rbp-20h]
//    ENTITY* e;       // [rsp+30h] [rbp-18h]
//
//    this->outputs(list);
//    i = 2;
//    while(i--) {
//        if(descriptors[i].datum_type == out) 
//        {
//            list.add(this->ents[i], 1);
//            if(this->ents[i]) 
//            {
//                if(is_EE_LIST(this->ents[i])) 
//                {
//                    elist = (EE_LIST*)this->ents[i];
//                    elist->init();
//                    for(e = elist->next(); e; e = elist->next()) list.add(e, 1);
//                }
//            }
//        }
//    }
//}
//void WIRE_OFFSET_ANNO::unhook_members() 
//{
//
//    if(this->members_are_hooked()) 
//    {
//        this->unhook_members();
//        int i = 2;
//        while(i--) 
//        {
//            this->unhooked_out_is_ee[i] = 0;
//            if(this->ents[i] && is_EE_LIST(this->ents[i])) 
//            {
//                if(descriptors[i].datum_type == out) this->unhooked_out_is_ee[i] = 1;
//                EE_LIST* elist = (EE_LIST*)this->ents[i];
//                elist->init();
//                for(ENTITY* e = elist->next(); e; e = elist->next()) this->unhook(e);
//            } 
//            else 
//            {
//                this->unhook(this->ents[i]);
//            }
//        }
//    }
//
//}
//void WIRE_OFFSET_ANNO::hook_members() 
//{
//
//    if(!this->members_are_hooked()) 
//    {
//        this->hook_members();
//        int i = 2;
//        while(i--) 
//        {
//            if(this->ents[i] && is_EE_LIST(this->ents[i])) 
//            {
//                EE_LIST* elist = (EE_LIST*)this->ents[i];
//                elist->init();
//                for(ENTITY* e = elist->next(); e; e = elist->next()) this->hook(e);
//            } 
//            else 
//            {
//                this->hook(this->ents[i]);
//            }
//        }
//    }
//}
//void WIRE_OFFSET_ANNO::lose_input_tags() 
//{
//
//    this->lose_input_tags();
//    int i = 2;
//    while(i--) {
//        if(descriptors[i].datum_type == in) 
//        {
//            if(is_ATTRIB_TAG(this->ents[i])) 
//            {
//                this->ents[i]->lose();
//                this->backup();
//                this->ents[i] = nullptr;
//            } 
//            else if(this->ents[i] && is_EE_LIST(this->ents[i])) 
//            {
//                EE_LIST* elist = (EE_LIST*)this->ents[i];
//                elist->init();
//                for(ENTITY* e = elist->next(); e; e = elist->next())
//                {
//                    if(is_ATTRIB_TAG(e)) elist->remove(e);
//                }
//            }
//        }
//    }
//}
//void WIRE_OFFSET_ANNO::lose_lists() 
//{
//
//
//    this->lose_lists();
//    int i = 2;
//    while(i--) 
//    {
//        if(this->ents[i] && (this->members_are_hooked() || descriptors[i].datum_type == in || this->unhooked_out_is_ee[i] == 1)) 
//        {
//            if(is_EE_LIST(this->ents[i])) this->ents[i]->lose();
//        }
//    }
//}
//int strcmp_0(const char* Str1, const char* Str2) 
//{
//    return strcmp(Str1, Str2);
//}
//
//ENTITY*& WIRE_OFFSET_ANNO::find_entity_ref_by_name(const char* name, int& isInput) {
//    int i=2;
//    while(i--) 
//    {
//        if(!strcmp_0(WIRE_OFFSET_ANNO::descriptors[i].display_name, name))
//        {
//            isInput = descriptors[i].datum_type == in;
//            return this->ents[i];
//        }
//    }
//    return this->find_entity_ref_by_name(name, isInput);
//}
//ENTITY* WIRE_OFFSET_ANNO::get_entity_by_name(const char* name) 
//{
//    int i = 2;
//    while(i--) 
//    {
//        if(!strcmp_0(descriptors[i].display_name, name)) return get_actual_entity(this->ents[i]);
//    }
//    return this->get_entity_by_name(name);
//}
//void WIRE_OFFSET_ANNO::set_entity_by_name(const char* name, ENTITY* value) 
//{
//    int i= 2;
//    do {
//        if(!i--) {
//            this->set_entity_by_name(name, value);
//            return;
//        }
//    } 
//    while(strcmp_0(WIRE_OFFSET_ANNO::descriptors[i].display_name, name));
//    if(descriptors[i].datum_type == out)
//        this->set_output_entity(this->ents[i], value);
//    else
//        this->set_input_entity(this->ents[i], value);
//}
//int WIRE_OFFSET_ANNO::is_entity_by_name(const char* name, ENTITY* entity) 
//{
//    int i = 2;
//    do 
//    {
//        if(!i--) return this->is_entity_by_name(name, entity);
//    } 
//    while(strcmp_0(descriptors[i].display_name, name));
//    if(this->members_are_hooked() || WIRE_OFFSET_ANNO::descriptors[i].datum_type == in) return this->contains_this_entity(this->ents[i], entity, 1);
//    if(this->unhooked_out_is_ee[i] == 1) return (EE_LIST*)this->ents[i]->lookup(entity) >= 0;//不懂
//    return entity == this->ents[i];
//}
//ENTITY* WIRE_OFFSET_ANNO::make_copy() 
//{
//    int* __formal;                                                                      // [rsp+40h] [rbp-18h]
//    WIRE_OFFSET_ANNO* __formal = ACIS_NEW WIRE_OFFSET_ANNO(nullptr, nullptr, nullptr);//不懂 int* __formal
//
//    (WIRE_OFFSET_ANNO*)__formal=this;//不懂
//    this->fixup_copy((WIRE_OFFSET_ANNO*)__formal);
//    return (ENTITY*)__formal;
//}
//WIRE_OFFSET_ANNO::~WIRE_OFFSET_ANNO() 
//{
//    this->__vftable = (WIRE_OFFSET_ANNO_vtbl*)WIRE_OFFSET_ANNO::`vftable'; 
//    OFST_ANNOTATION::~OFST_ANNOTATION(this);//不懂
//}
//void WIRE_OFFSET_ANNO::fixup_copy(WIRE_OFFSET_ANNO* rollback) 
//{
//    this->fixup_copy(rollback);
//}
//int WIRE_OFFSET_ANNO::identity(int level) 
//{
//    if(level) {
//        if(level >= 0) {
//            if(level <= 3) {
//                if(level == 3)
//                    return (unsigned int)WIRE_OFFSET_ANNO_TYPE;
//                else
//                    return (unsigned int)this->identity(level);
//            } else {
//                return (unsigned int)-1;
//            }
//        } else {
//            return (unsigned int)this->identity(level + 1);
//        }
//    } else {
//        return (unsigned int)WIRE_OFFSET_ANNO_TYPE;
//    }
//}
//const char* WIRE_OFFSET_ANNO::type_name() 
//{
//    return "wire_offset_anno";
//}
//unsigned int WIRE_OFFSET_ANNO::size() 
//{
//    return 88i64;//不懂
//}
//void WIRE_OFFSET_ANNO::debug_ent(_iobuf* fp) 
//{
// 
//
//    this->debug_ent(fp);
//    for(int i = 0; i < 2; ++i) this->debug_helper(descriptors[i].display_name, this->ents[i], fp);
//    this->debug_extra(fp);
//}
//
//void WIRE_OFFSET_ANNO::save(ENTITY_LIST& list) 
//{
//    this->save_begin(0);
//    this->save_common(list);
//    this->save_end(list);
//}
//void WIRE_OFFSET_ANNO::copy_scan(ENTITY_LIST& list, SCAN_TYPE reason, int dpcpy_skip) 
//{
//    int i;  // [rsp+20h] [rbp-18h]
//
//    OFST_ANNOTATION::copy_scan(this, list, reason, dpcpy_skip);
//    i = 2;
//    while(i--) {
//        if(reason == SCAN_DISTRIBUTE) {
//            if(ANNOTATION::members_are_hooked(this)) ENTITY_LIST::add(list, this->ents[i], 1);
//        }
//    }
//}
//int WIRE_OFFSET_ANNO::is_deepcopyable();
//ENTITY* WIRE_OFFSET_ANNO::copy_data(ENTITY_LIST& list, pointer_map* pm, int dpcpy_skip, SCAN_TYPE reason);
//void WIRE_OFFSET_ANNO::fix_pointers(ENTITY** array, SCAN_TYPE reason);
//void WIRE_OFFSET_ANNO::restore_common();
//void WIRE_OFFSET_ANNO::save_common(ENTITY_LIST& list);
//void WIRE_OFFSET_ANNO::copy_common(ENTITY_LIST& list, WIRE_OFFSET_ANNO* from, pointer_map* pm, int dpcpy_skip, SCAN_TYPE reason);
//void WIRE_OFFSET_ANNO::fix_common(ENTITY** array, SCAN_TYPE reason);
//// virtual int (__cdecl *)(void*, METHOD_ARGS&) add_method(METHOD_ID& id, int (__cdecl *)(void*, METHOD_ARGS&) func) ;
//virtual int WIRE_OFFSET_ANNO::call_method(METHOD_ID& id, METHOD_ARGS& args);
//virtual void WIRE_OFFSET_ANNO::lose();
//WIRE_OFFSET_ANNO::WIRE_OFFSET_ANNO(WIRE_OFFSET_ANNO& __that);
//WIRE_OFFSET_ANNO::WIRE_OFFSET_ANNO(ENTITY* original_ent, ENTITY* offset_ent, ENTITY* second_original_ent);
//ENTITY* WIRE_OFFSET_ANNO::original_ents();
//ENTITY* WIRE_OFFSET_ANNO::offset_ent();
//virtual void WIRE_OFFSET_ANNO::split_member(ENTITY* member_entity, ENTITY* new_split_entity);
//WIRE_OFFSET_ANNO& WIRE_OFFSET_ANNO::operator=(WIRE_OFFSET_ANNO& __that);
//virtual void* WIRE_OFFSET_ANNO::__vecDelDtor();