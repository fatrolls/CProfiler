#include <stdio.h>
#include <stack>
#include <time.h>
#include <assert.h>
using namespace std;

#ifdef NO_HASH_MAP
#	define MAP map
#	include <map>
#else
#	define MAP hash_map
#	ifdef __GNUC__
#		include <ext/hash_map>
		using namespace __gnu_cxx;
#	elif defined(_MSC_VER)
#		include <hash_map>
		using namespace stdext;
#	endif
#endif /* NO_HASH_MAP */

#ifdef MULTITHREAD_SUPPORT
#	ifdef __GNUC__
#		define TLS __thread
#	elif defined(_MSC_VER)
#		define TLS __declspec(thread)
#	else
#		error "don't support multithread"
#	endif
#
#   define AUTOLOCK(locker)	/* FIXME */
#else
#	define TLS
#   define AUTOLOCK(locker)		
#endif /* MULTITHREAD_SUPPORT */

#include "address2symbol/Address2Symbol.h"
#include "profiler.h"

#ifdef DEBUG_PROFILER
#define ASSERT assert
#define Exists(x, y) (x.find(y) != x.end())
#else
#define ASSERT(...) 
#endif

#undef uint
typedef unsigned int uint;

/* The statistics of executing information of a function
ticks_total = ticks_self + ticks_profiler + sum([subfunc.ticks_total for each subfunc called in this function ])
 */
struct FuncInfo
{
	uint count;	/* Indicate how many times this function be called. */
	uint ticks_total;	/* total times consumed by this function, exclude times consumed by profiler when enter and leave this function */
	uint ticks_self; /* times only consumed by this function self, include times of jumping to sub functions, exculde times consumed by functions called in it. */ 
	uint ticks_profiler; /* times consumed by the profiler itself in this function */
};

/* Frame: the information during executing a function at once. */
struct Frame
{
	uint func;	/* current function */
	uint tick_enter; /* the time when enter this function */
	uint tick_enter_subfunc; /* the last time when call other functions in this function */
	uint ticks_subfunc; /* times consumed by functions called in this function */
	uint ticks_profiler; /* times consumed by the profiler in this function */
};

typedef MAP<uint, FuncInfo> FuncInfos;

static int initialized = 0;
static FuncInfos funcinfos;
static TLS stack<Frame> frames;
#ifdef _MSC_VER
static TLS uint s_current_function = 0;
#endif
// static const FuncInfo empty_info = {0};

#ifndef TICK
#define TICK	(clock())
#endif

static void do_enter(uint current_function)
{
	uint tick_begin = TICK;
#ifndef MULTITHREAD_SUPPORT
	if (!initialized)
	{
		profiler_reset();
	}
#endif
	if (frames.size() > 0)
	{
		Frame &f = frames.top();
		f.tick_enter_subfunc = tick_begin;
	}
	{
		AUTOLOCK(COUNTS_LOCKER);
		FuncInfos::iterator iter = funcinfos.find(current_function);
		if (iter != funcinfos.end())
			iter->second.count ++;
		else
		{
			FuncInfo info = {1, 0, 0, 0};
			funcinfos.insert(pair<uint, FuncInfo>(current_function, info));
		}
	}
	Frame *p_parent_frame = frames.size() > 0 ? (&frames.top()) : NULL; 
	Frame frame = {current_function, 0, 0, 0, 0};
	frames.push(frame);
	Frame &topf = frames.top();
	uint tick_end = TICK;
	if (p_parent_frame)
	{
		p_parent_frame->ticks_profiler += tick_end - tick_begin;
	}
	/* Get tick in the end, above code may take much time, which makes profiler not exact */
	topf.tick_enter = tick_end;
}

static void do_exit()
{
	/* Get tick first, following code may take much time, which makes profiler not exact */
	uint tick_begin = TICK;
	ASSERT(initialized);
	Frame &f = frames.top();
	{
		AUTOLOCK(COUNTS_LOCKER);
		ASSERT(Exists(funcinfos, f.func));
		FuncInfo &info = funcinfos[f.func];
		info.ticks_total += tick_begin - f.tick_enter;
		info.ticks_self += tick_begin - f.tick_enter - f.ticks_subfunc;
		info.ticks_profiler += f.ticks_profiler;
	}
	frames.pop();
	if (frames.size() > 0)
	{
		Frame &f = frames.top();
		ASSERT(f.tick_enter_subfunc > 0);
		uint tick_end = TICK;
		f.ticks_subfunc += tick_end - f.tick_enter_subfunc;
		f.ticks_profiler += tick_end - tick_begin;
	}
}

extern "C" void profiler_reset()
{
	funcinfos.clear();
	// while(!frames.empty())
	// 	frames.pop();
	initialized = 1;
}

extern "C" void profiler_print_info2(void* fileHandler)
{
	FILE* fout = (FILE*)fileHandler;
	FuncInfos::const_iterator iter;
	char* symbol = NULL;
#ifndef NO_SYMBOL
	Address2Symbol *a2s;
	if(0 != Address2Symbol::create(&a2s))
	{
		fprintf(stderr, "%s\n", "Address2Symbol::create failed");
		return;
	}
	if(!a2s->init())
	{
		fprintf(stderr, "%s\n", "Address2Symbol::init failed");		
		return;
	}
#endif
	{
		fprintf(fout, "Function\tAddress\tCount\tTotal(ms)\tSelf\tProfiler\n");
		AUTOLOCK(COUNTS_LOCKER);
		for (iter = funcinfos.begin(); iter != funcinfos.end(); iter++)
		{
#ifndef NO_SYMBOL
			symbol = a2s->getSymbol(iter->first);
#endif
			fprintf(fout, "%s\t0x%08x\t%d\t%d\t%d\t%d\n", symbol ? symbol : "UnknownSymbol"
				, iter->first, iter->second.count, iter->second.ticks_total, iter->second.ticks_self, iter->second.ticks_profiler);
#ifndef NO_SYMBOL 
			if (symbol)
				a2s->freeSymbol(symbol);
#endif
		}
	}
}

extern "C" void profiler_print_info(const char* filename)
{
	FILE* fout = fopen(filename, "w");
	if (!fout)
	{
		fprintf(stderr, "Open %s failed", filename);
		return;
	}
	profiler_print_info2(fout);
	fclose(fout);
}

#ifdef _MSC_VER
extern "C" void __declspec(naked) _cdecl _penter( void ) {
	_asm {
		/* Get the value in the top of stack. */
		pop s_current_function
		push s_current_function
		push eax
		push ebx
		push ecx
		push edx
		push ebp
		push edi
		push esi
	}
	do_enter(s_current_function);
	_asm {
		pop esi
		pop edi
		pop ebp
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
	}
}

extern "C" void __declspec(naked) _cdecl _pexit( void )
{
	_asm {
		push eax
		push ebx
		push ecx
		push edx
		push ebp
		push edi
		push esi
	}

	do_exit();

	_asm {
		pop esi
		pop edi
		pop ebp
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
	}
}
#endif

#ifdef __GNUC__

#define DUMP(func, call) \
        printf("%s: func = 0x%08x, caller = 0x%08x\n", __FUNCTION__, func, call)

extern "C" void __cyg_profile_func_enter(void *this_func, void *call_site)
{
	// DUMP(this_func, call_site);
	do_enter((uint)this_func);
}

extern "C" void __cyg_profile_func_exit(void *this_func, void *call_site)
{
	// DUMP(this_func, call_site);
	do_exit();
}

#endif /* __GNUC__ */