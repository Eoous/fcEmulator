#include "sfc_ppu_t.h"



sfc_ppu_t::sfc_ppu_t()
{
}


sfc_ppu_t::~sfc_ppu_t()
{
}

//========================================
//读取PPU地址
uint8_t sfc_ppu_t::sfc_read_ppu_address(uint16_t address) {
	const uint16_t real_address = address&(uint16_t)0x3FFF;
	//使用BANK读取
	if (real_address < (uint16_t)0x3f00) {
		const uint16_t index = real_address >> 10;
		const uint16_t offset = real_address&(uint16_t)0x3ff;
		assert(banks[index]);
		const uint8_t data = pseudo;
		pseudo = banks[index][offset];
		return data;
	}
	else {
		return pseudo = spindexes[real_address*(uint16_t)0x1f];
	}
}


//写入PPU地址
void sfc_ppu_t::sfc_write_ppu_address(uint16_t address, uint8_t data) {
	const uint16_t real_address = address&(uint16_t)0x3FFF;
	//使用BANK写入
	if (real_address < (uint16_t)0x3F00) {
		assert(real_address >= 0x2000);
		const uint16_t index = real_address >> 10;
		const uint16_t offset = real_address&(uint16_t)0x3FF;
		assert(banks[index]);
		banks[index][offset] = data;
	}
	//调色板索引
	else {
		if (real_address&(uint16_t)0x03) {
			spindexes[real_address&(uint16_t)0x1f] = data;
		}
		//镜像$3F00/$3F04/$3F08/$3F0C
		else {
			const uint16_t offset = real_address&(uint16_t)0x0f;
			spindexes[offset] = data;
			spindexes[offset | (uint16_t)0x10] = data;
		}
	}
}

//
uint8_t sfc_ppu_t::sfc_read_ppu_register_via_cpu(uint16_t address) {
	uint8_t data = 0x00;
	switch (address&(uint16_t)0x7) {
	case 0:
		//0x2000 :controller($2000) >write
		//只写寄存器
	case 1:
		//0x2001 :Mask($2001) >write
		//只写寄存器
		assert(!"Write Only!");
		break;
	case 2:
		// 0x2002: Status ($2002) < read
		// 只读状态寄存器
		data = status;
		status &= ~(uint8_t)SFC_PPU2002_VBlank;
		break;
	case 3:
		// 0x2003: OAM address port ($2003) > write
		// 只写寄存器
		assert(!"Write Only!");
		break;
	case 4:
		// 0x2004: OAM data ($2004) <> read/write
		// 读写寄存器
		data = sprites[oamaddr++];
		break;
	case 5:
		// 0x2005: Scroll ($2005) >> write x2
		// 双写寄存器
	case 6:
		// 0x2006: Address ($2006) >> write x2
		// 双写寄存器
		assert(!"write only!");
		break;
	case 7:
		// 0x2007: Data ($2007) <> read/write
		// PPU VRAM读写端口
		data = sfc_read_ppu_address(vramaddr);
		vramaddr += (uint16_t)((ctrl & SFC_PPU2000_VINC32) ? 32 : 1);
		break;
	}
	return data;
}

void sfc_ppu_t::sfc_write_ppu_register_via_cpu(uint16_t address, uint8_t data) {
	switch (address&(uint16_t)0x7) {
	case 0:
		// PPU 控制寄存器
		// 0x2000: Controller ($2000) > write
		ctrl = data;
		break;
	case 1:
		// PPU 掩码寄存器
		// 0x2001: Mask ($2001) > write
		mask = data;
		break;
	case 2:
		// 0x2002: Status ($2002) < read
		// 只读
		assert(!"read only");
		break;
	case 3:
		// 0x2003: OAM address port ($2003) > write
		// PPU OAM 地址端口
		oamaddr = data;
		break;
	case 4:
		// 0x2004: OAM data ($2004) <> read/write
		// PPU OAM 数据端口
		sprites[oamaddr++] = data;
		break;
	case 5:
		// 0x2005: Scroll ($2005) >> write x2
		// PPU 滚动位置寄存器 - 双写
		scroll[writex2 & 1] = data;
		++writex2;
		break;
	case 6:
		// 0x2006: Address ($2006) >> write x2
		// PPU 地址寄存器 - 双写
		// 写入高字节
		if (writex2 & 1) {
			vramaddr = (vramaddr & (uint16_t)0xFF00) | (uint16_t)data;
		}
		// 写入低字节
		else {
			vramaddr = (vramaddr & (uint16_t)0x00FF) | ((uint16_t)data << 8);
		}
		++writex2;
		break;
	case 7:
		// 0x2007: Data ($2007) <> read/write
		// PPU VRAM数据端
		sfc_write_ppu_address(vramaddr, data);
		vramaddr += (uint16_t)((ctrl & SFC_PPU2000_VINC32) ? 32 : 1);
		break;
	}
}