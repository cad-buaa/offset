﻿// $Id: blend_smoke_test.hxx,v 1.4 2002/01/28 16:53:03 rocon Exp $
/*******************************************************************/
/*    Copyright (c) 1989-2020 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/

#include "ACIS_Win.h"
#include "blend_examples.hxx"
#include "bool_examples.hxx"
#include "constructors_examples.hxx"
#include "incr_examples.hxx"
#include "intcur.h"
#include "lop_examples1.hxx"
#include "mt_examples.hxx"
#include "ofst_examples1.hxx"
#include "resource.h"
#include "skin_examples.hxx"
#include "stdafx.h"
#include "stitch_examples.hxx"
#include "sweep_examples.hxx"
#include "template_examples.hxx"
#include "test.h"

// This function converts a Windows menu resource ID into an aei function pointer.
ACIS_fp find_which_ACIS(WORD id) {
    switch(id) {
        case ID_BOOLEAN_UNITE:
            return aei_BOOL_UNITE;
        case ID_BOOLEAN_INTERSECTION:
            return aei_BOOL_INTERSECTION;
        case ID_BOOLEAN_SUBTRACTION:
            return aei_BOOL_SUBTRACTION;
        case ID_BOOLEAN_CHOP:
            return aei_BOOL_CHOP;
        case ID_BOOLEAN_PROJECTWIRETOBODY:
            return aei_BOOL_PROJECT_WIRE_TO_BODY;
        case ID_BOOLEAN_PROJECTEDGETOFACE:
            return aei_BOOL_PROJECT_EDGE_TO_FACE;
        case ID_BOOLEAN_MAKESHADOW:
            return aei_BOOL_MAKE_SHADOW;
        case ID_BOOLEAN_FACESUBDIVIDE:
            return aei_BOOL_SUBDIVIDE_FACE;
        case ID_BOOLEAN_SPLITWIREEDGE:
            return aei_BOOL_SPLIT_WIRE_EDGE;
        case ID_SWEEP_SWEEP_PIPE_THRU:
            return aei_SWEEP_PIPE_THROUGH_E_BLOCK;
        case ID_SWEEP_SWEEP_WEDGE:
            return aei_SWEEP_WEDGE;
        case ID_SWEEP_SWEEP_DRAFTEDCONE1:
            return aei_SWEEP_DRAFTED_CONE1;
        case ID_SWEEP_SWEEP_DRAFTEDBOXWITHHOLE:
            return aei_SWEEP_DRAFTED_BOX_WITH_HOLE;
        case ID_SWEEP_SWEEP_DEFORMEDTUBING:
            return aei_SWEEP_DEFORMED_TUBING;
        case ID_SWEEP_SWEEP_DEGENDRAFTBOX:
            return aei_SWEEP_DEGEN_DRAFT_BOX;
        case ID_SWEEP_SWEEP_PIPETHRUSLOTTEDBLOCK:
            return aei_SWEEP_PIPE_THROUGH_SLOTTED_BLOCK;
        case ID_SWEEP_SWEEP_DRAFTEDBOXCYL:
            return aei_SWEEP_DRAFTED_BOX_CYL;
        case ID_SWEEP_SWEEP_TORUSTHRUEBLOCK:
            return aei_SWEEP_TORUS_THROUGH_E_BLOCK;
        case ID_SWEEP_SWEEP_HEART:
            return aei_SWEEP_HEART;
        case ID_SWEEP_SWEEP_APPLEPEAL:
            return aei_SWEEP_APPLE_PEEL;
        case ID_SWEEP_SWEEP_RIGIDTUBE1:
            return aei_SWEEP_RIGID_TUBE1;
        case ID_SWEEP_SWEEP_UFO1:
            return aei_SWEEP_UFO1;
        case ID_SWEEP_SWEEPTWISTEDTUBE:
            return aei_SWEEP_TWISTED_TUBE;
        case ID_SWEEP_SWEEP_RIGIDTUBE2:
            return aei_SWEEP_RIGID_TUBE2;
        case ID_SWEEP_SWEEPSHAFT:
            return aei_SWEEP_SHAFT;
        case ID_SWEEP_SWEEPRIGIDPIPE:
            return aei_SWEEP_RIGID_PIPE;
        case ID_SWEEP_SWEEPRIGIDPIPESHAFT:
            return aei_SWEEP_RIGID_PIPE_SHAFT;
        case ID_SWEEP_SWEEPFIN:
            return aei_SWEEP_FIN;
        case ID_SWEEP_SWEEPABSTRACTART:
            return aei_SWEEP_ABSTRACT_ART;
        case ID_SWEEP_SWEEPDRAFTEDTUBE:
            return aei_SWEEP_DRAFTED_TUBE;
        case ID_SWEEP_SWEEP_DRAFTEDPOLYGON1:
            return aei_SWEEP_DRAFTED_POLYGON1;
        case ID_SWEEP_SWEEPDRAFTEDPOLYGON2:
            return aei_SWEEP_DRAFTED_POLYGON2;
        case ID_SWEEP_SWEEPARCH:
            return aei_SWEEP_ARCH;
        case ID_SWEEP_SWEEPDRAFTEDBOX1:
            return aei_SWEEP_DRAFTED_BOX1;
        case ID_SWEEP_SWEEPDRAFTEDCONE2:
            return aei_SWEEP_DRAFTED_CONE2;
        case ID_SWEEP_SWEEPANGLEDPOLYGON:
            return aei_SWEEP_ANGLED_POLYGON;
        case ID_SWEEP_SWEEPDRAFTEDBOX2:
            return aei_SWEEP_DRAFTED_BOX2;
        case ID_SWEEP_SWEEPUFO2:
            return aei_SWEEP_UFO2;
        case ID_SKIN_SKIN_YO_YO:
            return aei_SKIN_YO_YO;
        case ID_MULTI_SLICE:
            return aei_MT_EGG_SLICE;
        case ID_PROCESS_MT_API:
            return aei_PROCESS_MT;
        case ID_MT_WORK_PACKET_QUEUE:
            return aei_MT_WORK_PACKET_QUEUE;
        case ID_STITCH_MTSTITCH:
            return aei_STITCH_FULL;
        case ID_CONSTRUCTORS_TEXT_HELLO:
            return aei_TEXT_FROM_WIRE_HELLO;
        case ID_HLC_INCR_BOOL:
            return aei_INCR_BOOL;

            // Blending
        case ID_WIGGLE_CONSTRADIUS:
            return aei_BLEND_WIGGLE_CONST;
        case ID_WIGGLE_VARIABLERADIUS:
            return aei_BLEND_WIGGLE_VAR;
        case ID_BLENDING_HOLDLINE:
            return aei_BLEND_HOLDLINE;
        case ID_BLENDING_ELLIPTICAL:
            return aei_BLEND_ELLIPTICAL;
        case ID_BLEND_BLEND_RADIUS_TRANSITION:
            return aei_BLEND_RADIUS_TRANSITION1;
        case ID_BLENDING_GRIP:
            return aei_BLEND_GRIP;
        case ID_BLENDING_PENCIL:
            return aei_BLEND_PENCIL;
        case ID_BLENDING_SPLINECHAMFER:
            return aei_BLEND_SPLINE_CHAMFER;
        case ID_BLENDING_TWISTEDBAR:
            return aei_BLEND_TWISTED_BAR;
        case ID_BLENDING_WINEGLASS:
            return aei_BLEND_WINE_GLASS;
        case ID_BLENDING_NUMBERNINE:
            return aei_BLEND_NUMBER_NINE;
        case ID_BLENDING_ANNOTATIONS:
            return aei_BLEND_ANNOTATION;
        case ID_BLENDING_DICE:
            return aei_BLEND_DICE;
        case ID_BLENDING_BRITISHDICE:
            return aei_BLEND_BRITISH_DICE;
        case ID_BLENDING_BLENDMITREANDCAP:
            return aei_BLEND_MITRE_AND_CAP;
        case ID_BLENDING_CLOWNHAT:
            return aei_BLEND_CLOWN_HAT;

            // Skinning
        case ID_SKIN_STARSEED1:
            return aei_SKIN_STAR_SEED1;
        case ID_SKIN_STARSEED2:
            return aei_SKIN_STAR_SEED2;
        case ID_SKIN_STARTDRUM:
            return aei_SKIN_STAR_DRUM;
        case ID_SKIN_SEEDPOD:
            return aei_SKIN_SEED_POD;
        case ID_SKIN_TOP:
            return aei_SKIN_TOP;
        case ID_SKIN_BOTTLE:
            return aei_SKIN_BOTTLE;
        case ID_SKIN_VASE:
            return aei_SKIN_VASE;
        case ID_SKIN_WORM:
            return aei_SKIN_WORM;
        case ID_SKIN_NOSESTAR:
            return aei_SKIN_NOSE_STAR;
        case ID_SKIN_CURVEDSTARBO:
            return aei_SKIN_CURVED_STAR_BODY;
        case ID_SKIN_BADSTIRRUP:
            return aei_SKIN_BAD_STIRRUP;
        case ID_SKIN_GOODSTIRRUP:
            return aei_SKIN_GOOD_STIRRUP;
        case ID_SKIN_LENSEBODY:
            return aei_SKIN_LENSE_BODY;
        case ID_SKIN_BALLOON:
            return aei_SKIN_BALLOON;
        case ID_SKIN_TWISTEDSTARSEED:
            return aei_SKIN_TWISTED_STAR_SEED;
        case ID_SKIN_BALOONONCYLINDRICALFACE1:
            return aei_SKIN_BALLOON_ON_CYLINDRICAL_FACE1;
        case ID_SKIN_BALOONONCYLINDRICALFACE2:
            return aei_SKIN_BALLOON_ON_CYLINDRICAL_FACE2;
        case ID_SKIN_ROBOTTOOTH:
            return aei_SKIN_ROBOT_TOOTH;
        case ID_SKIN_BADSQUAREEGG:
            return aei_SKIN_BAD_SQUARE_EGG;
        case ID_SKIN_GOODSQUAREEGG:
            return aei_SKIN_GOOD_SQUARE_EGG;
        case ID_SKIN_DENTEDDOUGHNUT:
            return aei_SKIN_DENTED_DOUGHNUT;
        case ID_SKIN_FLOWER:
            return aei_SKIN_FLOWER;

            // Lop
        case ID_LOP_TAPER_FACES1:
            return aei_LOP_EDGE_TAPER_FACES1;
        case ID_LOP_TAPER_FACES2:
            return aei_LOP_EDGE_TAPER_FACES2;
        case ID_LOP_TAPER_FACES3:
            return aei_LOP_PLANE_TAPER_FACES;
        case ID_MOVE_MOVEFACES:
            return aei_LOP_MOVE_FACES_TRANSLATE;
        case ID_MOVE_ROTATEFACES:
            return aei_LOP_MOVE_FACES_ROTATE;
        case ID_OFFSET_OFFSETFACES:
            return aei_LOP_OFFSET_FACES;
        case ID_OFFSET_OFFSETBODY1:
            return aei_LOP_OFFSET_BODY_NEGOFST;

        case ID_CUBOID_DEMO:
            return aei_CUBOID_DEMO;

        // OFST
        case ID_OFST_FACE1:
            return aei_OFFSET_FACE_1;
        case ID_OFST_FACE2:
            return aei_OFFSET_FACE_2;
        case ID_OFST_FACE3:
            return aei_OFFSET_FACE_3;
        case ID_OFST_FACE4:
            return aei_OFFSET_FACE_4;
        case ID_OFST_FACE5:
            return aei_OFFSET_FACE_5;
        case ID_OFST_FACE6:
            return aei_OFFSET_FACE_6;

        case ID_OFST_WIRE1:
            return aei_OFFSET_WIRE_1;

        case IDM_TEST:
            return aei_TEST_1;

        case INT_CUR:
            return aei_INTCUR_1;

        default:
            return nullptr;
    }
}
