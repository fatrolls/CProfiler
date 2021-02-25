#include <vector>
#include <algorithm>
#include <assert.h>
using namespace std;
#include "CommonAddress2Symbol.h"

#define ASSERT assert

int Address2Symbol::create(Address2Symbol** p_a2s)
{
	*p_a2s = new CommonAddress2Symbol();
	return 0;
}

CommonAddress2Symbol::CommonAddress2Symbol()
{
	m_symbols = new std::vector<Symbol>(4096);
	m_symbols->clear();
}

CommonAddress2Symbol::~CommonAddress2Symbol()
{
	for(std::vector<Symbol>::iterator it = m_symbols->begin(); it != m_symbols->end(); ++it)
		free(it->symbol);
	delete m_symbols;
}

char* CommonAddress2Symbol::strdup(const char*s)
{
	return ::strdup(s);
}

static bool symbol_cmp(const CommonAddress2Symbol::Symbol &s1, const CommonAddress2Symbol::Symbol &s2)
{
	return s1.address < s2.address;
}

char* CommonAddress2Symbol::getSymbol(U64 address)
{
	const CommonAddress2Symbol::Symbol* s = getSymbolStruct(address);
	return s ? s->symbol : NULL;
}

unsigned int CommonAddress2Symbol::getBaseAddress(U64 address)
{
	const CommonAddress2Symbol::Symbol* s = getSymbolStruct(address);
	return s ? s->address : 0;	
}

const CommonAddress2Symbol::Symbol* CommonAddress2Symbol::getSymbolStruct(U64 address)
{
	Symbol s = {(unsigned int)address, NULL};
	std::vector<Symbol>::iterator it = upper_bound(m_symbols->begin(), m_symbols->end(), s, symbol_cmp);
	if(it == m_symbols->begin())
	{
		return NULL;
	}
	else
	{
		ASSERT(it >m_symbols->begin() && it <= m_symbols->end());
		return &(*(it-1));
	}
}

void CommonAddress2Symbol::sortSymbol()
{
	sort(m_symbols->begin(), m_symbols->end(), symbol_cmp);
}

void CommonAddress2Symbol::freeSymbol(char *symbol)
{
	/* do nothing */
}

BOOL CommonAddress2Symbol::init()
{
	char line[1024];
	unsigned int address = 0;
	char * symbol = NULL;
	char * endp;
	FILE* fin = fopen("symbols.txt", "r");
	if (!fin)
		return FALSE;
	while(NULL != fgets(line, sizeof(line), fin))
	{
		address = strtoul(line, &endp, 16);
		if(endp != line + 8)
			continue;
		line[strlen(line)-1] = 0; //delete tailing \n
		Symbol s = {address, this->strdup(line+ 8 + 3/*space T|t|b|B space*/)};
		m_symbols->push_back(s);
	}
	fclose(fin);
	return TRUE;
}
