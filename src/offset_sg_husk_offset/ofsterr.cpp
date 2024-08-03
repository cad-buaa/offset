#include "PrivateInterfaces/ofst_husk_thread_ctx.hxx"
#include "acis/DSYWarningManager.h"
#include "acis/SPA_approx_options.hxx"
#include "acis/SPA_edge_line_arc_options.hxx"
#include "acis/acis_options.hxx"
#include "acis/acistype.hxx"
#include "acis/add_pcu.hxx"
#include "acis/alltop.hxx"
#include "acis/box.hxx"
#include "acis/check.hxx"
#include "acis/ckoutcom.hxx"
#include "acis/condef.hxx"
#include "acis/cone.hxx"
#include "acis/copyent.hxx"
#include "acis/cucuint.hxx"
#include "acis/err_info.hxx"
#include "acis/errmsg.hxx"
#include "acis/exct_int.hxx"
#include "acis/faceutil.hxx"
#include "acis/gen_rendobj.hxx"
#include "acis/importexport.h"
#include "acis/intdef.hxx"
#include "acis/interval.hxx"
#include "acis/kernopts.hxx"
#include "acis/law.hxx"
#include "acis/lists_iterator.hxx"
#include "acis/method.hxx"
#include "acis/module.hxx"
#include "acis/offset_opts.hxx"
#include "acis/option.hxx"
#include "acis/rm_pcu.hxx"
#include "acis/rot_spl.hxx"
#include "acis/sp2crtn.hxx"
#include "acis/spa_null_base.hxx"
#include "acis/spa_progress_info.hxx"
#include "acis/spldef.hxx"
#include "acis/sur.hxx"
#include "acis/transfrm.hxx"
#include "acis/unitvec.hxx"
#include "acis/vlists.hxx"
#include "ProtectedInterfaces/ofsttools.hxx"



void ofst_init() {
    ofst_husk_thread_ctx* v0;  // rax
    ofsterr_thread_ctx* ctx;   // [rsp+20h] [rbp-18h]

    v0 = ofst_husk_context();
    // ctx = ofst_husk_thread_ctx::ofsterr(v0);
    ctx = &(v0->ofsterr());
    ctx->ofst_error_no = -1;
    ctx->err_ent = NULL;
}

int ofst_error_set(int* err_no)
{
    ofst_husk_thread_ctx* v1;  // rax
    ofst_husk_thread_ctx* v2;  // rax

    if(err_no) {
        v1 = ofst_husk_context();
        *err_no = v1->ofsterr().ofst_error_no;
    }
    v2 = ofst_husk_context();
    return v2->ofsterr().ofst_error_no > -1;
}

void ofst_term()
{
    ofst_husk_thread_ctx* v0;  // rax
    ofsterr_thread_ctx* ctx;   // [rsp+20h] [rbp-18h]

    if(ofst_error_set(0i64)) 
    {
        v0 = ofst_husk_context();
        ctx = &(v0->ofsterr());
        if(ctx->err_ent)
            sys_error(ctx->ofst_error_no, ctx->err_ent, 0i64);
        else
            sys_error(ctx->ofst_error_no);
    }
}

void ofst_error(int err_num, int hopeless, ENTITY* e_ent)
{
    
    
    //breakpoint_callback* breakpoint_callback;  // [rsp+28h] [rbp-50h]
    //RenderingObject* v6;                       // [rsp+30h] [rbp-48h]
    //breakpoint_callback* v7;                   // [rsp+38h] [rbp-40h]
    //const char* err_mess;                      // [rsp+48h] [rbp-30h]
    //const char* err_ident;                     // [rsp+50h] [rbp-28h]

    /*if(Debug_Break_Active("ofst_error", "Ofst")) {
        if(get_breakpoint_callback()) {
            breakpoint_callback = get_breakpoint_callback();
            v6 = breakpoint_callback->new_render_object(breakpoint_callback, 1i64);
        } else {
            v6 = 0i64;
        }
        if(v6) {
            if(e_ent) show_entity(e_ent, TOPOLOGY_ERROR, v6);
            err_mess = find_err_mess(err_num);
            err_ident = find_err_ident(err_num);
            acis_fprintf(debug_file_ptr, "ofst_error : %s / %s\n", err_ident, err_mess);
            Debug_Break("ofst_error", "Ofst", "E:\\build\\acis\\NTSwin_b64_debug\\SPAofst\\offset_sg_husk_offset.m\\src\\ofsterr.cpp", 75);
            if(get_breakpoint_callback()) {
                v7 = get_breakpoint_callback();
                v7->delete_render_object(v7, v6);
            }
        }
    }*/
    if(hopeless) {
        if(e_ent)
            sys_error(err_num, e_ent, 0i64);
        else
            sys_error(err_num);
    } else {
        //ofst_husk_thread_ctx* v3 = ofst_husk_context();
        ofst_husk_context()->ofsterr().err_ent = e_ent;
        //ofst_husk_thread_ctx* v4 = ofst_husk_context();
        ofst_husk_context()->ofsterr().ofst_error_no = err_num;
    }
}
