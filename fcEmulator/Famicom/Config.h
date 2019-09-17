#pragma once
#include <stdint.h>
constexpr int MASTER_CYCLE_PER_CPU = 12;

class sfc_config_t
{
public:
	constexpr sfc_config_t(float cc=0.f, uint16_t rr=0, uint16_t scanline=0, uint16_t drawline=0,
		uint16_t hblank=0, uint16_t vi_scanline=0, uint16_t vb_scanline=0) : cpu_clock(cc), refresh_rate(rr), master_cycle_per_scanline(scanline),
		master_cycle_per_drawline(drawline), master_cycle_per_hblank(hblank), visible_scanline(vi_scanline), vblank_scanline(vb_scanline)
	{

	}
	constexpr const uint16_t& get_rr() const {
		return refresh_rate;
	}
	constexpr const uint16_t& get_vb_s() const {
		return vblank_scanline;
	}
	constexpr const uint16_t& get_m_cps() const {
		return master_cycle_per_scanline;
	}
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

