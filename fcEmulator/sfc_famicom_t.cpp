#include "sfc_famicom_t.h"
#include <assert.h>
#include <string>

sfc_famicom_t::sfc_famicom_t(void *argument):argument(argument)
{
	sfc_famicom_init();
}

sfc_famicom_t::~sfc_famicom_t()
{
	
	sfc_famicom_uninit();
}

sfc_ecode sfc_famicom_t::sfc_famicom_init() {
	assert(this && "bad famicom");

	//清空数据
	memset(&rom_info, 0, sizeof(rom_info));

	//初步BANK
	prg_banks[0] = main_memory;
	prg_banks[1] = save_memory;

	return sfc_load_new_rom();
	return SFC_ERROR_OK;
}

void sfc_famicom_t::sfc_famicom_uninit() {
	sfc_free_default_rom();
}

sfc_ecode sfc_famicom_t::sfc_load_default_rom() {
	assert(rom_info.data_prgrom == nullptr && "FREE FIRST");
	FILE *const file = fopen("nestest.nes", "rb");

	if (!file) {
		return SFC_ERROR_FILE_NOT_FOUNT;
	}
	sfc_ecode code = SFC_ERROR_ILLEGAL_FILE;

	//read header
	sfc_nes_header_t nes_header;
	if (fread(&nes_header, sizeof(nes_header), 1, file)) {
		union { uint32_t u32; uint8_t id[4]; }this_union;
		this_union.id[0] = 'N';
		this_union.id[1] = 'E';
		this_union.id[2] = 'S';
		this_union.id[3] = '\x1A';

		if (this_union.u32 == nes_header.id) {
			const size_t size1 = 16 * 1024 * nes_header.count_prgrom16kb;
			const size_t size2 = 8 * 1024 * nes_header.count_chrrom_8kb;
			uint8_t* const ptr = new uint8_t[size1+size2];

			//内存申请成功
			if (ptr) {
				code = SFC_ERROR_OK;


				if (nes_header.control1 & SFC_NES_TRAINER) {
					fseek(file, 512, SEEK_CUR);
				}
				fread(ptr, size1 + size2, 1, file);

				rom_info.data_prgrom = ptr;
				rom_info.data_chrrom = ptr + size1;
				rom_info.count_prgrom16kb = nes_header.count_prgrom16kb;
				rom_info.count_chrrom_8kb = nes_header.count_chrrom_8kb;

				//nes_header.control1 >> 4 		0000 NNNN			   nes_header.control2 & 0xF0	NNNN 0000
				//表示取control1的高4位NNNN，对应Mapper低4位		   表示取control2的高4位NNNN，对应Mapper高4位(涉及到& 0xF0运算技巧)
				rom_info.mapper_number = (nes_header.control1 >> 4) | (nes_header.control2 & 0xF0);	// 或运算后为NNNN NNNN
				rom_info.vmirroring = (nes_header.control1 & SFC_NES_VMIRROR) > 0;
				rom_info.four_screen = (nes_header.control1 & SFC_NES_4SCREEN) > 0;
				rom_info.save_ram = (nes_header.control1 & SFC_NES_SAVERAM) > 0;
				assert(!(nes_header.control1 &SFC_NES_TRAINER) && "unsuppported");
				assert(!(nes_header.control2 &SFC_NES_VS_UNISYSTEM) && "unsuppported");
				assert(!(nes_header.control2 &SFC_NES_Playchoice10) && "unsuppported");
			}

			else {
				code = SFC_ERROR_OUT_OF_MEMORY;
			}
		}
		//非法文件
	}
	fclose(file);
	return code;
}

sfc_ecode sfc_famicom_t::sfc_free_default_rom() {
	delete [](rom_info.data_prgrom);
	rom_info.data_prgrom = nullptr;
	//printf("析构了famicomm");


	return SFC_ERROR_OK;
}

sfc_rom_info_t sfc_famicom_t::get_rom_info() const {
	return rom_info;
}

std::shared_ptr<sfc_famicom_t> sfc_famicom_t::singleFamicom = nullptr;

std::shared_ptr<sfc_famicom_t> sfc_famicom_t::getInstance(void* arg) {
	if (!singleFamicom) {
		singleFamicom = std::shared_ptr<sfc_famicom_t>(new sfc_famicom_t(arg));
	}
	return singleFamicom;
}

//========================
//3
//载入8k PRG-ROM





void sfc_famicom_t::sfc_load_prgrom_8k(const int& des, const int& src) {
	prg_banks[4 + des] = rom_info.data_prgrom + 8 * 1024 * src;
}






sfc_ecode sfc_famicom_t::sfc_mapper_00_reset() {
	assert(rom_info.count_prgrom16kb && "bad count");
	assert(rom_info.count_prgrom16kb <= 2 && "bad count");
	//16kb -> 载入 $8000-$BFFF ,$C000-$FFFF 为镜像
	const int id2 = rom_info.count_prgrom16kb & 2;

	//32kb -> 载入 $8000-$FFFF
	sfc_load_prgrom_8k(0, 0);
	sfc_load_prgrom_8k(1, 1);
	sfc_load_prgrom_8k(2, id2 + 0);
	sfc_load_prgrom_8k(3, id2 + 1);

	return SFC_ERROR_OK;
}

