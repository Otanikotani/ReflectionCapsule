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

#include "poco.h"
#include "descs.h"
#include "pocoreflx.h"
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "convert.h"

#include <stdio.h>

#include "appctxtimp.h"
#ifdef DEBUG
# include "debug.h"
#endif

static const char* TOKEN_APP_CONTEXT = "poco-application-context";
static const char* TOKEN_BEAN = "bean";
static const char* TOKEN_CLASS = "class";
static const char* TOKEN_IMPORT = "import";
static const char* TOKEN_ID = "id";
static const char* TOKEN_REF = "ref";
static const char* TOKEN_LAZY_INIT = "lazy-init";
static const char* TOKEN_FACTORY_BEAN = "factory-bean";
static const char* TOKEN_FACTORY_METHOD = "factory-method";
static const char* TOKEN_METHOD_ARG = "method-arg";
static const char* TOKEN_DUP_METHOD = "dup-method";
static const char* TOKEN_SELF_DUP_METHOD = "self-dup-method";
static const char* TOKEN_DESTROY_BEAN = "destroy-bean";
static const char* TOKEN_DESTROY_METHOD = "destroy-method";
static const char* TOKEN_SELF_DESTROY_METHOD = "self-destroy-method";
static const char* TOKEN_THIS_BEAN = "this-bean";
static const char* TOKEN_VOID = "void";
static const char* TOKEN_VALUE = "value";
static const char* TOKEN_ENV_VAR = "env-var";
static const char* TOKEN_ARRAY = "array";
static const char* TOKEN_BYTE = "byte";
static const char* TOKEN_CHAR = "char";
static const char* TOKEN_UCHAR = "uchar";
static const char* TOKEN_SHORT = "short";
static const char* TOKEN_USHORT = "ushort";
static const char* TOKEN_FLOAT = "float";
static const char* TOKEN_DOUBLE = "double";
static const char* TOKEN_STRING = "string";
static const char* TOKEN_CSTRING = "cstring";
static const char* TOKEN_NAMEVALUE = "namevalue";
static const char* TOKEN_NAME = "name";
static const char* TOKEN_ABSTRACT = "abstract";
static const char* TOKEN_TRUE = "true";
static const char* TOKEN_FALSE = "false";
static const char* TOKEN_SINGLETON = "singleton";
static const char* TOKEN_DEPENDS_ON = "depends-on";
static const char* TOKEN_IOC = "ioc";
static const char* TOKEN_ON_ERROR = "on-error";
static const char* TOKEN_ITEM = "item";
static const char* TOKEN_TYPE = "type";
static const char* TOKEN_LONG = "long";
static const char* TOKEN_ULONG = "ulong";
static const char* TOKEN_IOC_METHOD = "method";
static const char* TOKEN_INDEX_ARG = "index-arg";
static const char* TOKEN_PASS = "pass";
static const char* TOKEN_DEREF = "deref";
static const char* TOKEN_PTR = "ptr";
static const char* TOKEN_DUP = "dup";
//static const char* TOKEN_LOAD = "load";
static const char* TOKEN_LIBRARY = "library";
static const char* TOKEN_RESOURCE = "resource";
static const char* TOKEN_TARGET = "target";
static const char* TOKEN_NONE = "none";
static const char* TOKEN_KEY = "key";
static const char* TOKEN_FACTORY_WRAPPER = "factory-wrapper";
static const char* TOKEN_PUBLIC = "public";

POCO_AppContextImpl::POCO_AppContextImpl()
{
    is_closed = 0;
    lazy_resolve = 0;
}

POCO_AppContextImpl::~POCO_AppContextImpl()
{
}

//
// The entrant parser of a POCO <beans> document
//
int
POCO_AppContextImpl::parseDoc(POCO_DOM::Element* doc, POCO_AppEnv* env)
{
    if (is_closed)
    {
        POCO_AppEnvImpl::fatal_error(env,
                "[E001] Application Context already closed");
        return 0;
    }

    const char* tag = doc->getTagName();
    const char* attr;
    if (strcmp(tag, TOKEN_APP_CONTEXT) == 0)
    {
        id = doc->getAttribute(TOKEN_ID);

        if (this->id.in() != NULL)
        {
            // Many contexts
            static POCO_Hashtable namedCtxts;

            if (namedCtxts.containsKey(this->id.in()))
            {
                POCO_String msg = "[W001] duplicated context of id=\"";
                msg += this->id.in();
                msg += "\", skipped";
                env->warning(msg.in());
                return 1;
            }
            else
            {
                namedCtxts.put(this->id.in(), this);
            }
        }
    }

    //
    // resolve all types <bean> elements that has an id.
    //
    if (resolve_all_bean_ids(doc, env) == 0)
    {
        return 0;
    }

    int num_child_elems = doc->numOfChildElements();
    int i;

    // load all libraries
    for (i = 0; i < num_child_elems; i++)
    {
        POCO_DOM::Element* elem = doc->getChild(i);

        tag = elem->getTagName();

//        if (strcmp(tag, TOKEN_LOAD) == 0)
//        {
//            //if( parseLoadLib(elem, env) == 0 ) {
//            //	return 0;
//            //}
//        }
    }

    // all other nodes
    for (i = 0; i < num_child_elems; i++)
    {
        POCO_DOM::Element* elem = doc->getChild(i);

        tag = elem->getTagName();

        if (strcmp(tag, TOKEN_BEAN) == 0)
        {
            //
            // ignore outer abstract beans
            //
            attr = elem->getAttribute(TOKEN_ABSTRACT);

            if (attr != NULL && strcmp(attr, TOKEN_TRUE) == 0)
            {
                continue;
            }

            attr = elem->getAttribute(TOKEN_ID);

            if (attr == NULL)
            {
                attr = elem->getAttribute(TOKEN_SINGLETON);
                int is_singleton = (attr == NULL
                        || strcmp(attr, TOKEN_FALSE) != 0);

                attr = elem->getAttribute(TOKEN_LAZY_INIT);
                int is_lazy_init = (attr == NULL
                        || strcmp(attr, TOKEN_FALSE) != 0);

                //
                // ignore all beans that will
                // never be instantiated ...
                //
                if (is_singleton == 0 || is_lazy_init == 1)
                {
                    continue;
                }
            }

            POCO_BeanDesc* desc = parseBean(elem, env);

            if (desc == NULL)
            {
                return 0;
            }
        }
        else if (strcmp(tag, TOKEN_IMPORT) == 0)
        {
            if (parseImport(elem, env) == 0)
            {
                return 0;
            }
        }
    }

    // get trace setting
    _trace = 0;
    for (; env != NULL;)
    {
        const char* val = env->getValue("pococapsule.trace.enable");

        if (val != NULL)
        {
            _trace = (strcmp(val, "true") == 0);
            break;
        }

        const char** argv = env->getArray("pococapsule.init.argv");

        if (argv != NULL)
        {
            for (int i = 0; argv[i]; i++)
            {
                if (strcmp("-Dpococapsule.trace.enable=true", argv[i]) == 0)
                {
                    _trace = 1;
                    break;
                }
            }
        }

        break;
    }

    return 1;
}

