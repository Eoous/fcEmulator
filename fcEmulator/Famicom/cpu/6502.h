#pragma once

#include <stdint.h>
#include <string>
#include <assert.h>

#ifdef _MSC_VER
#ifdef _DEBUG
#define FORCEINLINE 
#else
#define FORCEINLINE __forceinline
#endif
#else
#define FORCEINLINE __attribute__((always_inline))
#endif

enum { DISASSEMBLY_BUF_LEN = 32 };
const char HEXDATA[] = "0123456789ABCDEF";
//=====================================================
//进制转换
void BToH(char o[], uint8_t b);

void BToD(char o[], uint8_t b);

//=====================================================
//=====================================================
//						6502
////===================================================
////===================================================
class Code
{
public:

	void Disassembly(Code code,char buf[DISASSEMBLY_BUF_LEN]);
	uint32_t data_;

//===================================================
//设置op a1_ a2_ ctrl值
	void SetOp(const uint8_t& op);
	void SetA1(const uint8_t& a1);
	void SetA2(const uint8_t& a2);
	void SetCtrl(const uint8_t& ctrl);
	const uint8_t& GetOp() {
		return op_;
	}

private:
	uint8_t op_;
	uint8_t a1_;
	uint8_t a2_;
	uint8_t ctrl_;
};


struct OpName {
	//3字名称
	char name[3];
	// 寻址模式[低4位] | 指令长度[高4位]
	uint8_t mode_inslen;
};




