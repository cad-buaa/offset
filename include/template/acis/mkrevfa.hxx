/* $Id: mkrevfa.hxx,v 1.3 2000/12/26 18:30:52 products Exp $ */
/*******************************************************************/
/*    Copyright (c) 1989-2020 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/

#if !defined(SG_REV_FACE)
#    define SG_REV_FACE

#    include "dcl_cstr.h"

class FACE;

DECL_CSTR FACE* make_revface(FACE*&);

#endif
