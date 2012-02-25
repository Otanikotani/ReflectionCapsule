#ifndef _poco_convert_h_
# define _poco_convert_h_

struct POCO_CVT {
	static int str2char(const char* str, char& c);
	static int str2uchar(const char* str, unsigned char& c);
	static int str2short(const char* str, short& s);
	static int str2ushort(const char* str, unsigned short& s);
	static int str2long(const char* str, long& l);
	static int str2ulong(const char* str, unsigned long& u);
	static int str2float(const char* str, float& f);
	static int str2double(const char* str, double& x);
	static int hex2char(char h, char& c);
};

#endif
