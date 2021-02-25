
#include "common/utils.h"
#include "address2symbol/ArmMapAddress2Symbol.h"
#include "ElfReader.h"
#include "Endian.h"
#include <vector>

u32 getBits(u32 src, int start, int end)
{
	return (src << (31 - end)) >> (31 - end + start);
}

u32 from2(const char* bs)
{
	u32 ret = 0;
	const char* s = bs;
	while (*s)
		ret = ret * 2 + *s++ - '0';
	return ret;
}

void testFrom2()
{
	ASSERT(from2("1011") == 11);
	ASSERT(from2("10110011") == 0xB3);
	ASSERT(from2("1111000011110000") == 0xF0F0U);
	ASSERT(from2("11111111111111111111111111111111") == 0xFFFFFFFFU);
}

void testGetBits()
{
	ASSERT(from2("1111") == getBits(from2("1111000011110000"), 4, 7));
	ASSERT(from2("111100001111") == getBits(from2("1111000011110000"), 4, 31));
	ASSERT(from2("10000") == getBits(from2("1111000011110000"), 0, 4));
}

static u32 getBit1Num(u32 code)
{
	int ret = 0;
	while(code)
	{
		ret ++;
		code &= (code - 1);
	}
	return ret;
}

void testGetBit1Num()
{
	ASSERT(getBit1Num(from2("0")) == 0);
	ASSERT(getBit1Num(from2("1")) == 1);
	ASSERT(getBit1Num(from2("010")) == 1);
	ASSERT(getBit1Num(from2("10110")) == 3);
	ASSERT(getBit1Num(from2("1111")) == 4);
	ASSERT(getBit1Num(from2("1111000011110000")) == 8);
	ASSERT(getBit1Num(from2("11111111111111111111111111111111")) == 32);
}

BOOL Instruction::isAddSp()
{
	return arm_mode ? (getBits(code, 26, 27) == 0x00 && getBits(code, 21, 24) == 0x4 && getBits(code, 12, 15) == 13)
	: (getBits(code, 7, 15) == from2("101100000"));
}

BOOL Instruction::isSubSp()
{
	return arm_mode ? (getBits(code, 26, 27) == 0x00 && getBits(code, 21, 24) == 0x2 && getBits(code, 12, 15) == 13)
	: (getBits(code, 7, 15) == from2("101100001"));
}

BOOL Instruction::isPush()
{
	return arm_mode ? (getBits(code, 16, 31) == 0xE92DU)
	: (getBits(code, 9, 15) == from2("1011010"));
}

BOOL Instruction::isPop()
{
	return arm_mode ? (getBits(code, 16, 31) == 0xE8BDU)
	: (getBits(code, 9, 15) == from2("1011110"));
}

BOOL Instruction::isPopPc()
{
	return arm_mode ? (getBits(code, 16, 31) == 0xE8BDU && getBits(code, 15, 15) == 1)
	: (getBits(code, 9, 15) == from2("1011110") && getBits(code, 8, 8) == 1);
}

s64 Instruction::getSpOffset()
{
	if (isPush() || isPop())
	{
		s64 ret = arm_mode ? (getBit1Num(getBits(code, 0, 15)) * 4)
		: (getBit1Num(getBits(code, 0, 8)) * 4);
		return isPush() ? -ret : ret;
	}
	else if(isAddSp() || isSubSp())
	{
		s64 ret = arm_mode ? (getBits(code, 0, 11))
		: (getBits(code, 0, 6) * 4);
		return isAddSp() ? ret : -ret;
	}
	else
		return 0;
}

