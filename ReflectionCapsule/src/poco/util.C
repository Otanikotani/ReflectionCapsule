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

#include "util.h"
#include "pocostr.h"

#include <stdlib.h>
#include <string.h>

char* POCO_Util::class_to_ptr(const char* class_name)
{
	if( class_name == NULL ) {
		return NULL;
	}

	POCO_String trimed_name = trim_string(class_name);

	if( trimed_name.in() == NULL || trimed_name.in()[0] == '\0' ) {
		// empty string
		return NULL;
	}

	//Asterisks in name? Is it like operator* ?
	if( trimed_name.in()[0] == '*' ) {
		// removing "*" from head
		trimed_name = trim_string(trimed_name.in() + 1);

		if( trimed_name.in() == NULL || trimed_name.in()[0] == '\0' ) {
			return NULL;
		}

		int len = strlen(trimed_name.in());
		if( trimed_name.in()[len - 1] == '*' ) {
			// trim possible white space between
			// tailing "*" and class name.
			trimed_name.inout()[len-1] = 0;
			trimed_name = trim_string(trimed_name.in());
			// appending "*" again at tail
			trimed_name += "*";
		}
	}
	else {
		// appending "*" at tail
		trimed_name += "*";
	}

	return trimed_name.retn();
}

char* POCO_Util::trim_string(const char* str)
{
	if( str == NULL ) {
		return NULL;
	}

	int len = strlen(str);
	// trim head space
	int i;
	for(i=0;i<len; i++) {
		if( str[i] == ' ' || str[i] == '\t' ) {
			continue;
		}

		break;
	}

	POCO_String trimed_str = (const char*)(str + i);
	str = trimed_str.in();
	len = strlen(str);

	// trim tail space
	for(;len>0;len--) {
		if( str[len-1] == ' ' || str[len-1] == '\t' ) {
			continue;
		}

		break;
	}

	if( len == 0 ) {
		return NULL;
	}

	trimed_str.inout()[len] = '\0';
	return trimed_str.retn();
}

char* POCO_Util::trim_last_char(const char* s)
{
	if( s == NULL ) {
		return NULL;
	}

	int len = strlen(s);

	// dup the string
	POCO_String svar = s;

	// get the dup'ed buf
	char* str = svar.retn();

	// trim the last char
	str[len - 1] = 0;
	len = len - 1;

	// trim tail spaces
	for(;len>0;len--) {
		if( str[len-1] == ' ' || str[len-1] == '\t' ) {
			continue;
		}

		break;
	}

	if( len == 0 ) {
		return NULL;
	}

	str[len] = '\0';
	return str;
}

#include <string.h>

char* POCO_Util::cvtcstr(const char* cstr)
{
	if( cstr == NULL ) {
		return NULL;
	}

	char* ret = POCO_String::alloc(strlen(cstr)*2);

	int i=0, j = 0;
	for(i=0;i<(int)strlen(cstr);i++) {
		if( cstr[i] != '\\' ) {
			ret[j++] = cstr[i];
			continue;
		}

		switch(cstr[i+1]) {
		case '"': ret[j++]='"'; i++; break;
		case '\\': ret[j++]='\\'; i++; break;
		case 'a': ret[j++]='\a'; i++; break;
		case 'b': ret[j++]='\b'; i++; break;
		case 'f': ret[j++]='\f'; i++; break;
		case 'n': ret[j++]='\n'; i++; break;
		case 'r': ret[j++]='\r'; i++; break;
		case 't': ret[j++]='\t'; i++; break;
		case 'v': ret[j++]='\v'; i++; break;
		default: ret[j++] = '\\'; break;
		}
	}

	ret[j]=0;

	return ret;
}
