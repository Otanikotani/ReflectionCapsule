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

#include "descs.h"
#include <stdlib.h>
#include <string.h>
#include "table.h"

#include "pocoreflx.h"
#include "pocostr.h"
#include "util.h"
#include "ctype.h"

#include <stdio.h>

static int _uid = 0;
//
// BeanDesc
// Added initialization of string values & is_self_dup as 0; -Ha
//
POCO_BeanDesc::POCO_BeanDesc(POCO_BeanScopeContext* sc): id(), bean_class_ptr(), bean_class(), factory_method_name(),
        dup_method_name(), destroy_method_name(), factory_method_signature(), the_key(), the_wrapper_id() {
	uid = _uid++;
	is_parsed = 0;
	is_singleton = 1;
	is_abstract = 0;
	is_lazy_init = 1;
	is_initing = 0;
	is_inited = 0;
	is_property = 0;
	is_no_target = 0;
	is_public = 1;

	depended_bean = NULL;
	factory_bean = NULL;
	this_bean = NULL;

	bean_addr = NULL;

	factory_method = NULL;

	num_args = 0;
	args = NULL;

	exh_ioc = NULL;

	num_props = 0;
	props = NULL;

	dup_method = NULL;
	is_self_dup = 0;
	destroy_method = NULL;
	destroy_bean = NULL;
	ref_count = 1;

	scope_context = sc;
	keyed_count = 0;

	the_wrapper = NULL;

	sc->add_bean(this);

	// printf("created bean desc %d %p\n", uid, this); 
}

void POCO_BeanDesc::destroy()
{
	// printf("deleting desc %d %p\n", uid, this);

	if( num_args && args != NULL ) {
		for(int i=0; i<num_args; i++) {
			if( args[i] != NULL ) {
				delete args[i];
			}
		}

		delete [] args;
	}

	if( num_props && props != NULL ) {
		// properties are also registered to
		// context's all_beans table and
		// to be destroyed on ctxt destuction.
		/********
		for(int i=0; i<num_props; i++) {
			if( props[i] != NULL ) {
				delete props[i];
			}
		}
		********/

		delete [] props;
	}

	delete this;
}

int POCO_BeanDesc::_resolve_method(const char* method_type, POCO_AppEnv* env)
{
	if( factory_method != NULL ) {
		return 1;
	}

	const char* factory_class_ptr = NULL;

	if( factory_bean != NULL ) {
		factory_class_ptr = factory_bean->bean_class_ptr.in();
	}

	const char* sig = get_factory_method_signature();

	factory_method = POCO_Reflection::get_method(
				bean_class_ptr.in(),
				factory_class_ptr,
				sig);

	if( factory_method == NULL ) {
		POCO_String msg = (const char*)
				"[E011] fail to resolve the dynamic proxy of ";
		msg += method_type;
		msg += " \"";
		
		if( factory_bean != NULL ) {
			if( factory_bean->bean_class.in() != NULL ) {
				msg += factory_bean->bean_class.in();
				msg += "::";
			}
			else 
			if( factory_bean->bean_class_ptr.in() != NULL ) {
				msg += "(*";
				msg += factory_bean->bean_class_ptr.in();
				msg += ")::";
			}
		}
		msg += sig;
		msg += "\"";
		POCO_AppEnvImpl::fatal_error(env, msg.in());

		return 0;
	}

	return 1;
}

int POCO_BeanDesc::resolve_factory_method(POCO_AppEnv* env) 
{
	if( factory_method_name.in() == NULL 
         && (bean_class_ptr.in() == NULL || strcmp(bean_class_ptr.in(), "void") == 0) ) {
		return 0;
	}
	return _resolve_method("factory-method", env);
}

int POCO_BeanDesc::resolve_inject_method(POCO_AppEnv* env)
{
	return _resolve_method("method", env);
}

