#include <vector>
using namespace std;
#include "Address2Symbol.h"

class CommonAddress2Symbol : public Address2Symbol
{
public:
	struct Symbol
	{
		unsigned int address;
		char* symbol;
	};
protected:
	vector<Symbol> * m_symbols;
	char* strdup(const char* s1);
public:
	CommonAddress2Symbol();
	~CommonAddress2Symbol();
	void addSymbol(int address, const char*symbol) 
	{
		Symbol s = {address, this->strdup(symbol)};
		m_symbols->push_back(s); 
	}
	void sortSymbol();
	
	virtual BOOL init();
	virtual char* getSymbol(U64 address);
	virtual void freeSymbol(char *symbol);
	virtual const Symbol* getSymbolStruct(U64 address);
	virtual unsigned getBaseAddress(U64 address);
};