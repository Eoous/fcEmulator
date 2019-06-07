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
		//更新处于调色板“下方”的伪缓存值
		const uint16_t underneath = real_address - 0x1000;
		const uint16_t index = real_address >> 10;
		const uint16_t offset = real_address&(uint16_t)0x3ff;
		assert(banks[index]);
		pseudo = banks[index][offset];;
		//读取调色板能返回即时值
		return spindexes[real_address&(uint16_t)0x1f];
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
		//读取后会清楚VBlank状态
		status &= ~(uint8_t)SFC_PPU2002_VBlank;
		// wiki.nesdev.com/w/index.php/PPU_scrolling:  $2002 read
		writex2 = 0;
		break;
	case 3:
		// 0x2003: OAM address port ($2003) > write
		// 只写寄存器
		assert(!"Write Only!");
		break;
	case 4:
		// 0x2004: OAM data ($2004) <> read/write
		// 读写寄存器
		// - [???] Address should not increment on $2004 read 
		//data = sprites[oamaddr++];
		data = sprites[oamaddr];
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
		//D0表示窗口x坐标高位0:00 - 10;1:01 - 11
		//D1表示窗口y坐标高位0:00 - 01;1:10 - 11
		//通过这两位可以将窗口移动到任意一页
		nametable_select = data & 3;
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
		//写2次 所以叫双写?
		scroll[writex2 & 1] = data;
		++writex2;
		break;
	case 6:
		// 0x2006: Address ($2006) >> write x2
		// PPU 地址寄存器 - 双写
		// 调试显示以下正确
		// 写入低字节
		if (writex2 & 1) {
			const uint16_t tmp = (vramaddr&(uint16_t)0xff00) | (uint16_t)data;
			vramaddr = tmp;
			//A - B bit
			nametable_select = (tmp >> 10) & 3;
			//0 - 4 bit
			scroll[0] = (scroll[0] & (uint8_t)7) | ((uint8_t)tmp & (uint8_t)0x1f) << 3;
			//5 - 9 bit
			scroll[1] = ((tmp&(uint16_t)0x3e0) >> 2) | (tmp&(uint16_t)data << 8);
			++writex2;
			break;
		}
		// 写入高字节
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

