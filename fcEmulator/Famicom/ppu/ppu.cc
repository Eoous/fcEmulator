#include "ppu.h"



ppu::ppu()
{
}


ppu::~ppu()
{
}

//========================================
//读取PPU地址
uint8_t ppu::ReadAddressOfPPU(uint16_t address) {
	const uint16_t real_address = address&(uint16_t)0x3FFF;
	//使用BANK读取
	if (real_address < (uint16_t)0x3f00) {
		const uint16_t index = real_address >> 10;
		const uint16_t offset = real_address&(uint16_t)0x3ff;
		assert(banks_[index]);
		const uint8_t data = pseudo_;
		pseudo_ = banks_[index][offset];
		return data;
	}
	else {
		//更新处于调色板“下方”的伪缓存值
		const uint16_t underneath = real_address - 0x1000;
		const uint16_t index = real_address >> 10;
		const uint16_t offset = real_address&(uint16_t)0x3ff;
		assert(banks_[index]);
		pseudo_ = banks_[index][offset];;
		//读取调色板能返回即时值
		return spindexes_[real_address&(uint16_t)0x1f];
	}
}


//写入PPU地址
void ppu::WriteAddressOfPPU(uint16_t address, uint8_t data) {
	const uint16_t real_address = address&(uint16_t)0x3FFF;
	//使用BANK写入
	if (real_address < (uint16_t)0x3F00) {
		assert(real_address >= 0x2000);
		const uint16_t index = real_address >> 10;
		const uint16_t offset = real_address&(uint16_t)0x3FF;
		assert(banks_[index]);
		banks_[index][offset] = data;
	}
	//调色板索引
	else {
		if (real_address&(uint16_t)0x03) {
			spindexes_[real_address&(uint16_t)0x1f] = data;
		}
		//镜像$3F00/$3F04/$3F08/$3F0C
		else {
			const uint16_t offset = real_address&(uint16_t)0x0f;
			spindexes_[offset] = data;
			spindexes_[offset | (uint16_t)0x10] = data;
		}
	}
}

//
uint8_t ppu::ReadRegisterViaCPU(uint16_t address) {
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
		data = status_;
		//读取后会清楚VBlank状态
		status_ &= ~(uint8_t)PPU2002_VBlank;
		// wiki.nesdev.com/w/index.php/PPU_scrolling:  $2002 read
		writex2_ = 0;
		break;
	case 3:
		// 0x2003: OAM address port ($2003) > write
		// 只写寄存器
		assert(!"Write Only!");
		break;
	case 4:
		// 0x2004: OAM data_ ($2004) <> read/write
		// 读写寄存器
		// - [???] Address should not increment on $2004 read 
		//data_ = sprites_[oamaddr_++];
		data = sprites_[oamaddr_];
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
		data = ReadAddressOfPPU(vramaddr_);
		vramaddr_ += (uint16_t)((ctrl_ & PPU2000_VINC32) ? 32 : 1);
		break;
	}
	return data;
}

void ppu::WriteRegisterViaCPU(uint16_t address, uint8_t data) {
	switch (address&(uint16_t)0x7) {
	case 0:
		// PPU 控制寄存器
		// 0x2000: Controller ($2000) > write
		ctrl_ = data;
		//D0表示窗口x坐标高位0:00 - 10;1:01 - 11
		//D1表示窗口y坐标高位0:00 - 01;1:10 - 11
		//通过这两位可以将窗口移动到任意一页
		nametable_select_ = data & 3;
		break;
	case 1:
		// PPU 掩码寄存器
		// 0x2001: Mask ($2001) > write
		mask_ = data;
		break;
	case 2:
		// 0x2002: Status ($2002) < read
		// 只读
		assert(!"read only");
		break;
	case 3:
		// 0x2003: OAM address port ($2003) > write
		// PPU OAM 地址端口
		oamaddr_ = data;
		break;
	case 4:
		// 0x2004: OAM data_ ($2004) <> read/write
		// PPU OAM 数据端口
		sprites_[oamaddr_++] = data;
		break;
	case 5:
		// 0x2005: Scroll ($2005) >> write x2
		// PPU 滚动位置寄存器 - 双写
		//写2次 所以叫双写?
		scroll_[writex2_ & 1] = data;
		++writex2_;
		break;
	case 6:
		// 0x2006: Address ($2006) >> write x2
		// PPU 地址寄存器 - 双写
		// 调试显示以下正确
		// 写入低字节
		if (writex2_ & 1) {
			const uint16_t tmp = (vramaddr_&(uint16_t)0xff00) | (uint16_t)data;
			vramaddr_ = tmp;
			//A - B bit
			nametable_select_ = (tmp >> 10) & 3;
			//0 - 4 bit
			scroll_[0] = (scroll_[0] & (uint8_t)7) | ((uint8_t)tmp & (uint8_t)0x1f) << 3;
			//5 - 9 bit
			scroll_[1] = ((tmp&(uint16_t)0x3e0) >> 2) | (tmp&(uint16_t)data << 8);
			++writex2_;
			break;
		}
		// 写入高字节
		else {
			vramaddr_ = (vramaddr_ & (uint16_t)0x00FF) | ((uint16_t)data << 8);
		}
		++writex2_;
		break;
	case 7:
		// 0x2007: Data ($2007) <> read/write
		// PPU VRAM数据端
		WriteAddressOfPPU(vramaddr_, data);
		vramaddr_ += (uint16_t)((ctrl_ & PPU2000_VINC32) ? 32 : 1);
		break;
	}
}

