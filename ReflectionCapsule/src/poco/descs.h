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

#ifndef _all_desc_h_
#define _all_desc_h_

#include "poco.h"
#include "pocostr.h"
#include "pocoenv.h"

class POCO_BeanDesc;
class POCO_ArgDesc;
class POCO_ValDesc;

typedef POCO_BeanDesc POCO_PropDesc;

//
// describe basic non-aggragated data
//
class POCO_ValDesc {
	int	uid;
	friend class POCO_AppContextImpl;
	friend class POCO_ArgDesc;

	union {
		char		char_val;
		unsigned char	uchar_val;
		unsigned short	ushort_val;
		short		short_val;
		long		long_val;
		unsigned long	ulong_val;
		float		float_val;
		double		double_val;
	};
	POCO_String		string_val;
	POCO_String		name; // for name value pair

	// val_addr points to above _val. if val_addr is NULL,
	// it indicates a bean value.
	void*			val_addr;

	// if non-null, the bean should be casted.
	POCO_proxy_t		cast_method;
	POCO_BeanDesc*		bean;

	POCO_ValDesc();
	~POCO_ValDesc();

  public:
	int   post_invoke(POCO_BeanDesc*, POCO_AppEnv*);
	void* get_value(int dup, POCO_BeanDesc* this_bean, void* this_addr, POCO_AppEnv* env);
	void* get_name();
};

class POCO_BeanScopeContext {
  public:
	virtual ~POCO_BeanScopeContext() { _trace = 0; }
	virtual void add_bean(POCO_BeanDesc*) = 0;
	virtual void push_inited_singleton(POCO_BeanDesc*) = 0;
	virtual void add_keyed_bean(POCO_BeanDesc*) = 0;
	virtual POCO_BeanDesc* get_keyed_bean(const char*) = 0;
	int	trace() { return _trace; }
  protected:
	int	_trace;
};

//
// argument of bean instantiation and ioc methods.
//
class POCO_ArgDesc {
	int		uid;
	friend class POCO_AppContextImpl;
	friend class POCO_BeanDesc;

	// arity info used to resolve factory
	int		is_bean;
	int		is_array;
	int		is_index_arg;
	int		pass_syntax;

	enum { PASS_DEFAULT, PASS_DUP, PASS_DEREF };

	POCO_String	type;
	POCO_String	class_ptr;

	// for array
	int		valdesc_array_size;
	POCO_ValDesc**	valdesc_array;

	// for non-array
	POCO_ValDesc*	valdesc;

	POCO_ArgDesc();
	~POCO_ArgDesc();

	POCO_String	signature;
	const char*	get_signature();

  public:
	int		post_invoke(POCO_BeanDesc*, POCO_AppEnv*);
	int		is_namevalue();

	int		pass_dup();
	int		pass_deref();
};

//
// beans or iocs
//
class POCO_BeanDesc {
	friend class POCO_AppContextImpl;
	friend class POCO_ValDesc;

  public:
	int		uid;
	POCO_String	id;

  private:
	//
	// info
	//
	int		is_parsed;
	int		is_singleton;
	int		is_abstract;
	int		is_lazy_init;
	int		is_initing;		// to prevent circulation
	int		is_inited;		// for singleton
	int		is_property;
	int		is_no_target;
	int		is_public;

	POCO_BeanDesc*	depended_bean;
	POCO_BeanDesc*	factory_bean;
	POCO_BeanDesc*	this_bean;

	POCO_String	bean_class_ptr;
	POCO_String	bean_class;
	void*		bean_addr;		// for singleton

	POCO_String	factory_method_name;
	POCO_proxy_t	factory_method;

	int		num_args;
	POCO_ArgDesc**	args;

	// exception handler ioc
	POCO_PropDesc*	exh_ioc;

	// properties
	int		num_props;
	POCO_PropDesc** props;

	POCO_String	dup_method_name;
	POCO_proxy_t	dup_method;
	int		is_self_dup;
	POCO_String	destroy_method_name;
	POCO_proxy_t	destroy_method;

	POCO_BeanDesc*	destroy_bean;

	POCO_BeanDesc(POCO_BeanScopeContext*);
	~POCO_BeanDesc() {}
	void destroy();

	int	ref_count;

	int	init(POCO_AppEnv*);
	int	fini(POCO_AppEnv*);

	POCO_String	factory_method_signature;
	const char*	get_factory_method_signature();
	int		_resolve_method(const char* method_type, POCO_AppEnv*);
	int		resolve_factory_method(POCO_AppEnv* env);
	int		resolve_inject_method(POCO_AppEnv* env);
	void*		_invoke_method(void* factory_addr, const char*, int, POCO_AppEnv*);
	int		post_invoke(POCO_BeanDesc*, POCO_AppEnv*);

	POCO_BeanScopeContext* scope_context;

	POCO_String	the_key;
	int		keyed_count;

	POCO_String		the_wrapper_id;
	POCO_factory_wrapper_t	the_wrapper;

  public:
	int		destroy_non_singleton(POCO_AppEnv*);

	//void ref();
	//void unref();

	void* get_bean_nodup(POCO_AppEnv*); // never dup
	void* get_bean_dup(POCO_AppEnv*); // dup, if is singleton
	void* bean_dup(void* bean_addr, POCO_AppEnv*);
	const char* get_bean_class();
	const char* get_bean_class_ptr();
};

#endif
