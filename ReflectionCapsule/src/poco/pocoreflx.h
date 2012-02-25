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

#ifndef _poco_reflection_h_
# define _poco_reflection_h_

# include "poco.h"
# include "pocotypes.h"

class _POCO_CAPSULE_EXPORT POCO_Reflection {
  public:
	static POCO_proxy_t get_method(
			const char* ret_class_ptr,
			const char* this_class_ptr,
			const char* signature);
	static POCO_proxy_t get_typecast_method(
			const char* to_class_ptr,
			const char* from_class_ptr);
};

#endif
