#include "Render.h"
//======================



#ifdef SFC_NO_SSE
/// <summary>
/// SFCs the pack bool8 into byte.
/// </summary>
/// <param name="array">The array.</param>
/// <returns></returns>
uint8_t PackBoolIntoByte(const uint8_t array[8]) {
	uint8_t hittest = 0;
	for (uint16_t i = 0; i != 8; ++i) {
		hittest <<= 1;
		hittest |= array[i] & 1;
	}
	return hittest;
}

/// <summary>
/// SFCs the expand backgorund 8.
/// </summary>
/// <param name="p0">The p0.</param>
/// <param name="p1">The p1.</param>
/// <param name="high">The high.</param>
/// <param name="output">The output.</param>
 void sfc_expand_backgorund_8(
	uint8_t p0,
	uint8_t p1,
	uint8_t high,
	uint8_t* output) {
#if 0
	FMT = << -EOF
		// %d - D%d
	{
		const uint8_t low%da = p0 & (uint8_t)0x % 02x;
	const uint8_t low%db = p1 & (uint8_t)0x % 02x;
	output[%d] = high | low%da >> %d | low%db >> %d | low%da >> %d | low%db >> %d;
	}
		EOF

		def op_(i)
		print FMT %[
			i, 7 - i,
				i, 1 << (7 - i),
				i, 1 << (7 - i),
				i, i, 6 - i, i, 5 - i,
				i, 7 - i, i, 7 - i,
		]
		end

				8.times{ | i | op_ i }
#endif
				// 0 - D7
			{
				const uint8_t low0a = p0 & (uint8_t)0x80;
				const uint8_t low0b = p1 & (uint8_t)0x80;
				output[0] = high | low0a >> 6 | low0b >> 5 | low0a >> 7 | low0b >> 7;
			}
			// 1 - D6
			{
				const uint8_t low1a = p0 & (uint8_t)0x40;
				const uint8_t low1b = p1 & (uint8_t)0x40;
				output[1] = high | low1a >> 5 | low1b >> 4 | low1a >> 6 | low1b >> 6;
			}
			// 2 - D5
			{
				const uint8_t low2a = p0 & (uint8_t)0x20;
				const uint8_t low2b = p1 & (uint8_t)0x20;
				output[2] = high | low2a >> 4 | low2b >> 3 | low2a >> 5 | low2b >> 5;
			}
			// 3 - D4
			{
				const uint8_t low3a = p0 & (uint8_t)0x10;
				const uint8_t low3b = p1 & (uint8_t)0x10;
				output[3] = high | low3a >> 3 | low3b >> 2 | low3a >> 4 | low3b >> 4;
			}
			// 4 - D3
			{
				const uint8_t low4a = p0 & (uint8_t)0x08;
				const uint8_t low4b = p1 & (uint8_t)0x08;
				output[4] = high | low4a >> 2 | low4b >> 1 | low4a >> 3 | low4b >> 3;
			}
			// 5 - D2
			{
				const uint8_t low5a = p0 & (uint8_t)0x04;
				const uint8_t low5b = p1 & (uint8_t)0x04;
				output[5] = high | low5a >> 1 | low5b >> 0 | low5a >> 2 | low5b >> 2;
			}
			// 6 - D1
			{
				const uint8_t low6a = p0 & (uint8_t)0x02;
				const uint8_t low6b = p1 & (uint8_t)0x02;
				output[6] = high | low6a >> 0 | low6b << 1 | low6a >> 1 | low6b >> 1;
			}
			// 7 - D0
			{
				const uint8_t low7a = p0 & (uint8_t)0x01;
				const uint8_t low7b = p1 & (uint8_t)0x01;
				output[7] = high | low7a << 1 | low7b << 2 | low7a >> 0 | low7b >> 0;
			}
}
#else

//swap byte
void SwapByte(uint8_t* a, uint8_t* b) {
	const auto temp = *a;
	*a = *b;
	*b = temp;
}

//sfcs the pack bool8 into byte
uint8_t PackBoolIntoByte(const uint8_t array[8]) {
	//载入一个允许不对齐的低64(高64置0)位数据
	__m128i values = _mm_loadl_epi64((__m128i*)array);
	//创建序列
	__m128i order = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7);
	//根据已有序列将数据重组
	values = _mm_shuffle_epi8(values, order);
	//将每个32(64也行)位整型左移7位(作为每个8位整型的符号位)
	//然后将8位整型符号位打包成16位整数
	return (uint8_t)_mm_movemask_epi8(_mm_slli_epi32(values, 7));
}



/// Creates the 128 mask_.

/// __m128i _mm_set_epi32(int i3, int i2,int i1, int i0);

/// 使用4个int(32bits)变量来设置__m128i变量
__m128i Create128Mask(uint8_t a, uint8_t b) {
	return _mm_set_epi32(
		//a的低4位
		U32BitLut[a & 0xF],
		//a的高4位
		U32BitLut[a >> 4],
		U32BitLut[b & 0xF],
		U32BitLut[b >> 4]
	);
}

/// SFCs the expand backgorund 16.
void ExpandBackground16(
	uint8_t p0,
	uint8_t p1,
	uint8_t p2,
	uint8_t p3,
	uint8_t high,
	uint8_t* output) {
	//p2是第二个tile的[0] p3是第二个tile的[8]
	//p0是第一个tile的[0] p1是第一个tile的[8]
	const __m128i value1 = Create128Mask(p2, p0);
	const __m128i value2 = Create128Mask(p3, p1);

	//这里实现的是32种的颜色表
	//暂时不知道最低位怎么计算的
	__m128i value = _mm_or_si128(value1, value2);
	value = _mm_or_si128(value, _mm_slli_epi32(value1, 1));
	value = _mm_or_si128(value, _mm_slli_epi32(value2, 2));
	__m128i t = _mm_set1_epi8(high);
	value = _mm_or_si128(value, _mm_set1_epi8(high));

	//value是128位 共16字节 对应16个像素
	(*(__m128i*)output) = value;
}
#endif

//简易模式渲染背景 - 以16像素为单位
void RenderBackgroundPixel16(
	uint8_t high,
	const uint8_t* plane_left,
	const uint8_t* plane_right,
	uint8_t* aligned_palette) {
	const uint8_t plane0 = plane_left[0];
	const uint8_t plane1 = plane_left[8];
	const uint8_t plane2 = plane_right[0];
	const uint8_t plane3 = plane_right[8];
#ifdef SFC_NO_SSE
	sfc_expand_backgorund_8(plane0, plane1, high, aligned_palette + 0);
	sfc_expand_backgorund_8(plane2, plane3, high, aligned_palette + 8);
#else
	ExpandBackground16(plane0, plane1, plane2, plane3, high, aligned_palette);
#endif
}