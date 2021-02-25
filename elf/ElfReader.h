
#ifndef ELF_READER_H
#define ELF_READER_H

#include "elf.h"
#include "common/utils.h"
#include "stdio.h"
#include "Endian.h"
#include <vector>

#define READ_DATA_FUNCTION	\
	template<typename T>	\
	void readData(u32 address, T&value)	\
	{									\
		int fseek_ret = fseek(m_fp, address, SEEK_SET);	\
		ASSERT(fseek_ret >= 0);							\
		u32 readed = fread(&value, 1, sizeof(value), m_fp);	\
		ASSERT(readed == sizeof(value));	\
		if(m_big_endian)					\
			EndianConverter::Little(value);	\
	}									\
	u32 readU32(u32 address)	\
	{							\
		u32 value;				\
		readData(address, value);\
		return value;			\
	}							\
	u16 readU16(u32 address)	\
	{							\
		u16 value;				\
		readData(address, value);\
		return value;			\
	}

class Instruction
{
public:
	BOOL arm_mode;
	u32 code;
	Instruction(u32 _code, BOOL _arm_mode=0): code(_code), arm_mode(_arm_mode)
	{}
	BOOL isAddSp();
	BOOL isSubSp();
	BOOL isPush();
	BOOL isPop();
	BOOL isPopPc();
	s64 getSpOffset();
};

class ArmMapAddress2Symbol;
class ElfReader
{
	FILE* m_fp;
	Elf32_Ehdr m_header;
	std::vector<Elf32_Shdr> m_sectionHeaders;
	BOOL m_arm_mode;
	BOOL m_big_endian;

	ArmMapAddress2Symbol* m_a2s;
public:
	ElfReader();
	~ElfReader();
	BOOL init(const char* filepath);
	Instruction readInstruction(u32* address);
	void handleInstruction(Instruction ins, s64 * sp);
	s64 getSpOffset(u32 start_address, u32 end_address);
	BOOL backtrace(u32 sp, u32 pc, const char* memdump_file, u32 memory_dump_base, const char* map_file);
	u32 getBaseAddress(u32 current_address);

	BOOL loadMap(const char* map_file);
	void beginArmMode() { m_arm_mode = TRUE; }
	void endArmMode() { m_arm_mode = FALSE; }
	BOOL inArmMode() { return m_arm_mode; }

	BOOL isBigEndian() { return m_big_endian; }
	// Convert runtime address of code/data to it's offset in elf file
	u32 getElfOffset(u32 exec_address);

	READ_DATA_FUNCTION

};

class MemoryDump
{
	BOOL m_big_endian;
	FILE* m_fp;
	u32 m_base_address;
public:
	MemoryDump(): m_big_endian(FALSE), m_fp(NULL), m_base_address(0)
	{}
	~MemoryDump()
	{
		if(m_fp)
			fclose(m_fp);
	}
	void setBigEndian(BOOL big_endian) { m_big_endian = big_endian; }
	void setBaseAddress(u32 base_address) { m_base_address = base_address; }
	u32 readU32ByAddress(u32 address) { return readU32(address - m_base_address); }
	BOOL load(const char* dump_file);

	READ_DATA_FUNCTION
};
#endif