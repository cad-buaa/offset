#include "ProtectedInterfaces/at_rel.hxx"

#include "acis/attrib.hxx"
#include "acis/datamsc.hxx"

#define THIS() ATTRIB_OFFREL
#define THIS_LIB NONE
#define PARENT() ATTRIB_SG
#define PARENT_LIB KERN

#define ATTRIB_OFFREL_NAME "offrel"

// ATTCOPY_DEF("offrel")
int ATTRIB_OFFREL_TYPE = 0;

int is_ATTRIB_OFFREL(const ENTITY* e) {
    return ((e != 0) && (e->identity(ATTRIB_OFFREL_LEVEL) == ATTRIB_OFFREL_TYPE));
}
static ENTITY* ATTRIB_OFFREL_restore_data();
extern DECL_KERN restore_def* ATTRIB_SG_subclasses;
DECL_NONE restore_def* ATTRIB_OFFREL_subclasses;
static restore_def ATTRIB_OFFREL_restore_def(ATTRIB_SG_subclasses, ATTRIB_OFFREL_NAME, ATTRIB_OFFREL_TYPE, ATTRIB_OFFREL_restore_data, &ATTRIB_OFFREL_subclasses);
static METHOD_TABLE ATTRIB_OFFREL_METHODS;
MethodFunction ATTRIB_OFFREL::add_method(METHOD_ID const& id, MethodFunction func) {
    return ATTRIB_OFFREL_METHODS.add(id, func);
}
int ATTRIB_OFFREL::call_method(METHOD_ID const& id, METHOD_ARGS const& args) {
    if(strcmp(args.id(), id.arg_id())) return 0;
    MethodFunction func = ATTRIB_OFFREL_METHODS.find(id);
    if(func)
        return (*func)(this, args);
    else
        return ATTRIB_SG::call_method(id, args);
}
ENTITY* ATTRIB_OFFREL::make_copy() const {
    ATTRIB_OFFREL* rollback = new(eDefault, "D:\\offsetting\\module-dev-offsetting\\src\\offset_sg_husk_cur_off\\at_rel.cpp", 14, &alloc_file_index) ATTRIB_OFFREL;
    *rollback = *this;
    fixup_copy(rollback);
    return rollback;
}
int ATTRIB_OFFREL::identity(int level) const {
    return level == 0 ? ATTRIB_OFFREL_TYPE : level < 0 ? ATTRIB_SG::identity(level + 1) : level > ATTRIB_OFFREL_LEVEL ? -1 : level == ATTRIB_OFFREL_LEVEL ? ATTRIB_OFFREL_TYPE : ATTRIB_SG::identity(level);
}
const char* ATTRIB_OFFREL::type_name() const {
    return ATTRIB_OFFREL_NAME;
}
unsigned ATTRIB_OFFREL::size() const {
    return sizeof(ATTRIB_OFFREL);
}
void ATTRIB_OFFREL::save(ENTITY_LIST& list) const {
    save_begin();
    save_common(list);
    save_end(list);
}
static ENTITY* ATTRIB_OFFREL_restore_data() {
    ATTRIB_OFFREL* new_ent = new(eDefault, "D:\\offsetting\\module-dev-offsetting\\src\\offset_sg_husk_cur_off\\at_rel.cpp", 14, &alloc_file_index) ATTRIB_OFFREL;
    new_ent->restore_common();
    return new_ent;
}
void ATTRIB_OFFREL::fix_pointers(ENTITY* array[], SCAN_TYPE reason) {
    fix_common(array, reason);
}
ENTITY* ATTRIB_OFFREL::copy_data(ENTITY_LIST& list, pointer_map* pm, int dpcpy_skip, SCAN_TYPE reason) const {
    ATTRIB_OFFREL* new_att = new(eDefault, "D:\\offsetting\\module-dev-offsetting\\src\\offset_sg_husk_cur_off\\at_rel.cpp", 14, &alloc_file_index) ATTRIB_OFFREL;
    new_att->copy_common(list, this, pm, dpcpy_skip, reason);
    return new_att;
}
void ATTRIB_OFFREL::fixup_copy(ATTRIB_OFFREL* rollback) const {
    ATTRIB_SG::fixup_copy(rollback);
}
void ATTRIB_OFFREL::lose() {
    ATTRIB_SG::lose();
}
ATTRIB_OFFREL::~ATTRIB_OFFREL() {
    check_destroy();
    // dtor, 不需要补充
}
void ATTRIB_OFFREL::debug_ent(FILE* fp) const {
    ATTRIB_SG::debug_ent(fp);
    if(this->c) {
        debug_old_pointer("From coedge", this->c, fp);
    } else if(this->v) {
        debug_old_pointer("From vertex", this->v, fp);
    } else {
        debug_string("From entity", "none", fp);
    }
}
void ATTRIB_OFFREL::save_common(ENTITY_LIST& list) const {
    write_id_level(ATTRIB_OFFREL_NAME, ATTRIB_OFFREL_LEVEL);
    ATTRIB_SG::save_common(list);
    write_ptr(this->c, list);
    write_ptr(this->v, list);
    // save_common, 不需要补充
}
void ATTRIB_OFFREL::restore_common() {
    ATTRIB_SG::restore_common();
    // restore_common, 不需要补充
}
int ATTRIB_OFFREL::is_deepcopyable() const {
    return ATTRIB_SG::is_deepcopyable();
}
void ATTRIB_OFFREL::copy_common(ENTITY_LIST& list, ATTRIB_OFFREL const* from, pointer_map* pm, int dpcpy_skip, SCAN_TYPE reason) {
    ATTRIB_SG::copy_common(list, from, pm, dpcpy_skip, reason);
    this->c = (COEDGE*)list.lookup(from->c);
    this->v = (VERTEX*)list.lookup(from->v);
}
void ATTRIB_OFFREL::copy_scan(ENTITY_LIST& list, SCAN_TYPE reason, int dpcpy_skip) const {
    ATTRIB_SG::copy_scan(list, reason, dpcpy_skip);
    // copy_scan, 不需要补充
}
void ATTRIB_OFFREL::fix_common(ENTITY* array[], SCAN_TYPE reason) {
    ATTRIB_SG::fix_common(array, reason);
    this->c = (COEDGE*)read_array(array, this->c);
    this->v = (VERTEX*)read_array(array, this->v);
    // fix_common, 不需要补充
}

