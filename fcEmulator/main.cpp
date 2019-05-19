#include "sfc_famicom_t.h"
#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>
int main(void) {
	std::shared_ptr<sfc_famicom_t> famicom=sfc_famicom_t::getInstance(nullptr);
	auto test = famicom->get_rom_info();

	printf("ROM:PRG-ROM: %d x 16kb	CHR-ROM %d x 8kb	Mapper: %03d\n", 
		(int)test.count_prgrom16kb,
		(int)test.count_chrrom_8kb,
		(int)test.mapper_number);
	uint16_t v0 = famicom->sfc_read_cpu_address(SFC_VECTOR_NMI + 0);
	uint16_t d0 = famicom->sfc_read_cpu_address(SFC_VECTOR_NMI + 1);
	v0 |= d0 << 8;
	uint16_t v1 = famicom->sfc_read_cpu_address(SFC_VECTOR_RESET + 0);
	uint16_t d1 = famicom->sfc_read_cpu_address(SFC_VECTOR_RESET + 1);
	v1 |= d1 << 8;
	uint16_t v2 = famicom->sfc_read_cpu_address(SFC_VECTOR_IRQBRK + 0);
	uint16_t d2 = famicom->sfc_read_cpu_address(SFC_VECTOR_IRQBRK + 1);
	v2 |= d2 << 8;

	printf("ROM: NMI: $%04X  RESET: $%04X  IRQ/BRK: $%04X\n", (int)v0, (int)v1, (int)v2);
	getchar();
	return 0;

}