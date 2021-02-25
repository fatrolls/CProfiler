
#ifndef PROFILER_H
#define PROFILER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Reset the profiler, normally no need to call it unless you want to clear every records and restart profiler again in a process. 
Note: In multithread context, you need to call it before the first time that "do_enter" being called. */
void profiler_reset(void);

/*
Print all informations to a file handler
fileHandler is actually a FILE* pointer, to avoid invoke system header files, don't use FILE*.
*/
void profiler_print_info2(void* fileHandler);

/*
Print all informations to a file with name 'filename'. That file will be overwritten.
*/
void profiler_print_info(const char* filename);

#ifdef __cplusplus
}
#endif

#endif /* PROFILER_H */

