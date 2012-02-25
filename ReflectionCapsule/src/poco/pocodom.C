#include "pocodom.h"
#include <stdlib.h>
#include <string.h>

POCO_DOM_Element::POCO_DOM_Element(POCO_DOM_Element* the_parent)
{
	num_attrs = 0;
	attrs = NULL;
	num_child_elems = 0;
	child_elems = NULL;
	parent = the_parent;
}

POCO_DOM_Element::~POCO_DOM_Element()
{
	int i;

	for(i=0; i<num_attrs; i++) {
		delete attrs[i];
	}

	if( attrs != NULL ) {
		delete [] attrs;
	}

	for(i=0; i<num_child_elems; i++) {
		delete child_elems[i];
	}

	if( child_elems != NULL ) {
		delete [] child_elems;
	}
}

const char* POCO_DOM_Element::getTextContent()
{
	return text_content.in();
}

const char* POCO_DOM_Element::getTagName()
{
	return tag_name.in();
}

const char* POCO_DOM_Element::getAttribute(const char* attr)
{
	for(int i=0; i<num_attrs; i++) {
		if( strcmp(attrs[i]->name.in(), attr) == 0 ) {
			return attrs[i]->value.in();
		}
	}

	return NULL;
}

POCO_DOM_Element* POCO_DOM_Element::getParent()
{
	return parent;
}

int POCO_DOM_Element::numOfChildElements()
{
	return num_child_elems;
}

POCO_DOM_Element* POCO_DOM_Element::getChild(int i)
{
	if( i < num_child_elems ) {
		return child_elems[i];
	}

	return NULL;
}