int
POCO_AppContextImpl::resolve_all_bean_ids(POCO_DOM::Element* elem,
        POCO_AppEnv* env)
{
    const char* tag = elem->getTagName();

    if (strcmp(tag, TOKEN_BEAN) == 0)
    {
        const char* id = elem->getAttribute(TOKEN_ID);
        if (id != NULL)
        {
            POCO_BeanDesc* desc = new POCO_BeanDesc(this);
            desc->is_parsed = 0;
            desc->id = id;
            const char* attr = elem->getAttribute(TOKEN_CLASS);
            if (strcmp(attr, TOKEN_VOID) == 0)
            {
                attr = NULL;
            }
            desc->bean_class = attr;
            desc->bean_class.replace('{', '<');
            desc->bean_class.replace('}', '>');
            desc->bean_class_ptr = POCO_Util::class_to_ptr(
                    desc->bean_class.in());
            attr = elem->getAttribute(TOKEN_ABSTRACT);
            if (attr != NULL && strcmp(attr, TOKEN_TRUE) == 0)
            {
                desc->is_abstract = 1;
            }

            if (bean_id_map.put(desc->id.in(), desc) == 0)
            {
                POCO_AppEnvImpl::fatal_error(env,
                        "[E002] a bean with the same id "
                                "already exists", elem);

                return 0;
            }
        }
    }

    int num_child_elems = elem->numOfChildElements();

    for (int i = 0; i < num_child_elems; i++)
    {
        POCO_DOM::Element* child = elem->getChild(i);

        if (resolve_all_bean_ids(child, env) == 0)
        {
            return 0;
        }
    }

    return 1;
}

static POCO_factory_wrapper_t
resolve_factory_wrapper(const char* id);

POCO_BeanDesc*
POCO_AppContextImpl::parseBean(POCO_DOM::Element* elem, POCO_AppEnv* env)
{
    const char* attr = elem->getAttribute(TOKEN_ABSTRACT);

    if (attr != NULL && strcmp(attr, TOKEN_TRUE) == 0)
    {
        POCO_AppEnvImpl::fatal_error(env,
                "[E003] an inner or depended bean is abstract", elem);
        return NULL;
    }

    const char* id = elem->getAttribute(TOKEN_ID);

    POCO_BeanDesc* desc = NULL;

    if (id == NULL)
    {
        desc = new POCO_BeanDesc(this);
        const char* attr = elem->getAttribute(TOKEN_CLASS);
        if (strcmp(attr, TOKEN_VOID) == 0)
        {
            attr = NULL;
        }
        desc->bean_class = attr;
        desc->bean_class.replace('{', '<');
        desc->bean_class.replace('}', '>');
        desc->bean_class_ptr = POCO_Util::class_to_ptr(desc->bean_class.in());
    }
    else
    {
        desc = (POCO_BeanDesc*) bean_id_map.get(id);

        if (desc == NULL)
        {
            POCO_AppEnvImpl::fatal_error(env, "[E004] a bean with the given id "
                    "does not exist in this context", elem);
            return NULL;
        }

        if (desc->is_parsed == 1)
        {
            return desc;
        }

        desc->is_parsed = 1;
    }

    attr = elem->getAttribute(TOKEN_SINGLETON);

    if (attr != NULL && strcmp(attr, TOKEN_FALSE) == 0)
    {
        desc->is_singleton = 0;
    }

    attr = elem->getAttribute(TOKEN_LAZY_INIT);

    if (attr != NULL && strcmp(attr, TOKEN_FALSE) == 0)
    {
        desc->is_lazy_init = 0;
    }

    attr = elem->getAttribute(TOKEN_PUBLIC);

    if (attr != NULL && strcmp(attr, TOKEN_FALSE) == 0)
    {
        desc->is_public = 0;
    }

    attr = elem->getAttribute(TOKEN_DEPENDS_ON);
    if (attr != NULL)
    {
        POCO_BeanDesc* depended = (POCO_BeanDesc*) bean_id_map.get(attr);

        if (depended != desc)
        {
            //
            // only need to care non-self dependent case
            //
            if (depended == NULL)
            {
                POCO_AppEnvImpl::fatal_error(env, "[E005] depended bean "
                        "does not exist", elem);
                return NULL;
            }

            if (depended->is_abstract)
            {
                POCO_AppEnvImpl::fatal_error(env, "[E006] depended bean is "
                        "abstract", elem);
                return NULL;
            }

            desc->depended_bean = depended;
        }
    }

    attr = elem->getAttribute(TOKEN_FACTORY_BEAN);
    if (attr != NULL)
    {
        POCO_BeanDesc* factory = (POCO_BeanDesc*) bean_id_map.get(attr);

        if (factory == NULL)
        {
            POCO_AppEnvImpl::fatal_error(env,
                    "[E008] factory bean does not exist", elem);
            return NULL;
        }

        if (factory == desc)
        {
            POCO_AppEnvImpl::fatal_error(env,
                    "[E007] a bean is used as factory of itself", elem);

            return NULL;
        }

        if (factory->is_abstract)
        {
            POCO_AppEnvImpl::fatal_error(env, "[E009] factory bean is abstract",
                    elem);
            return NULL;
        }

        if (factory->bean_class_ptr.in() == NULL)
        {
            POCO_AppEnvImpl::fatal_error(env,
                    "[E009] a void bean is used as factory", elem);
        }

        desc->factory_bean = factory;
    }

    attr = elem->getAttribute(TOKEN_FACTORY_METHOD);
    if (attr != NULL)
    {
        desc->factory_method_name = attr;
        desc->factory_method_name.replace('{', '<');
        desc->factory_method_name.replace('}', '>');
    }
    else
    {
        if (desc->bean_class.in() != NULL && desc->bean_class.in()[0] == '*')
        {
            POCO_AppEnvImpl::fatal_error(env, "[E010] missing factory name or "
                    "qualified class name", elem);
            return NULL;
        }
        else if (desc->bean_class.in() != NULL
                && strcmp(desc->bean_class.in(), TOKEN_VOID) != 0)
        {
            // constructor
            // Rolf-copters have been detected. Raise your spears!
            desc->factory_method_name = (const char*) "new ";
            desc->factory_method_name += desc->bean_class.in();
        }
    }

    attr = elem->getAttribute(TOKEN_FACTORY_WRAPPER);

    if (attr != NULL)
    {
        desc->the_wrapper = resolve_factory_wrapper(attr);
        if (desc->the_wrapper == NULL)
        {
            POCO_String msg =
                    (const char*) "[E047] fail to resolve factory wrapper of id: ";
            msg += attr;

            POCO_AppEnvImpl::fatal_error(env, msg, elem);

            return NULL;
        }

        desc->the_wrapper_id = (const char*) attr;
    }
    else
    {
        desc->the_wrapper = resolve_factory_wrapper("default");

        if (desc->the_wrapper != NULL)
        {
            desc->the_wrapper_id = (const char*) "default";
        }
    }

    if (desc->is_singleton)
    {
        desc->the_key = elem->getAttribute(TOKEN_KEY);
    }

    if (parseMethodArgs(elem, desc, NULL, env) == 0)
    {
        return NULL;
    }

    if (lazy_resolve == 0 && desc->factory_method_name.in() != NULL
            && desc->resolve_factory_method(env) == 0)
    {
        POCO_String msg = env->get_message();
        env->reset();
        POCO_AppEnvImpl::fatal_error(env, msg.in(), elem);
        return NULL;
    }

    if (parsePropSetters(elem, desc, env) == 0
            || parseDupMethod(elem, desc, env) == 0
            || parseReleaseMethod(elem, desc, env) == 0)
    {
        return NULL;
    }

    if (desc->is_singleton)
    {
        singleton_beans.put(desc);
    }

    return desc;
}