//=====================================================
//=====================================================
//6502指令集
enum Instruction {
	INS_UNK = 0,   // 未知指令
	INS_LDA,       // LDA--由存储器取数送入累加器A    M -> A
	INS_LDX,       // LDX--由存储器取数送入寄存器X    M -> X
	INS_LDY,       // LDY--由存储器取数送入寄存器Y    M -> Y
	INS_STA,       // STA--将累加器A的数送入存储器    A -> M
	INS_STX,       // STX--将寄存器X的数送入存储器    X -> M
	INS_STY,       // STY--将寄存器Y的数送入存储器    Y -> M
	INS_TAX,       // 将累加器A的内容送入变址寄存器X
	INS_TXA,       // 将变址寄存器X的内容送入累加器A
	INS_TAY,       // 将累加器A的内容送入变址寄存器Y
	INS_TYA,       // 将变址寄存器Y的内容送入累加器A
	INS_TSX,       // 堆栈指针S的内容送入变址寄存器X
	INS_TXS,       // 变址寄存器X的内容送入堆栈指针S
	INS_ADC,       // ADC--累加器,存储器,进位标志C相加,结果送累加器A  A+M+C -> A 
	INS_SBC,       // SBC--从累加器减去存储器和进位标志C取反,结果送累加器 A-M-(1-C) -> A
	INS_INC,       // INC--存储器单元内容增1  M+1 -> M
	INS_DEC,       // DEC--存储器单元内容减1  M-1 -> M
	INS_INX,       // INX--X寄存器+1 X+1 -> X
	INS_DEX,       // DEX--X寄存器-1 X-1 -> X
	INS_INY,       // INY--Y寄存器+1 Y+1 -> Y
	INS_DEY,       // DEY--Y寄存器-1 Y-1 -> Y
	INS_AND,       // AND--存储器与累加器相与,结果送累加器  A∧M -> A
	INS_ORA,       // ORA--存储器与累加器相或,结果送累加器  A∨M -> A
	INS_EOR,       // EOR--存储器与累加器异或,结果送累加器  A≮M -> A
	INS_CLC,       // CLC--清除进位标志C         0 -> C
	INS_SEC,       // SEC--设置进位标志C         1 -> C
	INS_CLD,       // CLD--清除十进标志D         0 -> D
	INS_SED,       // SED--设置十进标志D         1 -> D
	INS_CLV,       // CLV--清除溢出标志V         0 -> V
	INS_CLI,       // CLI--清除中断禁止V         0 -> I
	INS_SEI,       // SEI--设置中断禁止V         1 -> I
	INS_CMP,       // CMP--累加器和存储器比较
	INS_CPX,       // CPX--寄存器X的内容和存储器比较
	INS_CPY,       // CPY--寄存器Y的内容和存储器比较
	INS_BIT,       // BIT--位测试
	INS_ASL,       // ASL--算术左移 储存器
	INS_ASLA,      // ASL--算术左移 累加器
	INS_LSR,       // LSR--算术右移 储存器
	INS_LSRA,      // LSR--算术右移 累加器
	INS_ROL,       // ROL--循环算术左移 储存器
	INS_ROLA,      // ROL--循环算术左移 累加器
	INS_ROR,       // ROR--循环算术右移 储存器
	INS_RORA,      // ROR--循环算术右移 累加器
	INS_PHA,       // PHA--累加器进栈
	INS_PLA,       // PLA--累加器出栈
	INS_PHP,       // PHP--标志寄存器P进栈
	INS_PLP,       // PLP--标志寄存器P出栈
	INS_JMP,       // JMP--无条件跳转
	INS_BEQ,       // 如果标志位Z = 1则转移，否则继续
	INS_BNE,       // 如果标志位Z = 0则转移，否则继续
	INS_BCS,       // 如果标志位C = 1则转移，否则继续
	INS_BCC,       // 如果标志位C = 0则转移，否则继续
	INS_BMI,       // 如果标志位N = 1则转移，否则继续
	INS_BPL,       // 如果标志位N = 0则转移，否则继续
	INS_BVS,       // 如果标志位V = 1则转移，否则继续
	INS_BVC,       // 如果标志位V = 0则转移，否则继续
	INS_JSR,       // 跳转到子程序
	INS_RTS,       // 返回到主程序
	INS_NOP,       // 无操作
	INS_BRK,       // 强制中断
	INS_RTI,       // 从中断返回
	// --------  组合指令  ----------
	INS_ALR,       // [Unofficial&Combo] AND+LSR
	INS_ASR = INS_ALR,// 有消息称是叫这个
	INS_ANC,       // [Unofficial&Combo] AND+N2C?
	INS_AAC = INS_ANC,// 差不多一个意思
	INS_ARR,       // [Unofficial&Combo] AND+ROR [类似]
	INS_AXS,       // [Unofficial&Combo] AND+XSB?
	INS_SBX = INS_AXS,// 一个意思
	INS_LAX,       // [Unofficial&Combo] LDA+TAX
	INS_SAX,       // [Unofficial&Combo] STA&STX [类似]
	// -------- 读改写指令 ----------
	INS_DCP, // [Unofficial& RMW ] DEC+CMP
	INS_ISC, // [Unofficial& RMW ] INC+SBC
	INS_ISB = INS_ISC,// 差不多一个意思
	INS_RLA, // [Unofficial& RMW ] ROL+AND
	INS_RRA, // [Unofficial& RMW ] ROR+AND
	INS_SLO, // [Unofficial& RMW ] ASL+ORA
	INS_SRE, // [Unofficial& RMW ] LSR+EOR
	// -------- 卧槽 ----
	INS_LAS,
	INS_XAA,
	INS_AHX,
	INS_TAS,
	INS_SHX,
	INS_SHY,
};


//=========================================
//寻址方式
enum AddressingMode {
	AM_UNK = 0,     // 未知寻址
	AM_ACC,         // 操累加器A: Op Accumulator
	AM_IMP,         // 隐含 寻址: Implied    Addressing
	AM_IMM,         // 立即 寻址: Immediate  Addressing
	AM_ABS,         // 直接 寻址: Absolute   Addressing
	AM_ABX,         // 直接X变址: Absolute X Addressing
	AM_ABY,         // 直接Y变址: Absolute Y Addressing
	AM_ZPG,         // 零页 寻址: Zero-Page  Addressing
	AM_ZPX,         // 零页X变址: Zero-PageX Addressing
	AM_ZPY,         // 零页Y变址: Zero-PageY Addressing
	AM_INX,         // 间接X变址:  Pre-indexed Indirect Addressing
	AM_INY,         // 间接Y变址: Post-indexed Indirect Addressing
	AM_IND,         // 间接 寻址: Indirect   Addressing
	AM_REL,         // 相对 寻址: Relative   Addressing
};


