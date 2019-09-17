#pragma once
#include <memory>
#include <assert.h>
#include <string>
#include <algorithm>
#include <iostream>

//======================
#include "Rom.h"
#include "Code.h"
#include "cpu/cpu.h"
#include "ppu/ppu.h"
#include "ppu/Config.h"
#include "Render.h"
//======================
enum ButtonIndex {
	BUTTON_A = 0,
	BUTTON_B,
	BUTTON_SELECT,
	BUTTON_START,
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
};

enum Constant {
	WIDTH = 256,
	HEIGHT = 240,
	SPRITE_COUNT = 64,
};
class Famicom
{
public:
	enum { DISASSEMBLY_BUF_LEN2 = 48 };
	// 静态成员函数
	// 智能指针的单例设计
	//static std::shared_ptr<Famicom> getInstance(void* arg);

	// 加载8k PRG-ROM
	void Load8kPRG(const int& des, const int& src);

	// 析构函数
	~Famicom();

	//加载新的ROM
	ErrorCode LoadNewRom();
	//加载mapper
	ErrorCode LoadMapper(const uint8_t& id);
	// mapper 000 - NROM 重置
	ErrorCode Mapper00Reset();
	//CPU
	cpu cpu_;
	//PPU
	ppu ppu_;

	void BeforeExecute();
	//=============
	uint32_t palette_data_[16];
//========================================================
//	cpu部分

	//指定地方反汇编
	void FcDisassembly(uint16_t address, char buf[]);
	//sfc cpu execute one instruction
	
	RomInfo GetRomInfo() const;

	//=================================================
	//Step 4
	//设置名称表用仓库
	void SetupNametableBank();

	//开始垂直空白标记
	void StartVblankFlag();
	//结束垂直空白标记
	void EndVblankFlag();
	//不可屏蔽中断 垂直空白
	void DoVblank();

	//载入1kCHR ROM
	void Load1kCHR(int des, int src);
	//==================================================
	//使用简易模式渲染一帧，效率较高
	//D0 - 0为全局背景色 1为非全局背景色，是背景D1和D2"与"操作的结果
	//D1 - D5 调色板索引
	void RenderFrameEasy(uint8_t* buffer);
	// 带参构造函数
	Famicom(void* argument);
private:
	// 成员变量
	void *argument;
	RomInfo rom_info_;
	Config config_;
	//重置
	ErrorCode Reset();
	//==================================================
	// 静态成员变量
	// 智能指针的单例设计
	static std::shared_ptr<Famicom> singleFamicom;

	// 加载和卸载rom
	ErrorCode LoadDefaultRom();
	ErrorCode FreeDefaultRom();
	// ban
	Famicom(const Famicom& );
	Famicom operator=(const Famicom& );
	// 初始化和去初始化
	ErrorCode Init();
	void Uninit();
	//render background scanline
	void RenderBackgroundScanline(uint16_t line, const uint8_t sp0[HEIGHT + (16)], uint8_t* buffer);
	//the sprite0 hittest.
	void Sprite0HitTest(uint8_t buffer[WIDTH]);
	//the sprite overflow test.
	uint16_t SpriteOverflowTest();
	//the sprite expand 8.
	void SpriteExpandOn(uint8_t p0, uint8_t p1, uint8_t high, uint8_t* output);
	void SpriteExpandOp(uint8_t p0, uint8_t p1, uint8_t high, uint8_t* output);
	void SpriteExpandRn(uint8_t p0, uint8_t p1, uint8_t high, uint8_t* output);
	void SpriteExpandRp(uint8_t p0, uint8_t p1, uint8_t high, uint8_t* output);
	//the render sprites_
	void RenderSprites(uint8_t* buffer);

};