//加载ROM
sfc_ecode sfc_famicom_t::sfc_load_new_rom() {
	//先释放旧的ROM
	sfc_ecode code = sfc_free_default_rom();
	//清空数据
	memset(&rom_info, 0, sizeof(rom_info));
	//载入ROM
	if (code == SFC_ERROR_OK) {
		code = sfc_load_default_rom();
	}

	//载入新的Mapper
	if (code == SFC_ERROR_OK) {
		code = sfc_load_mapper(rom_info.mapper_number);
	}

	//首次重置
	if (code == SFC_ERROR_OK) {
		sfc_mapper_00_reset();
	}
	return code;
}

//加载Mapper
sfc_ecode sfc_famicom_t::sfc_load_mapper(const uint8_t& id) {
	switch (id) {
	case 0:
		return sfc_mapper_00_reset();
	}

	return SFC_ERROR_FILE_NOT_FOUNT;
}

//============================================
//	cpu部分
uint8_t sfc_famicom_t::sfc_read_cpu_address(uint16_t address) {
	/*
	CPU 地址空间
	+---------+-------+-------+-----------------------+
	| 地址    | 大小  | 标记  |         描述          |
	+---------+-------+-------+-----------------------+
	| $0000   | $800  |       | RAM                   |
	| $0800   | $800  | M     | RAM                   |
	| $1000   | $800  | M     | RAM                   |
	| $1800   | $800  | M     | RAM                   |
	| $2000   | 8     |       | Registers             |
	| $2008   | $1FF8 | R     | Registers             |
	| $4000   | $20   |       | Registers             |
	| $4020   | $1FDF |       | Expansion ROM         |
	| $6000   | $2000 |       | SRAM                  |
	| $8000   | $4000 |       | PRG-ROM               |
	| $C000   | $4000 |       | PRG-ROM               |
	+---------+-------+-------+-----------------------+
	标记图例: M = $0000的镜像
	R = $2000-2008 每 8 bytes 的镜像
	(e.g. $2008=$2000, $2018=$2000, etc.)
	*/
	switch (address >> 13) {
	case 0:
		//高三位为0，[$0000,$2000) :系统主内存，4次镜像
		return main_memory[address & (uint16_t)0x07ff];
	case 1:
		//高三位为1，[$2000,$4000) :PPU寄存器，8字节步进镜像
		assert(!"NOT IMPL");
		return 0;
	case 2:
		//高三位为2，[$4000,$6000) :pAPU寄存器 扩展ROM区
		assert(!"NOT IMPL");
		return 0;
	case 3:
		//高三位为3，[$6000,$8000) :存档 SRAM区
		return save_memory[address & (uint16_t)0x1fff];
	case 4:case 5:case 6:case 7 :
		//高一位为1，[$8000,$10000):程序PRG-ROM区
		return prg_banks[address >> 13][address & (uint16_t)0x1fff];
	}
	assert(!"invalid address");
	return 0;
}



void sfc_famicom_t::sfc_write_cpu_address(uint16_t address, uint8_t data) {
	/*
	CPU 地址空间
	+---------+-------+-------+-----------------------+
	| 地址    | 大小  | 标记  |         描述          |
	+---------+-------+-------+-----------------------+
	| $0000   | $800  |       | RAM                   |
	| $0800   | $800  | M     | RAM                   |
	| $1000   | $800  | M     | RAM                   |
	| $1800   | $800  | M     | RAM                   |
	| $2000   | 8     |       | Registers             |
	| $2008   | $1FF8 | R     | Registers             |
	| $4000   | $20   |       | Registers             |
	| $4020   | $1FDF |       | Expansion ROM         |
	| $6000   | $2000 |       | SRAM                  |
	| $8000   | $4000 |       | PRG-ROM               |
	| $C000   | $4000 |       | PRG-ROM               |
	+---------+-------+-------+-----------------------+
	标记图例: M = $0000的镜像
	R = $2000-2008 每 8 bytes 的镜像
	(e.g. $2008=$2000, $2018=$2000, etc.)
	*/
	switch (address >> 13) {
	case 0:
		//高三位为0，[$0000,$2000) :系统主内存，4次镜像
		main_memory[address & (uint16_t)0x07ff] = data;
		return;
	case 1:
		//高三位为1，[$2000,$4000) :PPU寄存器，8字节步进镜像
		assert(!"NOT IMPL");
		return;
	case 2:
		//高三位为2，[$4000,$6000) :pAPU寄存器 扩展ROM区
		assert(!"NOT IMPL");
		return;
	case 3:
		//高三位为3，[$6000,$8000) :存档 SRAM区
		save_memory[address & (uint16_t)0x1fff] = data;
	case 4:case 5:case 6:case 7:
		//高一位为1，[$8000,$10000):程序PRG-ROM区
		assert(!"WARNING:PRG-ROM");
		prg_banks[address >> 13][address & (uint16_t)0x1fff] = data;
		return;
	}
	assert(!"invalid address");
}

//
void sfc_famicom_t::sfc_fc_disassembly(uint16_t address, char buf[]) {
	enum {
		OFFSET_M= SFC_DISASSEMBLY_BUF_LEN2 -SFC_DISASSEMBLY_BUF_LEN,
		OFFSET=8
	};

	static_assert(OFFSET < OFFSET_M, "LESS!");
	memset(buf + 1, ' ', OFFSET);
	buf[0] = '$';
	sfc_btoh(buf + 1, (uint8_t)(address >> 8));
	sfc_btoh(buf + 3, (uint8_t)(address));

	sfc_6502_code_t code;
	code.data = 0;

	code.set_op(sfc_read_cpu_address(address));
	code.set_a1(sfc_read_cpu_address(address + 1));
	code.set_a2(sfc_read_cpu_address(address + 2));

	//反汇编
	code.sfc_6502_disassembly(buf + OFFSET);

}