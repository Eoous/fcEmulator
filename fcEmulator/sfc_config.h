#pragma once
#include <stdint.h>
constexpr int MASTER_CYCLE_PER_CPU = 12;

class sfc_config_t
{
public:
	sfc_config_t() = default;
	constexpr sfc_config_t(float cc, uint16_t rr, uint16_t scanline, uint16_t drawline,
		uint16_t hblank, uint16_t vi_scanline, uint16_t vb_scanline) : cpu_clock(cc), refresh_rate(rr), master_cycle_per_scanline(scanline),
		master_cycle_per_drawline(drawline), master_cycle_per_hblank(hblank), visible_scanline(vi_scanline), vblank_scanline(vb_scanline)
	{

	}
	constexpr sfc_config_t operator=(const sfc_config_t& con) {
		return con;
	}
	~sfc_config_t() = default;
private:

	//=================================
	//cpu 主频 Hz
	 float cpu_clock;
	//屏幕刷新率
	 uint16_t refresh_rate;
	//每条扫描线周期 Master-Clock
	 uint16_t master_cycle_per_scanline;
	//每条扫描线渲染周期 Master-Clock
	 uint16_t master_cycle_per_drawline;
	//每条扫描线水平空白周期
	 uint16_t master_cycle_per_hblank;
	//可见扫描线
	 uint16_t visible_scanline;
	//垂直空白扫描线
	 uint16_t vblank_scanline;
};

