
#ifndef ARM_MAP_ADDRESS_2_SYMBOL_H
#define ARM_MAP_ADDRESS_2_SYMBOL_H

#include "CommonAddress2Symbol.h"

/* If a function is compiled in thumb mode, 
it's address is a ODD value, we need to convert it a EVEN value(reset the end bit 1 to 0) 
*/
#define EVEN_ADDRESS(x)  ((x) & ~0x1U)

class ArmMapAddress2Symbol : public CommonAddress2Symbol
{
	const char* filename;
public:
	ArmMapAddress2Symbol(): filename("symbols.map") 
	{}
	void setFileName(const char* name) { filename = name; }
	
	virtual BOOL init();
	virtual const Symbol* getSymbolStruct(U64 address);
};

#endif