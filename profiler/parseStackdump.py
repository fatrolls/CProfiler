#! python
import sys,os,commands,re
from optparse import OptionParser
from bisect import bisect_right

def binary_search(seq, t, cmp):
    min = 0; max = len(seq) - 1
    while 1:
    	if max < min:
    		break
        m = (min + max) / 2
        if cmp(seq[m], t) <= 0:
            min = m + 1
        else: # cmp(seq[m], t) > 0:
            max = m - 1
    if min == 0:
    	raise Error("binary_search failed")
    r = min - 1
    assert cmp(seq[r], t) <= 0 and (r + 1 >= len(seq) or cmp(seq[r + 1], t) >= 0 )
    return seq[r]

def strcmp(x, y):
	if x < y:
		return -1
	elif x > y:
		return 1
	else:
		return 0

def get_symbols(exefile):
	return commands.getoutput("nm -C --defined-only %s |grep -vP '\w \.'| sort -k1" % exefile)

def get_symbols_list(exefile):
	return get_symbols(exefile).split('\n')

def run(exefile, iterable=sys.stdin):

	symbols = get_symbols_list(exefile)

	for line in iterable:
		line = line.strip().lower()
		if line:
			print binary_search(symbols, line, cmp=lambda x, y: strcmp(x[0:8], y))


def parseCoredump(file):
	addresses = re.findall(r"[0-9A-F]{8}  ([0-9A-F]{8})", open(dump).read())
	run(file, addresses)

def parseProfilerOutput(exefile, dump):
	profiler_lines = open(dump).readlines()
	# re.findall(r"func = 0x([0-9A-Fa-f]{8})", open(file+".profiler").read())
	symbols = get_symbols_list(exefile)

	for line in profiler_lines:
		g = re.findall(r"0x([0-9A-Fa-f]{8})", line)
		for address in g:
			address = address.strip().lower()
			r = binary_search(symbols, address, cmp=lambda x, y: strcmp(x[0:8], y))
			symbol = r[11:]
			line = line.replace('UnknownSymbol', symbol)
		print line

def main():
	parser = OptionParser()

	parser.add_option("-c", "--coredump", dest="coredump", default=False, 
		action="store_true")
	parser.add_option("-p", "--profiler", dest="profiler", default=False, 
		action="store_true")
	parser.add_option("-s", "--symbol_only", dest="symbol_only", default=False, 
		action="store_true")

	(options, args) = parser.parse_args()
	file = args[0]
	if options.symbol_only:
		print get_symbols(file)
		return

	dump = args[1]
	if options.coredump:
		parseCoredump(file, dump)
	
	if options.profiler:
		parseProfilerOutput(file, dump)

if __name__ == "__main__":
	sys.exit(main())