int
POCO_AppContextImpl::parseMethodArgs(POCO_DOM::Element* elem,
        POCO_BeanDesc* desc, POCO_BeanDesc* this_bean, POCO_AppEnv* env)
{
    POCO_Table table;

    for (int i = 0; i < elem->numOfChildElements(); i++)
    {
        POCO_DOM::Element* child = elem->getChild(i);

        const char* tag = child->getTagName();
        if (strcmp(tag, TOKEN_METHOD_ARG) != 0
                && strcmp(tag, TOKEN_INDEX_ARG) != 0)
        {
            continue;
        }

        POCO_ArgDesc* arg = parseArg(child, this_bean, env);

        if (arg == NULL)
        {
            //
            // cleanup and return failure
            //
            for (;;)
            {
                arg = (POCO_ArgDesc*) table.remove(0);
                if (arg == NULL)
                {
                    break;
                }
                delete arg;
            }
            return 0;
        }

        table.put(arg);
    }

    if (table.count() != 0)
    {
        desc->num_args = table.count();
        desc->args = new POCO_ArgDesc*[table.count()];
        for (int i = 0; i < table.count(); i++)
        {
            desc->args[i] = (POCO_ArgDesc*) table.get(i);
        }
    }

    return 1;
}

int
POCO_AppContextImpl::parsePropSetters(POCO_DOM::Element* elem,
        POCO_BeanDesc* desc, POCO_AppEnv* env)
{
    POCO_Table table;

    for (int i = 0; i < elem->numOfChildElements(); i++)
    {
        POCO_DOM::Element* child = elem->getChild(i);

        const char* tag = child->getTagName();
        if (strcmp(tag, TOKEN_ON_ERROR) == 0)
        {
            desc->exh_ioc = parseOnError(child, desc, env);

            if (desc->exh_ioc == NULL)
            {
                return 0;
            }
        }

        if (strcmp(tag, TOKEN_IOC) != 0)
        {
            continue;
        }

        POCO_PropDesc* prop = parsePropSetter(child, desc, env);

        if (prop == NULL)
        {
            /**************
             // will be cleaned on destroying the context. -Ke
             for(;;) {
             prop = (POCO_PropDesc*)table.remove(0);
             if( prop == NULL ) {
             break;
             }
             delete prop;
             }
             ************/
            return 0;
        }

        table.put(prop);
    }

    if (table.count() != 0)
    {
        desc->num_props = table.count();
        desc->props = new POCO_PropDesc*[table.count()];
        for (int i = 0; i < table.count(); i++)
        {
            desc->props[i] = (POCO_PropDesc*) table.get(i);
        }
    }

    return 1;
}

POCO_PropDesc*
POCO_AppContextImpl::parsePropSetter(POCO_DOM::Element* elem,
        POCO_BeanDesc* desc, POCO_AppEnv* env)
{
    const char* attr = elem->getAttribute(TOKEN_IOC_METHOD);
    if (attr == NULL)
    {
        POCO_AppEnvImpl::fatal_error(env,
                "[E013] property missing ioc method attribute", elem);
        return NULL;
    }

    POCO_PropDesc* prop = new POCO_PropDesc(this);
    prop->is_property = 1;
    prop->factory_bean = NULL;
    prop->this_bean = desc;
    prop->factory_method_name = attr;
    prop->factory_method_name.replace('{', '<');
    prop->factory_method_name.replace('}', '>');

    if (desc->bean_class_ptr.in() != NULL)
    {
        // not a void bean
        const char* target = elem->getAttribute(TOKEN_TARGET);

        if (target == NULL || strcmp(target, TOKEN_THIS_BEAN) == 0)
        {
            prop->factory_bean = desc;
        }
        else if (strcmp(target, TOKEN_NONE) != 0)
        {
            POCO_AppEnvImpl::fatal_error(env,
                    "[E012] invalide target attribute value", elem);
            return NULL;
        }
        else
        {
            prop->is_no_target = 1;
        }
    }

    if (parseMethodArgs(elem, prop, desc, env) == 0)
    {
        return NULL;
    }

    if (lazy_resolve)
    {
        return prop;
    }

    if (prop->resolve_inject_method(env) == 0)
    {
        POCO_String msg = env->get_message();
        env->reset();
        POCO_AppEnvImpl::fatal_error(env, msg.in(), elem);
        return NULL;
    }

    return prop;
}

