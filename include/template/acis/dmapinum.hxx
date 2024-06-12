// $Id: dmapinum.hxx,v 1.15 2001/07/23 20:05:28 btomas Exp $
/*******************************************************************/
/*    Copyright (c) 1989-2020 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#ifndef DMAPI_CONSTS_H
#define DMAPI_CONSTS_H
#include "dcl_ds.h" //for DS_TAGS //for DS_TAGS

/**
* @file dmapinum.hxx
 * @CAA2Level L1
 * @CAA2Usage U1
 * \addtogroup DMAPI
 *
 * @{
 */
/**
 * @nodoc
 */

enum DS_ZONE    { ds_zone, ds_rzn, ds_pzn } ;

// DS_CSTRN type_id enum

/*
// tbrv
*/
/**
 * @nodoc
 */

enum DS_CST { ds_cst,      // DS_cstrn          base class identifier
              ds_pst,      // DS_pt_cstrn       derived class identifier
              ds_cct,      // DS_crv_cstrn      derived class identifier
              ds_lct,      // DS_link_cstrn     derived class identifier
              ds_lcl,      // DS_link_crv_load  derived class identifier
              ds_act,      // DS_area_cstrn     derived class identifier
              ds_ald,      // derived class DS_area_load
              ds_cnone } ;
// DS_pfunc type_id enumerator

/**
* Enumerator for <tt>DS_pfunc</tt> <tt>type_id</tt>.
* @param ds_pfn
* class DS_PFN.
* @param ds_tp1
* class DS_tprod_1d.
* @param ds_tp2
* class DS_tprod_2d.
* @param ds_rp1
* class DS_rprod_1d.
* @param ds_rp2
* class DS_rprod_2d.
* @param ds_cir
* optional class DS_circ.
**/

enum DS_PFN {ds_pfn,
             ds_tp1,       // class DS_tprod_1d
             ds_tp2,       // class DS_tprod_2d
             ds_rp1,       // class DS_rprod_1d
             ds_rp2,       // class DS_rprod_2d
             ds_cir } ;    // optional class DS_circ

// DS_dmod type_id enumerator
/*
// tbrv
*/
/**
 * @nodoc
 */

enum DS_DMO { ds_dsm,      // base    class DS_dmod
              ds_dsf,      // derived class DS_dsurf
              ds_dcv,      // derived class DS_dcurv
              ds_tpc,      // dcrv used as a tracking pos curve
              ds_ttc,      // dcrv used as a tracking tan curve
              ds_tcc,      // dcrv used as a tracking curvature curve
              ds_dbd  } ;  // bad dmod object

// DS_load type_id enum

/*
// tbrv
*/
/**
 * @nodoc
 */

enum DS_LDS { ds_lds,      // base class DS_load
              ds_ppr,      // derived class DS_pt_press
              ds_dpr,      // derived class DS_dist_press
              ds_dld,      // derived class DS_dyn_load
              ds_vec,      // derived class DS_vector_load
              ds_att,      // derived class DS_attractor
              ds_spr,      // derived class DS_spring
              ds_sps,      // derived class DS_spring_set
//              ds_crl,      // derived class DS_crv_load
              ds_lnone } ; // a null value for subroutine returns

// DS_CSTRN_SRC calssifies and identifies constraint purpose and behaviors.

/**
* Classifies and identifies constraint purpose and behavior.
* @param ds_solid_cstrn
* pt or crv_cstrn from connected ACIS solid model boundary.
* @param ds_bound_cstrn
* pt or crv_cstrn from unconnected ACIS solid model boundary.
* @param ds_user_cstrn
* pt or crv_cstrn generated by end-user at run-time.
* @param ds_seam_cstrn
* pt or crv_cstrn connecting a patch to a parent patch.
* @param ds_link_cstrn
* pt or crv_cstrn connecting two multisurf patches.
* @param ds_undef_cstrn
* default val for uninitiated states.
* @param ds_invalid_cstrn
* cstrn from constructor with invalid input arguments.
**/