// ATTRIB_OFFREL::ATTRIB_OFFREL(const ATTRIB_OFFREL& __that)
//{
//
//     this->ATTRIB_SG(__that);
//     this->c = __that.c;
//     this->v = __that.v;
//
// }

ATTRIB_OFFREL::ATTRIB_OFFREL(ENTITY* owner, COEDGE* orig_coedge): ATTRIB_SG(owner) {
    this->c = nullptr;
    this->v = nullptr;
    this->set_coedge(orig_coedge);
}

ATTRIB_OFFREL::ATTRIB_OFFREL(ENTITY* owner, const offset_segment& off_seg): ATTRIB_SG(owner) {
    this->c = nullptr;
    this->v = nullptr;
    if(off_seg.coedge_offset()) {
        this->set_coedge(off_seg.original_coedge());
    } else {
        this->set_vertex(off_seg.original_vertex());
    }
}
//不确定
ATTRIB_OFFREL::ATTRIB_OFFREL(ENTITY* owner, VERTEX* orig_vertex) 
{
    this->c = nullptr;
    this->v = nullptr;
    this->set_vertex(orig_vertex);
}


void ATTRIB_OFFREL::split_owner(ENTITY* new_ent) {
    if(this->c) {
        ATTRIB_OFFREL* v2 = ACIS_NEW ATTRIB_OFFREL(new_ent, this->c);
    } 
    else {
        ATTRIB_OFFREL* v3 = ACIS_NEW ATTRIB_OFFREL(new_ent, this->v);
    }
}

void ATTRIB_OFFREL::set_coedge(COEDGE* orig_coedge) {
    if(this->c != orig_coedge || this->v) {
        this->backup();
        this->c = orig_coedge;
        this->v = nullptr;
    }
}
void ATTRIB_OFFREL::set_vertex(VERTEX* orig_vertex) {
    if(this->v != orig_vertex) {
        this->backup();
        this->c = nullptr;
        this->v = orig_vertex;
    }
}

