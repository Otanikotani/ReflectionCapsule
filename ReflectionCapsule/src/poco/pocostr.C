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

#include "pocostr.h"
#include <string.h>
#include <stdlib.h>

#include <stdio.h>

POCO_String::POCO_String()
{
	_str = NULL;
}

POCO_String::POCO_String(char* str)
{
	_str = str;
}

POCO_String::POCO_String(const char* str)
{
	_str = POCO_String::dup(str);
}

POCO_String::POCO_String(const char** strs)
{
	_str = NULL;

	for(int i=0; strs!=NULL && strs[i]!=NULL; i++) {
		if( i==0 ) {
			_str = POCO_String::dup(strs[0]);
		}
		else {
			*this += strs[i];
		}
	}
}

POCO_String::~POCO_String()
{
	if( _str != NULL ) {
		POCO_String::free(_str);
	}
}

char* POCO_String::retn()
{
	char* ret = _str;
	_str = NULL;

	return ret;
}

POCO_String& POCO_String::operator=(char* str)
{
	if( str == _str ) {
		return *this;
	}

	if( _str != NULL ) {
		POCO_String::free(_str);
	}

	_str = str;

	return *this;
}

POCO_String& POCO_String::operator=(const char* str)
{
	if( str == _str ) {
		return *this;
	}

	if( _str != NULL ) {
		POCO_String::free(_str);
	}

	_str = POCO_String::dup(str);

	return *this;
}

POCO_String& POCO_String::operator+=(const char* str)
{
	if( str == NULL ) {
		return *this;
	}

	if( _str == NULL ) {
		_str = POCO_String::dup(str);
	}
	else {
		char* buf = new char[strlen(_str) + strlen(str) + 1];
		strcpy(buf, _str);
		strcat(buf,  str);

		POCO_String::free(_str);
		_str = buf;
	}

	return *this;
}

/*
POCO_String& POCO_String::operator+=(const char* str)
{
	return (*this) + str;
}
*/

char* POCO_String::alloc(int len)
{
	return new char[len];
}

char* POCO_String::dup(const char* str)
{
	if( str == NULL ) {
		return NULL;
	}

	char* ret = POCO_String::alloc(strlen(str) + 1);
	strcpy(ret, str);
	return ret;
}

void POCO_String::free(char* str)
{
	if( str != NULL ) {
		delete[] str;
	}
}

void POCO_String::replace(char old_char, char new_char)
{
	if( _str == NULL ) {
		return;
	}

	int len = strlen(_str);

	for(int i=0; i<len; i++) {
		if( _str[i] == old_char ) {
			_str[i] = new_char;
		}
	}

	return;
}