enum DS_CSTRN_SRC
{ ds_solid_cstrn,   // pt or crv_cstrn from connected solid model boundary
  ds_bound_cstrn,   // pt or crv_cstrn from un-connected solid model boundary
  ds_user_cstrn,    // pt or crv_cstrn generated by end-user at run-time
  ds_seam_cstrn,    // pt or crv_cstrn connecting a patch to a parent patch
  ds_link_cstrn,    // pt or crv_cstrn connecting two multisurf patches
  ds_undef_cstrn,   // default val for uninitiated states
  ds_invalid_cstrn  // cstrn from constructor with invalid input arguments
} ;
// render flags for GRAPHICS
//  CPTS   = draw control points
//  SEAMS  = draw child's crv-cstrns that connect it to a parent
//  CSTRNS = draw crv-cstrn data
//  LOADS  = draw load data
//  CURVE_COMB = draw curvature comb for deformable curve shape
//  ELEMS  = draw elem boundaries within deformable model shape
//  SEG_BNDS = draw crv-cstrn locs at which integrals segmented
//  CSTRN_NORMS = draw crv-cstrn tang vectors
//  CSTRN_COMBS = draw curvature comb for crv-cstrns within a surface
//  GAUSS_PTS = draw the gauss point locations used for integration
/**
 * 1 tags_fig
 */
#define SPA_DM_DRAW_CPTS        ( 1 << 0 )     //   
/**
 * 2 tags_fig
 */
#define SPA_DM_DRAW_SEAMS       ( 1 << 1 )     //  
/**
 *  4 tags_fig
 */
#define SPA_DM_DRAW_CSTRNS      ( 1 << 2 )     //  
/**
 *  8 tags_fig
 */
#define SPA_DM_DRAW_LOADS       ( 1 << 3 )     // 
/**
 * 16 tags_fig
 */
#define SPA_DM_DRAW_CURVE_COMB  ( 1 << 4 )     // 
/**
 * 32 tags_fig
 */
#define SPA_DM_DRAW_TORQUE_COMB ( 1 << 5 )     // 
/**
 * 64 tags_fig
 */
#define SPA_DM_DRAW_LOG_OF_COMB ( 1 << 6 )     // 
/**
 * 128 tags_fig
 */
#define SPA_DM_DRAW_ELEMS       ( 1 << 7 )     //
/**
 * 256 tags_fig
 */
#define SPA_DM_DRAW_SEG_BNDS    ( 1 << 8 )     //
/**
 * 512 tags_fig
 */
#define SPA_DM_DRAW_CSTRN_NORMS ( 1 << 9 )     //
/**
 * 1024 tags_fig
 */
#define SPA_DM_DRAW_CSTRN_COMBS ( 1 << 10)     //
/**
 * 2048 tags_fig
 */
#define SPA_DM_DRAW_GAUSS_PTS   ( 1 << 11)     //
/**
 * 4096 used to mark an eldest sibling
 */
#define SPA_DM_FIRST_SIBLING    ( 1 << 12)     //


// DS_CSTRN behavior bits  (bit values for cst_behavior bit array)
/**
 * DS_CSTRN behavior bits <br>
 * 1=cstrn may be deleted
 */
#define DS_CST_DELETABLE     (1 << 0 ) //
/**
 * DS_CSTRN behavior bits <br>
 * 1=cstrn may be disabled
 */
#define DS_CST_STOPABLE      (1 << 1 ) //
/**
 * DS_CSTRN behavior bits <br>
 * 1=cstrn is on,0=cstrn is off
 */
#define DS_CST_ON_OFF        (1 << 2 ) //

// pt_cstrn, crv_cstrn, link_cstrn 1st curve behaviors
/**
 * DS_CSTRN behavior bits <br>
 * SPAposition cstrn not active
 */
#define DS_CST_POS_FREE      (   0   ) //
/**
 * DS_CSTRN behavior bits <br>
 * pos cstrn is fixed
 */