POCO_PropDesc*
POCO_AppContextImpl::parseOnError(POCO_DOM::Element* elem, POCO_BeanDesc* desc,
        POCO_AppEnv* env)
{
    const char* attr = elem->getAttribute(TOKEN_IOC_METHOD);
    if (attr == NULL)
    {
        POCO_AppEnvImpl::fatal_error(env,
                "[E013] property missing ioc method attribute", elem);
        return NULL;
    }

    POCO_PropDesc* prop = new POCO_PropDesc(this);
    prop->is_property = 1;
    prop->is_no_target = 1;
    prop->factory_bean = NULL;
    prop->this_bean = NULL;
    prop->factory_method_name = attr;
    prop->factory_method_name.replace('{', '<');
    prop->factory_method_name.replace('}', '>');

    if (parseMethodArgs(elem, prop, desc, env) == 0)
    {
        return NULL;
    }

    if (lazy_resolve)
    {
        return prop;
    }

    if (prop->resolve_inject_method(env) == 0)
    {
        POCO_String msg = env->get_message();
        env->reset();
        POCO_AppEnvImpl::fatal_error(env, msg.in(), elem);
        return NULL;
    }

    return prop;
}

int
POCO_AppContextImpl::parseDupMethod(POCO_DOM::Element* elem,
        POCO_BeanDesc* desc, POCO_AppEnv* env)
{
    desc->is_self_dup = 1;
    desc->dup_method_name = elem->getAttribute(TOKEN_SELF_DUP_METHOD);
    desc->dup_method_name.replace('{', '<');
    desc->dup_method_name.replace('}', '>');

    if (desc->dup_method_name.in() == NULL)
    {
        desc->is_self_dup = 0;
        desc->dup_method_name = elem->getAttribute(TOKEN_DUP_METHOD);
        desc->dup_method_name.replace('{', '<');
        desc->dup_method_name.replace('}', '>');

        if (desc->dup_method_name.in() == NULL)
        {
            return 1;
        }
    }

    if (desc->bean_class_ptr.in() == NULL)
    {
        POCO_AppEnvImpl::fatal_error(env, "[E015] dup-method is specified on "
                "a bean without a qualified class name", elem);
        return 0;
    }

    POCO_String signature;

    if (desc->is_self_dup)
    {
        signature = desc->dup_method_name.in();
        signature += "()";

        desc->dup_method = POCO_Reflection::get_method(NULL,
                desc->bean_class_ptr.in(), signature.in());
    }
    else
    {
        signature = desc->dup_method_name.in();
        signature += "(";
        signature += desc->bean_class_ptr.in();
        signature += ")";

        desc->dup_method = POCO_Reflection::get_method(
                desc->bean_class_ptr.in(), NULL, signature.in());
    }

    if (desc->dup_method == NULL)
    {
        POCO_String msg = (const char*) "[E016] fail to resolve the "
                "dynamic proxy of the duplication method \"";
        msg += signature.in();
        msg += "\"";

        POCO_AppEnvImpl::fatal_error(env, msg.in(), elem);
        return 0;
    }

    return 1;
}

int
POCO_AppContextImpl::parseReleaseMethod(POCO_DOM::Element* elem,
        POCO_BeanDesc* desc, POCO_AppEnv* env)
{
    int self_destroy = 1;

    const char* attr = elem->getAttribute(TOKEN_SELF_DESTROY_METHOD);
    if (attr == NULL)
    {
        attr = elem->getAttribute(TOKEN_DESTROY_METHOD);
        self_destroy = 0;
    }

    if (attr == NULL)
    {
        // neither self-destroy-method nor destroy-method is
        // specified, safely ignore this case.
        return 1;
    }

    int is_void_bean = 0;
    if (desc->bean_class_ptr.in() == NULL)
    {
        is_void_bean = 1;
        /**
         POCO_AppEnvImpl::fatal_error(env,
         "[E017] release method is specified on "
         "a bean without a qualified class name", elem);
         return 0;
         **/
    }

    desc->destroy_method_name = attr;
    desc->destroy_method_name.replace('{', '<');
    desc->destroy_method_name.replace('}', '>');

    if (self_destroy)
    {
        attr = "self"; // set it to non-NULL, only to mark there is a release bean.
        if (is_void_bean)
        {
            POCO_AppEnvImpl::fatal_error(env,
                    "[E017] self destroy method is specified on a void bean",
                    elem);
            return 0;
        }
    }
    else
    {
        attr = elem->getAttribute(TOKEN_DESTROY_BEAN);
    }

    POCO_String signature = desc->destroy_method_name.in();

    if (attr == NULL)
    {
        //
        // no release bean, implies static call release(this_bean);
        //
        signature += "(";
        if (!is_void_bean)
        {
            signature += desc->bean_class_ptr.in();
        }
        signature += ")";
        desc->destroy_method = POCO_Reflection::get_method(NULL, NULL,
                signature.in());

        if (desc->destroy_method == NULL)
        {
            POCO_String msg =
                    (const char*) "[E018] fail to resolve the dynamic "
                            "proxy of the release-method \"";
            msg += signature.in();
            msg += "\"";
            POCO_AppEnvImpl::fatal_error(env, msg.in(), elem);
            return 0;
        }
    }
    else
    {
        POCO_BeanDesc* dtor_bean;

        if (self_destroy)
        {
            dtor_bean = desc;
        }
        else
        {
            dtor_bean = (POCO_BeanDesc*) bean_id_map.get(attr);
        }

        if (dtor_bean == desc)
        {
            if (is_void_bean)
            {
                POCO_AppEnvImpl::fatal_error(env,
                        "[E017] self destroy method is specified on a void bean",
                        elem);
                return 0;
            }

            //
            // this_bean->destroy_method()
            //
            signature += "()";
            desc->destroy_method = POCO_Reflection::get_method(NULL,
                    desc->bean_class_ptr.in(), signature.in());

            if (desc->destroy_method == NULL)
            {
                POCO_String msg =
                        (const char*) "[E018] fail to resolve the dynamic "
                                "proxy of the release-method \"";
                msg += signature.in();
                msg += "\"";
                POCO_AppEnvImpl::fatal_error(env, msg.in(), elem);
                return 0;
            }
        }
        else
        {
            //
            // releae_bean->destroy_method(the_bean);
            //
            if (dtor_bean->bean_class_ptr.in() == NULL)
            {
                POCO_AppEnvImpl::fatal_error(env,
                        "[E018] a void bean is used as release bean", elem);
                return 0;
            }

            signature += "(";
            if (!is_void_bean)
            {
                signature += desc->bean_class_ptr.in();
            }
            signature += ")";
            desc->destroy_method = POCO_Reflection::get_method(NULL,
                    dtor_bean->bean_class_ptr.in(), signature.in());

            if (desc->destroy_method == NULL)
            {
                POCO_String msg =
                        (const char*) "[E018] fail to resolve the dynamic "
                                "proxy of the release-method \"";
                msg += signature.in();
                msg += "\"";
                POCO_AppEnvImpl::fatal_error(env, msg.in(), elem);
                return 0;
            }
        }

        desc->destroy_bean = dtor_bean;
    }

    return 1;
}

