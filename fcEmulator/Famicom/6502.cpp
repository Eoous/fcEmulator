#include "sfc_6502.h"
#define sfc_fallthrough

//============================================
//进制转换
//SFC_HEXDATA[] = "0123456789ABCDEF";
void sfc_btoh(char o[], uint8_t b) {
	o[0] = SFC_HEXDATA[b >> 4];

	o[1] = SFC_HEXDATA[b & (uint8_t)0x0F];
}

void sfc_btod(char o[], uint8_t b) {
	const int8_t  sb = (int8_t)b;
	if (sb < 0) {
		o[0] = '-';
		b = -b;
	}
	else {
		o[0] = '+';
	}
	o[1] = SFC_HEXDATA[(uint8_t)b / 100];
	o[2] = SFC_HEXDATA[(uint8_t)b / 10 % 10];
	o[3] = SFC_HEXDATA[(uint8_t)b % 10];
}
//============================
//反汇编
void sfc_6502_code_t::sfc_6502_disassembly(sfc_6502_code_t code,char buf[SFC_DISASSEMBLY_BUF_LEN]) {
	enum{
		NAME_FIRSH=0,
		ADDR_FIRSH=NAME_FIRSH+4,
		LEN=ADDR_FIRSH+9
	};
	memset(buf, ' ', LEN);
	buf[LEN] = ';';
	buf[LEN + 1] = 0;
	static_assert(LEN + 1 < SFC_DISASSEMBLY_BUF_LEN, "");
	const struct sfc_opname opname = s_opname_data[op];
	//设置操作码
	buf[NAME_FIRSH + 0] = opname.name[0];
	buf[NAME_FIRSH + 1] = opname.name[1];
	buf[NAME_FIRSH + 2] = opname.name[2];

	//查看寻址模式
	const auto addrmode = opname.mode_inslen & 0x0f;
	switch (addrmode)
	{
	case SFC_AM_UNK:
		sfc_fallthrough;
	case SFC_AM_IMP:
		// XXX     ;
		break;
	case SFC_AM_ACC:
		// XXX A   ;
		buf[ADDR_FIRSH + 0] = 'A';
		break;
	case SFC_AM_IMM:
		// XXX #$AB
		buf[ADDR_FIRSH + 0] = '#';
		buf[ADDR_FIRSH + 1] = '$';
		sfc_btoh(buf + ADDR_FIRSH + 2, code.a1);
		break;
	case SFC_AM_ABS:
		// XXX $ABCD
		sfc_fallthrough;
	case SFC_AM_ABX:
		// XXX $ABCD, X
		sfc_fallthrough;
	case SFC_AM_ABY:
		// XXX $ABCD, Y
		// REAL
		buf[ADDR_FIRSH] = '$';
		sfc_btoh(buf + ADDR_FIRSH + 1, code.a2);
		sfc_btoh(buf + ADDR_FIRSH + 3, code.a1);
		if (addrmode == SFC_AM_ABS) break;
		buf[ADDR_FIRSH + 5] = ',';
		buf[ADDR_FIRSH + 7] = addrmode == SFC_AM_ABX ? 'X' : 'Y';
		break;
	case SFC_AM_ZPG:
		// XXX $AB
		sfc_fallthrough;
	case SFC_AM_ZPX:
		// XXX $AB, X
		sfc_fallthrough;
	case SFC_AM_ZPY:
		// XXX $AB, Y
		// REAL
		buf[ADDR_FIRSH] = '$';
		sfc_btoh(buf + ADDR_FIRSH + 1, code.a1);
		if (addrmode == SFC_AM_ZPG) break;
		buf[ADDR_FIRSH + 3] = ',';
		buf[ADDR_FIRSH + 5] = addrmode == SFC_AM_ABX ? 'X' : 'Y';
		break;
	case SFC_AM_INX:
		// XXX ($AB, X)
		buf[ADDR_FIRSH + 0] = '(';
		buf[ADDR_FIRSH + 1] = '$';
		sfc_btoh(buf + ADDR_FIRSH + 2, code.a1);
		buf[ADDR_FIRSH + 4] = ',';
		buf[ADDR_FIRSH + 6] = 'X';
		buf[ADDR_FIRSH + 7] = ')';
		break;
	case SFC_AM_INY:
		// XXX ($AB), Y
		buf[ADDR_FIRSH + 0] = '(';
		buf[ADDR_FIRSH + 1] = '$';
		sfc_btoh(buf + ADDR_FIRSH + 2, code.a1);
		buf[ADDR_FIRSH + 4] = ')';
		buf[ADDR_FIRSH + 5] = ',';
		buf[ADDR_FIRSH + 7] = 'Y';
		break;
	case SFC_AM_IND:
		// XXX ($ABCD)
		buf[ADDR_FIRSH + 0] = '(';
		buf[ADDR_FIRSH + 1] = '$';
		sfc_btoh(buf + ADDR_FIRSH + 2, code.a2);
		sfc_btoh(buf + ADDR_FIRSH + 4, code.a1);
		buf[ADDR_FIRSH + 6] = ')';
		break;
	case SFC_AM_REL:
		// XXX $AB(-085)
		// XXX $ABCD
		buf[ADDR_FIRSH + 0] = '$';

		//const uint16_t target = base + int8_t(data.a1);
		//sfc_btoh(buf + ADDR_FIRSH + 1, uint8_t(target >> 8));
		//sfc_btoh(buf + ADDR_FIRSH + 3, uint8_t(target & 0xFF));

		sfc_btoh(buf + ADDR_FIRSH + 1, code.a1);
		buf[ADDR_FIRSH + 3] = '(';
		sfc_btod(buf + ADDR_FIRSH + 4, code.a1);
		buf[ADDR_FIRSH + 8] = ')';
		break;
	}

}

////===================================================
//设置
void sfc_6502_code_t::set_op(const uint8_t& op) {
	this->op = op;
}

void sfc_6502_code_t::set_a1(const uint8_t& a1) {
	this->a1 = a1;
}

void sfc_6502_code_t::set_a2(const uint8_t& a2) {
	this->a2 = a2;
}

void sfc_6502_code_t::set_ctrl(const uint8_t& ctrl) {
	this->ctrl = ctrl;
}