#define DS_CST_POS_FIXED     (1 << 3 ) //
/**
 * DS_CSTRN behavior bits <br>
 * pos cstrn linked to another
 */
#define DS_CST_POS_LINKED    (1 << 4 ) //
/**
 * DS_CSTRN behavior bits <br>
 * tangent cstrn not active
 */
#define DS_CST_TAN_FREE      (   0   ) //
/**
 * DS_CSTRN behavior bits <br>
 * tan cstrn is fixed
 */
#define DS_CST_TAN_FIXED     (1 << 5 ) //
/**
 * DS_CSTRN behavior bits <br>
 * tan cstrn linked to another
 */
#define DS_CST_TAN_LINKED    (1 << 6 ) //
/**
 * DS_CSTRN behavior bits <br>
 * 1 Normal is fixed
 */
#define DS_CST_NORM_FIXED    (1 << 15) //
/**
 * DS_CSTRN behavior bits <br>
 * 1 curve binorm is fixed
 */
#define DS_CST_BINORM_FIXED  (1 << 16) //
/**
 * DS_CSTRN behavior bits <br>
 * curvature not active
 */
#define DS_CST_CURV_FREE     (   0   ) //
/**
 * DS_CSTRN behavior bits <br>
 * C2 cstrn is fixed
 */
#define DS_CST_CURV_FIXED    (1 << 7 ) //
/**
 * DS_CSTRN behavior bits <br>
 * C2 cstrn linked to another
 */
#define DS_CST_CURV_LINKED   (1 << 8 ) //
/**
 * DS_CSTRN behavior bits <br>
 * SPAposition cstrn not active
 */
#define DS_CST_POS_2_FREE    (   0   ) //
/**
 * DS_CSTRN behavior bits <br>
 * pos cstrn is fixed
 */
#define DS_CST_POS_2_FIXED   (1 << 9 ) //
/**
 * DS_CSTRN behavior bits <br>
 * pos cstrn linked to another
 */
#define DS_CST_POS_2_LINKED  (1 << 10) //
/**
 * DS_CSTRN behavior bits <br>
 * tangent cstrn not active
 */
#define DS_CST_TAN_2_FREE    (   0   ) //
/**
 * DS_CSTRN behavior bits <br>
 *.
 */
#define DS_CST_TAN_2_FIXED   (1 << 11) //
/**
 * DS_CSTRN behavior bits <br>
 * tan cstrn linked to another
 */
#define DS_CST_TAN_2_LINKED  (1 << 12) //
/**
 * DS_CSTRN behavior bits <br>
 * curvature not active
 */
#define DS_CST_CURV_2_FREE   (   0   ) //
/**
 * DS_CSTRN behavior bits <br>
 * C2 cstrn is fixed
 */
#define DS_CST_CURV_2_FIXED  (1 << 13) //
/**
 * DS_CSTRN behavior bits <br>
 * C2 cstrn linked to another
 */
#define DS_CST_CURV_2_LINKED (1 << 14) //
/**
 * DS_CSTRN behavior bits <br>
 * uncoupled curvature
 */
#define DS_CST_CURV_ONLY_FIXED  (1 << 17)
/**
 * DS_CSTRN behavior bits <br>
 * uncoupled curvature
 */
#define DS_CST_CURV_ONLY_LINKED (1 << 18)
/**
 * DS_CSTRN behavior bits <br>
 * uncoupled curvature
 */
#define DS_CST_CURV_2_ONLY_FIXED (1 << 19)
/**
 * DS_CSTRN behavior bits <br>
 * uncoupled curvature
 */
#define DS_CST_CURV_2_ONLY_LINKED (1 << 20)


// for compatability with previous versions
//  to be used as behavior values
/**
 *.
 */
#define DS_CST_POSITION   DS_CST_POS_FIXED
/**
 *.
 */
