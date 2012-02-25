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

#include "pocoapp.h"
#include "appctxtimp.h"
#include "pocoenv.h"
#include <string.h>

POCO_AppContext*
POCO_AppContext::create(const char* desc, const char* type, POCO_AppEnv* env)
{
	if( env == NULL ) {
		env = getDefaultAppEnv();
	}

	if( strcmp(type, "file") == 0 ) {
		return POCO_AppContextImpl::create_from_file(desc, env);
	}
	else 
	if( strcmp(type, "string") == 0 ) {
		return POCO_AppContextImpl::create_from_string(desc, env);
	}

	POCO_String msg = (const char*)"[E101] Unknow descriptor source type: \"";
	msg += type;
	msg += "\"";
	env->fatal_error(msg.in());

	return NULL;
}

static POCO_AppEnv* _default_app_env = POCO_AppEnv::staticInit();

POCO_AppEnv*
POCO_AppEnv::staticInit()
{
    static POCO_AppEnv env;
    return &env;
}

POCO_AppEnv*
POCO_AppContext::getDefaultAppEnv()
{
	return _default_app_env;
}

POCO_AppEnv*
POCO_AppContext::setDefaultAppEnv(POCO_AppEnv* env)
{
	if( env == NULL ) {
		return _default_app_env;
	}

	POCO_AppEnv* old = _default_app_env;
	_default_app_env = env;
	return old;
}
POCO_AppEnv*
POCO_AppContext::initDefaultAppEnv(int argc, char** argv)
{
	POCO_AppEnv* env = getDefaultAppEnv();
	env->setArray("pococapsule.init.argv", argc, (const char**)argv);
	return env;
}
