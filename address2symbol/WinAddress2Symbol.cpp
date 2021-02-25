
#ifdef _MSC_VER

#include <stdio.h>
#include <assert.h>

#include "windows.h"
#include "imagehlp.h"
#include "Address2Symbol.h"

#pragma comment(lib, "imagehlp.lib")

#ifndef MIN
#define MIN(x, y)  ((x) < (y) ? (x) : (y))
#endif

class WinAddress2Symbol : public Address2Symbol
{
public:
	char* getSymbol(U64 address);
	BOOL init();
	void freeSymbol(char *symbol);
};

int Address2Symbol::create(Address2Symbol** p_a2s)
{
	*p_a2s = new WinAddress2Symbol();
	return 0;
}

char* strndup(char* s1, int n)
{
	char* s = (char*)malloc(n + 1);
	if (!s)
		return NULL;
	strncpy(s, s1, n);
	s[n] = 0;
	return s;
}

char* WinAddress2Symbol::getSymbol(U64 address)
{
	DWORD64 dwDisplacement = 0;
	DWORD64 dwAddress = address;
	HANDLE hProcess = GetCurrentProcess();

	char *buffer = NULL;
	char pSymbolBuffer[ sizeof (SYMBOL_INFO) + MAX_SYM_NAME * sizeof (TCHAR)];
	PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)pSymbolBuffer;

	pSymbol->SizeOfStruct = sizeof (SYMBOL_INFO);
	pSymbol->MaxNameLen = MAX_SYM_NAME;

	if (SymFromAddr(hProcess, dwAddress, &dwDisplacement, pSymbol))
	{
	     // SymFromAddr returned success
		buffer = strndup(pSymbol->Name, pSymbol->NameLen);
	}
	else
	{
	     // SymFromAddr failed
		DWORD error = GetLastError();
		printf("SymFromAddr %lld returned error : %d\n" , address, error);
	}
	return buffer;
}

void WinAddress2Symbol::freeSymbol(char *symbol)
{
	assert(symbol);
	free(symbol);
}

BOOL WinAddress2Symbol::init()
{
	DWORD  error;
	HANDLE hProcess;
	DWORD  processId;

	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);

	hProcess = GetCurrentProcess();
	// hProcess = (HANDLE)processId;

	if (SymInitialize(hProcess, NULL, TRUE))
	{
		// SymInitialize returned success
		return TRUE;
	}
	else
	{
		// SymInitialize failed
		error = GetLastError();
		printf("SymInitialize returned error : %d\n", error);
		return FALSE;
	}
}

#endif /* _MSC_VER */