POCO_ArgDesc*
POCO_AppContextImpl::parseArg(POCO_DOM_Element* elem, POCO_BeanDesc* this_bean,
        POCO_AppEnv* env)
{
    POCO_ArgDesc* desc = new POCO_ArgDesc();

    desc->is_index_arg = (strcmp(elem->getTagName(), TOKEN_INDEX_ARG) == 0);

    desc->type = elem->getAttribute(TOKEN_TYPE);

    if (desc->type.in() == NULL)
    {
        if (desc->is_index_arg)
        {
            POCO_AppEnvImpl::fatal_error(env,
                    "[E043] type attribute of index-arg is "
                            "missing", elem);
            return NULL;
        }

        desc->type = TOKEN_BEAN;
    }

    desc->is_bean = (strcmp(desc->type.in(), TOKEN_BEAN) == 0);
    desc->is_array = (strcmp(desc->type.in(), TOKEN_ARRAY) == 0);

    const char* attr = NULL;

    if (desc->is_bean)
    {
        attr = elem->getAttribute(TOKEN_CLASS);
        desc->class_ptr = POCO_Util::class_to_ptr(attr);
        desc->class_ptr.replace('{', '<');
        desc->class_ptr.replace('}', '>');
        if (attr != NULL && desc->class_ptr.in() == NULL)
        {
            delete desc;
            POCO_AppEnvImpl::fatal_error(env,
                    "[E019] a bean argument is specified with "
                            "an invalid class name", elem);
            return NULL;
        }

        attr = elem->getAttribute(TOKEN_PASS);

        if (attr == NULL)
        {
            attr = TOKEN_PTR;
        }
        else if (strcmp(attr, TOKEN_DUP) == 0)
        {
            desc->pass_syntax = POCO_ArgDesc::PASS_DUP;
        }
        else if (strcmp(attr, TOKEN_DEREF) == 0)
        {
            desc->pass_syntax = POCO_ArgDesc::PASS_DEREF;
        }

        desc->valdesc = parseBeanRef(elem, desc->class_ptr.in(), this_bean,
                env);

        if (desc->valdesc == NULL)
        {
            return NULL;
        }

        if (desc->class_ptr.in() == NULL)
        {
            if (desc->valdesc->bean == NULL
                    || desc->valdesc->bean->bean_class_ptr.in() == NULL)
            {
                delete desc;
                POCO_AppEnvImpl::fatal_error(env,
                        "[E019] the bean argument needs "
                                "a qualified class name", elem);
                return NULL;
            }

            desc->class_ptr = desc->valdesc->bean->bean_class_ptr.in();
        }
    }
    else if (desc->is_array)
    {
        POCO_DOM::Element* child = elem->getChild(0);

        if (child == NULL || strcmp(child->getTagName(), TOKEN_ARRAY) != 0)
        {
            delete desc;
            POCO_AppEnvImpl::fatal_error(env,
                    "[E020] an expected <array> element "
                            "is missing", elem);

            return NULL;
        }

        int is_bean_array = 0;
        int is_namevalue_array = 0;
        desc->type = child->getAttribute(TOKEN_TYPE);

        if (desc->type.in() == NULL)
        {
            desc->type = TOKEN_BEAN;
        }

        if (strcmp(desc->type.in(), TOKEN_BEAN) == 0)
        {
            attr = child->getAttribute(TOKEN_CLASS);
            desc->class_ptr = POCO_Util::class_to_ptr(attr);
            desc->class_ptr.replace('{', '<');
            desc->class_ptr.replace('}', '>');
            if (desc->class_ptr.in() == NULL)
            {
                delete desc;
                POCO_AppEnvImpl::fatal_error(env,
                        "[E021] a bean array is specified "
                                "without a qualified class name", elem);
                return NULL;
            }

            is_bean_array = 1;
        }
        else if (strcmp(desc->type.in(), TOKEN_NAMEVALUE) == 0)
        {
            is_namevalue_array = 1;
        }

        POCO_Table table;

        attr = child->getAttribute(TOKEN_ENV_VAR);
        if (!is_namevalue_array && !is_bean_array && attr
                && env->getArray(attr))
        {
            const char** arr = env->getArray(attr);

            for (int i = 0; arr[i] != NULL; i++)
            {
                POCO_ValDesc* vdesc = NULL;

                vdesc = getAndConvertValue(arr[i], NULL, desc->type.in(), env);

                if (vdesc == NULL)
                {
                    //
                    // cleanup and return failure
                    //
                    for (;;)
                    {
                        vdesc = (POCO_ValDesc*) table.remove(0);
                        if (vdesc == NULL)
                        {
                            break;
                        }
                        delete vdesc;
                    }
                    delete desc;
                    return NULL;
                }

                table.put(vdesc);
            }
        }
        else
        {
            for (int i = 0; i < child->numOfChildElements(); i++)
            {
                POCO_DOM::Element* item = child->getChild(i);
                if (strcmp(item->getTagName(), TOKEN_ITEM) != 0)
                {
                    continue;
                }

                POCO_ValDesc* vdesc = NULL;

                if (is_bean_array)
                {
                    vdesc = parseBeanRef(item, desc->class_ptr.in(), this_bean,
                            env);
                }
                else
                {
                    vdesc = parseValue(item, desc->type.in(), env);
                }

                if (vdesc == NULL)
                {
                    //
                    // cleanup and return failure
                    //
                    for (;;)
                    {
                        vdesc = (POCO_ValDesc*) table.remove(0);
                        if (vdesc == NULL)
                        {
                            break;
                        }
                        delete vdesc;
                    }
                    delete desc;
                    return NULL;
                }

                table.put(vdesc);
            }
        }

        desc->valdesc_array_size = table.count();
        desc->valdesc_array = new POCO_ValDesc*[table.count()];

        for (int i = 0; i < table.count(); i++)
        {
            desc->valdesc_array[i] = (POCO_ValDesc*) table.get(i);
        }
    }
    else
    {
        desc->valdesc = parseValue(elem, desc->type.in(), env);

        if (desc->valdesc == NULL)
        {
            delete desc;
            return NULL;
        }
    }

    return desc;
}