void* POCO_BeanDesc::_invoke_method(void* factory_bean_addr, const char* factory_bean_class_ptr, 
					int is_ioc, POCO_AppEnv* env)
{
	if( factory_method_name.in() == NULL 
	 && (bean_class_ptr.in() == NULL || strcmp(bean_class_ptr.in(), "void") == 0) ) {
		post_invoke(NULL, env);
		return NULL;
	}

	if( resolve_factory_method(env) == 0 ) {
		return NULL;
	}

	POCO_Table params;

	env->reset();

	if( is_ioc == 0 && factory_bean_addr == NULL ) {
		if( factory_bean != NULL ) {
			factory_bean_addr = factory_bean->get_bean_nodup(env);

			if( factory_bean_addr == NULL ) {
				return NULL;
			}
		}
	}

	for(int i=0;i<num_args;i++) {
		int do_dup = args[i]->pass_dup();

		if( args[i]->is_array ) {
			params.put(&(args[i]->valdesc_array_size));

			for(int j=0;j<args[i]->valdesc_array_size;j++) {
				if( args[i]->is_namevalue() ) {
					params.put(args[i]->valdesc_array[j]
						->get_name());
				}

				void* v = args[i]->valdesc_array[j]
					->get_value(do_dup, this_bean, factory_bean_addr, env);

				if( v == NULL && env->has_error() ) {
					return NULL;
				}

				params.put(v);
			}
		}
		else {
			void* v = args[i]->valdesc->get_value(do_dup, this_bean, factory_bean_addr, env);
			if( v == NULL && env->has_error() ) {
				return NULL;
			}
			params.put(v);
		}
	}

	POCO_AppEnv* dftenv = POCO_AppContext::getDefaultAppEnv();
	dftenv->reset();

	if( scope_context->trace() ) {
		if( factory_bean_addr != NULL && !is_no_target ) {
			const char* class_ptr = "..";
			if( is_ioc ) {
				class_ptr = factory_bean_class_ptr;
			}
			else {
				class_ptr = factory_bean->bean_class_ptr.in();
			}
				
			fprintf(stderr, "((%s)%p)->%s(...) ", 
					class_ptr,
					factory_bean_addr,
					factory_method_name.in());
		}
		else {
			fprintf(stderr, "%s(...) ", factory_method_name.in());
		}
		fflush(stderr);
	}

	void* ret_bean_addr = NULL;

	if( the_wrapper == NULL ) {
		ret_bean_addr = factory_method(factory_bean_addr,
		                                params.get());
	}
	else {
		ret_bean_addr = the_wrapper(
					the_wrapper_id.in(),
					factory_method,
					factory_bean_addr,
					params.get(), env);
	}

	if( scope_context->trace() ) {
		if( !is_ioc && bean_class_ptr.in() ) {
			fprintf(stderr, "= (%s)%p\n", bean_class_ptr.in(), ret_bean_addr);
		}
		else {
			fprintf(stderr, "\n");
		}
	}

	if( ret_bean_addr == NULL && dftenv->has_error() ) {
		POCO_String msg = "[E199] Error from invoked bean operation ";
		msg += factory_method_name.in();
		msg += "(...):\n";
		msg += dftenv->get_message();
		dftenv->reset();
		POCO_AppEnvImpl::fatal_error(env, msg.in());
		return NULL;
	}

	post_invoke(this_bean, env);

	return ret_bean_addr;
}

int POCO_BeanDesc::post_invoke(POCO_BeanDesc* this_bean, POCO_AppEnv* env)
{
	int flag = 1;

	for(int i=0; i<num_args;i++) {
		if( args[i]->post_invoke(this_bean, env) == 0 ) {
			flag = 0;
		}
	}

	return flag;
}

void* POCO_BeanDesc::get_bean_dup(POCO_AppEnv* env)
{
	void* bean = get_bean_nodup(env);

	return bean_dup(bean, env);
}

void* POCO_BeanDesc::bean_dup(void* bean, POCO_AppEnv* env)
{
	if( bean == NULL ) {
		return NULL;
	}

	if( is_singleton && dup_method != NULL ) {
		if( is_self_dup ) {
			if( scope_context->trace() ) {
				fprintf(stderr, "(%s)%p->%s() ", bean_class_ptr.in(), bean, dup_method_name.in());
				fflush(stderr);
			}
			dup_method(bean, NULL);
			if( scope_context->trace() ) {
				fprintf(stderr, "\n");
			}
		}
		else {
			if( scope_context->trace() ) {
				fprintf(stderr, "%s((%s)%p) ", dup_method_name.in(), bean_class_ptr.in(), bean);
				fflush(stderr);
			}
			void* params[] = {bean};
			bean = dup_method(NULL, params);
			if( scope_context->trace() ) {
				fprintf(stderr, "\n");
			}
		}
	}

	return bean;
}

