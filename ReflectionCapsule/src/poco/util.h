#ifndef _poco_util_h_
# define _poco_util_h_

#include "pocotypes.h"

class _POCO_CAPSULE_EXPORT POCO_Util {
  public:
	static char* class_to_ptr(const char* class_name);
	static char* trim_string(const char* str);
	static char* trim_last_char(const char* str);
	static char* cvtcstr(const char* cstr);
};

#endif
