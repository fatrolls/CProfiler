
#ifndef ADDRESS_2_SYMBOL_H
#define ADDRESS_2_SYMBOL_H

#ifdef WIN32
typedef unsigned __int64 U64;
#elif defined( __GNUC__)
typedef unsigned long long U64;
#endif

#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

class Address2Symbol
{
protected:
	Address2Symbol(){}
public:
	static int create(Address2Symbol** a2s);
	virtual ~Address2Symbol(){}
	virtual BOOL init() = 0;
	virtual char* getSymbol(U64 address) = 0;
	virtual void freeSymbol(char* symbol) = 0;
};

#endif /* ADDRESS_2_SYMBOL_H */
