#pragma once
/*******************************************************************/
/*    @file     gme_dcl_ofst.h                                     */
/*    @brief    Offsetting模块的模块声明                           */
/*    @details  仿照 dcl_ofst.h                                    */
/*                                                                 */
/*    @author   YangHongYuan                                       */
/*    @date     2024.7.4                                           */
/*******************************************************************/
#ifndef DECL_GME_OFST

#    if defined(__offset) || defined(__SpaACIS) || defined(__SPAAcisDs)
#        define BUILDING_LOCAL_FILE
#    endif

#    include "acis/importexport.h"
#    ifdef IMPORT_EXPORT_SYMBOLS
#        ifdef BUILDING_LOCAL_FILE
#            define DECL_GME_OFST EXPORT_SYMBOL  // 更改为DECL_GME_OFST
#        else
#            define DECL_GME_OFST IMPORT_SYMBOL  // 更改为DECL_GME_OFST
#        endif
#    else
#        define DECL_GME_OFST  // 更改为DECL_GME_OFST
#    endif

/* force link in VC++ */

#    if !defined(CONCAT)
#        define CONCAT2(a, b) a##b
#        define CONCAT(a, b) CONCAT2(a, b)
#    endif

#    ifndef SPA_NO_AUTO_LINK
#        ifndef BUILDING_LOCAL_FILE
#            if defined(_MSC_VER)
#                if(defined(SPA_INTERNAL_BUILD) && !defined(SPAACISDS)) || defined(NOBIGLIB)
#                    define spa_lib_name "offset"
#                else
#                    if defined(SPAACISDS)
#                        define spa_lib_name "SPAAcisDs"
#                    else
#                        define spa_lib_name "SpaACIS"
#                    endif
#                endif
#                if defined(SPA_DEBUG) && !defined(SPA_INTERNAL_BUILD) && !defined(SPAACISDS)
#                    pragma comment(lib, CONCAT(spa_lib_name, "d.lib"))
#                else
#                    pragma comment(lib, CONCAT(spa_lib_name, ".lib"))
#                endif
#            endif
#        endif
#    endif

#    undef BUILDING_LOCAL_FILE
#    undef spa_lib_name

#endif /* DECL_OFST */