POCO_ValDesc*
POCO_AppContextImpl::parseBeanRef(POCO_DOM::Element* elem,
        const char* class_ptr, POCO_BeanDesc* this_bean, POCO_AppEnv* env)
{
    const char* attr = elem->getAttribute(TOKEN_REF);
    POCO_BeanDesc* bean = NULL;

    if (attr != NULL)
    {
        //
        // ref attribute
        //
        bean = (POCO_BeanDesc*) bean_id_map.get(attr);

        if (bean == NULL)
        {
            POCO_AppEnvImpl::fatal_error(env,
                    "[E022] the bean ref'ed by the 'ref' "
                            "attribute does not exist in this "
                            "context", elem);
            return NULL;
        }
    }
    else
    {
        POCO_DOM::Element* child = elem->getChild(0);

        if (child == NULL)
        {
            bean = NULL; // null bean
        }
        else
        {
            const char* tag = child->getTagName();

            if (strcmp(tag, TOKEN_REF) == 0)
            {
                attr = child->getAttribute(TOKEN_BEAN);

                if (attr == NULL)
                {
                    POCO_AppEnvImpl::fatal_error(env,
                            "[E023] the 'bean' attribute in the "
                                    "<ref> element is missing", child);
                    return NULL;
                }

                bean = (POCO_BeanDesc*) bean_id_map.get(attr);

                if (bean == NULL)
                {
                    POCO_AppEnvImpl::fatal_error(env,
                            "[E024] the bean ref'ed by "
                                    "the 'bean' attribute of the <ref> "
                                    "element does not exist in this "
                                    "context", elem);
                    return NULL;
                }
            }
            else if (strcmp(tag, TOKEN_BEAN) == 0)
            {
                bean = parseBean(child, env);

                if (bean == NULL)
                {
                    return NULL;
                }
            }
            else if (strcmp(tag, TOKEN_THIS_BEAN) == 0)
            {
                if (this_bean == NULL)
                {
                    POCO_AppEnvImpl::fatal_error(env,
                            "[E025] invalid use of <this-bean> element", elem);
                    return NULL;
                }

                bean = this_bean;
            }
            else
            {
                POCO_AppEnvImpl::fatal_error(env,
                        "[E025] invalid child element", elem);
                return NULL;
            }
        }
    }

    POCO_ValDesc* val = new POCO_ValDesc;

    if (bean != NULL)
    {
        if (class_ptr != NULL
                && strcmp(class_ptr, bean->bean_class_ptr.in()) != 0)
        {

            val->cast_method = POCO_Reflection::get_typecast_method(class_ptr,
                    bean->bean_class_ptr.in());

            if (val->cast_method == NULL)
            {
                POCO_String msg = (const char*) "[E026] fail to resolve the "
                        "dynamic proxy of the type "
                        "cast method \"";
                msg += "(";
                msg += class_ptr;
                msg += ")(";
                msg += bean->bean_class_ptr.in();
                msg += ")\"";
                delete val;
                POCO_AppEnvImpl::fatal_error(env, msg.in(), elem);
                return NULL;
            }
        }

        val->bean = bean;
    }
    else
    {
        //
        // this is fine -- a null bean is valid argument
        //
    }

    return val;
}

POCO_ValDesc*
POCO_AppContextImpl::parseValue(POCO_DOM::Element* elem, const char* type,
        POCO_AppEnv* env)
{
    const char* attr = elem->getAttribute(TOKEN_ENV_VAR);

    if (attr != NULL)
    {
        attr = env->getValue(attr);
    }

    if (attr == NULL)
    {
        attr = elem->getAttribute(TOKEN_VALUE);
    }

    // no need to check attr against NULL. A NULL value implies
    // default value.
    return getAndConvertValue(attr, elem, type, env);
}

POCO_ValDesc*
POCO_AppContextImpl::getAndConvertValue(const char* attr,
        POCO_DOM::Element* elem, const char* type, POCO_AppEnv* env)
{
    POCO_ValDesc* val = new POCO_ValDesc;

    if (strcmp(type, TOKEN_BYTE) == 0 || strcmp(type, TOKEN_UCHAR) == 0)
    {
        if (POCO_CVT::str2uchar(attr, val->uchar_val) == 1)
        {
            val->val_addr = &(val->uchar_val);
            return val;
        }
    }
    else if (strcmp(type, TOKEN_CHAR) == 0)
    {
        if (POCO_CVT::str2char(attr, val->char_val) == 1)
        {
            val->val_addr = &(val->char_val);
            return val;
        }
    }
    else if (strcmp(type, TOKEN_SHORT) == 0)
    {
        if (POCO_CVT::str2short(attr, val->short_val) == 1)
        {
            val->val_addr = &(val->short_val);
            return val;
        }
    }
    else if (strcmp(type, TOKEN_USHORT) == 0)
    {
        if (POCO_CVT::str2ushort(attr, val->ushort_val) == 1)
        {
            val->val_addr = &(val->ushort_val);
            return val;
        }
    }
    else if (strcmp(type, TOKEN_LONG) == 0)
    {
        if (POCO_CVT::str2long(attr, val->long_val) == 1)
        {
            val->val_addr = &(val->long_val);
            return val;
        }
    }
    else if (strcmp(type, TOKEN_ULONG) == 0)
    {
        if (POCO_CVT::str2ulong(attr, val->ulong_val) == 1)
        {
            val->val_addr = &(val->ulong_val);
            return val;
        }
    }
    else if (strcmp(type, TOKEN_FLOAT) == 0)
    {
        if (POCO_CVT::str2float(attr, val->float_val) == 1)
        {
            val->val_addr = &(val->float_val);
            return val;
        }
    }
    else if (strcmp(type, TOKEN_DOUBLE) == 0)
    {
        if (POCO_CVT::str2double(attr, val->double_val) == 1)
        {
            val->val_addr = &(val->double_val);
            return val;
        }
    }
    else if (strcmp(type, TOKEN_STRING) == 0)
    {
        val->string_val = (const char*) attr;
        val->val_addr = (void*) (val->string_val.inout());
        return val;
    }
    else if (strcmp(type, TOKEN_CSTRING) == 0)
    {
        val->string_val = POCO_Util::cvtcstr(attr);
        val->val_addr = (void*) (val->string_val.inout());
        return val;
    }
    else if (elem && strcmp(type, TOKEN_NAMEVALUE) == 0)
    {
        val->string_val = (const char*) attr;
        val->val_addr = (void*) (val->string_val.inout());

        attr = elem->getAttribute(TOKEN_NAME);

        if (attr == NULL)
        {
            delete val;
            POCO_AppEnvImpl::fatal_error(env,
                    "[E027] the 'name' attribute of a namevalue "
                            "array item is missing", elem);
            return NULL;
        }

        val->name = attr;
        return val;
    }
    else
    {
        delete val;
        POCO_AppEnvImpl::fatal_error(env,
                "[E028] the value of 'type' attribute is invalid", elem);
        return NULL;
    }

    delete val;
    POCO_AppEnvImpl::fatal_error(env,
            "[E029] the value of the 'value' attribute is invalid", elem);
    return NULL;
}

