#ifndef _poco_string_h_
# define _poco_string_h_

#include "pocotypes.h"

class _POCO_CAPSULE_EXPORT POCO_String {
	char*	_str;
  public:
	POCO_String();
	POCO_String(char*);
	POCO_String(const char*);
	POCO_String(const char**);
	~POCO_String();

	const char* in() { return _str; }
	char*&	    inout() { return _str; }
	char*	    retn();

	POCO_String& operator=(const char*);
	POCO_String& operator=(char*);
	
	operator const char *() const { return _str; }

	// POCO_String& operator+(const char*);
	POCO_String& operator+=(const char*);

	static char*	alloc(int size);
	static char*	dup(const char* str);
	static void	free(char* str);

	void replace(char old_char, char new_char);
};

#endif
