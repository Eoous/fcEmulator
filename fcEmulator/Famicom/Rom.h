#pragma once

#include <stdint.h>
typedef struct {
/*
	typedef unsigned char           uint8_t;
	typedef unsigned short int      uint16_t;
	typedef unsigned int            uint32_t;	*/


	uint8_t *data_prgrom;
	uint8_t *data_chrrom;

	uint32_t count_prgrom16kb;
	uint32_t count_chrrom_8kb;

	uint8_t mapper_number;

	// 是否Vertical Mirroring(否即为水平)
	uint8_t vmirroring;

	uint8_t four_screen;

	// 是否有SRAM(电池供电的)
	uint8_t save_ram;

	// 保留以对齐
	uint8_t reserved[4];

}sfc_rom_info_t;

typedef struct {
	//Identification String. Must be "NES<EOF>".
	uint32_t id;

	//PRG - ROM size LSB
	uint8_t count_prgrom16kb;

	//CHR-ROM size LSB
	uint8_t count_chrrom_8kb;

	//Flags #6
	uint8_t control1;

	//Flags #7
	uint8_t control2;

	// 保留数据 Flags#8-#15
	uint8_t reserved[8];
}sfc_nes_header_t;

// ROM control 字节 #1
enum {
	SFC_NES_VMIRROR=0x01,
	SFC_NES_SAVERAM=0x02,
	SFC_NES_TRAINER=0x04,
	SFC_NES_4SCREEN=0x08
};

// ROM control byte #2
enum {
	SFC_NES_VS_UNISYSTEM=0x01,
	SFC_NES_Playchoice10=0x02
};