POCO_BeanDesc*
POCO_AppContextImpl::getBeanDesc(const char* id, POCO_AppEnv* env)
{
    if (is_closed)
    {
        POCO_AppEnvImpl::fatal_error(env,
                "[E030] application context closed already");
        return NULL;
    }

    if (id == NULL)
    {
        POCO_AppEnvImpl::fatal_error(env, "[E031] invalid bean id");
        return NULL;
    }

    POCO_BeanDesc* desc = (POCO_BeanDesc*) bean_id_map.get(id);
    if (desc != NULL)
    {
        return desc;
    }

    // if not found, search the child context
    for (int i = 0; i < imported_ctxts.count(); i++)
    {
        POCO_AppContextImpl* ctxt = (POCO_AppContextImpl*) imported_ctxts.get(
                i);

        env->reset();
        desc = ctxt->getBeanDesc(id, env);

        if (desc != NULL)
        {
            break;
        }
    }

    if (desc == NULL)
    {
        POCO_AppEnvImpl::fatal_error(env,
                "[E032] bean with the specified id not found");
    }

    return desc;
}

void*
POCO_AppContextImpl::getBean(const char* id, POCO_AppEnv* env)
{
    if (env == NULL)
    {
        env = POCO_AppContext::getDefaultAppEnv();
    }

    POCO_BeanDesc* desc = getBeanDesc(id, env);

    if (desc == NULL || !desc->is_public)
    {
        return NULL;
    }

    void* bean;

    /*
     * see user guide section 3.8.
     */
    if (desc->is_singleton)
    {
        bean = desc->get_bean_dup(env);
    }
    else
    {
        bean = desc->get_bean_nodup(env);
    }

    return bean;
}

void*
POCO_AppContextImpl::getBean(const char* id, const char* class_name,
        POCO_AppEnv* env)
{
    if (env == NULL)
    {
        env = POCO_AppContext::getDefaultAppEnv();
    }

    POCO_String class_ptr_name = POCO_Util::class_to_ptr(class_name);

    if (class_ptr_name.in() == NULL)
    {
        POCO_AppEnvImpl::fatal_error(env,
                "[E033] invalide bean id in calling getBean()");
        return NULL;
    }

    POCO_BeanDesc* desc = getBeanDesc(id, env);

    if (desc == NULL || !desc->is_public)
    {
        return NULL;
    }

    POCO_proxy_t typecast_method = NULL;

    if (strcmp(desc->bean_class_ptr.in(), class_ptr_name.in()) != 0)
    {
        POCO_proxy_t typecast_method = POCO_Reflection::get_typecast_method(
                class_ptr_name.in(), desc->bean_class_ptr.in());

        if (typecast_method == NULL)
        {
            POCO_AppEnvImpl::fatal_error(env,
                    "[E034] fail to resolve needed typecast "
                            "operation in calling getBean()");

            return NULL;
        }
    }

    void* bean = NULL;

    /*
     * see user guide section 3.8
     */
    if (desc->is_singleton)
    {
        bean = desc->get_bean_dup(env);
    }
    else
    {
        bean = desc->get_bean_nodup(env);
    }

    if (typecast_method == NULL)
    {
        // No need to do cast
        return bean;
    }

    void* params[] = { bean };
    return typecast_method(NULL, params);
}

const char*
POCO_AppContextImpl::getBeanPtrTypeId(const char* id, POCO_AppEnv* env)
{
    if (env == NULL)
    {
        env = POCO_AppContext::getDefaultAppEnv();
    }

    POCO_BeanDesc* desc = getBeanDesc(id, env);

    if (desc == NULL)
    {
        return NULL;
    }

    const char* bean_class_ptr = desc->bean_class_ptr.in();

    return bean_class_ptr;
}

int
POCO_AppContextImpl::initSingletons(POCO_AppEnv* env)
{
    if (env == NULL)
    {
        env = POCO_AppContext::getDefaultAppEnv();
    }

    if (is_closed)
    {
        return 0;
    }

    int flag = 1;
    int i;

    //
    // instantiate imported context first
    //
    for (i = 0; i < imported_ctxts.count(); i++)
    {
        POCO_AppContextImpl* ctxt = (POCO_AppContextImpl*) imported_ctxts.get(
                i);

        if (ctxt->initSingletons(env) == 0)
        {
            flag = 0;
        }
    }

    for (i = 0; i < singleton_beans.count(); i++)
    {
        POCO_BeanDesc* desc = (POCO_BeanDesc*) singleton_beans.get(i);

        if (desc->init(env) == 0)
        {
            return 0;
            // flag = 0;
        }
    }

    return flag;
}

int
POCO_AppContextImpl::terminate(POCO_AppEnv* env)
{
    if (env == NULL)
    {
        env = POCO_AppContext::getDefaultAppEnv();
    }

    is_closed = 1;

    int flag = 1;
    int i, count = bean_term_stack.count();

    //
    // terminate singletons in FILO order
    //
    for (i = count; i != 0; i--)
    {
        POCO_BeanDesc* desc = (POCO_BeanDesc*) bean_term_stack.get(i - 1);

        if (desc->fini(env) == 0)
        {
            flag = 0;
        }
    }

    for (i = 0; i < imported_ctxts.count(); i++)
    {
        POCO_AppContextImpl* ctxt = (POCO_AppContextImpl*) imported_ctxts.get(
                i);

        if (ctxt->terminate(env) == 0)
        {
            flag = 0;
        }
    }

    return flag;
}

static POCO_AppContextImpl*
create_context(POCO_DOM_Element* desc, POCO_AppEnv* env);

int
POCO_AppContextImpl::parseImport(POCO_DOM::Element* elem, POCO_AppEnv* env)
{
    POCO_AppContextImpl* ctxt = NULL;

    POCO_DOM::Element* imported_dom = elem->getChild(0);

    if (imported_dom != NULL)
    {
        ctxt = create_context(imported_dom, env);
    }
    else
    {
        const char* res_type = elem->getAttribute(TOKEN_TYPE);

        if (res_type == NULL || strcmp(res_type, "file") == 0)
        {
            ctxt = create_from_file(elem->getAttribute(TOKEN_RESOURCE), env);
        }
        else
        {
            // todo -- Ke
            POCO_String msg =
                    (const char*) "[E300] unsupported import resource type \"";
            msg += res_type;
            msg += "\"";
            POCO_AppEnvImpl::fatal_error(env, msg.in(), elem);
        }
    }

    if (ctxt == NULL)
    {
        return 0;
    }

    imported_ctxts.put(ctxt);

    return 1;
}

