#pragma once
#include "ProtectedInterfaces/off_cu.hxx"
#include "acis/at_sg.hxx"
#include "acis/metharg.hxx"

extern int ATTRIB_OFFREL_TYPE;
#define ATTRIB_OFFREL_LEVEL (ATTRIB_SG_LEVEL + 1)

class ATTRIB_OFFREL : public ATTRIB_SG {
    COEDGE* c;
    VERTEX* v;

  public:
    // ATTRIB_OFFREL(const ATTRIB_OFFREL& __that);
    ATTRIB_OFFREL(ENTITY* owner = NULL, COEDGE* orig_coedge = NULL);
    ATTRIB_OFFREL(ENTITY* owner, const offset_segment& off_seg);
    ATTRIB_OFFREL(ENTITY* owner, VERTEX* orig_vertex);
    // ATTRIB_OFFREL& operator=(const ATTRIB_OFFREL& __that);

    COEDGE* coedge() { return this->c; }
    VERTEX* vertex() { return this->v; }
    int from_coedge() { return this->c != 0; }
    int from_vertex() { return this->v != 0; }

    void split_owner(ENTITY* new_ent);
    void set_coedge(COEDGE* orig_coedge);
    void set_vertex(VERTEX* orig_vertex);

  private:
    virtual ENTITY* make_copy() const;

  protected:
    virtual ~ATTRIB_OFFREL();
    void fixup_copy(ATTRIB_OFFREL*) const;

  public:
    virtual int identity(int = 0) const;
    virtual const char* type_name() const;
    virtual unsigned size() const;
    virtual void debug_ent(FILE*) const;

  public:
    virtual void save(ENTITY_LIST&) const;
    virtual void copy_scan(ENTITY_LIST&, SCAN_TYPE reason = SCAN_COPY, int dpcpy_skip = 0) const;
    virtual int is_deepcopyable() const;
    virtual ENTITY* copy_data(ENTITY_LIST&, pointer_map* pm = 0, int dpcpy_skip = 0, SCAN_TYPE reason = SCAN_COPY) const;
    virtual void fix_pointers(ENTITY*[], SCAN_TYPE reason = SCAN_COPY);
    void restore_common();

  protected:
    void save_common(ENTITY_LIST&) const;
    void copy_common(ENTITY_LIST&, ATTRIB_OFFREL const*, pointer_map* pm = 0, int dpcpy_skip = 0, SCAN_TYPE reason = SCAN_COPY);
    void fix_common(ENTITY*[], SCAN_TYPE reason = SCAN_COPY);

  public:
    static MethodFunction add_method(METHOD_ID const&, MethodFunction);
    virtual int call_method(METHOD_ID const&, METHOD_ARGS const&);
    friend DECL_NONE int is_ATTRIB_OFFREL(const ENTITY*);  // 不懂

  public:
    virtual void lose();
    ;
};
