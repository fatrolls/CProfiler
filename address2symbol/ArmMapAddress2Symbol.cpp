#include <vector>
#include <algorithm>
#include <assert.h>
#include <ctype.h>
using namespace std;
#include "ArmMapAddress2Symbol.h"

#define ASSERT assert

#define SYMBOL_TABLE_STRAT_ID "Image Symbol Table"
#define SYMBOL_TABLE_END_ID "======="

static bool symbol_cmp(const CommonAddress2Symbol::Symbol &s1, const CommonAddress2Symbol::Symbol &s2)
{
	return EVEN_ADDRESS(s1.address) < EVEN_ADDRESS(s2.address);
}

const CommonAddress2Symbol::Symbol* ArmMapAddress2Symbol::getSymbolStruct(U64 address)
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

BOOL ArmMapAddress2Symbol::init()
{
	char line[1024];
	char symbol_buffer[1024], address_buffer[20];
	unsigned int address = 0;
	char * endp;
	FILE* fin = fopen(filename, "r");
	if (!fin)
		return FALSE;
	int start_id_len = strlen(SYMBOL_TABLE_STRAT_ID);
	int end_id_len = strlen(SYMBOL_TABLE_END_ID);
	BOOL in_symbol_regin = FALSE;
	while(NULL != fgets(line, sizeof(line), fin))
	{
		if(in_symbol_regin)
		{
			if (strstr(line, "ARM Code") || strstr(line, "Thumb Code"))
			{
				int ret = sscanf(line, "%s%s", symbol_buffer, address_buffer);
				ASSERT(ret == 2);
				if(ret != 2 || 0 != strncmp("0x", address_buffer, 2))
					continue;
				address = strtoul(address_buffer+2/*remove 0x prefix */, &endp, 16);
				addSymbol((address), symbol_buffer);
			}
		}
		else if(0 == strncmp(line, SYMBOL_TABLE_STRAT_ID, start_id_len))
		{
			in_symbol_regin = TRUE;
		}
		else if(0 == strncmp(line, SYMBOL_TABLE_END_ID, end_id_len) && in_symbol_regin)
		{
			in_symbol_regin = FALSE;
			break;
		}
	}
	sortSymbol();
	fclose(fin);
	return TRUE;
}

#ifdef BUILD_ARM_MAP_ADDRESS2SYMBOL
int main(int argc, char** argv)
{
	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s <map file>\n", argv[0]);
		return 1;
	}
	ArmMapAddress2Symbol a2s;
	a2s.setFileName(argv[1]);
	if(! a2s.init())
	{
		fprintf(stderr, "Parse map file failed\n");
		return 1;
	}
	char line[1024];
	char * address, *address_end;
	while(NULL != fgets(line, sizeof(line), stdin))
	{
		address = strstr(line, "0x");
		if(address == 0)
			continue;
		address_end = address + 2;
		while(isalnum(*address_end))
			address_end++;
		*address_end = 0;
		char* symbol = a2s.getSymbol((strtoul(address+2, NULL, 16)));
		printf("%s\n", symbol ? symbol : "UnknownSymbol");
		if(symbol)
			a2s.freeSymbol(symbol);
	}
	return 0;
}
#endif