int
POCO_AppContextImpl::parseLoadLib(POCO_DOM::Element* elem, POCO_AppEnv* env)
{
    const char* attr = elem->getAttribute(TOKEN_LIBRARY);

    if (attr == NULL)
    {
        return 1;
    }

    return 0;
}

static POCO_Hashtable&
_desc_readers()
{
    static POCO_Hashtable table;

    return table;
}

extern "C"
{
    int
    pocomatic_register_desc_reader(POCO_AppDescReader* reader)
    {
        if (reader == NULL)
        {
            return 0;
        }

        return _desc_readers().put(reader->tid(), reader);
    }

}
;

#if defined(WIN32)
# if defined(_DEBUG)
#   define POCOXSL_LIB	"pocoxsld.dll"
# else
#   define POCOXSL_LIB	"pocoxsl.dll"
# endif
#else
# define POCOXSL_LIB	"libpocoxsl.so"
#endif

static POCO_AppDescReader*
get_desc_reader(const char* tid, POCO_AppEnv* env)
{
    POCO_AppDescReader* reader = (POCO_AppDescReader*) _desc_readers().get(tid);
    if (reader == NULL && strcmp(tid, "xml") == 0)
    {
        //POCO_LibLoader::load(POCOXSL_LIB, env);
        reader = (POCO_AppDescReader*) _desc_readers().get(tid);
    }

    if (reader == NULL)
    {
        POCO_AppEnvImpl::fatal_error(env,
                "[E100] specified application descriptor "
                        "reader not found");
    }

    return reader;
}

static POCO_AppContextImpl*
create_context(POCO_DOM_Element* desc, POCO_AppEnv* env)
{
    POCO_AppContextImpl* ctxt = new POCO_AppContextImpl;

    if (ctxt->parseDoc(desc, env) == 0)
    {
        delete ctxt;
        return NULL;
    }

    return ctxt;
}

POCO_AppContextImpl*
POCO_AppContextImpl::create_from_file(const char* loc, POCO_AppEnv* env)
{
    if (env == NULL)
    {
        env = getDefaultAppEnv();
    }

    FILE* strm = fopen(loc, "r");
    if (strm == NULL)
    {
        POCO_String msg =
                (const char*) "[E043] fail to open the descriptor file ";
        msg += loc;
        POCO_AppEnvImpl::fatal_error(env, msg.in());
        return NULL;
    }

    char buf[16];
    if (fgets(buf, 15, strm) == NULL)
    {
        POCO_String msg = (const char*) "[E042] bad descriptor file ";
        msg += loc;
        POCO_AppEnvImpl::fatal_error(env, msg.in());
        fclose(strm);
        return NULL;
    }

    fclose(strm);

    const char* tid = NULL;

    if (strncmp(buf, "<?xml", 5) == 0)
    {
        tid = "xml";
    }
    else if (strncmp(buf, "pococtx:", 8) == 0)
    {
        tid = "ctx";
    }
    else
    {
        POCO_String msg =
                (const char*) "[E041] the type of the descriptor in file ";
        msg += loc;
        msg += " is not supported";
        POCO_AppEnvImpl::fatal_error(env, msg.in());
        return NULL;
    }

    POCO_AppDescReader* reader = get_desc_reader(tid, env);

    if (reader == NULL)
    {
        return NULL;
    }

    POCO_DOM_Element* desc = reader->read(loc, env);

    if (desc == NULL)
    {
        return NULL;
    }

    POCO_AppContextImpl* ctxt = create_context(desc, env);
    delete desc;
    return ctxt;
}

POCO_AppContextImpl*
POCO_AppContextImpl::create_from_string(const char* buf, // memory buffer
        POCO_AppEnv* env)
{
    if (env == NULL)
    {
        env = getDefaultAppEnv();
    }

    if (buf == NULL || strlen(buf) < 32)
    {
        POCO_AppEnvImpl::fatal_error(env, "[E035] bad descriptor buffer");
        return NULL;
    }

    const char* tid = NULL;

    if (strncmp(buf, "<?xml", 5) == 0)
    {
        tid = "xml";
    }
    else if (strncmp(buf, "pococtx:", 8) == 0)
    {
        tid = "ctx";
    }
    else if (strncmp(buf, "str-ctx:", 8) == 0)
    {
        tid = "ctx"; // str-ctx is only possible from buffer input
    }
    else
    {
        POCO_AppEnvImpl::fatal_error(env,
                "[E036] the type of the descriptor buffer is not supported");
        return NULL;
    }

    POCO_AppDescReader* reader = get_desc_reader(tid, env);

    if (reader == NULL)
    {
        return NULL;
    }

    POCO_DOM_Element* desc = reader->read(buf, strlen(buf), env);
    POCO_AppContextImpl* ctxt = create_context(desc, env);
    delete desc;
    return ctxt;
}

void
POCO_AppContextImpl::add_bean(POCO_BeanDesc* bean)
{
    all_beans.put(bean);
}

void
POCO_AppContextImpl::push_inited_singleton(POCO_BeanDesc* bean)
{
    bean_term_stack.put(bean);
}

void
POCO_AppContextImpl::add_keyed_bean(POCO_BeanDesc* bean)
{
    keyed_beans.put(bean->the_key.in(), bean);
}

POCO_BeanDesc*
POCO_AppContextImpl::get_keyed_bean(const char* key)
{
    return (POCO_BeanDesc*) keyed_beans.get(key);
}

void
POCO_AppContextImpl::destroy()
{
    for (int i = all_beans.count(); i; i--)
    {
        POCO_BeanDesc* bean = (POCO_BeanDesc*) all_beans.get(i - 1);
        bean->destroy();
    }

    for (int i = 0; i < imported_ctxts.count(); i++)
    {
        POCO_AppContextImpl* ctxt = (POCO_AppContextImpl*) imported_ctxts.get(
                i);

        ctxt->destroy();
    }

    delete this;
}

//
// factory wrapper repository
//

static POCO_Hashtable factory_wrappers;

POCO_factory_wrapper_t
resolve_factory_wrapper(const char* id)
{
    return (POCO_factory_wrapper_t) factory_wrappers.get(id);
}

int
pocomatic_register_factory_wrapper(const char* id,
        POCO_factory_wrapper_t wrapper)
{
    return factory_wrappers.put(id, (void*) wrapper);
}
