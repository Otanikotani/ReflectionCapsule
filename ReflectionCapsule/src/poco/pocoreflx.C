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

#include "pocoreflx.h"
#include <stdlib.h>
#include <string.h>
#include "pocostr.h"
#include "table.h"
#include "hashtable.h"

#include <stdio.h>

#include <string>
#include <vector>

#include <iostream>
#ifdef DEBUG
# include <debug.h>
#endif
struct POCO_ProxyDesc
{
    int kind;
    std::string bean_class_ptr;
    std::string factory_class_ptr;
    std::string signature;
    POCO_proxy_t proxy;
    std::string filename;
    std::string proxy_name;

    std::string key;
//    POCO_String bean_class_ptr;
//    POCO_String factory_class_ptr;
//    POCO_String signature;
//    POCO_proxy_t proxy;
//    POCO_String filename;
//    POCO_String proxy_name;
//
//    POCO_String key;
};

static POCO_Hashtable&
theProxyTable()
{
    static POCO_Hashtable table;

    return table;
}

POCO_proxy_t
POCO_Reflection::get_method(const char* ret_class_ptr,
        const char* this_class_ptr, const char* signature)
{
    POCO_String key = signature;

    key += "@";
    key += this_class_ptr;
    key += "@";
    key += ret_class_ptr;

    //printf("looking method %s\n",  key.in());

    POCO_Hashtable& table = theProxyTable();

    POCO_ProxyDesc* desc = (POCO_ProxyDesc*) table.get(key.in());

    if (desc == NULL)
    {
        return NULL;
    }

    return desc->proxy;
}

POCO_proxy_t
POCO_Reflection::get_typecast_method(const char* to_class_ptr,
        const char* from_class_ptr)
{
    POCO_String key;
    key += "(";
    key += to_class_ptr;
    key += ")(";
    key += from_class_ptr;
    key += ")";
    key += "@@";
    key += to_class_ptr;

    POCO_Hashtable& table = theProxyTable();
    POCO_ProxyDesc* desc = (POCO_ProxyDesc*) table.get(key.in());

    if (desc == NULL)
    {
        return NULL;
    }
    return desc->proxy;
}

class POCO_ProxyDescFactory
{
public:

    POCO_ProxyDesc*
    get()
    {
        objs.push_back(new POCO_ProxyDesc);
        return objs.back();
    }

    ~POCO_ProxyDescFactory()
    {
        for (std::vector<POCO_ProxyDesc*>::iterator it = objs.begin(); it
                != objs.end(); ++it)
        {
            delete *it;
        }
    }

private:
    std::vector<POCO_ProxyDesc*> objs;
};

POCO_ProxyDescFactory*
getFactory()
{
    static POCO_ProxyDescFactory factory;
    return &factory;
}

//
// this method should be exported on win and aix etc...
//
pocomatic_register_proxy::pocomatic_register_proxy(int kind,
        const char* bean_class_ptr, const char* factory_class_ptr,
        const char* signature, POCO_proxy_t proxy, const char* filename,
        const char* proxy_name)
{
    POCO_Hashtable& table = theProxyTable();

    POCO_ProxyDesc* desc = getFactory()->get();

    desc->kind = kind;
    desc->bean_class_ptr = bean_class_ptr;
    desc->factory_class_ptr = factory_class_ptr;
    desc->signature = signature;
    desc->proxy = proxy;
    desc->filename = filename;
    desc->proxy_name = proxy_name;

    desc->key += signature;
    desc->key += "@";
    desc->key += factory_class_ptr;
    desc->key += "@";
    desc->key += bean_class_ptr;

    table.put(desc->key.c_str(), desc);

    if (bean_class_ptr != NULL && bean_class_ptr[0] != 0 && kind == 0)
    {
        ///< Why calling second time??? Makes no sense so far
        pocomatic_register_proxy(0, "", factory_class_ptr, signature, proxy,
                filename, proxy_name);
    }
}
