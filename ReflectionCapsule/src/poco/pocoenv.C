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

#include "pocoenv.h"
#include "pocoapp.h"

POCO_AppEnv::POCO_AppEnv()
{
	_impl = new POCO_AppEnvImpl;
	reset();
}

POCO_AppEnv::POCO_AppEnv(int argc, const char** argv)
{
	_impl = new POCO_AppEnvImpl;
	reset();

	setArray("pococapsule.init.argv", argc, (const char**)argv);
}

//
// external public methods
//
void POCO_AppEnv::fatal_error(const char* the_msg)
{
	_impl->m_flag = 1;

	if( _impl->m_error.in() != NULL ) {
		_impl->m_error += "\n";
	}

	_impl->m_error += the_msg;
}

int POCO_AppEnv::has_error()
{
	return _impl->m_flag;
}

void POCO_AppEnv::warning(const char* the_msg)
{
	if( _impl->m_warning.in() != NULL ) {
		_impl->m_warning += "\n";
	}
	_impl->m_warning += the_msg;
}

const char* POCO_AppEnv::get_message()
{
	if( _impl->m_error.in() != NULL ) {
		return _impl->m_error.in();
	}
	else {
		return _impl->m_warning.in();
	}
}

void POCO_AppEnv::reset()
{
	_impl->m_flag = 0;
	_impl->m_error = (const char*)NULL;
	_impl->m_warning = (const char*)NULL;
}

void POCO_AppEnv::setValue(const char* name, const char* value)
{
	if( name == NULL ) {
		return;
	}

	POCO_String::free((char*)(_impl->m_vars.remove(name)));

	if( value == NULL ) {
		return;
	}

	_impl->m_vars.put(name, (void*)POCO_String::dup(value));
}

const char* POCO_AppEnv::getValue(const char* name)
{
	return (const char*)(_impl->m_vars.get(name));
}

typedef char*	ArgType;

static void release_argv(ArgType* argv)
{
	if( argv == NULL ) {
		return;
	}

	for(int i=0;argv[i]!=NULL;i++) {
		POCO_String::free(argv[i]);
	}

	delete [] argv;
}

void POCO_AppEnv::setArray(const char* name, int argc, const char** argv)
{
	if( name == NULL ) {
		return;
	}

	ArgType* copyed_argv = new ArgType[argc+1];
	for(int i=0;i<argc;i++) {
		copyed_argv[i] = POCO_String::dup(argv[i]);
	}
	copyed_argv[argc] = NULL;

	release_argv((ArgType*)(_impl->m_arrs.remove(name)));
	_impl->m_arrs.put(name, (void*)copyed_argv);
}

const char** POCO_AppEnv::getArray(const char* name)
{
	return (const char**)(_impl->m_arrs.get(name));
}

POCO_AppEnv::~POCO_AppEnv()
{
	const char** keys = _impl->m_arrs.allKeys();
	int cnt = _impl->m_arrs.count();
	int i;

	for(i=0;i<cnt;i++) {
		release_argv((ArgType*)(_impl->m_arrs.remove(keys[i])));
	}

	keys = _impl->m_vars.allKeys();
	cnt = _impl->m_vars.count();

	for(i=0;i<cnt;i++) {
		POCO_String::free((char*)(_impl->m_vars.remove(keys[i])));
	}

	delete [] keys;
}

//
// used internally by the capsule
//
void POCO_AppEnvImpl::fatal_error(POCO_AppEnv* env, const char* the_msg, POCO_DOM::Element* elem)
{
	const char* label = NULL;
	const char* id = NULL;
	POCO_DOM::Element* node;

	POCO_String msg = the_msg;

	for(node=elem;node!=NULL;node=node->getParent()) {
		id = node->getAttribute("id");
		label = node->getAttribute("label");

		if( id != NULL || label != NULL ) {
			break;
		}
	}

	if( id != NULL ) {
		if( node != NULL && node != elem ) {
			msg += " at the <";
			msg += elem->getTagName();
			msg += " ...> under the <";
			msg += node->getTagName();
			msg += " ... id=\"";
			msg += id;
			msg += "\" ...>";
		}
		else {
			msg += " at the <";
			msg += elem->getTagName();
			msg += " ... id=\"";
			msg += id;
			msg += "\" ...>";
		}
	}
	else
	if( label != NULL ) {
		if( node != NULL && node != elem ) {
			msg += " at the <";
			msg += elem->getTagName();
			msg += " ...> under the <";
			msg += node->getTagName();
			msg += " ... label=\"";
			msg += label;
			msg += "\" ...>";
		}
		else {
			msg += " at the <";
			msg += elem->getTagName();
			msg += " ... label=\"";
			msg += label;
			msg += "\" ...>";
		}
	}
	else {
		msg += " at the <";
		msg += elem->getTagName();
		msg += " ...>";
	}

	env->fatal_error(msg.in());
}

void POCO_AppEnvImpl::fatal_error(POCO_AppEnv* env, const char* msg)
{
	env->fatal_error(msg);
}
