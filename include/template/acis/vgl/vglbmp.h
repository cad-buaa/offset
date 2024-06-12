/*********************************************************************
 *                                                                   *
 *                          DevTools 1.8.0                           *
 *                                                                   *
 *  These coded instructions, statements and computer programs       *
 *  contain unpublished proprietary information of Tech Soft 3D,     *
 *  and are protected by Federal copyright law.  They may not be     *
 *  disclosed to third parties or copied or duplicated in any form,  *
 *  in whole or in part, without the prior written consent of        *
 *  Tech Soft 3D.                                                    *
 *                                                                   *
 *                 Copyright (C) 2023, Tech Soft 3D                  *
 *                                                                   *
 *********************************************************************/
/* local functions header file */
#ifndef VGLBMP_DEF
#define VGLBMP_DEF

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int
vgl_bmpFileToMem(FILE*, unsigned int*);
extern int
vgl_bmpSizeOfImage(FILE*, int*, int*);
extern void
vgl_bmpMemToFile(FILE*, unsigned int*, int, int, int);
extern void
vgl_bmpMemToMem(unsigned int*, int, int, int, unsigned char**, int*);

#ifdef __cplusplus
}
#endif

#endif
