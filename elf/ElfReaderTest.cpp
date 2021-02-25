
#include "ElfReader.h"

void unittest()
{
	ElfReader reader;
	BOOL ret = reader.init("E:\\BUGS\\HULA-862\\operamini_6_21\\SC6530_sc6530_64X64_320X240BAR_QW.axf");
	ASSERT(ret == TRUE);
	u32 original_address, address;

	original_address = address = 0x00000716UL;
	Instruction code = reader.readInstruction(&address);
	ASSERT(code.code == 0xb510 && address == original_address + 2);

	original_address = address = 0x0000071aUL;
	code = reader.readInstruction(&address);
	ASSERT(code.code == 0xf000 && address == original_address + 2);


	s64 spOffset = reader.getSpOffset(0x00000720UL, 0x00000720UL);
	ASSERT(spOffset == 0);
	spOffset = reader.getSpOffset(0x00000720UL, 0x00000722UL);
	ASSERT(spOffset == -36);
	spOffset = reader.getSpOffset(0x00000720UL, 0x00000742UL);
	ASSERT(spOffset == -40);

	spOffset = reader.getSpOffset(0x00000720UL, 0x00000748UL);
	ASSERT(spOffset == -20);
	spOffset = reader.getSpOffset(0x00000720UL, 0x0000074aUL);
	ASSERT(spOffset == 0);
}

int main()
{
	ElfReader reader;
	BOOL ret = reader.init("E:\\BUGS\\HULA-862\\operamini_6_21\\SC6530_sc6530_64X64_320X240BAR_QW.axf");
	ASSERT(ret == TRUE);
	// u32 sp = 0x045cd408, pc = 0x00002ff0;
	 u32 sp = 0x040318a8 + 48, pc = 0x0400b800;
	//u32 sp = 0x040318f0, pc = 0x0400d745;
	reader.backtrace(sp, pc, "E:\\BUGS\\HULA-862\\NEWMS00199772\\NEWMS00199772\\assert1.mem", 0x4000000U, "E:\\BUGS\\HULA-862\\operamini_6_21\\SC6530_sc6530_64X64_320X240BAR_QW.map");
	return 0;
}