void* POCO_BeanDesc::get_bean_nodup(POCO_AppEnv* env)
{
	if( is_singleton ) {
		if( is_inited ) {
			return bean_addr;
		}

		if( is_initing ) {
			POCO_AppEnvImpl::fatal_error(env, 
				"[E050] Circulated constuctor or destory bean dependency");
			return NULL;
		}

		if( the_key.in() != NULL ) {
			POCO_BeanDesc* the_keyed_bean = scope_context->get_keyed_bean(the_key.in());
			if( the_keyed_bean != NULL ) {
				if( strcmp(the_keyed_bean->bean_class_ptr.in(), bean_class_ptr.in()) ) {
					POCO_String msg 
					= (const char*)"[E201] beans in different pointer types ";
					msg += the_keyed_bean->bean_class_ptr.in();
					msg += " and ";
					msg += bean_class_ptr.in();
					msg += " taking same key value \"";
					msg += the_key.in();
					msg += "\"";
					POCO_AppEnvImpl::fatal_error(env, msg.in());
					return NULL;
				}
				the_keyed_bean->keyed_count++;
				keyed_count = 1;
				is_inited = 1;
				bean_addr = the_keyed_bean->bean_addr;
				return bean_addr;
			}
		}
	}

	env->reset();

	if( !is_inited ) {
		if( depended_bean != NULL ) {
			if( depended_bean->get_bean_nodup(env) == NULL
			 && env->has_error() ) {
				return NULL;
			}
		}

		if( destroy_bean != NULL && destroy_bean != this && destroy_bean->is_singleton ) {
			// 
			// bring up the destroy bean before the bean itself
			// to prevent circulated destory dependency, and also
			// to ensure the destory bean itself should be destoryed
			// after the to be destroyed bean(s).
			//
			if( destroy_bean->get_bean_nodup(env) == NULL
			 && env->has_error() ) {
				return NULL;
			}
		}
	}

	is_initing = 1;
	void* the_bean = _invoke_method(NULL, NULL, 0, env);
	is_initing = 0;

	if( the_bean == NULL && bean_class_ptr.in() != NULL ) {
		if(env->has_error()) {
			return NULL;
		}

		if( exh_ioc ) {
			exh_ioc->_invoke_method(NULL, NULL, 1, env);
			return NULL;
		}

		POCO_String msg = (const char*)"[E014] bean factory returns a NULL ";
		msg += bean_class_ptr.in();
		msg += " pointer.";

		POCO_AppEnvImpl::fatal_error(env, msg.in());
		return NULL;
	}

	is_inited = 1;

	if( is_singleton || destroy_method != NULL ) {
		bean_addr = the_bean;

		if( is_singleton && scope_context && the_key.in() == NULL ) {
			scope_context->push_inited_singleton(this);
		}
	}

	for(int i=0; i<num_props; i++) {
		env->reset();

		props[i]->_invoke_method(the_bean, bean_class_ptr.in(), 1, env);

		if( env->has_error() ) {
			is_inited = 0;
			bean_addr = NULL;
			return NULL;
		}
	}

	if( is_singleton && the_key.in() != NULL ) {
		keyed_count = 1;
		scope_context->add_keyed_bean(this);
	}

	return the_bean;
}

int POCO_BeanDesc::init(POCO_AppEnv* env)
{
	if( !is_singleton || is_inited || is_lazy_init ) {
		return 1;
	}

	env->reset();

	if( get_bean_nodup(env) == NULL &&
	        env->has_error() ) {
		return 0;
	}

	return 1;
}

