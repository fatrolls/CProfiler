for gcc version, steps to do profiler:
1 make
2 ../profiler/parseStackdump.py -s program_with_profiler.exe >symbols.txt
3 ./program_with_profiler.exe

for Vistual Studio version, just open test.sln, then build everything and run
