#pragma once
/*
		3.类的顺序错误
		如果一个类的成员中包含另一个类，则被包含的类的生命一定要在包含的类之前。
*/

#include <stdint.h>
#include <assert.h>
#include "sfc_6502.h"


// nes的cpu把最后几个地址称为向量
enum sfc_cpu_vector {
	SFC_VECTOR_NMI=0xFFFA,
	SFC_VECTOR_RESET=0xFFFC,
	SFC_VECTOR_IRQBRK=0xFFFE,
};

//====================================
//CPU寄存器
class sfc_cpu_register_t {
public:
	uint16_t& get_program_counter();

	uint8_t& get_status() { return status; }

	uint8_t& get_accumulator() { return accumulator; }

	uint8_t& get_x_index() { return x_index; }

	uint8_t& get_y_index() { return y_index; }

	uint8_t& get_stack_pointer() { return stack_pointer; }

	uint8_t& get_unused() { return unused; }



private:
	//指令计数器
	uint16_t program_counter;
	//状态寄存器
	uint8_t status;
	//累加寄存器
	uint8_t accumulator;
	//X变址寄存器
	uint8_t x_index;
	//Y变址寄存器
	uint8_t y_index;
	//栈指针
	uint8_t stack_pointer;
	//保留对齐用
	uint8_t unused;
};


//=====================
//状态寄存器标志
enum sfc_status_index {
	SFC_INDEX_C = 0,
	SFC_INDEX_Z = 1,
	SFC_INDEX_I = 2,
	SFC_INDEX_D = 3,
	SFC_INDEX_B = 4,
	SFC_INDEX_R = 5,
	SFC_INDEX_V = 6,
	SFC_INDEX_S = 7,
	SFC_INDEX_N = SFC_INDEX_S
};
//状态寄存器标记
enum sfc_status_flag {
	SFC_FLAG_C = 1 << 0,		//进位标记(Carry flag)
	SFC_FLAG_Z = 1 << 1,		//零标记(Zero flag)
	SFC_FLAG_I = 1 << 2,		//禁止中断(Irq disabled flag)
	SFC_FLAG_D = 1 << 3,		//十进制模式(Decimal mode flag)
	SFC_FLAG_B = 1 << 4,		//软件中断(BRK flag)
	SFC_FLAG_R = 1 << 5,		//保留标记(Reserved)，一直为1
	SFC_FLAG_V = 1 << 6,		//溢出标记(Overflow flag)
	SFC_FLAG_S = 1 << 7,		//符号标记(Sign flag)
	SFC_FLAG_N = SFC_FLAG_S	//又叫(Negative flag)
};
//=====================================================
// 实用宏定义

// 寄存器
#define SFC_REG (registers_)
#define SFC_PC (registers_.get_program_counter())
#define SFC_SP (registers_.get_stack_pointer())
#define SFC_A (registers_.get_accumulator())
#define SFC_X (registers_.get_x_index())
#define SFC_Y (registers_.get_y_index())
#define SFC_P (registers_.get_status())