int POCO_BeanDesc::fini(POCO_AppEnv* env)
{
	if( bean_addr == NULL && bean_class_ptr.in() != NULL ) {
		// singleton
	    return 1;
	}

	if( destroy_method == NULL || keyed_count != 0 ) {
		//
		// this relesae will by-pass all key'ed bean's destruction. may change later on
		//
		return 1;
	}



	void* bean_addr_copy = bean_addr;
	bean_addr = NULL;

	if( destroy_bean == NULL ) {
		if( scope_context->trace() ) {
			POCO_String arg = bean_class_ptr.in();
			if( bean_class_ptr.in() == NULL ) {
				fprintf(stderr, "%s(void) ", destroy_method_name.in());
			}
			else {
				fprintf(stderr, "%s((%s)%p) ", destroy_method_name.in(), 
						bean_class_ptr.in(), bean_addr_copy);
			}
			fflush(stderr);
		}
		void* params[] = { bean_addr_copy };
		destroy_method(NULL, params);
		if( scope_context->trace() ) {
			fprintf(stderr, "\n");
		}
	}
	else
	if( destroy_bean == this ) {
		if( bean_addr_copy == NULL ) {
			return 1;
		}

		if( scope_context->trace() ) {
			fprintf(stderr, "(%s)%p->%s() ", 
				bean_class_ptr.in(), bean_addr_copy, destroy_method_name.in());
			fflush(stderr);
		}
		destroy_method(bean_addr_copy, NULL);
		if( scope_context->trace() ) {
			fprintf(stderr, "\n");
		}
	}
	else {
		void* destroy_bean_addr = destroy_bean->get_bean_nodup(env);

		if( destroy_bean_addr == NULL ) {
			return 0;
		}

		if( scope_context->trace() ) {
			if( bean_class_ptr.in() == NULL ) {
				fprintf(stderr, "(%s)%p)->%s(void) ",
				destroy_bean->bean_class_ptr.in(),
				destroy_bean_addr,
				destroy_method_name.in());
			}
			else {
				fprintf(stderr, "((%s)%p)->%s((%s)%p) ", 
				destroy_bean->bean_class_ptr.in(),
				destroy_bean_addr,
				destroy_method_name.in(),
				bean_class_ptr.in(), bean_addr_copy);
			}
			fflush(stderr);
		}
		void* params[] = { bean_addr_copy };
		destroy_method(destroy_bean_addr, params);
		if( scope_context->trace() ) {
			fprintf(stderr, "\n");
		}
	}

	return 1;
}

int POCO_BeanDesc::destroy_non_singleton(POCO_AppEnv* env)
{
	if( is_singleton ) {
		return 1;
	}

	return fini(env);
}

const char* POCO_BeanDesc::get_factory_method_signature()
{
	if( factory_method_signature.in() != NULL ) {
		return factory_method_signature.in();
	}

	if( num_args == 0 ) {
		factory_method_signature = factory_method_name.in();
		factory_method_signature += "()";
		return factory_method_signature.in();
	}

	int i = 0;

	if( args[i]->is_index_arg ) {
		factory_method_signature += "operator[](";
		factory_method_signature += args[0]->get_signature();
		factory_method_signature += ")";

		if( isalpha(factory_method_name.in()[0]) ) {
			factory_method_signature += "->";
		}

		i=1;
	}

	factory_method_signature += factory_method_name.in();
	factory_method_signature += "(";

	for(;i< num_args; i++) {
		factory_method_signature += args[i]->get_signature();

		if( i+1 != num_args ) {
			factory_method_signature += ", ";
		}
	}

	factory_method_signature += ")";

	return factory_method_signature.in();
}

//
// ArgDesc
//
POCO_ArgDesc::POCO_ArgDesc() {
	uid = _uid++;
	is_array = 0;
	valdesc_array_size = 0;
	valdesc_array = NULL;
	valdesc = NULL;
	pass_syntax = PASS_DEFAULT;

	// printf("created arg desc %d %p\n", uid, this);
}

POCO_ArgDesc::~POCO_ArgDesc() {
	// printf("deleting desc %d %p\n", uid, this);

	// valdesc is registered to the destroy list, and will be deleted on
	// destory the context. -Ke
    // No, its not... -Ha
	if( valdesc != NULL ) {
		delete valdesc;
	}


	if( valdesc_array != NULL ) {
		/****
		// for the same reason. -Ke
		for(int i=0; i<valdesc_array_size; i++) {
			if( valdesc_array[i] != NULL ) {
				delete valdesc_array[i];
			}
		}
		****/

		delete [] valdesc_array;
	}
}

