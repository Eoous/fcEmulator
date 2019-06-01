#pragma once

#include <stdint.h>
#include <string>
#include <assert.h>

#ifdef _MSC_VER
#ifdef _DEBUG
#define SFC_FORCEINLINE 
#else
#define SFC_FORCEINLINE __forceinline
#endif
#else
#define SFC_FORCEINLINE __attribute__((always_inline))
#endif

enum { SFC_DISASSEMBLY_BUF_LEN = 32 };
const char SFC_HEXDATA[] = "0123456789ABCDEF";
//=====================================================
//进制转换
void sfc_btoh(char o[], uint8_t b);

void sfc_btod(char o[], uint8_t b);

//=====================================================
//=====================================================
//						6502
////===================================================
////===================================================
class sfc_6502_code_t
{
public:

	void sfc_6502_disassembly(sfc_6502_code_t code,char buf[SFC_DISASSEMBLY_BUF_LEN]);
	uint32_t data;

//===================================================
//设置op a1 a2 ctrl值
	void set_op(const uint8_t& op);
	void set_a1(const uint8_t& a1);
	void set_a2(const uint8_t& a2);
	void set_ctrl(const uint8_t& ctrl);
	const uint8_t& get_op() {
		return op;
	}

private:
	uint8_t op;
	uint8_t a1;
	uint8_t a2;
	uint8_t ctrl;
};


struct sfc_opname {
	//3字名称
	char name[3];
	// 寻址模式[低4位] | 指令长度[高4位]
	uint8_t mode_inslen;
};




