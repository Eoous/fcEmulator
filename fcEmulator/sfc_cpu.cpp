#include "sfc_cpu.h"

uint16_t& sfc_cpu_register_t::get_program_counter()
{
	return program_counter;
}

//
uint8_t sfc_cpu::sfc_read_cpu_address(uint16_t address) {
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
		return pppu_->sfc_read_ppu_register_via_cpu(address);
	case 2:
		//高三位为2，[$4000,$6000) :pAPU寄存器 扩展ROM区
		if (address < 0x4020) {
		}
		else assert(!"NOT IMPL");
		return 0;
	case 3:
		//高三位为3，[$6000,$8000) :存档 SRAM区
		return save_memory[address & (uint16_t)0x1fff];
	case 4:case 5:case 6:case 7:
		//高一位为1，[$8000,$10000):程序PRG-ROM区
		return prg_banks[address >> 13][address & (uint16_t)0x1fff];
	}
	assert(!"invalid address");
	return 0;
}


void sfc_cpu::sfc_write_cpu_address(uint16_t address, uint8_t data) {
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
		pppu_->sfc_write_ppu_register_via_cpu(address, data);		
		return;
	case 2:
		//高三位为2，[$4000,$6000) :pAPU寄存器 扩展ROM区
		// 前0x20字节为APU, I / O寄存器
		if (address < 0x4020) {

		}
		else assert(!"NOT IMPL");
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



void sfc_cpu::sfc_cpu_execute_one() {
	const uint8_t opcode = SFC_READ(SFC_PC++);

	//每种指令有相对应的寻址模式
	//LDA 8种 
	//LDX和LDY 5种
	switch (opcode)
	{
			OP(00, IMP, BRK) OP(01, INX, ORA) OP(02, UNK, UNK)  OP(03, INX, SLO) OP(04, ZPG, NOP) OP(05, ZPG, ORA) OP(06, ZPG, ASL) OP(07, ZPG, SLO)
			OP(08, IMP, PHP) OP(09, IMM, ORA) OP(0A, IMP, ASLA) OP(0B, IMM, ANC) OP(0C, ABS, NOP) OP(0D, ABS, ORA) OP(0E, ABS, ASL) OP(0F, ABS, SLO)
			OP(10, REL, BPL) OP(11, INY, ORA) OP(12, UNK, UNK)  OP(13, INY, SLO) OP(14, ZPX, NOP) OP(15, ZPX, ORA) OP(16, ZPX, ASL) OP(17, ZPX, SLO)
			OP(18, IMP, CLC) OP(19, ABY, ORA) OP(1A, IMP, NOP)  OP(1B, ABY, SLO) OP(1C, ABX, NOP) OP(1D, ABX, ORA) OP(1E, ABX, ASL) OP(1F, ABX, SLO)
			OP(20, ABS, JSR) OP(21, INX, AND) OP(22, UNK, UNK)  OP(23, INX, RLA) OP(24, ZPG, BIT) OP(25, ZPG, AND) OP(26, ZPG, ROL) OP(27, ZPG, RLA)
			OP(28, IMP, PLP) OP(29, IMM, AND) OP(2A, IMP, ROLA) OP(2B, IMM, ANC) OP(2C, ABS, BIT) OP(2D, ABS, AND) OP(2E, ABS, ROL) OP(2F, ABS, RLA)
			OP(30, REL, BMI) OP(31, INY, AND) OP(32, UNK, UNK)  OP(33, INY, RLA) OP(34, ZPX, NOP) OP(35, ZPX, AND) OP(36, ZPX, ROL) OP(37, ZPX, RLA)
			OP(38, IMP, SEC) OP(39, ABY, AND) OP(3A, IMP, NOP)  OP(3B, ABY, RLA) OP(3C, ABX, NOP) OP(3D, ABX, AND) OP(3E, ABX, ROL) OP(3F, ABX, RLA)
			OP(40, IMP, RTI) OP(41, INX, EOR) OP(42, UNK, UNK)  OP(43, INX, SRE) OP(44, ZPG, NOP) OP(45, ZPG, EOR) OP(46, ZPG, LSR) OP(47, ZPG, SRE)
			OP(48, IMP, PHA) OP(49, IMM, EOR) OP(4A, IMP, LSRA) OP(4B, IMM, ASR) OP(4C, ABS, JMP) OP(4D, ABS, EOR) OP(4E, ABS, LSR) OP(4F, ABS, SRE)
			OP(50, REL, BVC) OP(51, INY, EOR) OP(52, UNK, UNK)  OP(53, INY, SRE) OP(54, ZPX, NOP) OP(55, ZPX, EOR) OP(56, ZPX, LSR) OP(57, ZPX, SRE)
			OP(58, IMP, CLI) OP(59, ABY, EOR) OP(5A, IMP, NOP)  OP(5B, ABY, SRE) OP(5C, ABX, NOP) OP(5D, ABX, EOR) OP(5E, ABX, LSR) OP(5F, ABX, SRE)
			OP(60, IMP, RTS) OP(61, INX, ADC) OP(62, UNK, UNK)  OP(63, INX, RRA) OP(64, ZPG, NOP) OP(65, ZPG, ADC) OP(66, ZPG, ROR) OP(67, ZPG, RRA)
			OP(68, IMP, PLA) OP(69, IMM, ADC) OP(6A, IMP, RORA) OP(6B, IMM, ARR) OP(6C, IND, JMP) OP(6D, ABS, ADC) OP(6E, ABS, ROR) OP(6F, ABS, RRA)
			OP(70, REL, BVS) OP(71, INY, ADC) OP(72, UNK, UNK)  OP(73, INY, RRA) OP(74, ZPX, NOP) OP(75, ZPX, ADC) OP(76, ZPX, ROR) OP(77, ZPX, RRA)
			OP(78, IMP, SEI) OP(79, ABY, ADC) OP(7A, IMP, NOP)  OP(7B, ABY, RRA) OP(7C, ABX, NOP) OP(7D, ABX, ADC) OP(7E, ABX, ROR) OP(7F, ABX, RRA)
			OP(80, IMM, NOP) OP(81, INX, STA) OP(82, IMM, NOP)  OP(83, INX, SAX) OP(84, ZPG, STY) OP(85, ZPG, STA) OP(86, ZPG, STX) OP(87, ZPG, SAX)
			OP(88, IMP, DEY) OP(89, IMM, NOP) OP(8A, IMP, TXA)  OP(8B, IMM, XAA) OP(8C, ABS, STY) OP(8D, ABS, STA) OP(8E, ABS, STX) OP(8F, ABS, SAX)
			OP(90, REL, BCC) OP(91, INY, STA) OP(92, UNK, UNK)  OP(93, INY, AHX) OP(94, ZPX, STY) OP(95, ZPX, STA) OP(96, ZPY, STX) OP(97, ZPY, SAX)
			OP(98, IMP, TYA) OP(99, ABY, STA) OP(9A, IMP, TXS)  OP(9B, ABY, TAS) OP(9C, ABX, SHY) OP(9D, ABX, STA) OP(9E, ABY, SHX) OP(9F, ABY, AHX)
			OP(A0, IMM, LDY) OP(A1, INX, LDA) OP(A2, IMM, LDX)  OP(A3, INX, LAX) OP(A4, ZPG, LDY) OP(A5, ZPG, LDA) OP(A6, ZPG, LDX) OP(A7, ZPG, LAX)
			OP(A8, IMP, TAY) OP(A9, IMM, LDA) OP(AA, IMP, TAX)  OP(AB, IMM, LAX) OP(AC, ABS, LDY) OP(AD, ABS, LDA) OP(AE, ABS, LDX) OP(AF, ABS, LAX)
			OP(B0, REL, BCS) OP(B1, INY, LDA) OP(B2, UNK, UNK)  OP(B3, INY, LAX) OP(B4, ZPX, LDY) OP(B5, ZPX, LDA) OP(B6, ZPY, LDX) OP(B7, ZPY, LAX)
			OP(B8, IMP, CLV) OP(B9, ABY, LDA) OP(BA, IMP, TSX)  OP(BB, ABY, LAS) OP(BC, ABX, LDY) OP(BD, ABX, LDA) OP(BE, ABY, LDX) OP(BF, ABY, LAX)
			OP(C0, IMM, CPY) OP(C1, INX, CMP) OP(C2, IMM, NOP)  OP(C3, INX, DCP) OP(C4, ZPG, CPY) OP(C5, ZPG, CMP) OP(C6, ZPG, DEC) OP(C7, ZPG, DCP)
			OP(C8, IMP, INY) OP(C9, IMM, CMP) OP(CA, IMP, DEX)  OP(CB, IMM, AXS) OP(CC, ABS, CPY) OP(CD, ABS, CMP) OP(CE, ABS, DEC) OP(CF, ABS, DCP)
			OP(D0, REL, BNE) OP(D1, INY, CMP) OP(D2, UNK, UNK)  OP(D3, INY, DCP) OP(D4, ZPX, NOP) OP(D5, ZPX, CMP) OP(D6, ZPX, DEC) OP(D7, ZPX, DCP)
			OP(D8, IMP, CLD) OP(D9, ABY, CMP) OP(DA, IMP, NOP)  OP(DB, ABY, DCP) OP(DC, ABX, NOP) OP(DD, ABX, CMP) OP(DE, ABX, DEC) OP(DF, ABX, DCP)
			OP(E0, IMM, CPX) OP(E1, INX, SBC) OP(E2, IMM, NOP)  OP(E3, INX, ISB) OP(E4, ZPG, CPX) OP(E5, ZPG, SBC) OP(E6, ZPG, INC) OP(E7, ZPG, ISB)
			OP(E8, IMP, INX) OP(E9, IMM, SBC) OP(EA, IMP, NOP)  OP(EB, IMM, SBC) OP(EC, ABS, CPX) OP(ED, ABS, SBC) OP(EE, ABS, INC) OP(EF, ABS, ISB)
			OP(F0, REL, BEQ) OP(F1, INY, SBC) OP(F2, UNK, UNK)  OP(F3, INY, ISB) OP(F4, ZPX, NOP) OP(F5, ZPX, SBC) OP(F6, ZPX, INC) OP(F7, ZPX, ISB)
			OP(F8, IMP, SED) OP(F9, ABY, SBC) OP(FA, IMP, NOP)  OP(FB, ABY, ISB) OP(FC, ABX, NOP) OP(FD, ABX, SBC) OP(FE, ABX, INC) OP(FF, ABX, ISB)
	}
}


void sfc_cpu::sfc_operation_NMI() {
	const uint8_t pch = (uint8_t)((SFC_PC) >> 8);
	const uint8_t pcl = (uint8_t)SFC_PC;
	SFC_PUSH(pch);
	SFC_PUSH(pcl);
	SFC_PUSH(SFC_P | (uint8_t)(SFC_FLAG_R));
	SFC_IF_SE;
	const uint8_t pcl2 = sfc_read_cpu_address(SFC_VECTOR_NMI + 0);
	const uint8_t pch2 = sfc_read_cpu_address(SFC_VECTOR_NMI + 1);
	registers_.get_program_counter() = (uint16_t)pcl2 | (uint16_t)pch2 << 8;
}