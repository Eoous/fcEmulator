#pragma once

#include <memory>
#include "sfc_rom.h"
#include "sfc_code.h"
#include "sfc_cpu.h"
#include "sfc_ppu_t.h"
#include <assert.h>
#include <string>



class sfc_famicom_t
{
public:
	enum { SFC_DISASSEMBLY_BUF_LEN2 = 48 };
	// 静态成员函数
	// 智能指针的单例设计
	static std::shared_ptr<sfc_famicom_t> getInstance(void* arg);

	// 加载8k PRG-ROM
	void sfc_load_prgrom_8k(const int& des, const int& src);

	// 析构函数
	~sfc_famicom_t();

	//加载新的ROM
	sfc_ecode sfc_load_new_rom();
	//加载mapper
	sfc_ecode sfc_load_mapper(const uint8_t& id);
	// mapper 000 - NROM 重置
	sfc_ecode sfc_mapper_00_reset();
	//CPU
	sfc_cpu cpu_;
	//PPU
	sfc_ppu_t ppu_;


	void sfc_before_execute();

//========================================================
//	cpu部分

	//指定地方反汇编
	void sfc_fc_disassembly(uint16_t address, char buf[]);
	//sfc cpu execute one instruction
	
	sfc_rom_info_t get_rom_info() const;

	//=================================================
	//Step 4
	//设置名称表用仓库
	void sfc_setup_nametable_bank();

	//开始垂直空白标记
	void sfc_vblank_flag_start();
	//结束垂直空白标记
	void sfc_vblank_flag_end();



	//不可屏蔽中断 垂直空白
	void sfc_do_vblank();

private:
	// 成员变量
	void *argument;
	sfc_rom_info_t rom_info;

	//重置
	sfc_ecode sfc_famicom_reset();

	//==================================================
	// 静态成员变量
	// 智能指针的单例设计
	static std::shared_ptr<sfc_famicom_t> singleFamicom;
	// 带参构造函数
	sfc_famicom_t(void* argument);

	// 加载和卸载rom
	sfc_ecode sfc_load_default_rom();
	sfc_ecode sfc_free_default_rom();

	// ban
	sfc_famicom_t(const sfc_famicom_t& );
	sfc_famicom_t operator=(const sfc_famicom_t& );

	// 初始化和去初始化
	sfc_ecode sfc_famicom_init();
	void sfc_famicom_uninit();


};
