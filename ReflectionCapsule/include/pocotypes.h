/****************************************************************************/
/*									    */
/*  Copyright 2006, by Pocomatic Software, LLC. All Rights Reserved.	    */
/*									    */
/****************************************************************************/
#ifndef _pocotypes_h_
# define _pocotypes_h_

//
// This header file is for export/import capsule 
// classes/functions on windows. 
//

typedef unsigned long POCO_ULong;

#if defined(WIN32)
#  if defined(BUILD_POCOCAPSULE_DLL)
#    define _POCO_CAPSULE_EXPORT __declspec(dllexport)
#  else
#    define _POCO_CAPSULE_EXPORT __declspec(dllimport)
#  endif
#else
#  define _POCO_CAPSULE_EXPORT
#endif


#endif
