#pragma once
#include <memory>
#include <assert.h>
#include <string>
#include <algorithm>

//======================
#include "sfc_rom.h"
#include "sfc_code.h"
#include "sfc_cpu.h"
#include "sfc_ppu_t.h"
#include "sfc_config.h"
#include "sfc_render_ez.h"
//======================
enum sfc_button_index {
	SFC_BUTTON_A = 0,
	SFC_BUTTON_B,
	SFC_BUTTON_SELECT,
	SFC_BUTTON_START,
	SFC_BUTTON_UP,
	SFC_BUTTON_DOWN,
	SFC_BUTTON_LEFT,
	SFC_BUTTON_RIGHT,
};

enum sfc_constant {
	SFC_WIDTH = 256,
	SFC_HEIGHT = 240,
	SFC_SPRITE_COUNT = 64,
};

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
	//=============
	uint32_t palette_data[16];
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

	//载入1kCHR ROM
	void sfc_load_chrrom_1k(int des, int src);
	//==================================================
	//使用简易模式渲染一帧，效率较高
	//D0 - 0为全局背景色 1为非全局背景色，是背景D1和D2"与"操作的结果
	//D1 - D5 调色板索引
	void sfc_render_frame_easy(uint8_t* buffer);
private:
	// 成员变量
	void *argument;
	sfc_rom_info_t rom_info;
	sfc_config_t config_;
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
	//render background scanline
	void sfc_render_background_scanline(uint16_t line, const uint8_t sp0[SFC_HEIGHT + (16)], uint8_t* buffer);
	//the sprite0 hittest.
	void sfc_sprite0_hittest(uint8_t buffer[SFC_WIDTH]);
	//the sprite overflow test.
	uint16_t sfc_sprite_overflow_test();
	//the sprite expand 8.
	void sfc_sprite_expand_8_on(uint8_t p0, uint8_t p1, uint8_t high, uint8_t* output);
	void sfc_sprite_expand_8_op(uint8_t p0, uint8_t p1, uint8_t high, uint8_t* output);
	void sfc_sprite_expand_8_rn(uint8_t p0, uint8_t p1, uint8_t high, uint8_t* output);
	void sfc_sprite_expand_8_rp(uint8_t p0, uint8_t p1, uint8_t high, uint8_t* output);
	//the render sprites
	void sfc_render_sprites(uint8_t* buffer);

};