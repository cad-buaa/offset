#pragma once
#include "acis/tlkit_cb.hxx"
#include "acis/context_cb.hxx"
#include "acis/lists.hxx"

class ofsterr_thread_ctx : public ACIS_OBJECT {
  public:
    ofsterr_thread_ctx();
    ~ofsterr_thread_ctx();
    void clear();
    void clear_no_dtors();
    void inc_access();
    int get_access();

    int count_accesses;
    int ofst_error_no;
    ENTITY* err_ent;
    ENTITY_LIST* error_list;
};

class ofst_husk_thread_ctx : public context_callbacks {
  public:
    ofsterr_thread_ctx& ofsterr();
    ofst_husk_thread_ctx();
    ~ofst_husk_thread_ctx();
    virtual void clear_no_dtors();
    virtual void clear();
    virtual void print_access();
    //virtual void* __vecDelDtor();

    ofsterr_thread_ctx m_ofsterr;
    int count_accesses;
};

ofst_husk_thread_ctx* ofst_husk_context();
