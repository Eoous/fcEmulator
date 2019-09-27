#pragma once
/*
		3.类的顺序错误
		如果一个类的成员中包含另一个类，则被包含的类的生命一定要在包含的类之前。
*/

#include <stdint.h>
#include <assert.h>
#include "../ppu/ppu.h"
#include "6502.h"

// nes的cpu把最后几个地址称为向量
enum cpuVector {
	VECTOR_NMI=0xFFFA,
	VECTOR_RESET=0xFFFC,
	VECTOR_BRK = 0xFFFE,   // 中断重定向
	VECTOR_IRQ=0xFFFE,
};

//====================================
//CPU寄存器
class Register {
public:
	uint16_t& get_program_counter();

	uint8_t& get_status() { return status_; }

	uint8_t& get_accumulator() { return accumulator_; }

	uint8_t& get_x_index() { return x_index_; }

	uint8_t& get_y_index() { return y_index_; }

	uint8_t& get_stack_pointer() { return stack_pointer_; }

	uint8_t& get_unused() { return unused_; }



private:
	//指令计数器
	uint16_t program_counter_;
	//状态寄存器
	uint8_t status_;
	//累加寄存器
	uint8_t accumulator_;
	//X变址寄存器
	uint8_t x_index_;
	//Y变址寄存器
	uint8_t y_index_;
	//栈指针
	uint8_t stack_pointer_;
	//保留对齐用
	uint8_t unused_;
};


//=====================
//状态寄存器标志
enum StatusIndex {
	INDEX_C = 0,
	INDEX_Z = 1,
	INDEX_I = 2,
	INDEX_D = 3,
	INDEX_B = 4,
	INDEX_R = 5,
	INDEX_V = 6,
	INDEX_S = 7,
	INDEX_N = INDEX_S
};
//状态寄存器标记
enum StatusFlag {
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
#define SFC_READ(a) ReadAddress(a)
#define SFC_READ_PC(a) ReadPRG(a)
#define SFC_PUSH(a) (main_memory + 0x100)[SFC_SP--] = a;
#define SFC_POP() (main_memory + 0x100)[++SFC_SP];
#define SFC_WRITE(a,v) WriteAddress(a, v )
#define CHECK_ZSFLAG(x) { SFC_SF_IF(x & (uint8_t)0x80); SFC_ZF_IF(x == 0); }

//sfc_addressing_##a == 通过寻址模式取得地址
//sfc_operation_##o(address) == 用得到的地址进行操作
#define OP(n, a, o) \
case 0x##n:\
{           \
	cycle_add+= (uint32_t)SFC_BAISC_CYCLE_##n;\
    const uint16_t address = sfc_addressing_##a(&cycle_add);\
    sfc_operation_##o(address,&cycle_add);\
    break;\
}
//=====================================================
class cpu {
public:
	//read cpu address
	uint8_t ReadAddress(uint16_t address);
	//write cpu address
	void WriteAddress(uint16_t address, uint8_t data);
	//
	uint8_t ReadPRG(uint16_t address);
	//================================================================
	//读4020
	uint8_t ReadAddress4020(uint16_t address);
	void WriteAddress4020(uint16_t address, uint8_t data);
	//================================================================
	const uint8_t* GetAddressOfDMA(uint8_t data);
	//================================================================
	//获取指令长度
	uint8_t GetInsLen(uint8_t opcode) {
		return s_opname_data[opcode].mode_inslen >> 4;
	}
	//寄存器
	Register registers_;

	//程序内存仓库(bank)/窗口(window)
	uint8_t* prg_banks_[0x10000 >> 13];

	//工作(work)/保存(save)内存
	uint8_t save_memory_[8 * 1024];

	//显存
	uint8_t video_memory_[2 * 1024];		//（FC的显存只有2kb，只能支持2屏幕）

	//4屏用额外显存
	uint8_t video_memory_ex_[2 * 1024];

	//主内存
	uint8_t main_memory[2 * 1024];

	ppu* pppu_;
	//==============================================
	// 手柄序列状态#1
	uint16_t            button_index_1_;
	// 手柄序列状态#2
	uint16_t            button_index_2_;
	// 手柄序列状态
	uint16_t            button_index_mask_;
	// 手柄按钮状态
	uint8_t             button_states_[16];

	//cpu 周期计数
	uint32_t cpu_cycle_count_;
	//====================================
	void ExecuteOnce();

	//NMI - 不可屏蔽中断
	void NMI();
	//IRQ_try
	void TryToDoIRQ();
private:


	//========================================

	// ---------------------------------- 寻址
	/// <summary>
	/// 寻址方式: 未知
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	  uint16_t sfc_addressing_UNK(uint32_t* const cycle) {
		assert(!"UNKNOWN ADDRESSING MODE");
		return 0;
	}

	/// <summary>
	/// 寻址方式: 累加器
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	  uint16_t sfc_addressing_ACC(uint32_t* const cycle) {
		return 0;
	}

	/// <summary>
	/// 寻址方式: 隐含寻址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	  uint16_t sfc_addressing_IMP(uint32_t* const cycle) {
		return 0;
	}