int POCO_ArgDesc::post_invoke(POCO_BeanDesc* this_bean, POCO_AppEnv* env)
{
	if( valdesc != NULL ) {
		return valdesc->post_invoke(this_bean, env);
	}

//	int flag = 1;

	for(int i=0; i<valdesc_array_size; i++) {
		if( valdesc_array[i]->post_invoke(this_bean, env) == 0 ) {
//			flag = 0;
		}
	}

	return 0;
}

int POCO_ArgDesc::is_namevalue()
{
	return (strcmp(type.in(), "namevalue") == 0);
}

int POCO_ArgDesc::pass_dup()
{
	return (pass_syntax == PASS_DUP);
}

int POCO_ArgDesc::pass_deref()
{
	return (pass_syntax == PASS_DEREF);
}

const char* POCO_ArgDesc::get_signature()
{
	if( signature.in() != NULL ) {
		return signature.in();
	}

	if( is_array ) {
		signature = (const char*)"int, ";
	}

	if( strcmp(type.in(), "bean") == 0 ) {
		if( pass_deref() ) {
			if( class_ptr.in()[strlen(class_ptr.in())-1] == '*' ) {
				POCO_String derefed
				= POCO_Util::trim_last_char(class_ptr.in());

				signature += derefed.in();
			}
			else {
				signature += "*";
				signature += class_ptr.in();
			}
		}
		else {
			signature += class_ptr.in();
		}
	}
	else
	if( strcmp(type.in(), "namevalue") == 0 ) {
		signature += "POCO_NameValue";
	}
	else
	if( strcmp(type.in(), "byte") == 0
	 || strcmp(type.in(), "uchar") == 0 ) {
		signature += "unsigned char";
	}
	else
	if( strcmp(type.in(), "char") == 0 ) {
		signature += "char";
	}
	else
	if( strcmp(type.in(), "string") == 0 || strcmp(type.in(), "cstring") == 0 ) {
		signature += "const char*";
	}
	else
	if( strcmp(type.in(), "short") == 0 ) {
		signature += "short";
	}
	else
	if( strcmp(type.in(), "ushort") == 0 ) {
		signature += "unsigned short";
	}
	else
	if( strcmp(type.in(), "long") == 0 ) {
		signature += "long";
	}
	else
	if( strcmp(type.in(), "ulong") == 0 ) {
		signature += "unsigned long";
	}
	else
	if( strcmp(type.in(), "float") == 0 ) {
		signature += "float";
	}
	else
	if( strcmp(type.in(), "double") == 0 ) {
		signature += "double";
	}

	if( is_array ) {
		signature += "[]";
	}

	return signature.in();
}

//
// ValDesc
//
POCO_ValDesc::POCO_ValDesc()
{
	uid = _uid ++;
	// printf("created val desc %d %p\n", uid, this); 
	long_val = 0UL;
	val_addr = NULL;
	cast_method = NULL;
	bean = NULL;
}

POCO_ValDesc::~POCO_ValDesc() 
{
	// printf("deleting desc %d %p\n", uid, this); 
}

int POCO_ValDesc::post_invoke(POCO_BeanDesc* this_bean, POCO_AppEnv* env)
{
	if( bean == NULL || bean == this_bean ) {
		return 1;
	}

	return bean->destroy_non_singleton(env);
}

void* POCO_ValDesc::get_value(int do_dup, POCO_BeanDesc* this_bean, void* this_bean_addr, POCO_AppEnv* env)
{
	if( val_addr != NULL ) {
		// not bean
		return val_addr;
	}

	// bean
	if( bean == NULL ) {
		return NULL;
	}

	void* bean_addr;

	if( bean == this_bean ) {
		bean_addr = this_bean_addr;

		if( do_dup ) {
			bean_addr = bean->bean_dup(bean_addr, env);
		}
	}
	else {
		if( do_dup ) {
			bean_addr = bean->get_bean_dup(env);
		}
		else {
			bean_addr = bean->get_bean_nodup(env);
		}
	}

	if( bean_addr == NULL ) {
		return NULL;
	}

	if( cast_method != NULL ) {
		void*	params[] = { bean_addr };
		bean_addr = cast_method(NULL, params);
	}

	return bean_addr;
}

void* POCO_ValDesc::get_name()
{
	return name.inout();
}
