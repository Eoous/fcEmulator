#include "sfc_famicom_t.h"
#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "sfc_ppu_t.h"
#include "common\d2d_interface.h"
uint32_t palette_data[16];
SFC_EXTERN_C void user_input(int index, unsigned char data) SFC_NOEXCEPT {

}
SFC_EXTERN_C int sub_render(void* rgba) SFC_NOEXCEPT {
	return 0;
}
SFC_EXTERN_C void qsave() SFC_NOEXCEPT {

}
SFC_EXTERN_C void qload() SFC_NOEXCEPT {

}

uint32_t get_pixel(unsigned x, unsigned y, const uint8_t* nt, const uint8_t* bg) {
	// 获取所在名称表
	const unsigned id = (x >> 3) + (y >> 3) * 32;
	const uint32_t name = nt[id];
	// 查找对应图样表
	const uint8_t* nowp0 = bg + name * 16;		//前8字节代表2位中的低位
	const uint8_t* nowp1 = nowp0 + 8;			//后8字节代表2位中的高位
	// Y坐标为平面内偏移						  共8个字节 y为第offset个字节
	const int offset = y & 0x7;
	const uint8_t p0 = nowp0[offset];
	const uint8_t p1 = nowp1[offset];
	/* X坐标为字节内偏移(字节内对应pixel)
	0 - 111		1 - 110		2 - 101		...
	1000 0000	0100 0000	0010 0000   ...
	*/
	const uint8_t shift = (~x) & 0x7;			
	const uint8_t mask = 1 << shift;					 
	/* 计算低二位	
	mask已经算出来是第几位是1了 p0和p1里面数只可能是1或0
	对应像素&运算后也是1或0
	*/
	const uint8_t low = ((p0 & mask) >> shift) | ((p1 & mask) >> shift << 1);
	/* 计算所在属性表
	每个属性表里面有4个tiles
	*/ 
	// 第aid个attr
	const unsigned aid = (x >> 5) + (y >> 5) * 8;
	const uint8_t attr = nt[aid + (32 * 30)];
	// 获取属性表内位偏移
	const uint8_t aoffset = ((x & 0x10) >> 3) | ((y & 0x10) >> 2);
	// 计算高两位
	const uint8_t high = (attr & (3 << aoffset)) >> aoffset << 2;
	// 合并作为颜色
	const uint8_t index = high | low;

	return palette_data[index];
}


/// <summary>
/// 主渲染
/// </summary>
/// <param name="rgba">The RGBA.</param>
void main_render(void* rgba,sfc_famicom_t& famicom)noexcept {
	uint32_t* data = (uint32_t*)rgba;

	for (int i = 0; i != 10000; ++i)
	{
		famicom.sfc_before_execute();
		famicom.cpu_.sfc_cpu_execute_one();
	}
	famicom.sfc_do_vblank();

	// 生成调色板颜色
	{
		for (int i = 0; i != 16; ++i) {
			palette_data[i] =sfc_stdpalette[famicom.ppu_.spindexes[i]].data;
		}
		palette_data[4 * 1] = palette_data[0];
		palette_data[4 * 2] = palette_data[0];
		palette_data[4 * 3] = palette_data[0];
	}
	// 背景
	const uint8_t* now = famicom.ppu_.banks[8];
	const uint8_t* bgp = famicom.ppu_.banks[
		(famicom.ppu_.ctrl & SFC_PPU2000_BgTabl ? 4 : 0)];
	for (unsigned i = 0; i != 256 * 240; ++i) {
		data[i] = get_pixel(i & 0xff, i >> 8, now, bgp);
	}
}


int main() {
	std::shared_ptr<sfc_famicom_t> famicom = sfc_famicom_t::getInstance(nullptr);

	auto test = famicom->get_rom_info();

	printf("ROM:PRG-ROM: %d x 16kb	CHR-ROM %d x 8kb	Mapper: %03d\n",
		(int)test.count_prgrom16kb,
		(int)test.count_chrrom_8kb,
		(int)test.mapper_number);
	uint16_t v0 = famicom->cpu_.sfc_read_cpu_address(SFC_VECTOR_NMI + 0);
	uint16_t d0 = famicom->cpu_.sfc_read_cpu_address(SFC_VECTOR_NMI + 1);
	v0 |= d0 << 8;
	uint16_t v1 = famicom->cpu_.sfc_read_cpu_address(SFC_VECTOR_RESET + 0);
	uint16_t d1 = famicom->cpu_.sfc_read_cpu_address(SFC_VECTOR_RESET + 1);
	v1 |= d1 << 8;
	uint16_t v2 = famicom->cpu_.sfc_read_cpu_address(SFC_VECTOR_IRQBRK + 0);
	uint16_t d2 = famicom->cpu_.sfc_read_cpu_address(SFC_VECTOR_IRQBRK + 1);
	v2 |= d2 << 8;

	printf("ROM: NMI: $%04X  RESET: $%04X  IRQ/BRK: $%04X\n", (int)v0, (int)v1, (int)v2);

	//===================================
	char b0[48], b1[48], b2[48];

	famicom->sfc_fc_disassembly(v0, b0);
	famicom->sfc_fc_disassembly(v1, b1);
	famicom->sfc_fc_disassembly(v2, b2);
	printf(
		"NMI:     %s\n"
		"RESET:   %s\n"
		"IRQ/BRK: %s\n",
		b0, b1, b2
	);
	printf("\n");

	main_cpp(*famicom);
	getchar();
	return 0;

}