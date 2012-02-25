#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#include <debug.h>
#endif

const POCO_ULong GoldenRatioBits = 2654435769UL;
const int	 MaxShiftBits	    = 8*sizeof(unsigned);

struct POCO_KeyValue {
	const char*	key;
	void*		value;
};

POCO_Hashtable::POCO_Hashtable()
{
	_cnt = 0;
	_size  = _setSize(16);
	_array = new POCO_KeyValue[_size];

	for(unsigned long i=0; i<_size;i++) {
		_array[i].key = NULL;
	}
}

POCO_Hashtable::~POCO_Hashtable()
{
	delete [] _array;
}

int POCO_Hashtable::put(const char* key, void* value)
{
	if( key == NULL || value == NULL ) {
		return 0;
	}

	unsigned long offset = _offsetOf(key);

	if( _array[offset].key == NULL ) {
		_array[offset].key = key;
		_array[offset].value = value;
		if( ++_cnt + 4 > _size ) {
			_enlarge();
		}
		return 1;
	}

	return 0;
}

void* POCO_Hashtable::get(const char* key)
{
	if( key == NULL ) {
		return NULL;
	}
	unsigned long offset = _offsetOf(key);
	if( _array[offset].key == NULL ) {
		return NULL;
	}

	return _array[offset].value;
}

void* POCO_Hashtable::remove(const char* key)
{
	if( key == NULL ) {
		return NULL;
	}

	unsigned long offset = _offsetOf(key);
	if( _array[offset].key == NULL ) {
		return NULL;
	}

	void* value = _array[offset].value;

	POCO_ULong i, h;
	for(;;) {
		_array[i=offset].key = NULL;
		for(;;) {
			offset = (offset-1)&_mask;
			if( _array[offset].key == NULL ) {
				_cnt--;
				return value; // _array[offset].value;
			}
			h = _hash(_array[offset].key);
			if( (offset<=h && h<i)
			 || (h<i && i<offset)
			 || (i<offset && offset<=h) ) {
				continue;
			}

			break;
		}
		_array[i] = _array[offset];
	}

	return value;
}

int POCO_Hashtable::containsKey(const char* key)
{
	if( key == NULL ) {
		return 0;
	}

	unsigned long offset = _offsetOf(key);
	if( _array[offset].key == NULL ) {
		return 0;
	}

	return 1;
}

int POCO_Hashtable::count() {
	return _cnt;
}

const char** POCO_Hashtable::allKeys()
{
	if( _cnt == 0 ) {
		return NULL;
	}

	const char** ret = new const char*[_cnt];

	for(unsigned long i=0, j=0; j<_cnt; i++) {
		if( _array[i].key != NULL ) {
			ret[j++] = _array[i].key;
		}
	}

	return ret;
}

void** POCO_Hashtable::allValues()
{
	if( _cnt == 0 ) {
		return NULL;
	}

	void** ret = new void*[_cnt];

	for(unsigned long i=0, j=0; j<_cnt; i++) {
		if( _array[i].key != NULL ) {
			ret[j++] = _array[i].value;
		}
	}

	return ret;
};

POCO_ULong POCO_Hashtable::_offsetOf(const char* key)
{
	POCO_ULong offset = _hash(key);

	for(;_array[offset].key != NULL; offset=(offset-1)&_mask) {
		if( strcmp(_array[offset].key, key) == 0 ) {
			return offset;
		}
	}

	return offset;
}

POCO_ULong POCO_Hashtable::_hash(const char* key)
{
	if( key == NULL ) {
		return 0UL;
	}

	POCO_ULong h = 0;

	for(;*key;key++) {
		h = (h<<1)^(*key);
	}

	return ((GoldenRatioBits*h) >> (MaxShiftBits - _shift_bits)) & _mask;
}

POCO_ULong POCO_Hashtable::_setSize(POCO_ULong n)
{
	_cnt = 0;
	_shift_bits = 0;

	// find the highest bit
	for(POCO_ULong m=n; m!=0; m>>=1) {
		_shift_bits++;
	}

	if((n&(n-1)) != 0) {
		// add one more shift
		_shift_bits++;
	}

	n = 1<<_shift_bits;
	_mask = n - 1;

	return n;
}

void POCO_Hashtable::_enlarge()
{
	POCO_KeyValue*	old_array = _array;
	POCO_ULong old_size = _size;
	POCO_ULong old_count = _cnt;

	_cnt = old_count;
	_size = _setSize(2*_size);
	_array = new POCO_KeyValue[_size];

	unsigned long i=0;

	for(i=0;i<_size;i++) {
		_array[i].key = NULL;
	}

	for(i=0;i<old_size;i++) {
		if( old_array[i].key != NULL ) {
			put(old_array[i].key, old_array[i].value);
		}
	}

	delete [] old_array;
}