#define DS_CST_TANGENT    DS_CST_TAN_FIXED
/**
 *.
 */
#define DS_CST_CURVATURE  DS_CST_CURV_FIXED
// 

/**
 * valid states for all constraints
 */
#define SPA_DM_DELETABLE      DS_CST_DELETABLE
/**
 * valid states for all constraints
 */
#define SPA_DM_STOPABLE       DS_CST_STOPABLE
/**
 * valid states for all constraints
 */
#define SPA_DM_ON_OFF         DS_CST_ON_OFF

/**
 * valid states for all constraints
 */
#define SPA_DM_POS_FREE       DS_CST_POS_FREE
/**
 * valid states for all constraints
 */
#define SPA_DM_POS_FIXED      DS_CST_POS_FIXED
/**
 * valid states for all constraints
 */
#define SPA_DM_POS_LINKED     DS_CST_POS_LINKED
/**
 * valid states for all constraints
 */
#define SPA_DM_TAN_FREE       DS_CST_TAN_FREE
/**
 * valid states for all constraints
 */
#define SPA_DM_TAN_FIXED      DS_CST_TAN_FIXED
/**
 * valid states for all constraints
 */
#define SPA_DM_TAN_LINKED     DS_CST_TAN_LINKED
/**
 * valid states for all constraints
 */
#define SPA_DM_NORM_FIXED     DS_CST_NORM_FIXED
/**
 * valid states for all constraints
 */
#define SPA_DM_BINORM_FIXED   DS_CST_BINORM_FIXED
/**
 * valid states for all constraints
 */
#define SPA_DM_CURV_FREE      DS_CST_CURV_FREE
/**
 * valid states for all constraints
 */
#define SPA_DM_CURV_FIXED     DS_CST_CURV_FIXED
/**
 * valid states for all constraints
 */
#define SPA_DM_CURV_LINKED    DS_CST_CURV_LINKED

/**
 * valid states for all constraints
 */
#define SPA_DM_CURV_ONLY_LINKED      DS_CST_CURV_ONLY_LINKED
/**
 * valid states for all constraints
 */
#define SPA_DM_CURV_ONLY_FIXED       DS_CST_CURV_ONLY_FIXED
/**
 * valid states for all constraints
 */
#define SPA_DM_CURV_2_ONLY_LINKED    DS_CST_CURV_2_ONLY_LINKED
/**
 * valid states for all constraints
 */
#define SPA_DM_CURV_2_ONLY_FIXED     DS_CST_CURV_2_ONLY_FIXED

//

/**
 * constraint states used only by link_cstrns
 */
#define SPA_DM_POS_2_FREE     DS_CST_POS_2_FREE
/**
 * constraint states used only by link_cstrns
 */
#define SPA_DM_POS_2_FIXED    DS_CST_POS_2_FIXED
/**
 * constraint states used only by link_cstrns
 */
#define SPA_DM_POS_2_LINKED   DS_CST_POS_2_LINKED

/**
 * constraint states used only by link_cstrns
 */
#define SPA_DM_TAN_2_FREE     DS_CST_TAN_2_FREE
/**
 * constraint states used only by link_cstrns
 */
#define SPA_DM_TAN_2_FIXED    DS_CST_TAN_2_FIXED
/**
 * constraint states used only by link_cstrns
 * Note: SPA_DM_TAN_2_FIXED is also used by point tangent constraints
 */
#define SPA_DM_TAN_2_LINKED   DS_CST_TAN_2_LINKED
/**
 * constraint states used only by link_cstrns
 */
#define SPA_DM_CURV_2_FREE    DS_CST_CURV_2_FREE
/**
 * constraint states used only by link_cstrns
 */
#define SPA_DM_CURV_2_FIXED   DS_CST_CURV_2_FIXED
/**
 * constraint states used only by link_cstrns
 */
#define SPA_DM_CURV_2_LINKED  DS_CST_CURV_2_LINKED

