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

#include "table.h"
#include <stdlib.h>

POCO_Table::POCO_Table()
{
	_size = 8;
	_cnt = 0;
	_array = new void*[_size];

	for(unsigned long i=0; i<_size; i++) {
		_array[i] = NULL;
	}
}

POCO_Table::~POCO_Table()
{
	delete [] _array;
}

void POCO_Table::put(void* value)
{
	if( _cnt == _size ) {
		void** new_buf = new void*[2*_size];
		for(unsigned long i=0;i<_size;i++) {
			new_buf[i] = _array[i];
		}
		_size = 2*_size;
		delete [] _array;
		_array = new_buf;
	}

	_array[_cnt] = value;
	_cnt++;
}

void* POCO_Table::get(int i)
{
	if( i >= (int)_cnt ) {
		return NULL;
	}

	return _array[i];
}

void** POCO_Table::get()
{
	if( _cnt == 0 ) {
		return NULL;
	}

	return _array;
}

void* POCO_Table::remove(int i)
{
	if( i >= (int)_cnt ) {
		return NULL;
	}

	_cnt--;
	void* ret = _array[i];

	if( i != (int)_cnt ) {
		// not the last item
		_array[i] = _array[_cnt];
	}

	return ret;
}

int POCO_Table::count()
{
	return _cnt;
}
