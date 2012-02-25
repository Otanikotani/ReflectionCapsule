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

#ifndef _poco_dom_reader_h
# define _poco_dom_reader_h

# include "pocotypes.h"
# include "pocodom.h"
# include "pocostr.h"
# include <stdio.h>

class _POCO_CAPSULE_EXPORT POCO_DOM_Reader : public POCO_AppDescReader {
  public:
	const char*	   tid();
	POCO_DOM::Element* read(const char* file, POCO_AppEnv*);
	POCO_DOM::Element* read(const char* buf, int len, POCO_AppEnv*);
};

#endif
