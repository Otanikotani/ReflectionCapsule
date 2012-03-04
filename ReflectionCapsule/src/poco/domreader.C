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

# include "pocodom.h"
# include "pocostr.h"
# include "domreader.h"
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <ctype.h>
# include <fcntl.h>
# if defined(WIN32)
#   include <io.h>
# else
#   include <unistd.h>
# endif

class POCO_DOM_InputStream
{
public:
    virtual
    ~POCO_DOM_InputStream()
    {
    }
    virtual const char*
    get_string(POCO_AppEnv*) = 0;
    virtual int
    get_int(POCO_AppEnv*) = 0;

    POCO_DOM::Element*
    read_element(POCO_DOM::Element* parent, POCO_AppEnv* env);
};

class POCO_DOM_BufferInputStream : public POCO_DOM_InputStream
{
    const char* buf;
    POCO_String holder;
    int offset;

public:
    POCO_DOM_BufferInputStream(const char* b, int len, POCO_AppEnv*);
    const char*
    get_string(POCO_AppEnv*);
    int
    get_int(POCO_AppEnv*);
};

static unsigned char
hex_decode(char c, POCO_AppEnv* env)
{
    switch (c)
    {
    case '0':
        return 0x00;
    case '1':
        return 0x01;
    case '2':
        return 0x02;
    case '3':
        return 0x03;
    case '4':
        return 0x04;
    case '5':
        return 0x05;
    case '6':
        return 0x06;
    case '7':
        return 0x07;
    case '8':
        return 0x08;
    case '9':
        return 0x09;
    case 'a':
    case 'A':
        return 0x0a;
    case 'b':
    case 'B':
        return 0x0b;
    case 'c':
    case 'C':
        return 0x0c;
    case 'd':
    case 'D':
        return 0x0d;
    case 'e':
    case 'E':
        return 0x0e;
    case 'f':
    case 'F':
        return 0x0f;
    default:
        break;
    }

    env->fatal_error("[E701] bad application context descriptor");
    return 0xff;
}

POCO_DOM_BufferInputStream::POCO_DOM_BufferInputStream(const char* b, int,
        POCO_AppEnv* env)
{
    if (strncmp(b, "str-ctx:", 8) == 0)
    {
        buf = b;
        offset = strlen("str-ctx:") + 1;
        return;
    }

    if (strncmp(b, "pococtx:", 8) != 0)
    {
        env->fatal_error("[E702] bad descriptor");
        return;
    }

    //
    // must be pococtx
    //
    int len = strlen(b);
    holder = POCO_String::alloc((len / 2) + 1);
    buf = holder.in();
    char* ptr = holder.inout();
    offset = 0;

    // Can use stream to morph to string;
    for (int i = 0, j = 8; j < len - 1; i++)
    {
        for (; j < len - 1; j++)
        {
            if (isalnum(b[j]))
            {
                ptr[i] = hex_decode(b[j++], env) * 0x10;
                break;
            }
        }

        for (; j < len - 1; j++)
        {
            if (isalnum(b[j]))
            {
                ptr[i] += hex_decode(b[j++], env);
                break;
            }
        }

        if (env->has_error())
        {
            return;
        }

        // otherwise, ignore (e.g. '\n' etc.)
    }
}

const char*
POCO_DOM_BufferInputStream::get_string(POCO_AppEnv* env)
{
    const char* ptr = buf + offset;
    offset += strlen(ptr) + 1;

    return ptr;
}

int
POCO_DOM_BufferInputStream::get_int(POCO_AppEnv* env)
{
    return atoi(get_string(env));
}

class POCO_DOM_FileInputStream : public POCO_DOM_InputStream
{
    POCO_DOM_BufferInputStream* istrm;

public:
    POCO_DOM_FileInputStream(const char* filename, POCO_AppEnv*);
    ~POCO_DOM_FileInputStream()
    {
        delete istrm;
    }
    const char*
    get_string(POCO_AppEnv* env)
    {
        return istrm->get_string(env);
    }
    int
    get_int(POCO_AppEnv* env)
    {
        return istrm->get_int(env);
    }
};

POCO_DOM_FileInputStream::POCO_DOM_FileInputStream(const char* filename,
        POCO_AppEnv* env)
{
    istrm = NULL;
    int fd = open(filename, O_RDONLY, 0);

    if (fd == -1)
    {
        POCO_String msg = "[E703] Unable to open resource file ";
        msg += filename;
        env->fatal_error(msg.in());
        return;
    }

    long size = lseek(fd, 0, SEEK_END) + 1;
    lseek(fd, 0L, SEEK_SET);

    POCO_String holder = POCO_String::alloc(size + 1);
    char* buf = holder.inout();

    int readResult = read(fd, buf, size);
    if (readResult < 0)
    {
        // TODO: Add error handling
    }
    buf[size] = 0;

    close(fd);

    istrm = new POCO_DOM_BufferInputStream(buf, size, env);
}

const char*
POCO_DOM_Reader::tid()
{
    return "ctx";
}

POCO_DOM::Element*
POCO_DOM_Reader::read(const char* file, POCO_AppEnv* env)
{
    POCO_DOM_FileInputStream istrm(file, env);

    if (env->has_error())
    {
        return NULL;
    }

    return istrm.read_element(NULL, env);
}

POCO_DOM::Element*
POCO_DOM_Reader::read(const char* buf, int len, POCO_AppEnv* env)
{
    if (strncmp(buf, "str-ctx:", 8) == 0 || strncmp(buf, "pococtx:", 8) == 0)
    {
        POCO_DOM_BufferInputStream istrm(buf, len, env);

        return istrm.read_element(NULL, env);
    }

    env->fatal_error("[E704] bad buffer descriptor");
    return NULL;
}

POCO_DOM::Element*
POCO_DOM_InputStream::read_element(POCO_DOM::Element* parent, POCO_AppEnv* env)
{
    POCO_DOM::Element* elem = new POCO_DOM::Element(parent);

    elem->tag_name = get_string(env);
    int count = get_int(env);

    if (count != 0)
    {
        elem->text_content = get_string(env);
    }

    elem->num_attrs = get_int(env);

    if (elem->num_attrs != 0)
    {
        elem->attrs = new POCO_DOM::Attribute*[elem->num_attrs];
    }

    for (int i = 0; i < elem->num_attrs; i++)
    {
        elem->attrs[i] = new POCO_DOM::Attribute;
        elem->attrs[i]->name = get_string(env);
        elem->attrs[i]->value = get_string(env);
    }

    elem->num_child_elems = get_int(env);
    if (elem->num_child_elems != 0)
    {
        elem->child_elems = new POCO_DOM::Element*[elem->num_child_elems];
    }

    for (int i = 0; i < elem->num_child_elems; i++)
    {
        elem->child_elems[i] = read_element(elem, env);
    }

    if (env->has_error())
    {
        delete elem;
        return NULL;
    }

    return elem;
}

POCO_DOM_Reader*
createReader()
{
    static POCO_DOM_Reader reader;
    return &reader;
}

static int retcode = pocomatic_register_desc_reader(createReader());

