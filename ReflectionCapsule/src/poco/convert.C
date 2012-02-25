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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "convert.h"

int POCO_CVT::str2char(const char* str, char& c)
{
	if( str == NULL ) {
		c = '\0';
		return 1;
	}

	int len = strlen(str);

	if( len == 1 ) {
		c = str[0];
		return 1;
	}

	if( len >= 3
	 && str[0] == '0'
	 && (str[1] == 'x' || str[1] == 'X') ) {
		if( hex2char(str[2], c) == 0 ) {
			return 0;
		}

		if( len > 3 ) {
			char a;
			if( hex2char(str[3], a) == 0 ) {
				return 0;
			}

			c = c*0x10 + a;
		}
	}

	return 1;
}

int POCO_CVT::str2uchar(const char* str, unsigned char& c)
{
	return str2char(str, (char&)c);
}

int POCO_CVT::str2short(const char* str, short& s)
{
	if( str == NULL ) {
		s = 0;
		return 1;
	}

	s = (short)atoi(str);

	return 1;
}

int POCO_CVT::str2ushort(const char* str, unsigned short& s)
{
	s = (unsigned short)atoi(str);

	return 1;
}

int POCO_CVT::str2long(const char* str, long& l)
{
	if( str == NULL ) {
		l = 0L;
		return 1;
	}

	l = atol(str);

	return 1;
}

int POCO_CVT::str2ulong(const char* str, unsigned long& u)
{
	if( str == NULL ) {
		u = 0UL;
		return 1;
	}

	u = (unsigned long)atol(str);

	return 1;
}

int POCO_CVT::str2float(const char* str, float& f)
{
	if( str == NULL ) {
		f = 0.0;
		return 1;
	}

	if( sscanf(str, "%f", &f) == EOF ) {
		return 0;
	}

	return 1;
}

int POCO_CVT::str2double(const char* str, double& x)
{
	if( str == NULL ) {
		x = 0.0;
		return 1;
	}

	if( sscanf(str, "%lf", &x) == EOF ) {
		return 0;
	}

	return 1;
}

int POCO_CVT::hex2char(char h, char& c)
{
	switch(h) {
		case '0': c = 0x00; break;
		case '1': c = 0x01; break;
		case '2': c = 0x02; break;
		case '3': c = 0x03; break;
		case '4': c = 0x04; break;
		case '5': c = 0x05; break;
		case '6': c = 0x06; break;
		case '7': c = 0x07; break;
		case '8': c = 0x08; break;
		case '9': c = 0x09; break;
		case 'a': case 'A': c = 0x0a; break;
		case 'b': case 'B': c = 0x0b; break;
		case 'c': case 'C': c = 0x0c; break;
		case 'd': case 'D': c = 0x0d; break;
		case 'e': case 'E': c = 0x0e; break;
		case 'f': case 'F': c = 0x0f; break;
		default: return 0;
	}

	return 1;
}
