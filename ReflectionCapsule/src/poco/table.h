#ifndef _POCO_Table_h_
#define POCO_Table_h_

#include "pocotypes.h"

class _POCO_CAPSULE_EXPORT POCO_Table {
	POCO_ULong	_size;
	POCO_ULong	_cnt;

	void**		_array;

  public:
	POCO_Table();
	~POCO_Table();

	void		put(void* value);
	void*		get(int);
	void**		get();
	void*		remove(int);
	int		count();
};

#endif
