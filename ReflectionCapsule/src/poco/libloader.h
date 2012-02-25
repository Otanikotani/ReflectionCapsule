#ifndef _poco_lib_loader_h
# define _poco_lib_loader_h

# include <pocoenv.h>

class _POCO_CAPSULE_EXPORT POCO_LibLoader {
  public:
	static int load(const char*, POCO_AppEnv* env);
};

#endif
