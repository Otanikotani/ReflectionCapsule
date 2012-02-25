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

#ifndef _application_context_h_
#define _application_context_h_

#include "pocodom.h"
#include "pocoenv.h"
#include "descs.h"

#include "hashtable.h"
#include "table.h"

class POCO_AppContextImpl : public POCO_AppContext, public POCO_BeanScopeContext {
	POCO_String		id;
	int			lazy_resolve;
	int			is_closed;

	POCO_AppContextImpl*	parent;

	POCO_Table		imported_ctxts;

	POCO_Hashtable		bean_id_map;
	POCO_Table		singleton_beans;
	POCO_Table		all_beans;
	POCO_Table		bean_term_stack;
	POCO_Hashtable		keyed_beans;

	int  resolve_all_bean_ids(POCO_DOM::Element*, POCO_AppEnv*);
	int  parseMethodArgs(POCO_DOM::Element*, POCO_BeanDesc*, POCO_BeanDesc*, POCO_AppEnv*);
	int  parsePropSetters(POCO_DOM::Element*, POCO_BeanDesc*, POCO_AppEnv*);
	POCO_PropDesc* parseOnError(POCO_DOM::Element*, POCO_BeanDesc*, POCO_AppEnv*);
	POCO_PropDesc* parsePropSetter(POCO_DOM::Element*, POCO_BeanDesc*, POCO_AppEnv*);
	int  parseDupMethod(POCO_DOM::Element*, POCO_BeanDesc*, POCO_AppEnv*);
	int  parseReleaseMethod(POCO_DOM::Element*, POCO_BeanDesc*, POCO_AppEnv*);
	POCO_ArgDesc*  parseArg(POCO_DOM::Element*, POCO_BeanDesc*, POCO_AppEnv*);
	POCO_ValDesc*  parseBeanRef(POCO_DOM::Element*, const char*, POCO_BeanDesc*, POCO_AppEnv*);
	POCO_ValDesc*  parseValue(POCO_DOM::Element*, const char*, POCO_AppEnv*);
	POCO_ValDesc* getAndConvertValue(const char* attr, POCO_DOM::Element* elem,
				const char* type, POCO_AppEnv* env);


  public: // protected:
	friend class POCO_BeanDesc;
	void add_bean(POCO_BeanDesc*);
	void push_inited_singleton(POCO_BeanDesc*);
	void add_keyed_bean(POCO_BeanDesc*);
	POCO_BeanDesc* get_keyed_bean(const char* key);

	POCO_AppContextImpl();
	~POCO_AppContextImpl();
	int  parseDoc(POCO_DOM::Element* elem, POCO_AppEnv* env);
	int  parseImport(POCO_DOM::Element* elem, POCO_AppEnv* env);
	int  parseLoadLib(POCO_DOM::Element* elem, POCO_AppEnv* env);
	POCO_BeanDesc* parseBean(POCO_DOM::Element* elem, POCO_AppEnv* env);

	POCO_BeanDesc*	getBeanDesc(const char* id, POCO_AppEnv* env);

  public:
	void*  getBean(const char* id, POCO_AppEnv*);
	void*  getBean(const char* id, const char* class_name, POCO_AppEnv*);
	const char*  getBeanPtrTypeId(const char* id, POCO_AppEnv*);

	int initSingletons(POCO_AppEnv*);
	int terminate(POCO_AppEnv*);

	void destroy();

	static POCO_AppContextImpl* create_from_file(
			const char* locator,	// xml or desc file name
			POCO_AppEnv* env = NULL);

	static POCO_AppContextImpl* create_from_string(
			const char* buf,	// memory buffer
			POCO_AppEnv* env = NULL);

};

#endif