const struct OpName s_opname_data[256] = {
	// 0
	{ 'B', 'R', 'K', AM_IMP | 1 << 4 },
	{ 'O', 'R', 'A', AM_INX | 2 << 4 },
	{ 'S', 'T', 'P', AM_UNK | 1 << 4 },
	{ 'S', 'L', 'O', AM_INX | 2 << 4 },
	{ 'N', 'O', 'P', AM_ZPG | 2 << 4 },
	{ 'O', 'R', 'A', AM_ZPG | 2 << 4 },
	{ 'A', 'S', 'L', AM_ZPG | 2 << 4 },
	{ 'S', 'L', 'O', AM_ZPG | 2 << 4 },
	{ 'P', 'H', 'P', AM_IMP | 1 << 4 },
	{ 'O', 'R', 'A', AM_IMM | 2 << 4 },
	{ 'A', 'S', 'L', AM_ACC | 1 << 4 },
	{ 'A', 'N', 'C', AM_IMM | 2 << 4 },
	{ 'N', 'O', 'P', AM_ABS | 3 << 4 },
	{ 'O', 'R', 'A', AM_ABS | 3 << 4 },
	{ 'A', 'S', 'L', AM_ABS | 3 << 4 },
	{ 'S', 'L', 'O', AM_ABS | 3 << 4 },
	// 1
	{ 'B', 'P', 'L', AM_REL | 2 << 4 },
	{ 'O', 'R', 'A', AM_INY | 2 << 4 },
	{ 'S', 'T', 'P', AM_UNK | 1 << 4 },
	{ 'S', 'L', 'O', AM_INY | 2 << 4 },
	{ 'N', 'O', 'P', AM_ZPX | 2 << 4 },
	{ 'O', 'R', 'A', AM_ZPX | 2 << 4 },
	{ 'A', 'S', 'L', AM_ZPX | 2 << 4 },
	{ 'S', 'L', 'O', AM_ZPX | 2 << 4 },
	{ 'C', 'L', 'C', AM_IMP | 1 << 4 },
	{ 'O', 'R', 'A', AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', AM_IMP | 1 << 4 },
	{ 'S', 'L', 'O', AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', AM_ABX | 3 << 4 },
	{ 'O', 'R', 'A', AM_ABX | 3 << 4 },
	{ 'A', 'S', 'L', AM_ABX | 3 << 4 },
	{ 'S', 'L', 'O', AM_ABX | 3 << 4 },
	// 2
	{ 'J', 'S', 'R', AM_ABS | 3 << 4 },
	{ 'A', 'N', 'D', AM_INX | 2 << 4 },
	{ 'S', 'T', 'P', AM_UNK | 1 << 4 },
	{ 'R', 'L', 'A', AM_INX | 2 << 4 },
	{ 'B', 'I', 'T', AM_ZPG | 2 << 4 },
	{ 'A', 'N', 'D', AM_ZPG | 2 << 4 },
	{ 'R', 'O', 'L', AM_ZPG | 2 << 4 },
	{ 'R', 'L', 'A', AM_ZPG | 2 << 4 },
	{ 'P', 'L', 'P', AM_IMP | 1 << 4 },
	{ 'A', 'N', 'D', AM_IMM | 2 << 4 },
	{ 'R', 'O', 'L', AM_ACC | 1 << 4 },
	{ 'A', 'N', 'C', AM_IMM | 2 << 4 },
	{ 'B', 'I', 'T', AM_ABS | 3 << 4 },
	{ 'A', 'N', 'D', AM_ABS | 3 << 4 },
	{ 'R', 'O', 'L', AM_ABS | 3 << 4 },
	{ 'R', 'L', 'A', AM_ABS | 3 << 4 },
	// 3
	{ 'B', 'M', 'I', AM_REL | 2 << 4 },
	{ 'A', 'N', 'D', AM_INY | 2 << 4 },
	{ 'S', 'T', 'P', AM_UNK | 1 << 4 },
	{ 'R', 'L', 'A', AM_INY | 2 << 4 },
	{ 'N', 'O', 'P', AM_ZPX | 2 << 4 },
	{ 'A', 'N', 'D', AM_ZPX | 2 << 4 },
	{ 'R', 'O', 'L', AM_ZPX | 2 << 4 },
	{ 'R', 'L', 'A', AM_ZPX | 2 << 4 },
	{ 'S', 'E', 'C', AM_IMP | 1 << 4 },
	{ 'A', 'N', 'D', AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', AM_IMP | 1 << 4 },
	{ 'R', 'L', 'A', AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', AM_ABX | 3 << 4 },
	{ 'A', 'N', 'D', AM_ABX | 3 << 4 },
	{ 'R', 'O', 'L', AM_ABX | 3 << 4 },
	{ 'R', 'L', 'A', AM_ABX | 3 << 4 },
	// 4
	{ 'R', 'T', 'I', AM_IMP | 1 << 4 },
	{ 'E', 'O', 'R', AM_INX | 2 << 4 },
	{ 'S', 'T', 'P', AM_UNK | 1 << 4 },
	{ 'S', 'R', 'E', AM_INX | 2 << 4 },
	{ 'N', 'O', 'P', AM_ZPG | 2 << 4 },
	{ 'E', 'O', 'R', AM_ZPG | 2 << 4 },
	{ 'L', 'S', 'R', AM_ZPG | 2 << 4 },
	{ 'S', 'R', 'E', AM_ZPG | 2 << 4 },
	{ 'P', 'H', 'A', AM_IMP | 1 << 4 },
	{ 'E', 'O', 'R', AM_IMM | 2 << 4 },
	{ 'L', 'S', 'R', AM_ACC | 1 << 4 },
	{ 'A', 'S', 'R', AM_IMM | 2 << 4 },
	{ 'J', 'M', 'P', AM_ABS | 3 << 4 },
	{ 'E', 'O', 'R', AM_ABS | 3 << 4 },
	{ 'L', 'S', 'R', AM_ABS | 3 << 4 },
	{ 'S', 'R', 'E', AM_ABS | 3 << 4 },
	// 5
	{ 'B', 'V', 'C', AM_REL | 2 << 4 },
	{ 'E', 'O', 'R', AM_INY | 2 << 4 },
	{ 'S', 'T', 'P', AM_UNK | 1 << 4 },
	{ 'S', 'R', 'E', AM_INY | 2 << 4 },
	{ 'N', 'O', 'P', AM_ZPX | 2 << 4 },
	{ 'E', 'O', 'R', AM_ZPX | 2 << 4 },
	{ 'L', 'S', 'R', AM_ZPX | 2 << 4 },
	{ 'S', 'R', 'E', AM_ZPX | 2 << 4 },
	{ 'C', 'L', 'I', AM_IMP | 1 << 4 },
	{ 'E', 'O', 'R', AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', AM_IMP | 1 << 4 },
	{ 'S', 'R', 'E', AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', AM_ABX | 3 << 4 },
	{ 'E', 'O', 'R', AM_ABX | 3 << 4 },
	{ 'L', 'S', 'R', AM_ABX | 3 << 4 },
	{ 'S', 'R', 'E', AM_ABX | 3 << 4 },
	// 6
	{ 'R', 'T', 'S', AM_IMP | 1 << 4 },
	{ 'A', 'D', 'C', AM_INX | 2 << 4 },
	{ 'S', 'T', 'P', AM_UNK | 1 << 4 },
	{ 'R', 'R', 'A', AM_INX | 2 << 4 },
	{ 'N', 'O', 'P', AM_ZPG | 2 << 4 },
	{ 'A', 'D', 'C', AM_ZPG | 2 << 4 },
	{ 'R', 'O', 'R', AM_ZPG | 2 << 4 },
	{ 'R', 'R', 'A', AM_ZPG | 2 << 4 },
	{ 'P', 'L', 'A', AM_IMP | 1 << 4 },
	{ 'A', 'D', 'C', AM_IMM | 2 << 4 },
	{ 'R', 'O', 'R', AM_ACC | 1 << 4 },
	{ 'A', 'R', 'R', AM_IMM | 2 << 4 },
	{ 'J', 'M', 'P', AM_IND | 3 << 4 },
	{ 'A', 'D', 'C', AM_ABS | 3 << 4 },
	{ 'R', 'O', 'R', AM_ABS | 3 << 4 },
	{ 'R', 'R', 'A', AM_ABS | 3 << 4 },
	// 7
	{ 'B', 'V', 'S', AM_REL | 2 << 4 },
	{ 'A', 'D', 'C', AM_INY | 2 << 4 },
	{ 'S', 'T', 'P', AM_UNK | 1 << 4 },
	{ 'R', 'R', 'A', AM_INY | 2 << 4 },
	{ 'N', 'O', 'P', AM_ZPX | 2 << 4 },
	{ 'A', 'D', 'C', AM_ZPX | 2 << 4 },
	{ 'R', 'O', 'R', AM_ZPX | 2 << 4 },
	{ 'R', 'R', 'A', AM_ZPX | 2 << 4 },
	{ 'S', 'E', 'I', AM_IMP | 1 << 4 },
	{ 'A', 'D', 'C', AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', AM_IMP | 1 << 4 },
	{ 'R', 'R', 'A', AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', AM_ABX | 3 << 4 },
	{ 'A', 'D', 'C', AM_ABX | 3 << 4 },
	{ 'R', 'O', 'R', AM_ABX | 3 << 4 },
	{ 'R', 'R', 'A', AM_ABX | 3 << 4 },
	// 8
	{ 'N', 'O', 'P', AM_IMM | 2 << 4 },
	{ 'S', 'T', 'A', AM_INX | 2 << 4 },
	{ 'N', 'O', 'P', AM_IMM | 2 << 4 },
	{ 'S', 'A', 'X', AM_INX | 2 << 4 },
	{ 'S', 'T', 'Y', AM_ZPG | 2 << 4 },
	{ 'S', 'T', 'A', AM_ZPG | 2 << 4 },
	{ 'S', 'T', 'X', AM_ZPG | 2 << 4 },
	{ 'S', 'A', 'X', AM_ZPG | 2 << 4 },
	{ 'D', 'E', 'Y', AM_IMP | 1 << 4 },
	{ 'N', 'O', 'P', AM_IMM | 2 << 4 },
	{ 'T', 'A', 'X', AM_IMP | 1 << 4 },
	{ 'X', 'X', 'A', AM_IMM | 2 << 4 },
	{ 'S', 'T', 'Y', AM_ABS | 3 << 4 },
	{ 'S', 'T', 'A', AM_ABS | 3 << 4 },
	{ 'S', 'T', 'X', AM_ABS | 3 << 4 },
	{ 'S', 'A', 'X', AM_ABS | 3 << 4 },
	// 9
	{ 'B', 'C', 'C', AM_REL | 2 << 4 },
	{ 'S', 'T', 'A', AM_INY | 2 << 4 },
	{ 'S', 'T', 'P', AM_UNK | 1 << 4 },
	{ 'A', 'H', 'X', AM_INY | 2 << 4 },
	{ 'S', 'T', 'Y', AM_ZPX | 2 << 4 },
	{ 'S', 'T', 'A', AM_ZPX | 2 << 4 },
	{ 'S', 'T', 'X', AM_ZPY | 2 << 4 },
	{ 'S', 'A', 'X', AM_ZPY | 2 << 4 },
	{ 'T', 'Y', 'A', AM_IMP | 1 << 4 },
	{ 'S', 'T', 'A', AM_ABY | 3 << 4 },
	{ 'T', 'X', 'S', AM_IMP | 1 << 4 },
	{ 'T', 'A', 'S', AM_ABY | 3 << 4 },
	{ 'S', 'H', 'Y', AM_ABX | 3 << 4 },
	{ 'S', 'T', 'A', AM_ABX | 3 << 4 },
	{ 'S', 'H', 'X', AM_ABY | 3 << 4 },
	{ 'A', 'H', 'X', AM_ABY | 3 << 4 },
	// A
	{ 'L', 'D', 'Y', AM_IMM | 2 << 4 },
	{ 'L', 'D', 'A', AM_INX | 2 << 4 },
	{ 'L', 'D', 'X', AM_IMM | 2 << 4 },
	{ 'L', 'A', 'X', AM_INX | 2 << 4 },
	{ 'L', 'D', 'Y', AM_ZPG | 2 << 4 },
	{ 'L', 'D', 'A', AM_ZPG | 2 << 4 },
	{ 'L', 'D', 'X', AM_ZPG | 2 << 4 },
	{ 'L', 'A', 'X', AM_ZPG | 2 << 4 },
	{ 'T', 'A', 'Y', AM_IMP | 1 << 4 },
	{ 'L', 'D', 'A', AM_IMM | 2 << 4 },
	{ 'T', 'A', 'X', AM_IMP | 1 << 4 },
	{ 'L', 'A', 'X', AM_IMM | 2 << 4 },
	{ 'L', 'D', 'Y', AM_ABS | 3 << 4 },
	{ 'L', 'D', 'A', AM_ABS | 3 << 4 },
	{ 'L', 'D', 'X', AM_ABS | 3 << 4 },
	{ 'L', 'A', 'X', AM_ABS | 3 << 4 },
	// B
	{ 'B', 'C', 'S', AM_REL | 2 << 4 },
	{ 'L', 'D', 'A', AM_INY | 2 << 4 },
	{ 'S', 'T', 'P', AM_UNK | 1 << 4 },
	{ 'L', 'A', 'X', AM_INY | 2 << 4 },
	{ 'L', 'D', 'Y', AM_ZPX | 2 << 4 },
	{ 'L', 'D', 'A', AM_ZPX | 2 << 4 },
	{ 'L', 'D', 'X', AM_ZPY | 2 << 4 },
	{ 'L', 'A', 'X', AM_ZPY | 2 << 4 },
	{ 'C', 'L', 'V', AM_IMP | 1 << 4 },
	{ 'L', 'D', 'A', AM_ABY | 3 << 4 },
	{ 'T', 'S', 'X', AM_IMP | 1 << 4 },
	{ 'L', 'A', 'S', AM_ABY | 3 << 4 },
	{ 'L', 'D', 'Y', AM_ABX | 3 << 4 },
	{ 'L', 'D', 'A', AM_ABX | 3 << 4 },
	{ 'L', 'D', 'X', AM_ABY | 3 << 4 },
	{ 'L', 'A', 'X', AM_ABY | 3 << 4 },
	// C
	{ 'C', 'P', 'Y', AM_IMM | 2 << 4 },
	{ 'C', 'M', 'P', AM_INX | 2 << 4 },
	{ 'N', 'O', 'P', AM_IMM | 2 << 4 },
	{ 'D', 'C', 'P', AM_INX | 2 << 4 },
	{ 'C', 'P', 'Y', AM_ZPG | 2 << 4 },
	{ 'C', 'M', 'P', AM_ZPG | 2 << 4 },
	{ 'D', 'E', 'C', AM_ZPG | 2 << 4 },
	{ 'D', 'C', 'P', AM_ZPG | 2 << 4 },
	{ 'I', 'N', 'Y', AM_IMP | 1 << 4 },
	{ 'C', 'M', 'P', AM_IMM | 2 << 4 },
	{ 'D', 'E', 'X', AM_IMP | 1 << 4 },
	{ 'A', 'X', 'S', AM_IMM | 2 << 4 },
	{ 'C', 'P', 'Y', AM_ABS | 3 << 4 },
	{ 'C', 'M', 'P', AM_ABS | 3 << 4 },
	{ 'D', 'E', 'C', AM_ABS | 3 << 4 },
	{ 'D', 'C', 'P', AM_ABS | 3 << 4 },
	// D
	{ 'B', 'N', 'E', AM_REL | 2 << 4 },
	{ 'C', 'M', 'P', AM_INY | 2 << 4 },
	{ 'S', 'T', 'P', AM_UNK | 1 << 4 },
	{ 'D', 'C', 'P', AM_INY | 2 << 4 },
	{ 'N', 'O', 'P', AM_ZPX | 2 << 4 },
	{ 'C', 'M', 'P', AM_ZPX | 2 << 4 },
	{ 'D', 'E', 'C', AM_ZPX | 2 << 4 },
	{ 'D', 'C', 'P', AM_ZPX | 2 << 4 },
	{ 'C', 'L', 'D', AM_IMP | 1 << 4 },
	{ 'C', 'M', 'P', AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', AM_IMP | 1 << 4 },
	{ 'D', 'C', 'P', AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', AM_ABX | 3 << 4 },
	{ 'C', 'M', 'P', AM_ABX | 3 << 4 },
	{ 'D', 'E', 'C', AM_ABX | 3 << 4 },
	{ 'D', 'C', 'P', AM_ABX | 3 << 4 },
	// E
	{ 'C', 'P', 'X', AM_IMM | 2 << 4 },
	{ 'S', 'B', 'C', AM_INX | 2 << 4 },
	{ 'N', 'O', 'P', AM_IMM | 2 << 4 },
	{ 'I', 'S', 'B', AM_INX | 2 << 4 },
	{ 'C', 'P', 'X', AM_ZPG | 2 << 4 },
	{ 'S', 'B', 'C', AM_ZPG | 2 << 4 },
	{ 'I', 'N', 'C', AM_ZPG | 2 << 4 },
	{ 'I', 'S', 'B', AM_ZPG | 2 << 4 },
	{ 'I', 'N', 'X', AM_IMP | 1 << 4 },
	{ 'S', 'B', 'C', AM_IMM | 2 << 4 },
	{ 'N', 'O', 'P', AM_IMP | 1 << 4 },
	{ 'S', 'B', 'C', AM_IMM | 2 << 4 },
	{ 'C', 'P', 'X', AM_ABS | 3 << 4 },
	{ 'S', 'B', 'C', AM_ABS | 3 << 4 },
	{ 'I', 'N', 'C', AM_ABS | 3 << 4 },
	{ 'I', 'S', 'B', AM_ABS | 3 << 4 },
	// F
	{ 'B', 'E', 'Q', AM_REL | 2 << 4 },
	{ 'S', 'B', 'C', AM_INY | 2 << 4 },
	{ 'S', 'T', 'P', AM_UNK | 1 << 4 },
	{ 'I', 'S', 'B', AM_INY | 2 << 4 },
	{ 'N', 'O', 'P', AM_ZPX | 2 << 4 },
	{ 'S', 'B', 'C', AM_ZPX | 2 << 4 },
	{ 'I', 'N', 'C', AM_ZPX | 2 << 4 },
	{ 'I', 'S', 'B', AM_ZPX | 2 << 4 },
	{ 'S', 'E', 'D', AM_IMP | 1 << 4 },
	{ 'S', 'B', 'C', AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', AM_IMP | 1 << 4 },
	{ 'I', 'S', 'B', AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', AM_ABX | 3 << 4 },
	{ 'S', 'B', 'C', AM_ABX | 3 << 4 },
	{ 'I', 'N', 'C', AM_ABX | 3 << 4 },
	{ 'I', 'S', 'B', AM_ABX | 3 << 4 },
};