// for backward compatability
// Note: SPA_DM_TANGENT has been replaced by
//              SPA_DM_TANG for point tangent constraints on deformable curves
//              SPA_DM_NORM for surface normal constraints
// the symbol SPA_DM_TANGENT has been retired so that the compiler will prompt
// users to examine all uses in their code and make the appropriate replacement.

/**
 *.
 */
#define SPA_DM_POSITN   SPA_DM_POS_FIXED
/**
 *.
 */
#define SPA_DM_TANG       SPA_DM_TAN_FIXED
/**
 *.
 */
#define SPA_DM_TANG1      SPA_DM_TAN_FIXED
/**
 *.
 */
#define SPA_DM_TANG2      SPA_DM_TAN_2_FIXED
/**
 *.
 */
#define SPA_DM_NORM       SPA_DM_NORM_FIXED
/**
 *.
 */
#define SPA_DM_CURVATURE  SPA_DM_CURV_FIXED

// enumerate all possible tag object types

/**
* Enumeration of all possible tag object types.
* @param DS_tag_none
* value for no such tag object.
* @param DS_tag_control_pt
* pfunc dof object (the control points).
* @param DS_tag_pt_press
* point pressures [Base DS_load].
* @param DS_tag_dist_press
* distributed pressure [Base DS_load].
* @param DS_tag_spring
* Spring load [Base DS_load].
* @param DS_tag_spring_set
* A set of spring loads [Base DS_load].
* @param DS_tag_crv_load
* Curve Spring load [Base DS_load].
* @param DS_tag_dyn_load
* Dynamic load [Base DS_load].
* @param DS_tag_pt_cstrn
* point constraint [Base DS_cstrn].
* @param DS_tag_pt_seam
* seam pt_cstrn [Base DS_cstrn].
* @param DS_tag_crv_cstrn
* curve constraint [Base DS_cstrn].
* @param DS_tag_crv_seam
* seam crv_cstrn [Base DS_cstrn].
* @param DS_tag_srf_dmod
* surface dmod [Base DS_dmod].
* @param DS_tag_crv_dmod
* curve dmod [Base DS_dmod].
* @param DS_tag_vector_load
* vector_load [Base DS_load].
* @param DS_tag_attractor
* attractor load [Base DS_load].
* @param DS_tag_link_cstrn
* multi-surf link cstrn [Base DS_cstrn].
* @param DS_tag_area_cstrn
* area cstrn [Base DS_cstrn].
* @param DS_tag_link_load
* link load [Base DS_cstrn].
* @param DS_tag_track_pos_crv
* tracking pos curve dmod [BASE DS_dmod].
* @param DS_tag_track_tan_crv
* tracking tan curve dmod.
* @param DS_tag_track_curv_crv
* tracking curvature curve dmod.
* @param DS_tag_area_load
* area load [Base DS_cstrn].
**/

enum DS_TAGS
{ DS_tag_none,           // 0 = value for no such tag object
  DS_tag_control_pt,     // 1 = pfunc dof object (the control points)
  DS_tag_pt_press,       // 2 = point pressures      [Base DS_load]
  DS_tag_dist_press,     // 3 = distributed pressure [Base DS_load]
  DS_tag_spring,         // 4 = Spring load          [Base DS_load]
  DS_tag_spring_set,     // 5 = A set of spring loads[Base DS_load]
  DS_tag_crv_load,       // 6 = Curve Spring load    [Base DS_load]
  DS_tag_dyn_load,       // 7 = Dynamic load         [Base DS_load]
  DS_tag_pt_cstrn,       // 8 = point constraint     [Base DS_cstrn]
  DS_tag_pt_seam,        // 9 = seam pt_cstrn        [Base DS_cstrn]
  DS_tag_crv_cstrn,      //10 = curve constraint     [Base DS_cstrn]
  DS_tag_crv_seam,       //11 = seam crv_cstrn       [Base DS_cstrn]
  DS_tag_srf_dmod,       //12 = surface dmod         [Base DS_dmod]
  DS_tag_crv_dmod,       //13 = curve dmod           [Base DS_dmod]
  DS_tag_vector_load,    //14 = vector_load          [Base DS_load]
  DS_tag_attractor,      //15 = attractor load       [Base DS_load]
  DS_tag_link_cstrn,     //16 = multi-surf link cstrn[Base DS_cstrn]
  DS_tag_area_cstrn,     //17 = area cstrn           [Base DS_cstrn]
  DS_tag_link_load,      //18 = link load            [Base DS_cstrn]
  DS_tag_track_pos_crv,  //19 = tracking pos curve dmod  [BASE DS_dmod]
  DS_tag_track_tan_crv,  //20 = tracking tan curve dmod
  DS_tag_track_curv_crv, //21 = tracking curvature curve dmod
  DS_tag_area_load  //22 = area load [Base DS_cstrn]
} ;

