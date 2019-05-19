#pragma once
#include <memory>
#include "sfc_rom.h"
#include "sfc_code.h"
#include "sfc_cpu.h"
class sfc_famicom_t
{
public:
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

//========================================================
//	cpu部分
	//read cpu address
	uint8_t sfc_read_cpu_address(uint16_t address);
	//write cpu address
	void sfc_write_cpu_address(uint16_t address, uint8_t data);

	
	sfc_rom_info_t get_rom_info() const;

private:
	// 成员变量
	void *argument;
	sfc_rom_info_t rom_info;

	//程序内存仓库(bank)/窗口(window)
	uint8_t* prg_banks[0x10000 >> 13];

	//工作(work)/保存(save)内存
	uint8_t save_memory[8 * 1024];

	//主内存
	uint8_t main_memory[2 * 1024];

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

