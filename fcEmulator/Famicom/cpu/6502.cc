#include "6502.h"
#define FallThrough

//============================================
//进制转换
//HEXDATA[] = "0123456789ABCDEF";
void BToH(char o[], uint8_t b) {
	o[0] = HEXDATA[b >> 4];

	o[1] = HEXDATA[b & (uint8_t)0x0F];
}

void BToD(char o[], uint8_t b) {
	const int8_t  sb = (int8_t)b;
	if (sb < 0) {
		o[0] = '-';
		b = -b;
	}
	else {
		o[0] = '+';
	}
	o[1] = HEXDATA[(uint8_t)b / 100];
	o[2] = HEXDATA[(uint8_t)b / 10 % 10];
	o[3] = HEXDATA[(uint8_t)b % 10];
}
//============================
//反汇编
void Code::Disassembly(Code code,char buf[DISASSEMBLY_BUF_LEN]) {
	enum{
		NAME_FIRSH=0,
		ADDR_FIRSH=NAME_FIRSH+4,
		LEN=ADDR_FIRSH+9
	};
	memset(buf, ' ', LEN);
	buf[LEN] = ';';
	buf[LEN + 1] = 0;
	static_assert(LEN + 1 < DISASSEMBLY_BUF_LEN, "");
	const struct OpName opname = s_opname_data[op_];
	//设置操作码
	buf[NAME_FIRSH + 0] = opname.name[0];
	buf[NAME_FIRSH + 1] = opname.name[1];
	buf[NAME_FIRSH + 2] = opname.name[2];

	//查看寻址模式
	const auto addrmode = opname.mode_inslen & 0x0f;
	switch (addrmode)
	{
	case AM_UNK:
		FallThrough;
	case AM_IMP:
		// XXX     ;
		break;
	case AM_ACC:
		// XXX A   ;
		buf[ADDR_FIRSH + 0] = 'A';
		break;
	case AM_IMM:
		// XXX #$AB
		buf[ADDR_FIRSH + 0] = '#';
		buf[ADDR_FIRSH + 1] = '$';
		BToH(buf + ADDR_FIRSH + 2, code.a1_);
		break;
	case AM_ABS:
		// XXX $ABCD
		FallThrough;
	case AM_ABX:
		// XXX $ABCD, X
		FallThrough;
	case AM_ABY:
		// XXX $ABCD, Y
		// REAL
		buf[ADDR_FIRSH] = '$';
		BToH(buf + ADDR_FIRSH + 1, code.a2_);
		BToH(buf + ADDR_FIRSH + 3, code.a1_);
		if (addrmode == AM_ABS) break;
		buf[ADDR_FIRSH + 5] = ',';
		buf[ADDR_FIRSH + 7] = addrmode == AM_ABX ? 'X' : 'Y';
		break;
	case AM_ZPG:
		// XXX $AB
		FallThrough;
	case AM_ZPX:
		// XXX $AB, X
		FallThrough;
	case AM_ZPY:
		// XXX $AB, Y
		// REAL
		buf[ADDR_FIRSH] = '$';
		BToH(buf + ADDR_FIRSH + 1, code.a1_);
		if (addrmode == AM_ZPG) break;
		buf[ADDR_FIRSH + 3] = ',';
		buf[ADDR_FIRSH + 5] = addrmode == AM_ABX ? 'X' : 'Y';
		break;
	case AM_INX:
		// XXX ($AB, X)
		buf[ADDR_FIRSH + 0] = '(';
		buf[ADDR_FIRSH + 1] = '$';
		BToH(buf + ADDR_FIRSH + 2, code.a1_);
		buf[ADDR_FIRSH + 4] = ',';
		buf[ADDR_FIRSH + 6] = 'X';
		buf[ADDR_FIRSH + 7] = ')';
		break;
	case AM_INY:
		// XXX ($AB), Y
		buf[ADDR_FIRSH + 0] = '(';
		buf[ADDR_FIRSH + 1] = '$';
		BToH(buf + ADDR_FIRSH + 2, code.a1_);
		buf[ADDR_FIRSH + 4] = ')';
		buf[ADDR_FIRSH + 5] = ',';
		buf[ADDR_FIRSH + 7] = 'Y';
		break;
	case AM_IND:
		// XXX ($ABCD)
		buf[ADDR_FIRSH + 0] = '(';
		buf[ADDR_FIRSH + 1] = '$';
		BToH(buf + ADDR_FIRSH + 2, code.a2_);
		BToH(buf + ADDR_FIRSH + 4, code.a1_);
		buf[ADDR_FIRSH + 6] = ')';
		break;
	case AM_REL:
		// XXX $AB(-085)
		// XXX $ABCD
		buf[ADDR_FIRSH + 0] = '$';

		//const uint16_t target = base + int8_t(data_.a1_);
		//BToH(buf + ADDR_FIRSH + 1, uint8_t(target >> 8));
		//BToH(buf + ADDR_FIRSH + 3, uint8_t(target & 0xFF));

		BToH(buf + ADDR_FIRSH + 1, code.a1_);
		buf[ADDR_FIRSH + 3] = '(';
		BToD(buf + ADDR_FIRSH + 4, code.a1_);
		buf[ADDR_FIRSH + 8] = ')';
		break;
	}

}

////===================================================
//设置
void Code::SetOp(const uint8_t& op) {
	this->op_ = op;
}

void Code::SetA1(const uint8_t& a1) {
	this->a1_ = a1;
}

void Code::SetA2(const uint8_t& a2) {
	this->a2_ = a2;
}

void Code::SetCtrl(const uint8_t& ctrl) {
	this->ctrl_ = ctrl;
}