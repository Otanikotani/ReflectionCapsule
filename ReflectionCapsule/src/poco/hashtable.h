#ifndef _POCO_Hashtable_h_
#define _POCO_Hashtable_h_

#include "pocotypes.h"

struct POCO_KeyValue;

class _POCO_CAPSULE_EXPORT POCO_Hashtable {
	POCO_ULong	_size;
	POCO_ULong	_cnt;

	POCO_KeyValue*	_array;

	void		_enlarge();
	POCO_ULong	_setSize(POCO_ULong n);
	POCO_ULong	_hash(const char* key);
	POCO_ULong	_offsetOf(const char* key);

	int		_shift_bits;
	POCO_ULong	_mask;

  public:
	POCO_Hashtable();
	~POCO_Hashtable();

	int		put(const char* key, void* value);
	void*		get(const char* key);
	void*		remove(const char* key);
	int		containsKey(const char* key);
	int		count();

	const char**	allKeys();
	void**		allValues();
};

#endif