// if中判断用FLAG
#define SFC_CF (SFC_P & (uint8_t)SFC_FLAG_C)
#define SFC_ZF (SFC_P & (uint8_t)SFC_FLAG_Z)
#define SFC_IF (SFC_P & (uint8_t)SFC_FLAG_I)
#define SFC_DF (SFC_P & (uint8_t)SFC_FLAG_D)
#define SFC_BF (SFC_P & (uint8_t)SFC_FLAG_B)
#define SFC_VF (SFC_P & (uint8_t)SFC_FLAG_V)
#define SFC_SF (SFC_P & (uint8_t)SFC_FLAG_S)
// 将FLAG将变为1
#define SFC_CF_SE (SFC_P |= (uint8_t)SFC_FLAG_C)
#define SFC_ZF_SE (SFC_P |= (uint8_t)SFC_FLAG_Z)
#define SFC_IF_SE (SFC_P |= (uint8_t)SFC_FLAG_I)
#define SFC_DF_SE (SFC_P |= (uint8_t)SFC_FLAG_D)
#define SFC_BF_SE (SFC_P |= (uint8_t)SFC_FLAG_B)
#define SFC_RF_SE (SFC_P |= (uint8_t)SFC_FLAG_R)
#define SFC_VF_SE (SFC_P |= (uint8_t)SFC_FLAG_V)
#define SFC_SF_SE (SFC_P |= (uint8_t)SFC_FLAG_S)
// 将FLAG将变为0
#define SFC_CF_CL (SFC_P &= ~(uint8_t)SFC_FLAG_C)
#define SFC_ZF_CL (SFC_P &= ~(uint8_t)SFC_FLAG_Z)
#define SFC_IF_CL (SFC_P &= ~(uint8_t)SFC_FLAG_I)
#define SFC_DF_CL (SFC_P &= ~(uint8_t)SFC_FLAG_D)
#define SFC_BF_CL (SFC_P &= ~(uint8_t)SFC_FLAG_B)
#define SFC_VF_CL (SFC_P &= ~(uint8_t)SFC_FLAG_V)
#define SFC_SF_CL (SFC_P &= ~(uint8_t)SFC_FLAG_S)
// 将FLAG将变为0或者1
#define SFC_CF_IF(x) (x ? SFC_CF_SE : SFC_CF_CL);
#define SFC_ZF_IF(x) (x ? SFC_ZF_SE : SFC_ZF_CL);
#define SFC_OF_IF(x) (x ? SFC_IF_SE : SFC_IF_CL);
#define SFC_DF_IF(x) (x ? SFC_DF_SE : SFC_DF_CL);
#define SFC_BF_IF(x) (x ? SFC_BF_SE : SFC_BF_CL);
#define SFC_VF_IF(x) (x ? SFC_VF_SE : SFC_VF_CL);
#define SFC_SF_IF(x) (x ? SFC_SF_SE : SFC_SF_CL);

// 实用函数
#define SFC_READ(a) sfc_read_cpu_address(a)
#define SFC_PUSH(a) (main_memory + 0x100)[SFC_SP--] = a;
#define SFC_POP() (main_memory + 0x100)[++SFC_SP];
#define SFC_WRITE(a,v) sfc_write_cpu_address(a, v )
#define CHECK_ZSFLAG(x) { SFC_SF_IF(x & (uint8_t)0x80); SFC_ZF_IF(x == 0); }
#define OP(n, a, o) \
case 0x##n:\
{           \
    const uint16_t address = sfc_addressing_##a();\
    sfc_operation_##o(address);\
    break;\
}
//=====================================================
class sfc_cpu {
public:
	//read cpu address
	uint8_t sfc_read_cpu_address(uint16_t address);
	//write cpu address
	void sfc_write_cpu_address(uint16_t address, uint8_t data);

	//寄存器
	sfc_cpu_register_t registers_;

	//程序内存仓库(bank)/窗口(window)
	uint8_t* prg_banks[0x10000 >> 13];

	//工作(work)/保存(save)内存
	uint8_t save_memory[8 * 1024];

	//主内存
	uint8_t main_memory[2 * 1024];

	void sfc_cpu_execute_one();


private:



	/// <summary>
	/// 寻址方式: 立即寻址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	    uint16_t sfc_addressing_IMM() {
		const uint16_t address = SFC_PC;
		SFC_PC++;
		return address;
	}

	/// <summary>
	/// 寻址方式: 绝对寻址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
		//保存的是16位地址 address0是低位 1是高位
	    uint16_t sfc_addressing_ABS() {
		const uint8_t address0 = SFC_READ(SFC_PC++);
		const uint8_t address1 = SFC_READ(SFC_PC++);
		return (uint16_t)address0 | (uint16_t)((uint16_t)address1 << 8);
	}

	/// <summary>
	/// 寻址方式: 绝对X变址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	    uint16_t sfc_addressing_ABX() {
		const uint16_t base = sfc_addressing_ABS();
		return base + SFC_X;
	}

	/// <summary>
	/// 寻址方式: 绝对Y变址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	    uint16_t sfc_addressing_ABY() {
		const uint16_t base = sfc_addressing_ABS();
		return base + SFC_Y;
	}


	/// <summary>
	/// 寻址方式: 零页寻址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	    uint16_t sfc_addressing_ZPG() {
		const uint16_t address = SFC_READ(SFC_PC++);
		return address;
	}

	/// <summary>
	/// 寻址方式: 零页X变址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	    uint16_t sfc_addressing_ZPX() {
		const uint16_t base = sfc_addressing_ZPG();
		const uint16_t index = base + SFC_X;
		return index & (uint16_t)0x00FF;
	}

	/// <summary>
	/// 寻址方式: 零页Y变址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	    uint16_t sfc_addressing_ZPY() {
		const uint16_t base = sfc_addressing_ZPG();
		const uint16_t index = base + SFC_Y;
		return index & (uint16_t)0x00FF;
	}

	/// <summary>
	/// 寻址方式: 间接X变址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	    uint16_t sfc_addressing_INX() {
		uint8_t base = SFC_READ(SFC_PC++) + SFC_X;
		const uint8_t address0 = SFC_READ(base++);
		const uint8_t address1 = SFC_READ(base++);
		return (uint16_t)address0 | (uint16_t)((uint16_t)address1 << 8);
	}

	/// <summary>
	/// 寻址方式: 间接Y变址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	    uint16_t sfc_addressing_INY() {
		uint8_t base = SFC_READ(SFC_PC++);
		const uint8_t address0 = SFC_READ(base++);
		const uint8_t address1 = SFC_READ(base++);
		const uint16_t address
			= (uint16_t)address0
			| (uint16_t)((uint16_t)address1 << 8)
			;
		return address + SFC_Y;
	}

	/// <summary>
	/// 寻址方式: 间接寻址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	    uint16_t sfc_addressing_IND() {
		// 读取地址
		const uint16_t base1 = sfc_addressing_ABS();
		// 刻意实现6502的BUG
		const uint16_t base2
			= (base1 & (uint16_t)0xFF00)
			| ((base1 + 1) & (uint16_t)0x00FF)
			;
		// 读取地址
		const uint16_t address
			= (uint16_t)SFC_READ(base1)
			| (uint16_t)((uint16_t)SFC_READ(base2) << 8)
			;
		return address;
	}

	/// <summary>
	/// 寻址方式: 相对寻址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	    uint16_t sfc_addressing_REL() {
		const uint8_t data = SFC_READ(SFC_PC++);
		const uint16_t address = SFC_PC + (int8_t)data;
		return address;
	}


	// ---------------------------------- 指令

	/// <summary>
	/// UNK: Unknown
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_UNK(uint16_t address) {
		assert(!"UNKNOWN INS");
	}

	/// <summary>
	/// SHY
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_SHY(uint16_t address) {
		sfc_operation_UNK(address);
	}

	/// <summary>
	/// SHX
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_SHX(uint16_t address) {
		sfc_operation_UNK(address);
	}

	/// <summary>
	/// TAS
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_TAS(uint16_t address) {
		sfc_operation_UNK(address);
	}

	/// <summary>
	/// AHX
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_AHX(uint16_t address) {
		sfc_operation_UNK(address);
	}

	/// <summary>
	/// XAA
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_XAA(uint16_t address) {
		sfc_operation_UNK(address);
	}

	/// <summary>
	/// LAS
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_LAS(uint16_t address) {
		sfc_operation_UNK(address);
	}

	/// <summary>
	/// SRE: Shift Right then "Exclusive-Or" - LSR + EOR
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_SRE(uint16_t address) {
		// LSR
		uint8_t data = SFC_READ(address);
		SFC_CF_IF(data & 1);
		data >>= 1;
		SFC_WRITE(address, data);
		// EOR
		SFC_A ^= data;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// SLO - Shift Left then 'Or' - ASL + ORA
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_SLO(uint16_t address) {
		// ASL
		uint8_t data = SFC_READ(address);
		SFC_CF_IF(data & (uint8_t)0x80);
		data <<= 1;
		SFC_WRITE(address, data);
		// ORA
		SFC_A |= data;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// RRA: Rotate Right then Add with Carry - ROR + ADC
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_RRA(uint16_t address) {
		// ROR
		uint16_t result16_ror = SFC_READ(address);
		result16_ror |= ((uint16_t)SFC_CF) << (8 - SFC_INDEX_C);
		const uint16_t tmpcf = result16_ror & 1;
		result16_ror >>= 1;
		const uint8_t result8_ror = (uint8_t)result16_ror;
		SFC_WRITE(address, result8_ror);
		// ADC
		const uint8_t src = result8_ror;
		const uint16_t result16 = (uint16_t)SFC_A + (uint16_t)src + tmpcf;
		SFC_CF_IF(result16 >> 8);
		const uint8_t result8 = (uint8_t)result16;
		SFC_VF_IF(!((SFC_A ^ src) & 0x80) && ((SFC_A ^ result8) & 0x80));
		SFC_A = result8;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// RLA: Rotate Left then 'And' - ROL + AND
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_RLA(uint16_t address) {
		// ROL
		uint16_t result16 = SFC_READ(address);
		result16 <<= 1;
		result16 |= ((uint16_t)SFC_CF) >> (SFC_INDEX_C);
		SFC_CF_IF(result16 & (uint16_t)0x100);
		const uint8_t result8 = (uint8_t)result16;
		SFC_WRITE(address, result8);
		// AND
		SFC_A &= result8;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// ISB: Increment memory then Subtract with Carry - INC + SBC
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ISB(uint16_t address) {
		// INC
		uint8_t data = SFC_READ(address);
		++data;
		SFC_WRITE(address, data);
		// SBC
		const uint8_t src = data;
		const uint16_t result16 = (uint16_t)SFC_A - (uint16_t)src - (SFC_CF ? 0 : 1);
		SFC_CF_IF(!(result16 >> 8));
		const uint8_t result8 = (uint8_t)result16;
		SFC_VF_IF(((SFC_A ^ src) & 0x80) && ((SFC_A ^ result8) & 0x80));
		SFC_A = result8;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// ISC
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ISC(uint16_t address) {
		sfc_operation_UNK(address);
	}

	/// <summary>
	/// DCP: Decrement memory then Compare with A - DEC + CMP
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_DCP(uint16_t address) {
		// DEC
		uint8_t data = SFC_READ(address);
		--data;
		SFC_WRITE(address, data);
		// CMP
		const uint16_t result16 = (uint16_t)SFC_A - (uint16_t)data;
		SFC_CF_IF(!(result16 & (uint16_t)0x8000));
		CHECK_ZSFLAG((uint8_t)result16);
	}

	/// <summary>
	/// SAX: Store A 'And' X - 
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_SAX(uint16_t address) {
		SFC_WRITE(address, SFC_A & SFC_X);
	}

	/// <summary>
	/// LAX: Load 'A' then Transfer X - LDA  + TAX
	/// </summary>
	/// <remarks>
	/// 非法指令
	/// </remarks>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_LAX(uint16_t address) {
		SFC_A = SFC_READ(address);
		SFC_X = SFC_A;
		CHECK_ZSFLAG(SFC_X);
	}

	/// <summary>
	/// SBX
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_SBX(uint16_t address) {
		sfc_operation_UNK(address);
	}

	/// <summary>
	/// AXS
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_AXS(uint16_t address) {
		sfc_operation_UNK(address);
	}

	/// <summary>
	/// ARR
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ARR(uint16_t address) {
		sfc_operation_UNK(address);
	}

	/// <summary>
	/// AAC
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_AAC(uint16_t address) {
		sfc_operation_UNK(address);
	}

	/// <summary>
	/// ANC
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ANC(uint16_t address) {
		sfc_operation_UNK(address);
	}

	/// <summary>
	/// ASR
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ASR(uint16_t address) {
		sfc_operation_UNK(address);
	}

	/// <summary>
	/// ALR
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ALR(uint16_t address) {
		sfc_operation_UNK(address);
	}

	/// <summary>
	/// RTI: Return from I
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_RTI(uint16_t address) {
		// P
		SFC_P = SFC_POP();
		SFC_RF_SE;
		SFC_BF_CL;
		// PC
		const uint8_t pcl = SFC_POP();
		const uint8_t pch = SFC_POP();
		SFC_PC
			= (uint16_t)pcl
			| (uint16_t)pch << 8
			;
	}

	/// <summary>
	/// BRK
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_BRK(uint16_t address) {
		sfc_operation_UNK(address);
	}

	/// <summary>
	/// NOP: No Operation
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_NOP(uint16_t address) {

	}

	/// <summary>
	/// RTS: Return from Subroutine
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_RTS(uint16_t address) {
		const uint8_t pcl = SFC_POP();
		const uint8_t pch = SFC_POP();
		SFC_PC
			= (uint16_t)pcl
			| (uint16_t)pch << 8
			;
		SFC_PC++;
	}

	/// <summary>
	/// JSR: Jump to Subroutine
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_JSR(uint16_t address) {
		const uint16_t pc1 = SFC_PC - 1;
		SFC_PUSH((uint8_t)(pc1 >> 8));
		SFC_PUSH((uint8_t)(pc1));
		SFC_PC = address;
	}

	/// <summary>
	/// BVC: Branch if Overflow Clear
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_BVC(uint16_t address) {
		if (!SFC_VF) SFC_PC = address;
	}

	/// <summary>
	/// BVC: Branch if Overflow Set
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_BVS(uint16_t address) {
		if (SFC_VF) SFC_PC = address;
	}

	/// <summary>
	/// BPL: Branch if Plus
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_BPL(uint16_t address) {
		if (!SFC_SF) SFC_PC = address;
	}

	/// <summary>
	/// BMI: Branch if Minus
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_BMI(uint16_t address) {
		if (SFC_SF) SFC_PC = address;
	}

	/// <summary>
	/// BCC: Branch if Carry Clear
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_BCC(uint16_t address) {
		if (!SFC_CF) SFC_PC = address;
	}

	/// <summary>
	/// BCS: Branch if Carry Set
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_BCS(uint16_t address) {
		if (SFC_CF) SFC_PC = address;
	}

	/// <summary>
	/// BNE: Branch if Not Equal
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_BNE(uint16_t address) {
		if (!SFC_ZF) SFC_PC = address;
	}

	/// <summary>
	/// BEQ: Branch if Equal
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_BEQ(uint16_t address) {
		if (SFC_ZF) SFC_PC = address;
	}

	/// <summary>
	/// JMP
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_JMP(uint16_t address) {
		SFC_PC = address;
	}

	/// <summary>
	/// PLP: Pull P
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_PLP(uint16_t address) {
		SFC_P = SFC_POP();
		SFC_RF_SE;
		SFC_BF_CL;
	}

	/// <summary>
	/// PHP: Push P
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_PHP(uint16_t address) {
		SFC_PUSH(SFC_P | (uint8_t)(SFC_FLAG_R | SFC_FLAG_B));
	}

	/// <summary>
	/// PLA: Pull A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_PLA(uint16_t address) {
		SFC_A = SFC_POP();
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// PHA: Push A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_PHA(uint16_t address) {
		SFC_PUSH(SFC_A);
	}

	/// <summary>
	/// ROR A : Rotate Right for A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_RORA(uint16_t address) {
		uint16_t result16 = SFC_A;
		result16 |= ((uint16_t)SFC_CF) << (8 - SFC_INDEX_C);
		SFC_CF_IF(result16 & 1);
		result16 >>= 1;
		SFC_A = (uint8_t)result16;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// ROR: Rotate Right
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ROR(uint16_t address) {
		uint16_t result16 = SFC_READ(address);
		result16 |= ((uint16_t)SFC_CF) << (8 - SFC_INDEX_C);
		SFC_CF_IF(result16 & 1);
		result16 >>= 1;
		const uint8_t result8 = (uint8_t)result16;
		SFC_WRITE(address, result8);
		CHECK_ZSFLAG(result8);
	}

	/// <summary>
	/// ROL: Rotate Left
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ROL(uint16_t address) {
		uint16_t result16 = SFC_READ(address);
		result16 <<= 1;
		result16 |= ((uint16_t)SFC_CF) >> (SFC_INDEX_C);
		SFC_CF_IF(result16 & (uint16_t)0x100);
		const uint8_t result8 = (uint8_t)result16;
		SFC_WRITE(address, result8);
		CHECK_ZSFLAG(result8);
	}

	/// <summary>
	/// ROL A : Rotate Left for A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ROLA(uint16_t address) {
		uint16_t result16 = SFC_A;
		result16 <<= 1;
		result16 |= ((uint16_t)SFC_CF) >> (SFC_INDEX_C);
		SFC_CF_IF(result16 & (uint16_t)0x100);
		SFC_A = (uint8_t)result16;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// LSR: Logical Shift Right
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_LSR(uint16_t address) {
		uint8_t data = SFC_READ(address);
		SFC_CF_IF(data & 1);
		data >>= 1;
		SFC_WRITE(address, data);
		CHECK_ZSFLAG(data);
	}

	/// <summary>
	/// LSR A : Logical Shift Right for A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_LSRA(uint16_t address) {
		SFC_CF_IF(SFC_A & 1);
		SFC_A >>= 1;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// ASL: Arithmetic Shift Left
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ASL(uint16_t address) {
		uint8_t data = SFC_READ(address);
		SFC_CF_IF(data & (uint8_t)0x80);
		data <<= 1;
		SFC_WRITE(address, data);
		CHECK_ZSFLAG(data);
	}

	/// <summary>
	/// ASL A : Arithmetic Shift Left for A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ASLA(uint16_t address) {
		SFC_CF_IF(SFC_A & (uint8_t)0x80);
		SFC_A <<= 1;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// BIT: Bit Test
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_BIT(uint16_t address) {
		const uint8_t value = SFC_READ(address);
		SFC_VF_IF(value & (uint8_t)(1 << 6));
		SFC_SF_IF(value & (uint8_t)(1 << 7));
		SFC_ZF_IF(!(SFC_A & value))
	}

	/// <summary>
	/// CPY: Compare memory with Y
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_CPY(uint16_t address) {
		const uint16_t result16 = (uint16_t)SFC_Y - (uint16_t)SFC_READ(address);
		SFC_CF_IF(!(result16 & (uint16_t)0x8000));
		CHECK_ZSFLAG((uint8_t)result16);
	}

	/// <summary>
	/// CPX
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_CPX(uint16_t address) {
		const uint16_t result16 = (uint16_t)SFC_X - (uint16_t)SFC_READ(address);
		SFC_CF_IF(!(result16 & (uint16_t)0x8000));
		CHECK_ZSFLAG((uint8_t)result16);
	}

	/// <summary>
	/// CMP: Compare memory with A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_CMP(uint16_t address) {
		const uint16_t result16 = (uint16_t)SFC_A - (uint16_t)SFC_READ(address);
		SFC_CF_IF(!(result16 & (uint16_t)0x8000));
		CHECK_ZSFLAG((uint8_t)result16);
	}

	/// <summary>
	/// SEI: Set I
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_SEI(uint16_t address) {
		SFC_IF_SE;
	}

	/// <summary>
	/// CLI - Clear I
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_CLI(uint16_t address) {
		sfc_operation_UNK(address);
	}

	/// <summary>
	/// CLV: Clear V
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_CLV(uint16_t address) {
		SFC_VF_CL;
	}

	/// <summary>
	/// SED: Set D
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_SED(uint16_t address) {
		SFC_DF_SE;
	}

	/// <summary>
	/// CLD: Clear D
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_CLD(uint16_t address) {
		SFC_DF_CL;
	}

	/// <summary>
	/// SEC: Set Carry
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_SEC(uint16_t address) {
		SFC_CF_SE;
	}

	/// <summary>
	/// CLC: Clear Carry
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_CLC(uint16_t address) {
		SFC_CF_CL;
	}

	/// <summary>
	/// EOR: "Exclusive-Or" memory with A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_EOR(uint16_t address) {
		SFC_A ^= SFC_READ(address);
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// ORA: 'Or' memory with A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ORA(uint16_t address) {
		SFC_A |= SFC_READ(address);
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// AND: 'And' memory with A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_AND(uint16_t address) {
		SFC_A &= SFC_READ(address);
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// DEY: Decrement Y
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_DEY(uint16_t address) {
		SFC_Y--;
		CHECK_ZSFLAG(SFC_Y);
	}

	/// <summary>
	/// INY:  Increment Y
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_INY(uint16_t address) {
		SFC_Y++;
		CHECK_ZSFLAG(SFC_Y);
	}

	/// <summary>
	/// DEX: Decrement X
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_DEX(uint16_t address) {
		SFC_X--;
		CHECK_ZSFLAG(SFC_X);
	}

	/// <summary>
	/// INX
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_INX(uint16_t address) {
		SFC_X++;
		CHECK_ZSFLAG(SFC_X);
	}

	/// <summary>
	/// DEC: Decrement memory
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_DEC(uint16_t address) {
		uint8_t data = SFC_READ(address);
		--data;
		SFC_WRITE(address, data);
		CHECK_ZSFLAG(data);
	}

	/// <summary>
	/// INC: Increment memory
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_INC(uint16_t address) {
		uint8_t data = SFC_READ(address);
		++data;
		SFC_WRITE(address, data);
		CHECK_ZSFLAG(data);
	}

	/// <summary>
	/// SBC: Subtract with Carry
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_SBC(uint16_t address) {
		const uint8_t src = SFC_READ(address);
		const uint16_t result16 = (uint16_t)SFC_A - (uint16_t)src - (SFC_CF ? 0 : 1);
		SFC_CF_IF(!(result16 >> 8));
		const uint8_t result8 = (uint8_t)result16;
		SFC_VF_IF(((SFC_A ^ src) & 0x80) && ((SFC_A ^ result8) & 0x80));
		SFC_A = result8;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// ADC: Add with Carry
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ADC(uint16_t address) {
		const uint8_t src = SFC_READ(address);
		const uint16_t result16 = (uint16_t)SFC_A + (uint16_t)src + (SFC_CF ? 1 : 0);
		SFC_CF_IF(result16 >> 8);
		const uint8_t result8 = (uint8_t)result16;
		SFC_VF_IF(!((SFC_A ^ src) & 0x80) && ((SFC_A ^ result8) & 0x80));
		SFC_A = result8;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// TXS: Transfer X to SP
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_TXS(uint16_t address) {
		SFC_SP = SFC_X;
	}

	/// <summary>
	/// TSX: Transfer SP to X
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_TSX(uint16_t address) {
		SFC_X = SFC_SP;
		CHECK_ZSFLAG(SFC_X);
	}

	/// <summary>
	/// TYA: Transfer Y to A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_TYA(uint16_t address) {
		SFC_A = SFC_Y;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// TAY: Transfer A to Y
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_TAY(uint16_t address) {
		SFC_Y = SFC_A;
		CHECK_ZSFLAG(SFC_Y);
	}

	/// <summary>
	/// TXA: Transfer X to A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_TXA(uint16_t address) {
		SFC_A = SFC_X;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// TAX: Transfer A to X
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_TAX(uint16_t address) {
		SFC_X = SFC_A;
		CHECK_ZSFLAG(SFC_X);
	}

	/// <summary>
	/// STY: Store 'Y'
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_STY(uint16_t address) {
		SFC_WRITE(address, SFC_Y);
	}

	/// <summary>
	/// STX: Store X
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_STX(uint16_t address) {
		SFC_WRITE(address, SFC_X);
	}

	/// <summary>
	/// STA: Store 'A'
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_STA(uint16_t address) {
		SFC_WRITE(address, SFC_A);
	}

	/// <summary>
	/// LDY: Load 'Y'
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_LDY(uint16_t address) {
		SFC_Y = SFC_READ(address);
		CHECK_ZSFLAG(SFC_Y);
	}

	/// <summary>
	/// LDX: Load X
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_LDX(uint16_t address) {
		SFC_X = SFC_READ(address);
		CHECK_ZSFLAG(SFC_X);
	}

	/// <summary>
	/// LDA: Load A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_LDA(uint16_t address) {
		SFC_A = SFC_READ(address);
		CHECK_ZSFLAG(SFC_A);
	}
};



