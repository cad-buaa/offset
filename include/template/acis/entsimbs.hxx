/*******************************************************************/
/*    Copyright (c) 1989-2020 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/

#if !defined (ATTRIB_HH_ENT_SIMPLIFY_BASE_CLASS)
#define ATTRIB_HH_ENT_SIMPLIFY_BASE_CLASS
#include "dcl_heal.h"
#include "att_hh.hxx"
#include "at_enty_stitch.hxx"
/**
 * @file entsimbs.hxx
 * @CAA2Level L1
 * @CAA2Usage U1
 * \addtogroup HEALAPIDEP
 *
 * @{
 */
/**
 * @nodoc
 */
ENTITY_IS_PROTOTYPE(ATTRIB_HH_ENT_SIMPLIFY_BASE, HEAL)
#if 0
; // semicolon needed for mkman (doc tool) parsing
#endif


DECL_HEAL extern int ATTRIB_HH_ENT_SIMPLIFY_BASE_TYPE;
/**
 * @nodoc
 */
#define ATTRIB_HH_ENT_SIMPLIFY_BASE_LEVEL (ATTRIB_HH_ENT_LEVEL + 1)

class ATTRIB_HH_AGGR_SIMPLIFY_BASE;
/**
 * @nodoc
 * <b>Obsolete:</b> This class has been deprecated. For healing, use API interfaces
 * and not Direct Interface.
 * <br><br>
 * Base HEAL individual entity-level attribute class for the geometry simplification phase.
 * <br>
 * <b>Role:</b> <tt>ATTRIB_HH_ENT_SIMPLIFY_BASE</tt> is the base geometry simplification individual
 * entity-level attribute class from which other HEAL individual entity-level attribute classes
 * used in the geometry simplification phase are derived. Individual entity-level attributes are
 * attached to the individual entities of body being healed to store entity-specific information
 * about each phase or subphase of the healing process. The individual entity-level attributes for
 * each phase are managed by the aggregate attribute for that phase.
 */
class DECL_HEAL ATTRIB_HH_ENT_SIMPLIFY_BASE : public ATTRIB_HH_ENT
{
public:
 		// Constructor
/**
 * C++ initialize constructor requests memory for this object and populates it with the data supplied as arguments.
 * <br><br>
 * <b>Role:</b> Applications should call this constructor only with the overloaded new operator
 * inherited from the <tt>ENTITY</tt> class (for example, <tt>x=new ATTRIB_HH_ENT_SIMPLIFY_BASE(...)</tt>), because
 * this reserves the memory on the heap, a requirement to support roll back and history management.
 * <br><br>
 * @param ent
 * owning entity.
 */
		ATTRIB_HH_ENT_SIMPLIFY_BASE(ENTITY* ent = NULL);
/**
 * Returns <tt>TRUE</tt> if this is pattern compatible.
 */
        virtual logical pattern_compatible() const;
                                    // returns TRUE
		/**
		 * @nodoc
		 */
		ATTRIB_FUNCTIONS (ATTRIB_HH_ENT_SIMPLIFY_BASE, HEAL)
#if 0
; // semicolon needed for mkman (doc tool) parsing)
#endif
// Because mkman isn't aware that ATTRIB_FUNCTIONS changes access to "public", we do so
// explicitly here:
public:

};
/** @} */
#endif // ATTRIB_HH_ENT_SIMPLIFY_BASE_CLASS