// enumeration for specifying relative handedness of two
// surfaces linked together
// NOTE: do **not** change unflipped==0 or flipped =1; these values
//       are implicitly converted to ints within the code

/**
* Specifies relative handedness of two surfaces linked together.
* @param SPA_DM_flip_unknown
* initial value - uses Calc_flipped_coords to decide.
* @param SPA_DM_unflipped
* surfaces have same handedness.
* @param SPA_DM_flipped
* surfaces have opposite handedness.
**/

enum SPA_DM_flipped_state
{SPA_DM_flip_unknown = -1,   // initial value - uses Calc_flipped_coords to decide
 SPA_DM_unflipped    =  0,   // surfaces have same handedness
 SPA_DM_flipped      =  1    // surfaces have opposite handedness
};

// enumeration for specifying what happens to target curve (or point)
// when converting between constraints and loads

/**
* Specifies what happens to target curve (or point) when converting between constraints and loads.
* @param SPA_DM_remember_target
* new behavior has same target.
* @param SPA_DM_forget_target
* target is surface shape at time of conversion.
**/

enum SPA_DM_target_memory
{SPA_DM_remember_target    =  0,   // new behavior has same target
 SPA_DM_forget_target      =  1    // target is surface shape at time of conversion
};

// see convenience functions
// SPA_DM_is_type_tracking_curve()
// SPA_DM_is_type_patch_dmod()


// obsolete tag types
//  DS_tag_tan_pt_cstrn,   //10 = point-tangent cstrn  [Base DS_cstrn]
//  DS_tag_dof_pair,       //11 = dof pair constraint  [Base DS_cstrn]
//  DS_tag_dof_tangent     //12 = dof tangent cstrain  [Base DS_cstrn]

// possible point_index values for point constraints

// exposes values of DS_CST_PT_INDEX as interface integers
// used in SPA_DM_set_pt_xyz
//      in SPA_DM_set_cstrn_value
/**
 * not used - or ill defined
 */
static const int SPA_DM_UNDEF_LEG   = -1 ;         //
/**
 * "position"
 */
static const int SPA_DM_BASE_PT     = 0; 
/**
 *.
 */
static const int SPA_DM_END_LEG     = 1;
/**
 * "tangent"
 */
static const int SPA_DM_TANG_LEG    = 2; 
/**
 *.
 */
static const int SPA_DM_TANG1_LEG   = SPA_DM_TANG_LEG;
/**
 *.
 */
static const int SPA_DM_TANG2_LEG   = 3;
/**
 * "normal"
 */
static const int SPA_DM_NORM_LEG    = 4; 
/**
 * "curvature"
 */
static const int SPA_DM_CURV_LEG    = 5 ;          //
/**
 *.
 */
static const int SPA_DM_CURV1_LEG   = SPA_DM_CURV_LEG ;
/**
 *.
 */
static const int SPA_DM_CURV2_LEG   = 6 ;
/**
 *.
 */
static const int SPA_DM_BINORM_LEG  = 7 ;
/** @} */
#endif /* DMAPI_CONSTS_H */