void testInstruction()
{
	// Arm
	{
		//e92d400e    .@-.    PUSH     {r1-r3,r14}
		Instruction i(0xe92d400eU, TRUE);
		ASSERT(i.isPush());
		ASSERT(!(i.isPop() || i.isSubSp() || i.isAddSp()));
		ASSERT(i.getSpOffset() == -16);
	}
	{
		//e8bd800e    ....    POP      {r1-r3,pc}
		Instruction i(0xe8bd800eU, TRUE);
		ASSERT(i.isPop());
		ASSERT(!(i.isPush() || i.isSubSp() || i.isAddSp()));
		ASSERT(i.getSpOffset() == 16);
	}
	{
		//e24dd008    ..M.    SUB      r13,r13,#8
		Instruction i(0xe24dd008U, TRUE);
		ASSERT(i.isSubSp());
		ASSERT(!(i.isPush() || i.isPop() || i.isAddSp()));
		ASSERT(i.getSpOffset() == -8);
	}
	{
		//e28dd018    ....    ADD      r13,r13,#0x18
		Instruction i(0xe28dd018U, TRUE);
		ASSERT(i.isAddSp());
		ASSERT(!(i.isPush() || i.isPop() || i.isSubSp()));
		ASSERT(i.getSpOffset() == 0x18);
	}
	{
		//e9205fff    ._ .    STMDB    r0!,{r0-r12,r14}
		Instruction i(0xe9205fffU, TRUE);
		ASSERT(!(i.isPop() || i.isSubSp() || i.isAddSp() || i.isPush()));
		ASSERT(i.getSpOffset() == 0);
	}
	// Thumb
	{
		//b508        ..      PUSH     {r3,r14}
		Instruction i(0xb508U, FALSE);
		ASSERT(i.isPush());
		ASSERT(!(i.isPop() || i.isSubSp() || i.isAddSp()));
		ASSERT(i.getSpOffset() == -8);
	}
	{
		//bd08        ..      POP      {r3,pc}
		Instruction i(0xbd08U, FALSE);
		ASSERT(i.isPop());
		ASSERT(!(i.isPush() || i.isSubSp() || i.isAddSp()));
		ASSERT(i.getSpOffset() == 8);
	}
	{
		//b004        ..      ADD      r13,r13,#0x10
		Instruction i(0xb004U, FALSE);
		ASSERT(i.isAddSp());
		ASSERT(!(i.isPush() || i.isPop() || i.isSubSp()));
		ASSERT(i.getSpOffset() == 0x10);
	}
	{
		//b090        ..      SUB      r13,r13,#0x40
		Instruction i(0xb090U, FALSE);
		ASSERT(i.isSubSp());
		ASSERT(!(i.isPush() || i.isPop() || i.isAddSp()));
		ASSERT(i.getSpOffset() == -0x40);
	}
	
}

ElfReader::ElfReader(): m_fp(NULL), m_a2s(NULL), m_arm_mode(FALSE), m_big_endian(FALSE)
{

}

BOOL ElfReader::init(const char* filepath)
{
	size_t readed = 0;
	m_fp = fopen(filepath, "rb");
	if (!m_fp)
		return FALSE;
	const char elf_magics[4] = { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3 };
	
	readed = fread(&m_header, 1, sizeof(m_header), m_fp);
	ASSERT(readed == sizeof(m_header));
	ASSERT(0 == memcmp(elf_magics, m_header.e_ident, 4));

	m_big_endian = (m_header.e_ident[5] == ELFDATA2MSB);

	if(m_big_endian)
	{
		EndianConverter::Little(m_header.e_type);
		EndianConverter::Little(m_header.e_shnum);
		EndianConverter::Little(m_header.e_shoff);
		EndianConverter::Little(m_header.e_shentsize);
		EndianConverter::Little(m_header.e_shstrndx);
	}

	// Read sections
	Elf32_Shdr shdr = {0};
	u32 shdr_table = m_header.e_shoff;
	int ret = fseek(m_fp, shdr_table, SEEK_SET);
	ASSERT(ret >= 0);
	for (int i = 0; i < m_header.e_shnum ; i++)
	{
		u32 shdr_address = m_header.e_shoff + m_header.e_shentsize * i;
		ret = fseek(m_fp, shdr_address, SEEK_SET);
		ASSERT(ret >= 0);
		readed = fread(&shdr, 1, sizeof(shdr), m_fp);
		ASSERT(readed == sizeof(shdr));
		if (m_big_endian)
		{
				EndianConverter::Little(shdr.sh_name);
				EndianConverter::Little(shdr.sh_type);
				EndianConverter::Little(shdr.sh_flags);
				EndianConverter::Little(shdr.sh_addr);
				EndianConverter::Little(shdr.sh_offset);
				EndianConverter::Little(shdr.sh_size);
				EndianConverter::Little(shdr.sh_link);
				EndianConverter::Little(shdr.sh_info);
				EndianConverter::Little(shdr.sh_addralign);
				EndianConverter::Little(shdr.sh_entsize);
		}
		m_sectionHeaders.push_back(shdr);
	}

	return TRUE;
}

