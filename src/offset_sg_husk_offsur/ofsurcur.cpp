#include "PrivateInterfaces/apiFinder.hxx"
#include "ProtectedInterfaces/off_cu.hxx"
#include "acis/api.err"
#include "acis/api.hxx"
#include "acis/curdef.hxx"
#include "acis/intdef.hxx"
#include "acis/sp2crtn.hxx"
#include "acis/surdef.hxx"
#include "acis/surface.hxx"

extern APIFINDER apiFinderACIS;
extern PROCSTATE prostate;

void sg_approx(COEDGE* in_coedge, bs2_curve_def*& coedge_approx_result, SPA_internal_approx_options& approxOpts) {
    typedef void (*sg_approx_type)(COEDGE* in_coedge, bs2_curve_def*& coedge_approx_result, SPA_internal_approx_options& approxOpts);
    sg_approx_type f = (sg_approx_type)apiFinderACIS.GetAddress("?sg_approx@@YAXPEBVCOEDGE@@AEAPEAVbs2_curve_def@@AEAVSPA_internal_approx_options@@@Z", prostate);
    if(f) {
        f(in_coedge, coedge_approx_result, approxOpts);
    }
}

curve* make_offset_curve_from_pcurve(COEDGE* pCoedge, surface& rOffsetSurface) {
    curve* pOffsetCurve = nullptr;
    bs2_curve_def* baseBs2 = nullptr;
    set_global_error_info();
    outcome result(0);
    int error_num = 0;
    problems_list_prop problems_prop;
    acis_exception error_info_holder(0, (error_info_base*)0);
    error_info_base* error_info_base_ptr = nullptr;
    exception_save exception_save_mark;
    api_bb_save make_bulletin_board(result, api_bb_save::e_bb_type::trial);
    exception_save_mark.begin();
    get_error_mark().buffer_init = 1;
    ACISExceptionCheck("API");
    int call_update_from_bb = 1;
    const surface& orig_surf = pCoedge->loop()->face()->geometry()->equation();
    SPAinterval orig_u_range = orig_surf.param_range_u();
    SPAinterval orig_v_range = orig_surf.param_range_v();
    double orig_u_len = orig_u_range.length();
    double orig_v_len = orig_v_range.length();
    int singular_u = (orig_surf.singular_u(orig_u_range.start_pt()) || orig_surf.singular_u(orig_u_range.end_pt()));
    int singular_v = (orig_surf.singular_v(orig_v_range.start_pt()) || orig_surf.singular_v(orig_v_range.end_pt()));
    SPA_COEDGE_approx_options coed_approxOpts;
    coed_approxOpts.set_extend_surface(1);
    SPA_approx_options* approxOpts = &coed_approxOpts;
    SPA_internal_approx_options* impl = coed_approxOpts.get_impl();
    sg_approx(pCoedge, baseBs2, *impl);
    if(baseBs2) {
        AcisVersion vt2 = AcisVersion(28, 0, 2);
        AcisVersion vt1 = GET_ALGORITHMIC_VERSION();
        if((vt1 > vt2) && (singular_u || singular_v)) {
            const surface& new_surf = pCoedge->loop()->face()->geometry()->equation();
            SPAinterval new_u_range = new_surf.param_range_u();
            SPAinterval new_v_range = new_surf.param_range_v();
            double new_u_len = new_u_range.length();
            double new_v_len = new_v_range.length();
            int u_is_shorter, v_is_shorter;
            if(fabs(new_u_range.start_pt() - orig_u_range.start_pt()) <= SPAresnor) {
                if(fabs(new_u_range.end_pt() - orig_u_range.end_pt()) <= SPAresnor) {
                    goto LABEL_17;
                }
            }
            if((orig_u_len - new_u_len) <= SPAresnor)
            LABEL_17:
                u_is_shorter = 0;
            else
                u_is_shorter = 1;

            if(fabs(new_v_range.start_pt() - orig_v_range.start_pt()) <= SPAresnor) {
                if(fabs(new_v_range.end_pt() - orig_v_range.end_pt()) <= SPAresnor) {
                    goto LABEL_22;
                }
            }
            if((orig_v_len - new_v_len) <= SPAresnor)
            LABEL_22:
                v_is_shorter = 0;
            else
                v_is_shorter = 1;

            if(u_is_shorter || v_is_shorter) {
                bs2_curve_delete(baseBs2);
                baseBs2 = nullptr;
                sys_error(API_FAILED);
            }
        }
    }
    if(result.ok() && call_update_from_bb) {
        update_from_bb();
    }
    // coed_approxOpts.~SPA_COEDGE_approx_options();
    // make_bulletin_board.~api_bb_save();
    // exception_save_mark.~exception_save();
    if(acis_interrupted()) {
        sys_error(0, error_info_base_ptr);
    }
    // error_info_holder.~acis_exception();
    error_num = result.error_number();
    problems_prop.process_result(result, PROBLEMS_LIST_PROP_OR_IGNORE, 0);
    if(baseBs2) {
        if(pCoedge->sense() != pCoedge->edge()->sense()) {
            bs2_curve_reverse(baseBs2);
        }
        SPAinterval bs2Range = bs2_curve_range(baseBs2);
        double fActualTol = -1.0;
        bs2_curve_def* pc1 = baseBs2;
        pOffsetCurve = (curve*)ACIS_NEW intcurve(NULL, fActualTol, rOffsetSurface, SpaAcis::NullObj::get_surface(), pc1, NULL, bs2Range, 1, 0);
        pOffsetCurve->deep_calc_disc_info();
    }
    // problems_prop.~problems_list_prop();
    // result.~outcome();
    return pOffsetCurve;
}
