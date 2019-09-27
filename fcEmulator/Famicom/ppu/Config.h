#pragma once
#include <stdint.h>
constexpr int MASTER_CYCLE_PER_CPU = 12;

class Config
{
public:
	constexpr Config(float cc=0.f, uint16_t rr=0, uint16_t scanline=0, uint16_t drawline=0,
		uint16_t hblank=0, uint16_t vi_scanline=0, uint16_t vb_scanline=0) : cpu_clock_(cc), refresh_rate_(rr), master_cycle_per_scanline_(scanline),
		master_cycle_per_drawline_(drawline), master_cycle_per_hblank_(hblank), visible_scanline_(vi_scanline), vblank_scanline_(vb_scanline)
	{

	}
	constexpr const uint16_t& get_rr() const {
		return refresh_rate_;
	}
	constexpr const uint16_t& get_vb_s() const {
		return vblank_scanline_;
	}
	constexpr const uint16_t& get_m_cps() const {
		return master_cycle_per_scanline_;
	}
private:

	//=================================
	//cpu 主频 Hz
	 float cpu_clock_;
	//屏幕刷新率
	 uint16_t refresh_rate_;
	//每条扫描线周期 Master-Clock
	 uint16_t master_cycle_per_scanline_;
	//每条扫描线渲染周期 Master-Clock
	 uint16_t master_cycle_per_drawline_;
	//每条扫描线水平空白周期
	 uint16_t master_cycle_per_hblank_;
	//可见扫描线
	 uint16_t visible_scanline_;
	//垂直空白扫描线
	 uint16_t vblank_scanline_;
};