u32 ElfReader::getElfOffset(u32 exec_address)
{
	for(std::vector<Elf32_Shdr>::iterator it = m_sectionHeaders.begin();
		it != m_sectionHeaders.end(); ++it)
	{
		if(it->sh_addr <= exec_address && exec_address < it->sh_addr + it->sh_size)
			return exec_address - it->sh_addr + it->sh_offset;
	}
	ASSERT(!"Unknow execute address");
	return 0;
}

Instruction ElfReader::readInstruction(u32* address)
{
	u32 file_offset = getElfOffset(*address);
	u32 code = 0;
	int should_read = inArmMode() ? 4 : 2;
	code = inArmMode() ? readU32(file_offset) : readU16(file_offset);
	*address += should_read;
	return Instruction(code, inArmMode());
}

void ElfReader::handleInstruction(Instruction ins, s64*sp)
{
	*sp += ins.getSpOffset();
}

s64 ElfReader::getSpOffset(u32 start_address, u32 end_address)
{
	ASSERT(end_address >= start_address);
	u32 address = start_address;
	Instruction ins(0);
	s64 sp = 0;
	s64 min_sp = sp;
	BOOL pcPoped = FALSE;
	while(address < end_address)
	{
		ins = readInstruction(&address);
		if(!pcPoped && ins.isPopPc())
		{
			pcPoped = TRUE;
		}
		handleInstruction(ins, &sp);
		min_sp = min(sp, min_sp);
	}
	if (sp >= 0 && pcPoped)
	{
		sp = min_sp;
	}
	return sp;
}

BOOL ElfReader::backtrace(u32 sp, u32 pc, const char* memdump_file, u32 memory_dump_base, const char* map_file)
{
#ifdef _DEBUG
	testFrom2();
	testGetBits();
	testGetBit1Num();
	testInstruction();
#endif

	MemoryDump memdump;
	memdump.setBigEndian(this->isBigEndian());
	memdump.setBaseAddress(memory_dump_base);
	if (!memdump.load(memdump_file))
		return FALSE;

	if (!loadMap(map_file))
		return FALSE;

	s32 base_address = 0, current_address = pc;
	s64 spOffset = 0;
	const CommonAddress2Symbol::Symbol* s = NULL;
	while(current_address > 0)
	{
		s = m_a2s->getSymbolStruct(current_address);
		printf ("Address: 0x%08x, Function: 0x%08x %s\n", current_address, s->address, s->symbol);
		base_address = s->address;
		if((base_address & 0x1) == 0)
			beginArmMode();
		spOffset = getSpOffset(EVEN_ADDRESS(base_address), EVEN_ADDRESS(current_address));
		if((base_address & 0x1) == 0)
			endArmMode();
		sp -= spOffset;
		// Get the pushed R14 on stack
		current_address = memdump.readU32ByAddress((sp-4));
	}
	return TRUE;
}

BOOL ElfReader::loadMap(const char* map_file)
{
	m_a2s = new ArmMapAddress2Symbol();
	m_a2s->setFileName(map_file);
	BOOL ret = m_a2s->init();
	return ret;
}

u32 ElfReader::getBaseAddress(u32 current_address)
{
	return m_a2s->getBaseAddress(current_address);
}

ElfReader::~ElfReader()
{
	if (m_fp)
		fclose(m_fp);
	if (m_a2s)
		delete m_a2s;
}

BOOL MemoryDump::load(const char* dump_file)
{
	m_fp = fopen(dump_file, "rb");
	return m_fp != NULL;
}
