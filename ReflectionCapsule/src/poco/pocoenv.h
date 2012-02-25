/****************************************************************************/
/*                                                                          */
/*  Copyright 2006,2009 by Pocomatic Software, LLC. All Rights Reserved.    */
/*                                                                          */
/*  This program is free software: you can redistribute it and/or modify    */
/*  it under the terms of the GNU Lesser General Public License (LGPL) as   */
/*  published by the Free Software Foundation                               */
/*                                                                          */
/*  This program is distributed in the hope that it will be useful,         */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/*  GNU Lesser General Public License for more details.                     */
/*                                                                          */
/*  You should have received a copy of the GNU Lesser General Public        */
/*  License along with this program. If not, see                            */
/*  <http://www.gnu.org/licenses/lgpl.html>.                                */
/*                                                                          */
/*  Author: Ke Jin <kejin@pocomatic.com>				    */
/*									    */
/****************************************************************************/

#ifndef _poco_env_h_
#define _poco_env_h_

#include <stdlib.h>
#include "pocodom.h"
#include "pocostr.h"
#include "pocoapp.h"
#include "hashtable.h"

//
// The env used internally. it delegate the external AppEnv.
// The purpose is to hide the POCO_DOM from application developers.
//
struct POCO_AppEnvImpl {
	int		m_flag;
	POCO_String	m_error;
	POCO_String	m_warning;

	POCO_Hashtable	m_vars;
	POCO_Hashtable	m_arrs;

	static void fatal_error(POCO_AppEnv*, const char* msg, POCO_DOM::Element* elem);
	static void fatal_error(POCO_AppEnv*, const char* msg);
};

#endif