//=====================================================
//=====================================================
//6502指令集
enum sfc_6502_instruction {
	SFC_INS_UNK = 0,   // 未知指令
	SFC_INS_LDA,       // LDA--由存储器取数送入累加器A    M -> A
	SFC_INS_LDX,       // LDX--由存储器取数送入寄存器X    M -> X
	SFC_INS_LDY,       // LDY--由存储器取数送入寄存器Y    M -> Y
	SFC_INS_STA,       // STA--将累加器A的数送入存储器    A -> M
	SFC_INS_STX,       // STX--将寄存器X的数送入存储器    X -> M
	SFC_INS_STY,       // STY--将寄存器Y的数送入存储器    Y -> M
	SFC_INS_TAX,       // 将累加器A的内容送入变址寄存器X
	SFC_INS_TXA,       // 将变址寄存器X的内容送入累加器A
	SFC_INS_TAY,       // 将累加器A的内容送入变址寄存器Y
	SFC_INS_TYA,       // 将变址寄存器Y的内容送入累加器A
	SFC_INS_TSX,       // 堆栈指针S的内容送入变址寄存器X
	SFC_INS_TXS,       // 变址寄存器X的内容送入堆栈指针S
	SFC_INS_ADC,       // ADC--累加器,存储器,进位标志C相加,结果送累加器A  A+M+C -> A 
	SFC_INS_SBC,       // SBC--从累加器减去存储器和进位标志C取反,结果送累加器 A-M-(1-C) -> A
	SFC_INS_INC,       // INC--存储器单元内容增1  M+1 -> M
	SFC_INS_DEC,       // DEC--存储器单元内容减1  M-1 -> M
	SFC_INS_INX,       // INX--X寄存器+1 X+1 -> X
	SFC_INS_DEX,       // DEX--X寄存器-1 X-1 -> X
	SFC_INS_INY,       // INY--Y寄存器+1 Y+1 -> Y
	SFC_INS_DEY,       // DEY--Y寄存器-1 Y-1 -> Y
	SFC_INS_AND,       // AND--存储器与累加器相与,结果送累加器  A∧M -> A
	SFC_INS_ORA,       // ORA--存储器与累加器相或,结果送累加器  A∨M -> A
	SFC_INS_EOR,       // EOR--存储器与累加器异或,结果送累加器  A≮M -> A
	SFC_INS_CLC,       // CLC--清除进位标志C         0 -> C
	SFC_INS_SEC,       // SEC--设置进位标志C         1 -> C
	SFC_INS_CLD,       // CLD--清除十进标志D         0 -> D
	SFC_INS_SED,       // SED--设置十进标志D         1 -> D
	SFC_INS_CLV,       // CLV--清除溢出标志V         0 -> V
	SFC_INS_CLI,       // CLI--清除中断禁止V         0 -> I
	SFC_INS_SEI,       // SEI--设置中断禁止V         1 -> I
	SFC_INS_CMP,       // CMP--累加器和存储器比较
	SFC_INS_CPX,       // CPX--寄存器X的内容和存储器比较
	SFC_INS_CPY,       // CPY--寄存器Y的内容和存储器比较
	SFC_INS_BIT,       // BIT--位测试
	SFC_INS_ASL,       // ASL--算术左移 储存器
	SFC_INS_ASLA,      // ASL--算术左移 累加器
	SFC_INS_LSR,       // LSR--算术右移 储存器
	SFC_INS_LSRA,      // LSR--算术右移 累加器
	SFC_INS_ROL,       // ROL--循环算术左移 储存器
	SFC_INS_ROLA,      // ROL--循环算术左移 累加器
	SFC_INS_ROR,       // ROR--循环算术右移 储存器
	SFC_INS_RORA,      // ROR--循环算术右移 累加器
	SFC_INS_PHA,       // PHA--累加器进栈
	SFC_INS_PLA,       // PLA--累加器出栈
	SFC_INS_PHP,       // PHP--标志寄存器P进栈
	SFC_INS_PLP,       // PLP--标志寄存器P出栈
	SFC_INS_JMP,       // JMP--无条件跳转
	SFC_INS_BEQ,       // 如果标志位Z = 1则转移，否则继续
	SFC_INS_BNE,       // 如果标志位Z = 0则转移，否则继续
	SFC_INS_BCS,       // 如果标志位C = 1则转移，否则继续
	SFC_INS_BCC,       // 如果标志位C = 0则转移，否则继续
	SFC_INS_BMI,       // 如果标志位N = 1则转移，否则继续
	SFC_INS_BPL,       // 如果标志位N = 0则转移，否则继续
	SFC_INS_BVS,       // 如果标志位V = 1则转移，否则继续
	SFC_INS_BVC,       // 如果标志位V = 0则转移，否则继续
	SFC_INS_JSR,       // 跳转到子程序
	SFC_INS_RTS,       // 返回到主程序
	SFC_INS_NOP,       // 无操作
	SFC_INS_BRK,       // 强制中断
	SFC_INS_RTI,       // 从中断返回
	// --------  组合指令  ----------
	SFC_INS_ALR,       // [Unofficial&Combo] AND+LSR
	SFC_INS_ASR = SFC_INS_ALR,// 有消息称是叫这个
	SFC_INS_ANC,       // [Unofficial&Combo] AND+N2C?
	SFC_INS_AAC = SFC_INS_ANC,// 差不多一个意思
	SFC_INS_ARR,       // [Unofficial&Combo] AND+ROR [类似]
	SFC_INS_AXS,       // [Unofficial&Combo] AND+XSB?
	SFC_INS_SBX = SFC_INS_AXS,// 一个意思
	SFC_INS_LAX,       // [Unofficial&Combo] LDA+TAX
	SFC_INS_SAX,       // [Unofficial&Combo] STA&STX [类似]
	// -------- 读改写指令 ----------
	SFC_INS_DCP, // [Unofficial& RMW ] DEC+CMP
	SFC_INS_ISC, // [Unofficial& RMW ] INC+SBC
	SFC_INS_ISB = SFC_INS_ISC,// 差不多一个意思
	SFC_INS_RLA, // [Unofficial& RMW ] ROL+AND
	SFC_INS_RRA, // [Unofficial& RMW ] ROR+AND
	SFC_INS_SLO, // [Unofficial& RMW ] ASL+ORA
	SFC_INS_SRE, // [Unofficial& RMW ] LSR+EOR
	// -------- 卧槽 ----
	SFC_INS_LAS,
	SFC_INS_XAA,
	SFC_INS_AHX,
	SFC_INS_TAS,
	SFC_INS_SHX,
	SFC_INS_SHY,
};