	/// <summary>
	/// 寻址方式: 立即寻址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	  uint16_t sfc_addressing_IMM(uint32_t* const cycle) {
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
	      uint16_t sfc_addressing_ABS(uint32_t* const cycle) {
		const uint8_t address0 = SFC_READ(SFC_PC++);
		const uint8_t address1 = SFC_READ(SFC_PC++);
		return (uint16_t)address0 | (uint16_t)((uint16_t)address1 << 8);
	}

	/// <summary>
	/// 寻址方式: 绝对X变址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	      uint16_t sfc_addressing_ABX(uint32_t* const cycle) {
		const uint16_t base = sfc_addressing_ABS(cycle);
		const uint16_t rvar = base + SFC_X;
		*cycle += ((base ^rvar) >> 8) & 1;
		return rvar;
	}

	  	uint16_t sfc_addressing_abx( uint32_t* const cycle) {
			const uint16_t base = sfc_addressing_ABS(cycle);
			const uint16_t rvar = base + SFC_X;
			return rvar;
		}

	/// <summary>
	/// 寻址方式: 绝对Y变址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	      uint16_t sfc_addressing_ABY(uint32_t* const cycle) {
		const uint16_t base = sfc_addressing_ABS(cycle);
		const uint16_t rvar = base + SFC_Y;
		*cycle += ((base ^ rvar) >> 8) & 1;
		return rvar;
	}

		/// <summary>
		/// 寻址方式: 绝对Y变址 - 没有额外一周期检测
		/// </summary>
		/// <param name="famicom">The famicom.</param>
		/// <param name="cycle">The cycle.</param>
		/// <returns></returns>
	  	 uint16_t sfc_addressing_aby(uint32_t* const cycle) {
			const uint16_t base = sfc_addressing_ABS(cycle);
			const uint16_t rvar = base + SFC_Y;
			return rvar;
		}

	/// <summary>
	/// 寻址方式: 零页寻址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	      uint16_t sfc_addressing_ZPG(uint32_t* const cycle) {
		const uint16_t address = SFC_READ(SFC_PC++);
		return address;
	}

	/// <summary>
	/// 寻址方式: 零页X变址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	      uint16_t sfc_addressing_ZPX(uint32_t* const cycle) {
		const uint16_t base = sfc_addressing_ZPG(cycle);
		const uint16_t index = base + SFC_X;
		return index & (uint16_t)0x00FF;
	}

	/// <summary>
	/// 寻址方式: 零页Y变址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	      uint16_t sfc_addressing_ZPY(uint32_t* const cycle) {
		const uint16_t base = sfc_addressing_ZPG(cycle);
		const uint16_t index = base + SFC_Y;
		return index & (uint16_t)0x00FF;
	}

	/// <summary>
	/// 寻址方式: 间接X变址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	      uint16_t sfc_addressing_INX(uint32_t* const cycle) {
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
	      uint16_t sfc_addressing_INY(uint32_t* const cycle) {
		uint8_t base = SFC_READ(SFC_PC++);
		const uint8_t address0 = SFC_READ(base++);
		const uint8_t address1 = SFC_READ(base++);
		const uint16_t address
			= (uint16_t)address0
			| (uint16_t)((uint16_t)address1 << 8)
			;
		const uint16_t rvar = address + SFC_Y;
		*cycle += ((address ^ rvar) >> 8) & 1;
		return rvar;
	}

		/// <summary>
		/// 寻址方式: 间接Y变址 - 没有额外一周期检测
		/// </summary>
		/// <param name="famicom">The famicom.</param>
		/// <param name="cycle">The cycle.</param>
		/// <returns></returns>
	 	 uint16_t sfc_addressing_iny( uint32_t* const cycle) {
			uint8_t base = SFC_READ_PC(SFC_PC++);
			const uint8_t address0 = SFC_READ(base++);
			const uint8_t address1 = SFC_READ(base++);
			const uint16_t address
				= (uint16_t)address0
				| (uint16_t)((uint16_t)address1 << 8)
				;

			const uint16_t rvar = address + SFC_Y;
			return rvar;
		}

	/// <summary>
	/// 寻址方式: 间接寻址
	/// </summary>
	/// <param name=" ">The  .</param>
	/// <returns></returns>
	     uint16_t sfc_addressing_IND(uint32_t* const cycle) {
		// 读取地址
		const uint16_t base1 = sfc_addressing_ABS(cycle);
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
	     uint16_t sfc_addressing_REL(uint32_t* const cycle) {
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
	      void sfc_operation_UNK(uint16_t address, uint32_t* const cycle) {
		assert(!"UNKNOWN INS");
	}

	/// <summary>
	/// SHY
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	      void sfc_operation_SHY(uint16_t address, uint32_t* const cycle) {
		sfc_operation_UNK(address,cycle);
		//const uint8_t result = SFC_Y & (uint8_t)(((uint8_t)address >> 8) + 1);
		//SFC_WRITE(address, result);
	}

	/// <summary>
	/// SHX
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	      void sfc_operation_SHX(uint16_t address, uint32_t* const cycle) {
		sfc_operation_UNK(address,cycle);
		//const uint8_t result = SFC_X & (uint8_t)(((uint8_t)address >> 8) + 1);
		//SFC_WRITE(address, result);
	}

	/// <summary>
	/// TAS
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	     void sfc_operation_TAS(uint16_t address, uint32_t* const cycle) {
		sfc_operation_UNK(address,cycle);
	}

	/// <summary>
	/// AHX
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	      void sfc_operation_AHX(uint16_t address, uint32_t* const cycle) {
		sfc_operation_UNK(address,cycle);
	}

	/// <summary>
	/// XAA
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	      void sfc_operation_XAA(uint16_t address, uint32_t* const cycle) {
		sfc_operation_UNK(address,cycle);
	}

	/// <summary>
	/// LAS
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_LAS(uint16_t address, uint32_t* const cycle) {
		sfc_operation_UNK(address,cycle);
	}

	/// <summary>
	/// SRE: Shift Right then "Exclusive-Or" - LSR + EOR
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_SRE(uint16_t address, uint32_t* const cycle) {
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
	    void sfc_operation_SLO(uint16_t address, uint32_t* const cycle) {
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
	    void sfc_operation_RRA(uint16_t address, uint32_t* const cycle) {
			// ROR
			uint16_t result16_ror = SFC_READ(address);
			result16_ror |= ((uint16_t)SFC_CF) << (8 - INDEX_C);
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
	    void sfc_operation_RLA(uint16_t address, uint32_t* const cycle) {
		// ROL
		uint16_t result16 = SFC_READ(address);
		result16 <<= 1;
		result16 |= ((uint16_t)SFC_CF) >> (INDEX_C);
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
	    void sfc_operation_ISB(uint16_t address, uint32_t* const cycle) {
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
		     void sfc_operation_ISC(uint16_t address, uint32_t* const cycle) {
		sfc_operation_UNK(address,cycle);
	}

	/// <summary>
	/// DCP: Decrement memory then Compare with A - DEC + CMP
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_DCP(uint16_t address, uint32_t* const cycle) {
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
		     void sfc_operation_SAX(uint16_t address, uint32_t* const cycle) {
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
	    void sfc_operation_LAX(uint16_t address, uint32_t* const cycle) {
		SFC_A = SFC_READ(address);
		SFC_X = SFC_A;
		CHECK_ZSFLAG(SFC_X);
	}

	/// <summary>
	/// SBX
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		     void sfc_operation_SBX(uint16_t address, uint32_t* const cycle) {
		sfc_operation_UNK(address,cycle);
	}

	/// <summary>
	/// AXS
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_AXS(uint16_t address, uint32_t* const cycle) {
			const uint16_t tmp = (SFC_A & SFC_X) - SFC_READ_PC(address);
			SFC_X = (uint8_t)tmp;
			CHECK_ZSFLAG(SFC_X);
			SFC_CF_IF((tmp & 0x8000) == 0);
		}

	/// <summary>
	/// ARR
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ARR(uint16_t address, uint32_t* const cycle) {
			//ARR 指令 是 立即数
			SFC_A &= SFC_READ_PC(address);
			SFC_CF;
			SFC_A = (SFC_A >> 1) | (SFC_CF << 7);
			CHECK_ZSFLAG(SFC_A);
			SFC_CF_IF((SFC_A >> 6) & 1);
			SFC_VF_IF(((SFC_A >> 5) ^ (SFC_A >> 6)) & 1);
		}

	/// <summary>
	/// AAC
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_AAC(uint16_t address, uint32_t* const cycle) {
		sfc_operation_UNK(address,cycle);
	}

	/// <summary>
	/// ANC
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ANC(uint16_t address, uint32_t* const cycle) {
			//ANC 两个指令都是立即数
			SFC_A &= SFC_READ_PC(address);
			CHECK_ZSFLAG(SFC_A);
			SFC_CF_IF(SFC_SF);
		}

	/// <summary>
	/// ASR
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ASR(uint16_t address, uint32_t* const cycle) {
			//ASR 指令 是 立即数
			SFC_A &= SFC_READ_PC(address);
			SFC_CF_IF(SFC_A & 1);
			SFC_A >>= 1;
			CHECK_ZSFLAG(SFC_A);
		}

	/// <summary>
	/// ALR
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		    void sfc_operation_ALR(uint16_t address, uint32_t* const cycle) {
		sfc_operation_UNK(address,cycle);
	}

	/// <summary>
	/// RTI: Return from I
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_RTI(uint16_t address, uint32_t* const cycle) {
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
	    void sfc_operation_BRK(uint16_t address, uint32_t* const cycle) {
			const uint16_t pcp1 = SFC_PC + 1;
			const uint8_t pch = (uint8_t)((pcp1) >> 8);
			const uint8_t pcl = (uint8_t)pcp1;
			SFC_PUSH(pch);
			SFC_PUSH(pcl);
			SFC_PUSH(SFC_P | (uint8_t)(SFC_FLAG_R) | (uint8_t)(SFC_FLAG_B));
			SFC_IF_SE;
			const uint8_t pcl2 = SFC_READ_PC(VECTOR_BRK + 0);
			const uint8_t pch2 = SFC_READ_PC(VECTOR_BRK + 1);
			registers_.get_program_counter() = (uint16_t)pcl2 | (uint16_t)pch2 << 8;

	}

	/// <summary>
	/// NOP: No Operation
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_NOP(uint16_t address, uint32_t* const cycle) {

	}

	/// <summary>
	/// RTS: Return from Subroutine
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_RTS(uint16_t address, uint32_t* const cycle) {
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
	    void sfc_operation_JSR(uint16_t address, uint32_t* const cycle) {
		const uint16_t pc1 = SFC_PC - 1;
		SFC_PUSH((uint8_t)(pc1 >> 8));
		SFC_PUSH((uint8_t)(pc1));
		SFC_PC = address;
	}


		void sfc_branch(uint16_t address, uint32_t* const cycle) {
			const uint16_t saved = SFC_PC;
			SFC_PC = address;
			++(*cycle);
			*cycle += (address ^ saved) >> 8 & 1;
		}
	/// <summary>
	/// BVC: Branch if Overflow Clear
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_BVC(uint16_t address, uint32_t* const cycle) {
			if (!SFC_VF) sfc_branch(address, cycle);
	}

	/// <summary>
	/// BVC: Branch if Overflow Set
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_BVS(uint16_t address, uint32_t* const cycle) {
		if (SFC_VF) sfc_branch(address, cycle);
	}

	/// <summary>
	/// BPL: Branch if Plus
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_BPL(uint16_t address, uint32_t* const cycle) {
		if (!SFC_SF) sfc_branch(address, cycle);
	}

	/// <summary>
	/// BMI: Branch if Minus
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_BMI(uint16_t address, uint32_t* const cycle) {
		if (SFC_SF) sfc_branch(address, cycle);
	}

	/// <summary>
	/// BCC: Branch if Carry Clear
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		     void sfc_operation_BCC(uint16_t address, uint32_t* const cycle) {
		if (!SFC_CF) sfc_branch(address, cycle);
	}

	/// <summary>
	/// BCS: Branch if Carry Set
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		     void sfc_operation_BCS(uint16_t address, uint32_t* const cycle) {
		if (SFC_CF) sfc_branch(address, cycle);
	}

	/// <summary>
	/// BNE: Branch if Not Equal
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_BNE(uint16_t address, uint32_t* const cycle) {
		if (!SFC_ZF) sfc_branch(address, cycle);
	}

	/// <summary>
	/// BEQ: Branch if Equal
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_BEQ(uint16_t address, uint32_t* const cycle) {
		if (SFC_ZF) sfc_branch(address, cycle);
	}

	/// <summary>
	/// JMP
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_JMP(uint16_t address, uint32_t* const cycle) {
		SFC_PC = address;
	}

	/// <summary>
	/// PLP: Pull P
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_PLP(uint16_t address, uint32_t* const cycle) {
		SFC_P = SFC_POP();
		SFC_RF_SE;
		SFC_BF_CL;
	}

	/// <summary>
	/// PHP: Push P
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_PHP(uint16_t address, uint32_t* const cycle) {
		SFC_PUSH(SFC_P | (uint8_t)(SFC_FLAG_R | SFC_FLAG_B));
	}

	/// <summary>
	/// PLA: Pull A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_PLA(uint16_t address, uint32_t* const cycle) {
		SFC_A = SFC_POP();
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// PHA: Push A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_PHA(uint16_t address, uint32_t* const cycle) {
		SFC_PUSH(SFC_A);
	}

	/// <summary>
	/// ROR A : Rotate Right for A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_RORA(uint16_t address, uint32_t* const cycle) {
		uint16_t result16 = SFC_A;
		result16 |= ((uint16_t)SFC_CF) << (8 - INDEX_C);
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
	    void sfc_operation_ROR(uint16_t address, uint32_t* const cycle) {
		uint16_t result16 = SFC_READ(address);
		result16 |= ((uint16_t)SFC_CF) << (8 - INDEX_C);
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
	    void sfc_operation_ROL(uint16_t address, uint32_t* const cycle) {
		uint16_t result16 = SFC_READ(address);
		result16 <<= 1;
		result16 |= ((uint16_t)SFC_CF) >> (INDEX_C);
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
	    void sfc_operation_ROLA(uint16_t address, uint32_t* const cycle) {
		uint16_t result16 = SFC_A;
		result16 <<= 1;
		result16 |= ((uint16_t)SFC_CF) >> (INDEX_C);
		SFC_CF_IF(result16 & (uint16_t)0x100);
		SFC_A = (uint8_t)result16;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// LSR: Logical Shift Right
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_LSR(uint16_t address, uint32_t* const cycle) {
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
	    void sfc_operation_LSRA(uint16_t address, uint32_t* const cycle) {
		SFC_CF_IF(SFC_A & 1);
		SFC_A >>= 1;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// ASL: Arithmetic Shift Left
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ASL(uint16_t address, uint32_t* const cycle) {
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
	    void sfc_operation_ASLA(uint16_t address, uint32_t* const cycle) {
		SFC_CF_IF(SFC_A & (uint8_t)0x80);
		SFC_A <<= 1;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// BIT: Bit Test
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_BIT(uint16_t address, uint32_t* const cycle) {
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
	    void sfc_operation_CPY(uint16_t address, uint32_t* const cycle) {
		const uint16_t result16 = (uint16_t)SFC_Y - (uint16_t)SFC_READ(address);
		SFC_CF_IF(!(result16 & (uint16_t)0x8000));
		CHECK_ZSFLAG((uint8_t)result16);
	}

	/// <summary>
	/// CPX
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_CPX(uint16_t address, uint32_t* const cycle) {
		const uint16_t result16 = (uint16_t)SFC_X - (uint16_t)SFC_READ(address);
		SFC_CF_IF(!(result16 & (uint16_t)0x8000));
		CHECK_ZSFLAG((uint8_t)result16);
	}

	/// <summary>
	/// CMP: Compare memory with A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_CMP(uint16_t address, uint32_t* const cycle) {
		const uint16_t result16 = (uint16_t)SFC_A - (uint16_t)SFC_READ(address);
		SFC_CF_IF(!(result16 & (uint16_t)0x8000));
		CHECK_ZSFLAG((uint8_t)result16);
	}

	/// <summary>
	/// SEI: Set I
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_SEI(uint16_t address, uint32_t* const cycle) {
		SFC_IF_SE;
	}

	/// <summary>
	/// CLI - Clear I
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_CLI(uint16_t address, uint32_t* const cycle) {
			SFC_IF_CL;
		}

	/// <summary>
	/// CLV: Clear V
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		     void sfc_operation_CLV(uint16_t address, uint32_t* const cycle) {
		SFC_VF_CL;
	}

	/// <summary>
	/// SED: Set D
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_SED(uint16_t address, uint32_t* const cycle) {
		SFC_DF_SE;
	}

	/// <summary>
	/// CLD: Clear D
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_CLD(uint16_t address, uint32_t* const cycle) {
		SFC_DF_CL;
	}

	/// <summary>
	/// SEC: Set Carry
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_SEC(uint16_t address, uint32_t* const cycle) {
		SFC_CF_SE;
	}

	/// <summary>
	/// CLC: Clear Carry
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_CLC(uint16_t address, uint32_t* const cycle) {
		SFC_CF_CL;
	}

	/// <summary>
	/// EOR: "Exclusive-Or" memory with A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_EOR(uint16_t address, uint32_t* const cycle) {
		SFC_A ^= SFC_READ(address);
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// ORA: 'Or' memory with A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_ORA(uint16_t address, uint32_t* const cycle) {
		SFC_A |= SFC_READ(address);
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// AND: 'And' memory with A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_AND(uint16_t address, uint32_t* const cycle) {
		SFC_A &= SFC_READ(address);
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// DEY: Decrement Y
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_DEY(uint16_t address, uint32_t* const cycle) {
		SFC_Y--;
		CHECK_ZSFLAG(SFC_Y);
	}

	/// <summary>
	/// INY:  Increment Y
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_INY(uint16_t address, uint32_t* const cycle) {
		SFC_Y++;
		CHECK_ZSFLAG(SFC_Y);
	}

	/// <summary>
	/// DEX: Decrement X
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_DEX(uint16_t address, uint32_t* const cycle) {
		SFC_X--;
		CHECK_ZSFLAG(SFC_X);
	}

	/// <summary>
	/// INX
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_INX(uint16_t address, uint32_t* const cycle) {
		SFC_X++;
		CHECK_ZSFLAG(SFC_X);
	}

	/// <summary>
	/// DEC: Decrement memory
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
	    void sfc_operation_DEC(uint16_t address, uint32_t* const cycle) {
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
	    void sfc_operation_INC(uint16_t address, uint32_t* const cycle) {
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
	    void sfc_operation_SBC(uint16_t address, uint32_t* const cycle) {
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
	    void sfc_operation_ADC(uint16_t address, uint32_t* const cycle) {
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
		      void sfc_operation_TXS(uint16_t address, uint32_t* const cycle) {
		SFC_SP = SFC_X;
	}

	/// <summary>
	/// TSX: Transfer SP to X
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_TSX(uint16_t address, uint32_t* const cycle) {
		SFC_X = SFC_SP;
		CHECK_ZSFLAG(SFC_X);
	}

	/// <summary>
	/// TYA: Transfer Y to A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_TYA(uint16_t address, uint32_t* const cycle) {
		SFC_A = SFC_Y;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// TAY: Transfer A to Y
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_TAY(uint16_t address, uint32_t* const cycle) {
		SFC_Y = SFC_A;
		CHECK_ZSFLAG(SFC_Y);
	}

	/// <summary>
	/// TXA: Transfer X to A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_TXA(uint16_t address, uint32_t* const cycle) {
		SFC_A = SFC_X;
		CHECK_ZSFLAG(SFC_A);
	}

	/// <summary>
	/// TAX: Transfer A to X
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_TAX(uint16_t address, uint32_t* const cycle) {
		SFC_X = SFC_A;
		CHECK_ZSFLAG(SFC_X);
	}

	/// <summary>
	/// STY: Store 'Y'
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		     void sfc_operation_STY(uint16_t address, uint32_t* const cycle) {
		SFC_WRITE(address, SFC_Y);
	}

	/// <summary>
	/// STX: Store X
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_STX(uint16_t address, uint32_t* const cycle) {
		SFC_WRITE(address, SFC_X);
	}

	/// <summary>
	/// STA: Store 'A'
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_STA(uint16_t address, uint32_t* const cycle) {
		SFC_WRITE(address, SFC_A);
	}

	/// <summary>
	/// LDY: Load 'Y'
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_LDY(uint16_t address, uint32_t* const cycle) {
		SFC_Y = SFC_READ(address);
		CHECK_ZSFLAG(SFC_Y);
	}

	/// <summary>
	/// LDX: Load X
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_LDX(uint16_t address, uint32_t* const cycle) {
		SFC_X = SFC_READ(address);
		CHECK_ZSFLAG(SFC_X);
	}

	/// <summary>
	/// LDA: Load A
	/// </summary>
	/// <param name="address">The address.</param>
	/// <param name=" ">The  .</param>
		      void sfc_operation_LDA(uint16_t address, uint32_t* const cycle) {
		SFC_A = SFC_READ(address);
		CHECK_ZSFLAG(SFC_A);
	}
};

// ---------------------------------- 指令周期数据

enum sfc_basic_cycle_data {
	SFC_BAISC_CYCLE_00 = 7, SFC_BAISC_CYCLE_01 = 6, SFC_BAISC_CYCLE_02 = 2, SFC_BAISC_CYCLE_03 = 8,
	SFC_BAISC_CYCLE_04 = 3, SFC_BAISC_CYCLE_05 = 3, SFC_BAISC_CYCLE_06 = 5, SFC_BAISC_CYCLE_07 = 5,
	SFC_BAISC_CYCLE_08 = 3, SFC_BAISC_CYCLE_09 = 2, SFC_BAISC_CYCLE_0A = 2, SFC_BAISC_CYCLE_0B = 2,
	SFC_BAISC_CYCLE_0C = 4, SFC_BAISC_CYCLE_0D = 4, SFC_BAISC_CYCLE_0E = 6, SFC_BAISC_CYCLE_0F = 6,
	SFC_BAISC_CYCLE_10 = 2, SFC_BAISC_CYCLE_11 = 5, SFC_BAISC_CYCLE_12 = 2, SFC_BAISC_CYCLE_13 = 8,
	SFC_BAISC_CYCLE_14 = 4, SFC_BAISC_CYCLE_15 = 4, SFC_BAISC_CYCLE_16 = 6, SFC_BAISC_CYCLE_17 = 6,
	SFC_BAISC_CYCLE_18 = 2, SFC_BAISC_CYCLE_19 = 4, SFC_BAISC_CYCLE_1A = 2, SFC_BAISC_CYCLE_1B = 7,
	SFC_BAISC_CYCLE_1C = 4, SFC_BAISC_CYCLE_1D = 4, SFC_BAISC_CYCLE_1E = 7, SFC_BAISC_CYCLE_1F = 7,
	SFC_BAISC_CYCLE_20 = 6, SFC_BAISC_CYCLE_21 = 6, SFC_BAISC_CYCLE_22 = 2, SFC_BAISC_CYCLE_23 = 8,
	SFC_BAISC_CYCLE_24 = 3, SFC_BAISC_CYCLE_25 = 3, SFC_BAISC_CYCLE_26 = 5, SFC_BAISC_CYCLE_27 = 5,
	SFC_BAISC_CYCLE_28 = 4, SFC_BAISC_CYCLE_29 = 2, SFC_BAISC_CYCLE_2A = 2, SFC_BAISC_CYCLE_2B = 2,
	SFC_BAISC_CYCLE_2C = 4, SFC_BAISC_CYCLE_2D = 4, SFC_BAISC_CYCLE_2E = 6, SFC_BAISC_CYCLE_2F = 6,
	SFC_BAISC_CYCLE_30 = 2, SFC_BAISC_CYCLE_31 = 5, SFC_BAISC_CYCLE_32 = 2, SFC_BAISC_CYCLE_33 = 8,
	SFC_BAISC_CYCLE_34 = 4, SFC_BAISC_CYCLE_35 = 4, SFC_BAISC_CYCLE_36 = 6, SFC_BAISC_CYCLE_37 = 6,
	SFC_BAISC_CYCLE_38 = 2, SFC_BAISC_CYCLE_39 = 4, SFC_BAISC_CYCLE_3A = 2, SFC_BAISC_CYCLE_3B = 7,
	SFC_BAISC_CYCLE_3C = 4, SFC_BAISC_CYCLE_3D = 4, SFC_BAISC_CYCLE_3E = 7, SFC_BAISC_CYCLE_3F = 7,
	SFC_BAISC_CYCLE_40 = 6, SFC_BAISC_CYCLE_41 = 6, SFC_BAISC_CYCLE_42 = 2, SFC_BAISC_CYCLE_43 = 8,
	SFC_BAISC_CYCLE_44 = 3, SFC_BAISC_CYCLE_45 = 3, SFC_BAISC_CYCLE_46 = 5, SFC_BAISC_CYCLE_47 = 5,
	SFC_BAISC_CYCLE_48 = 3, SFC_BAISC_CYCLE_49 = 2, SFC_BAISC_CYCLE_4A = 2, SFC_BAISC_CYCLE_4B = 2,
	SFC_BAISC_CYCLE_4C = 3, SFC_BAISC_CYCLE_4D = 4, SFC_BAISC_CYCLE_4E = 6, SFC_BAISC_CYCLE_4F = 6,
	SFC_BAISC_CYCLE_50 = 2, SFC_BAISC_CYCLE_51 = 5, SFC_BAISC_CYCLE_52 = 2, SFC_BAISC_CYCLE_53 = 8,
	SFC_BAISC_CYCLE_54 = 4, SFC_BAISC_CYCLE_55 = 4, SFC_BAISC_CYCLE_56 = 6, SFC_BAISC_CYCLE_57 = 6,
	SFC_BAISC_CYCLE_58 = 2, SFC_BAISC_CYCLE_59 = 4, SFC_BAISC_CYCLE_5A = 2, SFC_BAISC_CYCLE_5B = 7,
	SFC_BAISC_CYCLE_5C = 4, SFC_BAISC_CYCLE_5D = 4, SFC_BAISC_CYCLE_5E = 7, SFC_BAISC_CYCLE_5F = 7,
	SFC_BAISC_CYCLE_60 = 6, SFC_BAISC_CYCLE_61 = 6, SFC_BAISC_CYCLE_62 = 2, SFC_BAISC_CYCLE_63 = 8,
	SFC_BAISC_CYCLE_64 = 3, SFC_BAISC_CYCLE_65 = 3, SFC_BAISC_CYCLE_66 = 5, SFC_BAISC_CYCLE_67 = 5,
	SFC_BAISC_CYCLE_68 = 4, SFC_BAISC_CYCLE_69 = 2, SFC_BAISC_CYCLE_6A = 2, SFC_BAISC_CYCLE_6B = 2,
	SFC_BAISC_CYCLE_6C = 5, SFC_BAISC_CYCLE_6D = 4, SFC_BAISC_CYCLE_6E = 6, SFC_BAISC_CYCLE_6F = 6,
	SFC_BAISC_CYCLE_70 = 2, SFC_BAISC_CYCLE_71 = 5, SFC_BAISC_CYCLE_72 = 2, SFC_BAISC_CYCLE_73 = 8,
	SFC_BAISC_CYCLE_74 = 4, SFC_BAISC_CYCLE_75 = 4, SFC_BAISC_CYCLE_76 = 6, SFC_BAISC_CYCLE_77 = 6,
	SFC_BAISC_CYCLE_78 = 2, SFC_BAISC_CYCLE_79 = 4, SFC_BAISC_CYCLE_7A = 2, SFC_BAISC_CYCLE_7B = 7,
	SFC_BAISC_CYCLE_7C = 4, SFC_BAISC_CYCLE_7D = 4, SFC_BAISC_CYCLE_7E = 7, SFC_BAISC_CYCLE_7F = 7,
	SFC_BAISC_CYCLE_80 = 2, SFC_BAISC_CYCLE_81 = 6, SFC_BAISC_CYCLE_82 = 2, SFC_BAISC_CYCLE_83 = 6,
	SFC_BAISC_CYCLE_84 = 3, SFC_BAISC_CYCLE_85 = 3, SFC_BAISC_CYCLE_86 = 3, SFC_BAISC_CYCLE_87 = 3,
	SFC_BAISC_CYCLE_88 = 2, SFC_BAISC_CYCLE_89 = 2, SFC_BAISC_CYCLE_8A = 2, SFC_BAISC_CYCLE_8B = 2,
	SFC_BAISC_CYCLE_8C = 4, SFC_BAISC_CYCLE_8D = 4, SFC_BAISC_CYCLE_8E = 4, SFC_BAISC_CYCLE_8F = 4,
	SFC_BAISC_CYCLE_90 = 2, SFC_BAISC_CYCLE_91 = 6, SFC_BAISC_CYCLE_92 = 2, SFC_BAISC_CYCLE_93 = 6,
	SFC_BAISC_CYCLE_94 = 4, SFC_BAISC_CYCLE_95 = 4, SFC_BAISC_CYCLE_96 = 4, SFC_BAISC_CYCLE_97 = 4,
	SFC_BAISC_CYCLE_98 = 2, SFC_BAISC_CYCLE_99 = 5, SFC_BAISC_CYCLE_9A = 2, SFC_BAISC_CYCLE_9B = 5,
	SFC_BAISC_CYCLE_9C = 5, SFC_BAISC_CYCLE_9D = 5, SFC_BAISC_CYCLE_9E = 5, SFC_BAISC_CYCLE_9F = 5,
	SFC_BAISC_CYCLE_A0 = 2, SFC_BAISC_CYCLE_A1 = 6, SFC_BAISC_CYCLE_A2 = 2, SFC_BAISC_CYCLE_A3 = 6,
	SFC_BAISC_CYCLE_A4 = 3, SFC_BAISC_CYCLE_A5 = 3, SFC_BAISC_CYCLE_A6 = 3, SFC_BAISC_CYCLE_A7 = 3,
	SFC_BAISC_CYCLE_A8 = 2, SFC_BAISC_CYCLE_A9 = 2, SFC_BAISC_CYCLE_AA = 2, SFC_BAISC_CYCLE_AB = 2,
	SFC_BAISC_CYCLE_AC = 4, SFC_BAISC_CYCLE_AD = 4, SFC_BAISC_CYCLE_AE = 4, SFC_BAISC_CYCLE_AF = 4,
	SFC_BAISC_CYCLE_B0 = 2, SFC_BAISC_CYCLE_B1 = 5, SFC_BAISC_CYCLE_B2 = 2, SFC_BAISC_CYCLE_B3 = 5,
	SFC_BAISC_CYCLE_B4 = 4, SFC_BAISC_CYCLE_B5 = 4, SFC_BAISC_CYCLE_B6 = 4, SFC_BAISC_CYCLE_B7 = 4,
	SFC_BAISC_CYCLE_B8 = 2, SFC_BAISC_CYCLE_B9 = 4, SFC_BAISC_CYCLE_BA = 2, SFC_BAISC_CYCLE_BB = 4,
	SFC_BAISC_CYCLE_BC = 4, SFC_BAISC_CYCLE_BD = 4, SFC_BAISC_CYCLE_BE = 4, SFC_BAISC_CYCLE_BF = 4,
	SFC_BAISC_CYCLE_C0 = 2, SFC_BAISC_CYCLE_C1 = 6, SFC_BAISC_CYCLE_C2 = 2, SFC_BAISC_CYCLE_C3 = 8,
	SFC_BAISC_CYCLE_C4 = 3, SFC_BAISC_CYCLE_C5 = 3, SFC_BAISC_CYCLE_C6 = 5, SFC_BAISC_CYCLE_C7 = 5,
	SFC_BAISC_CYCLE_C8 = 2, SFC_BAISC_CYCLE_C9 = 2, SFC_BAISC_CYCLE_CA = 2, SFC_BAISC_CYCLE_CB = 2,
	SFC_BAISC_CYCLE_CC = 4, SFC_BAISC_CYCLE_CD = 4, SFC_BAISC_CYCLE_CE = 6, SFC_BAISC_CYCLE_CF = 6,
	SFC_BAISC_CYCLE_D0 = 2, SFC_BAISC_CYCLE_D1 = 5, SFC_BAISC_CYCLE_D2 = 2, SFC_BAISC_CYCLE_D3 = 8,
	SFC_BAISC_CYCLE_D4 = 4, SFC_BAISC_CYCLE_D5 = 4, SFC_BAISC_CYCLE_D6 = 6, SFC_BAISC_CYCLE_D7 = 6,
	SFC_BAISC_CYCLE_D8 = 2, SFC_BAISC_CYCLE_D9 = 4, SFC_BAISC_CYCLE_DA = 2, SFC_BAISC_CYCLE_DB = 7,
	SFC_BAISC_CYCLE_DC = 4, SFC_BAISC_CYCLE_DD = 4, SFC_BAISC_CYCLE_DE = 7, SFC_BAISC_CYCLE_DF = 7,
	SFC_BAISC_CYCLE_E0 = 2, SFC_BAISC_CYCLE_E1 = 6, SFC_BAISC_CYCLE_E2 = 2, SFC_BAISC_CYCLE_E3 = 8,
	SFC_BAISC_CYCLE_E4 = 3, SFC_BAISC_CYCLE_E5 = 3, SFC_BAISC_CYCLE_E6 = 5, SFC_BAISC_CYCLE_E7 = 5,
	SFC_BAISC_CYCLE_E8 = 2, SFC_BAISC_CYCLE_E9 = 2, SFC_BAISC_CYCLE_EA = 2, SFC_BAISC_CYCLE_EB = 2,
	SFC_BAISC_CYCLE_EC = 4, SFC_BAISC_CYCLE_ED = 4, SFC_BAISC_CYCLE_EE = 6, SFC_BAISC_CYCLE_EF = 6,
	SFC_BAISC_CYCLE_F0 = 2, SFC_BAISC_CYCLE_F1 = 5, SFC_BAISC_CYCLE_F2 = 2, SFC_BAISC_CYCLE_F3 = 8,
	SFC_BAISC_CYCLE_F4 = 4, SFC_BAISC_CYCLE_F5 = 4, SFC_BAISC_CYCLE_F6 = 6, SFC_BAISC_CYCLE_F7 = 6,
	SFC_BAISC_CYCLE_F8 = 2, SFC_BAISC_CYCLE_F9 = 4, SFC_BAISC_CYCLE_FA = 2, SFC_BAISC_CYCLE_FB = 7,
	SFC_BAISC_CYCLE_FC = 4, SFC_BAISC_CYCLE_FD = 4, SFC_BAISC_CYCLE_FE = 7, SFC_BAISC_CYCLE_FF = 7,
};

