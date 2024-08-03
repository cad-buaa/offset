#include "PrivateInterfaces/ofst_husk_thread_ctx.hxx"

#include "acis/acis_options.hxx"
#include "acis/api.hxx"
#include "acis/cstrapi.hxx"
#include "acis/offset_opts.hxx"
#include "acis/wire_offset_options.hxx"

safe_pointer_type<ofst_husk_thread_ctx> ofst_husk_thread_ctx_ptr;

ofst_husk_thread_ctx* ofst_husk_context() {
    // return safe_pointer_type<ofst_husk_thread_ctx>::operator ofst_husk_thread_ctx*(&ofst_husk_thread_ctx_ptr);
    if(!ofst_husk_thread_ctx_ptr) {
        ofst_husk_thread_ctx_ptr = ACIS_NEW ofst_husk_thread_ctx;
    }
    return ofst_husk_thread_ctx_ptr;
}

ofst_husk_thread_ctx::ofst_husk_thread_ctx() {
    this->m_ofsterr = ofsterr_thread_ctx();
    this->count_accesses = 0;
}

ofst_husk_thread_ctx::~ofst_husk_thread_ctx() {
    this->m_ofsterr.~ofsterr_thread_ctx();
}

ofsterr_thread_ctx& ofst_husk_thread_ctx::ofsterr() {
    this->m_ofsterr.inc_access();
    return this->m_ofsterr;
}

void ofst_husk_thread_ctx::clear_no_dtors() {
    this->m_ofsterr.clear_no_dtors();
}

void ofst_husk_thread_ctx::clear() {
    this->m_ofsterr.clear();
}

void ofst_husk_thread_ctx::print_access() {
    // unsigned int access;  // [rsp+20h] [rbp-18h]

    // access = ofsterr_thread_ctx::get_access(&this->m_ofsterr);
    // acis_fprintf(debug_file_ptr, (const char *)&ofst_husk_thread_ctx::`vftable'[4], access);
    // acis_fprintf(debug_file_ptr, "Total for ofst_husk_thread_ctx %d\n", (unsigned int)this->count_accesses);
}

ofsterr_thread_ctx::ofsterr_thread_ctx() {
    this->count_accesses = 0;
    this->ofst_error_no = 0;
    this->err_ent = nullptr;
    this->error_list = nullptr;
}

ofsterr_thread_ctx::~ofsterr_thread_ctx() {
    this->clear();
}

void ofsterr_thread_ctx::clear() {
    this->clear_no_dtors();
}

void ofsterr_thread_ctx::clear_no_dtors() {
    this->ofst_error_no = 0;
    this->err_ent = nullptr;
    this->error_list = nullptr;
}

void ofsterr_thread_ctx::inc_access() {
    ++this->count_accesses;
}

int ofsterr_thread_ctx::get_access() {
    return this->count_accesses;
}
