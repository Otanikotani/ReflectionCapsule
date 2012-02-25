#ifndef _poco_dom_h_
# define _poco_dom_h_

#include "pocostr.h"
#include <stdlib.h>

struct POCO_DOM_Attribute {
	POCO_String	name;
	POCO_String	value;
};

class _POCO_CAPSULE_EXPORT POCO_DOM_Element {
	friend class POCO_XML_Reader;
	friend class POCO_DOM_Reader;
	friend class POCO_DOM_InputStream;

	POCO_String		tag_name;
	POCO_String		text_content;

	int			num_attrs;
	POCO_DOM_Attribute**	attrs;

	int			num_child_elems;
	POCO_DOM_Element**	child_elems;

	POCO_DOM_Element*	parent;

   public:
	POCO_DOM_Element(POCO_DOM_Element* the_parent = NULL);
	~POCO_DOM_Element();

	const char*		getTextContent();
	const char*		getTagName();
	const char*		getAttribute(const char* attr_name);
	POCO_DOM_Element*	getParent();
	int			numOfChildElements();
	POCO_DOM_Element*	getChild(int i);
};

struct POCO_DOM {
	typedef POCO_DOM_Element	Element;
	typedef POCO_DOM_Attribute	Attribute;
};

#include "pocoapp.h"

class _POCO_CAPSULE_EXPORT POCO_AppDescReader {
  public:
	virtual ~POCO_AppDescReader() {}
	virtual const char*	  tid() = 0;
	virtual POCO_DOM_Element* read(const char*, POCO_AppEnv*) = 0;
	virtual POCO_DOM_Element* read(const char*, int, POCO_AppEnv*) = 0;
};

extern "C" {
int _POCO_CAPSULE_EXPORT pocomatic_register_desc_reader(POCO_AppDescReader*);
};

#endif