// ATTRIB_OFFREL::ATTRIB_OFFREL(ATTRIB_OFFREL& __that)
//{
//
// }
// ATTRIB_OFFREL::ATTRIB_OFFREL(ENTITY* owner, offset_segment& off_seg) {
//      //COEDGE* v3;  // rax
//      //VERTEX* v4;  // rax
//
//      //ATTRIB_SG::ATTRIB_SG(this, owner);
//      //this->__vftable = (ATTRIB_OFFREL_vtbl*)ATTRIB_OFFREL::`vftable';
//      //this->c = nullptr;
//      //this->v = nullptr;
//      //if(off_seg.coedge_offset()) {
//      //    this->set_coedge(off_seg.original_coedge());
//      //}
//      //else {
//      //    this->set_vertex(off_seg.original_vertex());
//      //}
// }
// ATTRIB_OFFREL::ATTRIB_OFFREL(ENTITY* owner, VERTEX* orig_vertex) {
//      //this->ATTRIB_SG(owner);
//      //this->__vftable = (ATTRIB_OFFREL_vtbl*)ATTRIB_OFFREL::`vftable'; this->c = 0i64;
//      //this->v = nullptr;
//      //this->set_vertex(orig_vertex);
// }
// ATTRIB_OFFREL::ATTRIB_OFFREL(ENTITY* owner, COEDGE* orig_coedge) {
//     // this->ATTRIB_SG(owner);
//     // this->__vftable = (ATTRIB_OFFREL_vtbl*)ATTRIB_OFFREL::`vftable'; this->c = 0i64;
//     // this->v = 0i64;
//     // this->set_coedge(orig_coedge);
// }
// void ATTRIB_OFFREL::set_coedge(COEDGE* orig_coedge) {
//     if(this->c != orig_coedge || this->v) {
//         this->backup();
//         this->c = orig_coedge;
//         this->v = nullptr;
//     }
// }
// void ATTRIB_OFFREL::set_vertex(VERTEX* orig_vertex) {
//     // if(this->c != __PAIR128__(orig_vertex, 0i64)) //不确定
//     //{
//     //     this->backup();
//     //     this->c = nullptr;
//     //     this->v = orig_vertex;
//     // }
// }
// int ATTRIB_OFFREL::from_coedge() {
//     return this->c != nullptr;
// }
// int ATTRIB_OFFREL::from_vertex() {
//     return this->v != nullptr;
// }
// COEDGE* ATTRIB_OFFREL::coedge() {
//     return this->c;
// }
// VERTEX* ATTRIB_OFFREL::vertex() {
//     return this->v;
// }
// void ATTRIB_OFFREL::split_owner(ENTITY* new_ent)  // 去掉了virtual
//{
//     if(this->c) {
//         ATTRIB_OFFREL* v2 = ACIS_NEW ATTRIB_OFFREL(new_ent, this->c);
//     } else {
//         ATTRIB_OFFREL* v3 = ACIS_NEW ATTRIB_OFFREL(new_ent, this->v);
//     }
// }
// ENTITY* ATTRIB_OFFREL::make_copy() {
//     return nullptr;
//     // int* v1;            // rax
//     // int* __formal;      // [rsp+40h] [rbp-18h]
//
//     // ATTRIB_OFFREL* v3 = ACIS_NEW ATTRIB_OFFREL(nullptr, nullptr);
//     //__formal = v3;
//
//     // (ATTRIB_OFFREL*)ACIS_OBJECT::operator new(0x60ui64, eDefault, "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_cur_off.m\\src\\at_rel.cpp", 42, &alloc_file_index_3491);
//     // if(v3) {
//     // ATTRIB_OFFREL::ATTRIB_OFFREL(v3, 0i64, 0i64);
//     // __formal = v3;
//     // } else {
//     // __formal = 0i64;
//     // }
//     //(ATTRIB_OFFREL*)__formal = this);
//     // ATTRIB_OFFREL::fixup_copy(this, (ATTRIB_OFFREL*)__formal);
//     // return (ENTITY*)__formal;
// }
// ATTRIB_OFFREL::~ATTRIB_OFFREL() {
//     // this->__vftable = (ATTRIB_OFFREL_vtbl*)ATTRIB_OFFREL::`vftable';
//
//     // this->check_destroy();
//     // this->~ATTRIB_SG();
// }
// void ATTRIB_OFFREL::fixup_copy(ATTRIB_OFFREL* rollback) {
// }
// int ATTRIB_OFFREL::identity(int level) {
//     return NULL;
// }
// const char* type_name() {
//     return nullptr;
// }
// int ATTRIB_OFFREL::size() {
//     return NULL;
// }
// void ATTRIB_OFFREL::debug_ent(_iobuf* fp) {
// }
// void ATTRIB_OFFREL::save(ENTITY_LIST& list) {
// }
// void ATTRIB_OFFREL::copy_scan(ENTITY_LIST& list, SCAN_TYPE reason, int dpcpy_skip) {
// }
// int ATTRIB_OFFREL::is_deepcopyable() {
//     return NULL;
// }
// ENTITY* ATTRIB_OFFREL::copy_data(ENTITY_LIST& list, pointer_map* pm, int dpcpy_skip, SCAN_TYPE reason) {
//     return nullptr;
// }
// void ATTRIB_OFFREL::fix_pointers(ENTITY** array, SCAN_TYPE reason) {
// }
// void ATTRIB_OFFREL::restore_common() {
// }
// void ATTRIB_OFFREL::save_common(ENTITY_LIST& list) {
// }
// void ATTRIB_OFFREL::copy_common(ENTITY_LIST& list, ATTRIB_OFFREL* from, pointer_map* pm, int dpcpy_skip, SCAN_TYPE reason) {
// }
// void ATTRIB_OFFREL::fix_common(ENTITY** array, SCAN_TYPE reason) {
// }