//=========================================
//寻址方式
enum sfc_6502_addressing_mode {
	SFC_AM_UNK = 0,     // 未知寻址
	SFC_AM_ACC,         // 操累加器A: Op Accumulator
	SFC_AM_IMP,         // 隐含 寻址: Implied    Addressing
	SFC_AM_IMM,         // 立即 寻址: Immediate  Addressing
	SFC_AM_ABS,         // 直接 寻址: Absolute   Addressing
	SFC_AM_ABX,         // 直接X变址: Absolute X Addressing
	SFC_AM_ABY,         // 直接Y变址: Absolute Y Addressing
	SFC_AM_ZPG,         // 零页 寻址: Zero-Page  Addressing
	SFC_AM_ZPX,         // 零页X变址: Zero-PageX Addressing
	SFC_AM_ZPY,         // 零页Y变址: Zero-PageY Addressing
	SFC_AM_INX,         // 间接X变址:  Pre-indexed Indirect Addressing
	SFC_AM_INY,         // 间接Y变址: Post-indexed Indirect Addressing
	SFC_AM_IND,         // 间接 寻址: Indirect   Addressing
	SFC_AM_REL,         // 相对 寻址: Relative   Addressing
};


const struct sfc_opname s_opname_data[256] = {
	// 0
	{ 'B', 'R', 'K', SFC_AM_IMP | 1 << 4 },
	{ 'O', 'R', 'A', SFC_AM_INX | 2 << 4 },
	{ 'S', 'T', 'P', SFC_AM_UNK | 1 << 4 },
	{ 'S', 'L', 'O', SFC_AM_INX | 2 << 4 },
	{ 'N', 'O', 'P', SFC_AM_ZPG | 2 << 4 },
	{ 'O', 'R', 'A', SFC_AM_ZPG | 2 << 4 },
	{ 'A', 'S', 'L', SFC_AM_ZPG | 2 << 4 },
	{ 'S', 'L', 'O', SFC_AM_ZPG | 2 << 4 },
	{ 'P', 'H', 'P', SFC_AM_IMP | 1 << 4 },
	{ 'O', 'R', 'A', SFC_AM_IMM | 2 << 4 },
	{ 'A', 'S', 'L', SFC_AM_ACC | 1 << 4 },
	{ 'A', 'N', 'C', SFC_AM_IMM | 2 << 4 },
	{ 'N', 'O', 'P', SFC_AM_ABS | 3 << 4 },
	{ 'O', 'R', 'A', SFC_AM_ABS | 3 << 4 },
	{ 'A', 'S', 'L', SFC_AM_ABS | 3 << 4 },
	{ 'S', 'L', 'O', SFC_AM_ABS | 3 << 4 },
	// 1
	{ 'B', 'P', 'L', SFC_AM_REL | 2 << 4 },
	{ 'O', 'R', 'A', SFC_AM_INY | 2 << 4 },
	{ 'S', 'T', 'P', SFC_AM_UNK | 1 << 4 },
	{ 'S', 'L', 'O', SFC_AM_INY | 2 << 4 },
	{ 'N', 'O', 'P', SFC_AM_ZPX | 2 << 4 },
	{ 'O', 'R', 'A', SFC_AM_ZPX | 2 << 4 },
	{ 'A', 'S', 'L', SFC_AM_ZPX | 2 << 4 },
	{ 'S', 'L', 'O', SFC_AM_ZPX | 2 << 4 },
	{ 'C', 'L', 'C', SFC_AM_IMP | 1 << 4 },
	{ 'O', 'R', 'A', SFC_AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', SFC_AM_IMP | 1 << 4 },
	{ 'S', 'L', 'O', SFC_AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', SFC_AM_ABX | 3 << 4 },
	{ 'O', 'R', 'A', SFC_AM_ABX | 3 << 4 },
	{ 'A', 'S', 'L', SFC_AM_ABX | 3 << 4 },
	{ 'S', 'L', 'O', SFC_AM_ABX | 3 << 4 },
	// 2
	{ 'J', 'S', 'R', SFC_AM_ABS | 3 << 4 },
	{ 'A', 'N', 'D', SFC_AM_INX | 2 << 4 },
	{ 'S', 'T', 'P', SFC_AM_UNK | 1 << 4 },
	{ 'R', 'L', 'A', SFC_AM_INX | 2 << 4 },
	{ 'B', 'I', 'T', SFC_AM_ZPG | 2 << 4 },
	{ 'A', 'N', 'D', SFC_AM_ZPG | 2 << 4 },
	{ 'R', 'O', 'L', SFC_AM_ZPG | 2 << 4 },
	{ 'R', 'L', 'A', SFC_AM_ZPG | 2 << 4 },
	{ 'P', 'L', 'P', SFC_AM_IMP | 1 << 4 },
	{ 'A', 'N', 'D', SFC_AM_IMM | 2 << 4 },
	{ 'R', 'O', 'L', SFC_AM_ACC | 1 << 4 },
	{ 'A', 'N', 'C', SFC_AM_IMM | 2 << 4 },
	{ 'B', 'I', 'T', SFC_AM_ABS | 3 << 4 },
	{ 'A', 'N', 'D', SFC_AM_ABS | 3 << 4 },
	{ 'R', 'O', 'L', SFC_AM_ABS | 3 << 4 },
	{ 'R', 'L', 'A', SFC_AM_ABS | 3 << 4 },
	// 3
	{ 'B', 'M', 'I', SFC_AM_REL | 2 << 4 },
	{ 'A', 'N', 'D', SFC_AM_INY | 2 << 4 },
	{ 'S', 'T', 'P', SFC_AM_UNK | 1 << 4 },
	{ 'R', 'L', 'A', SFC_AM_INY | 2 << 4 },
	{ 'N', 'O', 'P', SFC_AM_ZPX | 2 << 4 },
	{ 'A', 'N', 'D', SFC_AM_ZPX | 2 << 4 },
	{ 'R', 'O', 'L', SFC_AM_ZPX | 2 << 4 },
	{ 'R', 'L', 'A', SFC_AM_ZPX | 2 << 4 },
	{ 'S', 'E', 'C', SFC_AM_IMP | 1 << 4 },
	{ 'A', 'N', 'D', SFC_AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', SFC_AM_IMP | 1 << 4 },
	{ 'R', 'L', 'A', SFC_AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', SFC_AM_ABX | 3 << 4 },
	{ 'A', 'N', 'D', SFC_AM_ABX | 3 << 4 },
	{ 'R', 'O', 'L', SFC_AM_ABX | 3 << 4 },
	{ 'R', 'L', 'A', SFC_AM_ABX | 3 << 4 },
	// 4
	{ 'R', 'T', 'I', SFC_AM_IMP | 1 << 4 },
	{ 'E', 'O', 'R', SFC_AM_INX | 2 << 4 },
	{ 'S', 'T', 'P', SFC_AM_UNK | 1 << 4 },
	{ 'S', 'R', 'E', SFC_AM_INX | 2 << 4 },
	{ 'N', 'O', 'P', SFC_AM_ZPG | 2 << 4 },
	{ 'E', 'O', 'R', SFC_AM_ZPG | 2 << 4 },
	{ 'L', 'S', 'R', SFC_AM_ZPG | 2 << 4 },
	{ 'S', 'R', 'E', SFC_AM_ZPG | 2 << 4 },
	{ 'P', 'H', 'A', SFC_AM_IMP | 1 << 4 },
	{ 'E', 'O', 'R', SFC_AM_IMM | 2 << 4 },
	{ 'L', 'S', 'R', SFC_AM_ACC | 1 << 4 },
	{ 'A', 'S', 'R', SFC_AM_IMM | 2 << 4 },
	{ 'J', 'M', 'P', SFC_AM_ABS | 3 << 4 },
	{ 'E', 'O', 'R', SFC_AM_ABS | 3 << 4 },
	{ 'L', 'S', 'R', SFC_AM_ABS | 3 << 4 },
	{ 'S', 'R', 'E', SFC_AM_ABS | 3 << 4 },
	// 5
	{ 'B', 'V', 'C', SFC_AM_REL | 2 << 4 },
	{ 'E', 'O', 'R', SFC_AM_INY | 2 << 4 },
	{ 'S', 'T', 'P', SFC_AM_UNK | 1 << 4 },
	{ 'S', 'R', 'E', SFC_AM_INY | 2 << 4 },
	{ 'N', 'O', 'P', SFC_AM_ZPX | 2 << 4 },
	{ 'E', 'O', 'R', SFC_AM_ZPX | 2 << 4 },
	{ 'L', 'S', 'R', SFC_AM_ZPX | 2 << 4 },
	{ 'S', 'R', 'E', SFC_AM_ZPX | 2 << 4 },
	{ 'C', 'L', 'I', SFC_AM_IMP | 1 << 4 },
	{ 'E', 'O', 'R', SFC_AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', SFC_AM_IMP | 1 << 4 },
	{ 'S', 'R', 'E', SFC_AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', SFC_AM_ABX | 3 << 4 },
	{ 'E', 'O', 'R', SFC_AM_ABX | 3 << 4 },
	{ 'L', 'S', 'R', SFC_AM_ABX | 3 << 4 },
	{ 'S', 'R', 'E', SFC_AM_ABX | 3 << 4 },
	// 6
	{ 'R', 'T', 'S', SFC_AM_IMP | 1 << 4 },
	{ 'A', 'D', 'C', SFC_AM_INX | 2 << 4 },
	{ 'S', 'T', 'P', SFC_AM_UNK | 1 << 4 },
	{ 'R', 'R', 'A', SFC_AM_INX | 2 << 4 },
	{ 'N', 'O', 'P', SFC_AM_ZPG | 2 << 4 },
	{ 'A', 'D', 'C', SFC_AM_ZPG | 2 << 4 },
	{ 'R', 'O', 'R', SFC_AM_ZPG | 2 << 4 },
	{ 'R', 'R', 'A', SFC_AM_ZPG | 2 << 4 },
	{ 'P', 'L', 'A', SFC_AM_IMP | 1 << 4 },
	{ 'A', 'D', 'C', SFC_AM_IMM | 2 << 4 },
	{ 'R', 'O', 'R', SFC_AM_ACC | 1 << 4 },
	{ 'A', 'R', 'R', SFC_AM_IMM | 2 << 4 },
	{ 'J', 'M', 'P', SFC_AM_IND | 3 << 4 },
	{ 'A', 'D', 'C', SFC_AM_ABS | 3 << 4 },
	{ 'R', 'O', 'R', SFC_AM_ABS | 3 << 4 },
	{ 'R', 'R', 'A', SFC_AM_ABS | 3 << 4 },
	// 7
	{ 'B', 'V', 'S', SFC_AM_REL | 2 << 4 },
	{ 'A', 'D', 'C', SFC_AM_INY | 2 << 4 },
	{ 'S', 'T', 'P', SFC_AM_UNK | 1 << 4 },
	{ 'R', 'R', 'A', SFC_AM_INY | 2 << 4 },
	{ 'N', 'O', 'P', SFC_AM_ZPX | 2 << 4 },
	{ 'A', 'D', 'C', SFC_AM_ZPX | 2 << 4 },
	{ 'R', 'O', 'R', SFC_AM_ZPX | 2 << 4 },
	{ 'R', 'R', 'A', SFC_AM_ZPX | 2 << 4 },
	{ 'S', 'E', 'I', SFC_AM_IMP | 1 << 4 },
	{ 'A', 'D', 'C', SFC_AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', SFC_AM_IMP | 1 << 4 },
	{ 'R', 'R', 'A', SFC_AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', SFC_AM_ABX | 3 << 4 },
	{ 'A', 'D', 'C', SFC_AM_ABX | 3 << 4 },
	{ 'R', 'O', 'R', SFC_AM_ABX | 3 << 4 },
	{ 'R', 'R', 'A', SFC_AM_ABX | 3 << 4 },
	// 8
	{ 'N', 'O', 'P', SFC_AM_IMM | 2 << 4 },
	{ 'S', 'T', 'A', SFC_AM_INX | 2 << 4 },
	{ 'N', 'O', 'P', SFC_AM_IMM | 2 << 4 },
	{ 'S', 'A', 'X', SFC_AM_INX | 2 << 4 },
	{ 'S', 'T', 'Y', SFC_AM_ZPG | 2 << 4 },
	{ 'S', 'T', 'A', SFC_AM_ZPG | 2 << 4 },
	{ 'S', 'T', 'X', SFC_AM_ZPG | 2 << 4 },
	{ 'S', 'A', 'X', SFC_AM_ZPG | 2 << 4 },
	{ 'D', 'E', 'Y', SFC_AM_IMP | 1 << 4 },
	{ 'N', 'O', 'P', SFC_AM_IMM | 2 << 4 },
	{ 'T', 'A', 'X', SFC_AM_IMP | 1 << 4 },
	{ 'X', 'X', 'A', SFC_AM_IMM | 2 << 4 },
	{ 'S', 'T', 'Y', SFC_AM_ABS | 3 << 4 },
	{ 'S', 'T', 'A', SFC_AM_ABS | 3 << 4 },
	{ 'S', 'T', 'X', SFC_AM_ABS | 3 << 4 },
	{ 'S', 'A', 'X', SFC_AM_ABS | 3 << 4 },
	// 9
	{ 'B', 'C', 'C', SFC_AM_REL | 2 << 4 },
	{ 'S', 'T', 'A', SFC_AM_INY | 2 << 4 },
	{ 'S', 'T', 'P', SFC_AM_UNK | 1 << 4 },
	{ 'A', 'H', 'X', SFC_AM_INY | 2 << 4 },
	{ 'S', 'T', 'Y', SFC_AM_ZPX | 2 << 4 },
	{ 'S', 'T', 'A', SFC_AM_ZPX | 2 << 4 },
	{ 'S', 'T', 'X', SFC_AM_ZPY | 2 << 4 },
	{ 'S', 'A', 'X', SFC_AM_ZPY | 2 << 4 },
	{ 'T', 'Y', 'A', SFC_AM_IMP | 1 << 4 },
	{ 'S', 'T', 'A', SFC_AM_ABY | 3 << 4 },
	{ 'T', 'X', 'S', SFC_AM_IMP | 1 << 4 },
	{ 'T', 'A', 'S', SFC_AM_ABY | 3 << 4 },
	{ 'S', 'H', 'Y', SFC_AM_ABX | 3 << 4 },
	{ 'S', 'T', 'A', SFC_AM_ABX | 3 << 4 },
	{ 'S', 'H', 'X', SFC_AM_ABY | 3 << 4 },
	{ 'A', 'H', 'X', SFC_AM_ABY | 3 << 4 },
	// A
	{ 'L', 'D', 'Y', SFC_AM_IMM | 2 << 4 },
	{ 'L', 'D', 'A', SFC_AM_INX | 2 << 4 },
	{ 'L', 'D', 'X', SFC_AM_IMM | 2 << 4 },
	{ 'L', 'A', 'X', SFC_AM_INX | 2 << 4 },
	{ 'L', 'D', 'Y', SFC_AM_ZPG | 2 << 4 },
	{ 'L', 'D', 'A', SFC_AM_ZPG | 2 << 4 },
	{ 'L', 'D', 'X', SFC_AM_ZPG | 2 << 4 },
	{ 'L', 'A', 'X', SFC_AM_ZPG | 2 << 4 },
	{ 'T', 'A', 'Y', SFC_AM_IMP | 1 << 4 },
	{ 'L', 'D', 'A', SFC_AM_IMM | 2 << 4 },
	{ 'T', 'A', 'X', SFC_AM_IMP | 1 << 4 },
	{ 'L', 'A', 'X', SFC_AM_IMM | 2 << 4 },
	{ 'L', 'D', 'Y', SFC_AM_ABS | 3 << 4 },
	{ 'L', 'D', 'A', SFC_AM_ABS | 3 << 4 },
	{ 'L', 'D', 'X', SFC_AM_ABS | 3 << 4 },
	{ 'L', 'A', 'X', SFC_AM_ABS | 3 << 4 },
	// B
	{ 'B', 'C', 'S', SFC_AM_REL | 2 << 4 },
	{ 'L', 'D', 'A', SFC_AM_INY | 2 << 4 },
	{ 'S', 'T', 'P', SFC_AM_UNK | 1 << 4 },
	{ 'L', 'A', 'X', SFC_AM_INY | 2 << 4 },
	{ 'L', 'D', 'Y', SFC_AM_ZPX | 2 << 4 },
	{ 'L', 'D', 'A', SFC_AM_ZPX | 2 << 4 },
	{ 'L', 'D', 'X', SFC_AM_ZPY | 2 << 4 },
	{ 'L', 'A', 'X', SFC_AM_ZPY | 2 << 4 },
	{ 'C', 'L', 'V', SFC_AM_IMP | 1 << 4 },
	{ 'L', 'D', 'A', SFC_AM_ABY | 3 << 4 },
	{ 'T', 'S', 'X', SFC_AM_IMP | 1 << 4 },
	{ 'L', 'A', 'S', SFC_AM_ABY | 3 << 4 },
	{ 'L', 'D', 'Y', SFC_AM_ABX | 3 << 4 },
	{ 'L', 'D', 'A', SFC_AM_ABX | 3 << 4 },
	{ 'L', 'D', 'X', SFC_AM_ABY | 3 << 4 },
	{ 'L', 'A', 'X', SFC_AM_ABY | 3 << 4 },
	// C
	{ 'C', 'P', 'Y', SFC_AM_IMM | 2 << 4 },
	{ 'C', 'M', 'P', SFC_AM_INX | 2 << 4 },
	{ 'N', 'O', 'P', SFC_AM_IMM | 2 << 4 },
	{ 'D', 'C', 'P', SFC_AM_INX | 2 << 4 },
	{ 'C', 'P', 'Y', SFC_AM_ZPG | 2 << 4 },
	{ 'C', 'M', 'P', SFC_AM_ZPG | 2 << 4 },
	{ 'D', 'E', 'C', SFC_AM_ZPG | 2 << 4 },
	{ 'D', 'C', 'P', SFC_AM_ZPG | 2 << 4 },
	{ 'I', 'N', 'Y', SFC_AM_IMP | 1 << 4 },
	{ 'C', 'M', 'P', SFC_AM_IMM | 2 << 4 },
	{ 'D', 'E', 'X', SFC_AM_IMP | 1 << 4 },
	{ 'A', 'X', 'S', SFC_AM_IMM | 2 << 4 },
	{ 'C', 'P', 'Y', SFC_AM_ABS | 3 << 4 },
	{ 'C', 'M', 'P', SFC_AM_ABS | 3 << 4 },
	{ 'D', 'E', 'C', SFC_AM_ABS | 3 << 4 },
	{ 'D', 'C', 'P', SFC_AM_ABS | 3 << 4 },
	// D
	{ 'B', 'N', 'E', SFC_AM_REL | 2 << 4 },
	{ 'C', 'M', 'P', SFC_AM_INY | 2 << 4 },
	{ 'S', 'T', 'P', SFC_AM_UNK | 1 << 4 },
	{ 'D', 'C', 'P', SFC_AM_INY | 2 << 4 },
	{ 'N', 'O', 'P', SFC_AM_ZPX | 2 << 4 },
	{ 'C', 'M', 'P', SFC_AM_ZPX | 2 << 4 },
	{ 'D', 'E', 'C', SFC_AM_ZPX | 2 << 4 },
	{ 'D', 'C', 'P', SFC_AM_ZPX | 2 << 4 },
	{ 'C', 'L', 'D', SFC_AM_IMP | 1 << 4 },
	{ 'C', 'M', 'P', SFC_AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', SFC_AM_IMP | 1 << 4 },
	{ 'D', 'C', 'P', SFC_AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', SFC_AM_ABX | 3 << 4 },
	{ 'C', 'M', 'P', SFC_AM_ABX | 3 << 4 },
	{ 'D', 'E', 'C', SFC_AM_ABX | 3 << 4 },
	{ 'D', 'C', 'P', SFC_AM_ABX | 3 << 4 },
	// E
	{ 'C', 'P', 'X', SFC_AM_IMM | 2 << 4 },
	{ 'S', 'B', 'C', SFC_AM_INX | 2 << 4 },
	{ 'N', 'O', 'P', SFC_AM_IMM | 2 << 4 },
	{ 'I', 'S', 'B', SFC_AM_INX | 2 << 4 },
	{ 'C', 'P', 'X', SFC_AM_ZPG | 2 << 4 },
	{ 'S', 'B', 'C', SFC_AM_ZPG | 2 << 4 },
	{ 'I', 'N', 'C', SFC_AM_ZPG | 2 << 4 },
	{ 'I', 'S', 'B', SFC_AM_ZPG | 2 << 4 },
	{ 'I', 'N', 'X', SFC_AM_IMP | 1 << 4 },
	{ 'S', 'B', 'C', SFC_AM_IMM | 2 << 4 },
	{ 'N', 'O', 'P', SFC_AM_IMP | 1 << 4 },
	{ 'S', 'B', 'C', SFC_AM_IMM | 2 << 4 },
	{ 'C', 'P', 'X', SFC_AM_ABS | 3 << 4 },
	{ 'S', 'B', 'C', SFC_AM_ABS | 3 << 4 },
	{ 'I', 'N', 'C', SFC_AM_ABS | 3 << 4 },
	{ 'I', 'S', 'B', SFC_AM_ABS | 3 << 4 },
	// F
	{ 'B', 'E', 'Q', SFC_AM_REL | 2 << 4 },
	{ 'S', 'B', 'C', SFC_AM_INY | 2 << 4 },
	{ 'S', 'T', 'P', SFC_AM_UNK | 1 << 4 },
	{ 'I', 'S', 'B', SFC_AM_INY | 2 << 4 },
	{ 'N', 'O', 'P', SFC_AM_ZPX | 2 << 4 },
	{ 'S', 'B', 'C', SFC_AM_ZPX | 2 << 4 },
	{ 'I', 'N', 'C', SFC_AM_ZPX | 2 << 4 },
	{ 'I', 'S', 'B', SFC_AM_ZPX | 2 << 4 },
	{ 'S', 'E', 'D', SFC_AM_IMP | 1 << 4 },
	{ 'S', 'B', 'C', SFC_AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', SFC_AM_IMP | 1 << 4 },
	{ 'I', 'S', 'B', SFC_AM_ABY | 3 << 4 },
	{ 'N', 'O', 'P', SFC_AM_ABX | 3 << 4 },
	{ 'S', 'B', 'C', SFC_AM_ABX | 3 << 4 },
	{ 'I', 'N', 'C', SFC_AM_ABX | 3 << 4 },
	{ 'I', 'S', 'B', SFC_AM_ABX | 3 << 4 },
};