#include "sfc_famicom_t.h"



//===========================================
//config 信息
//NTSC制式 配置信息
constexpr sfc_config_t SFC_CONFIG_NTSC(1789773.f,
	60,
	1364,
	1024,
	340,
	240,
	20);
//PAL制式 配置信息
constexpr sfc_config_t SFC_CONFIG_PAL(1662607.f,
	50,
	1362,
	1024,
	338,
	312,
	70);


sfc_famicom_t::sfc_famicom_t(void *argument):argument(argument)
{
	sfc_famicom_init();
}

sfc_famicom_t::~sfc_famicom_t()
{
	
	sfc_famicom_uninit();
}

sfc_ecode sfc_famicom_t::sfc_famicom_init() {
	assert(this && "bad famicom");

	//清空数据
	memset(this, 0, sizeof(sfc_famicom_t));

	//初步BANK
	cpu_.prg_banks[0] = cpu_.main_memory;
	cpu_.prg_banks[3] = cpu_.save_memory;
	cpu_.pppu_ = &ppu_;

	config_ = SFC_CONFIG_NTSC;

	return sfc_load_new_rom();
	return SFC_ERROR_OK;
}

void sfc_famicom_t::sfc_famicom_uninit() {
	sfc_free_default_rom();
}



sfc_ecode sfc_famicom_t::sfc_load_default_rom() {
	assert(rom_info.data_prgrom == nullptr && "FREE FIRST");
	FILE *const file = fopen("color_test.nes", "rb");

	if (!file) {
		return SFC_ERROR_FILE_NOT_FOUNT;
	}
	sfc_ecode code = SFC_ERROR_ILLEGAL_FILE;

	//read header
	sfc_nes_header_t nes_header;
	if (fread(&nes_header, sizeof(nes_header), 1, file)) {
		union { uint32_t u32; uint8_t id[4]; }this_union;
		this_union.id[0] = 'N';
		this_union.id[1] = 'E';
		this_union.id[2] = 'S';
		this_union.id[3] = '\x1A';

		if (this_union.u32 == nes_header.id) {
			const size_t size1 = 16 * 1024 * nes_header.count_prgrom16kb;
			// 允许没有CHR-ROM(使用CHR-RAM代替)
			const size_t size2 = 8 * 1024 * std::max(nes_header.count_chrrom_8kb , (uint8_t)1);
			uint8_t* const ptr = new uint8_t[size1+size2];

			//内存申请成功
			if (ptr) {
				code = SFC_ERROR_OK;


				if (nes_header.control1 & SFC_NES_TRAINER) {
					fseek(file, 512, SEEK_CUR);
				}
				fread(ptr, size1 + size2, 1, file);

				rom_info.data_prgrom = ptr;
				rom_info.data_chrrom = ptr + size1;
				rom_info.count_prgrom16kb = nes_header.count_prgrom16kb;
				rom_info.count_chrrom_8kb = nes_header.count_chrrom_8kb;

				//nes_header.control1 >> 4 		0000 NNNN			   nes_header.control2 & 0xF0	NNNN 0000
				//表示取control1的高4位NNNN，对应Mapper低4位		   表示取control2的高4位NNNN，对应Mapper高4位(涉及到& 0xF0运算技巧)
				rom_info.mapper_number = (nes_header.control1 >> 4) | (nes_header.control2 & 0xF0);	// 或运算后为NNNN NNNN
				rom_info.vmirroring = (nes_header.control1 & SFC_NES_VMIRROR) > 0;
				rom_info.four_screen = (nes_header.control1 & SFC_NES_4SCREEN) > 0;
				rom_info.save_ram = (nes_header.control1 & SFC_NES_SAVERAM) > 0;
				assert(!(nes_header.control1 &SFC_NES_TRAINER) && "unsuppported");
				assert(!(nes_header.control2 &SFC_NES_VS_UNISYSTEM) && "unsuppported");
				assert(!(nes_header.control2 &SFC_NES_Playchoice10) && "unsuppported");
			}

			else {
				code = SFC_ERROR_OUT_OF_MEMORY;
			}
		}
		//非法文件
	}
	fclose(file);
	return code;
}

sfc_ecode sfc_famicom_t::sfc_free_default_rom() {
	delete [](rom_info.data_prgrom);
	rom_info.data_prgrom = nullptr;
	//printf("析构了famicomm");


	return SFC_ERROR_OK;
}

sfc_rom_info_t sfc_famicom_t::get_rom_info() const {
	return rom_info;
}

std::shared_ptr<sfc_famicom_t> sfc_famicom_t::singleFamicom = nullptr;

std::shared_ptr<sfc_famicom_t> sfc_famicom_t::getInstance(void* arg) {
	if (!singleFamicom) {
		singleFamicom = std::shared_ptr<sfc_famicom_t>(new sfc_famicom_t(arg));
	}
	return singleFamicom;
}

//========================
//3
//载入8k PRG-ROM
void sfc_famicom_t::sfc_load_prgrom_8k(const int& des, const int& src) {
	cpu_.prg_banks[4 + des] = rom_info.data_prgrom + 8 * 1024 * src;
}






sfc_ecode sfc_famicom_t::sfc_mapper_00_reset() {
	assert(rom_info.count_prgrom16kb && "bad count");
	assert(rom_info.count_prgrom16kb <= 2 && "bad count");
	//16kb -> 载入 $8000-$BFFF ,$C000-$FFFF 为镜像
	const int id2 = rom_info.count_prgrom16kb & 2;

	//32kb -> 载入 $8000-$FFFF
	sfc_load_prgrom_8k(0, 0);
	sfc_load_prgrom_8k(1, 1);
	sfc_load_prgrom_8k(2, id2 + 0);
	sfc_load_prgrom_8k(3, id2 + 1);

	for (int i = 0; i != 8; ++i) {				//CHR-ROM 角色只读存储器 放入PPU，8k为1个单位 此处应该是加载图样表(贴图)共8kb
		sfc_load_chrrom_1k(i, i);
	}
	return SFC_ERROR_OK;
}

//加载ROM
sfc_ecode sfc_famicom_t::sfc_load_new_rom() {
	//先释放旧的ROM
	sfc_ecode code = sfc_free_default_rom();
	//清空数据
	memset(&rom_info, 0, sizeof(rom_info));
	//载入ROM
	if (code == SFC_ERROR_OK) {
		code = sfc_load_default_rom();
	}

	//载入新的Mapper
	if (code == SFC_ERROR_OK) {
		code = sfc_load_mapper(rom_info.mapper_number);
	}

	//首次重置
	if (code == SFC_ERROR_OK) {
		sfc_famicom_reset();
	}
	return code;
}

//加载Mapper
sfc_ecode sfc_famicom_t::sfc_load_mapper(const uint8_t& id) {
	switch (id) {
	case 0:
		return sfc_mapper_00_reset();
	}

	return SFC_ERROR_FILE_NOT_FOUNT;
}

//============================================
//	cpu部分






//反汇编
void sfc_famicom_t::sfc_fc_disassembly(uint16_t address, char buf[]) {
	enum {
		OFFSET_M= SFC_DISASSEMBLY_BUF_LEN2 -SFC_DISASSEMBLY_BUF_LEN,
		OFFSET=8
	};

	static_assert(OFFSET < OFFSET_M, "LESS!");
	memset(buf + 1, ' ', OFFSET);
	buf[0] = '$';
	sfc_btoh(buf + 1, (uint8_t)(address >> 8));
	sfc_btoh(buf + 3, (uint8_t)(address));

	sfc_6502_code_t code;
	code.data = 0;

	code.set_op(cpu_.sfc_read_cpu_address(address));
	//获取指令长度
	switch (cpu_.sfc_get_inslen(code.get_op())) {
	case 3:
		code.set_a2(cpu_.sfc_read_cpu_address(address + 2));
	case 2:
		code.set_a1(cpu_.sfc_read_cpu_address(address + 1));
	}

	//反汇编
	code.sfc_6502_disassembly(code,buf + OFFSET);

}



sfc_ecode sfc_famicom_t::sfc_famicom_reset() {
	// 重置mapper
	sfc_ecode ecode = sfc_mapper_00_reset();
	if (ecode) return ecode;
	// 初始化寄存器
	const uint8_t pcl = cpu_.sfc_read_cpu_address(SFC_VECTOR_RESET + 0);
	const uint8_t pch = cpu_.sfc_read_cpu_address(SFC_VECTOR_RESET + 1);
	cpu_.registers_.get_program_counter() = (uint16_t)pcl | (uint16_t)pch << 8;			//设置pc初始地址c004 (下面会重置成c000) ,如果从c004开始会报错(可能因为有个没有完善)
	cpu_.registers_.get_accumulator() = 0;
	cpu_.registers_.get_x_index() = 0;
	cpu_.registers_.get_y_index() = 0;
	cpu_.registers_.get_stack_pointer() = 0xfd;
	cpu_.registers_.get_status() = 0x34| SFC_FLAG_R;    //  一直为1

	//调色板
	//名称表
	sfc_setup_nametable_bank();
	//镜像	$3000 - $3eff 大小$0f00 为$2000 - $2eff镜像
	ppu_.banks[0xc] = ppu_.banks[0x8];
	ppu_.banks[0xd] = ppu_.banks[0x9];
	ppu_.banks[0xe] = ppu_.banks[0xa];
	ppu_.banks[0xf] = ppu_.banks[0xb];
//#if 1
//	// 测试指令ROM(nestest.nes)
//	cpu_.registers_.get_program_counter() = 0xC000;
//#endif
	return SFC_ERROR_OK;
}

//sfcs the cpu execute one
void sfc_famicom_t::sfc_before_execute() {
	static int line = 0;
	line++;
	char buf[48];
	const uint16_t pc = cpu_.registers_.get_program_counter();
	sfc_fc_disassembly(pc, buf);
	printf(
		"%4d - %s   A:%02X X:%02X Y:%02X P:%02X SP:%02X\n",
		line, buf,
		(int)cpu_.registers_.get_accumulator(),
		(int)cpu_.registers_.get_x_index(),
		(int)cpu_.registers_.get_y_index(),
		(int)cpu_.registers_.get_status(),
		(int)cpu_.registers_.get_stack_pointer()
	);
}

//===========================
//step 4
void sfc_famicom_t::sfc_setup_nametable_bank() {				//加载名称表 nameteble  banks[8]=$2000 - $23ff (1kb)	名称表0
	//four screens == 4屏																banks[9]=$2400 - $27ff			名称表1
	if (rom_info.four_screen) {														//	banks[a]=$2800 - $2bff			名称表2
		ppu_.banks[0x8] = cpu_.video_memory + 0x400 * 0;							//	banks[b]=$2c00 - $2fff			名称表3
		ppu_.banks[0x9] = cpu_.video_memory + 0x400 * 1;
		ppu_.banks[0xa] = cpu_.video_memory_ex + 0x400 * 0;
		ppu_.banks[0xb] = cpu_.video_memory_ex + 0x400 * 1;
	}
	//横板
	else if (rom_info.vmirroring) {
		ppu_.banks[0x8] = cpu_.video_memory + 0x400 * 0;
		ppu_.banks[0x9] = cpu_.video_memory + 0x400 * 1;
		ppu_.banks[0xa] = cpu_.video_memory + 0x400 * 0;
		ppu_.banks[0xb] = cpu_.video_memory + 0x400 * 1;
	}
	//纵版
	else {
		ppu_.banks[0x8] = cpu_.video_memory + 0x400 * 0;		
		ppu_.banks[0x9] = cpu_.video_memory + 0x400 * 0;
		ppu_.banks[0xa] = cpu_.video_memory + 0x400 * 1;
		ppu_.banks[0xb] = cpu_.video_memory + 0x400 * 1;
	}
}

//开始垂直空白标记
void sfc_famicom_t::sfc_vblank_flag_start(){
	ppu_.status |= (uint8_t)SFC_PPU2002_VBlank;
}

void sfc_famicom_t::sfc_vblank_flag_end() {
	ppu_.status |= ~(uint8_t)SFC_PPU2002_VBlank;
}

void sfc_famicom_t::sfc_do_vblank() {
	sfc_vblank_flag_start();
	if (ppu_.ctrl&(uint8_t)SFC_PPU2000_NMIGen) {
		cpu_.sfc_operation_NMI();
	}
}

//
void sfc_famicom_t::sfc_load_chrrom_1k(int des, int src) {
	ppu_.banks[des] = rom_info.data_chrrom + 1024 * src;
}

//=========================================
//step7
void sfc_famicom_t::sfc_render_background_scanline(uint16_t line, const uint8_t sp0[SFC_HEIGHT + (16)], uint8_t * buffer) {
	//取消背景显示
	if (!(ppu_.mask&(uint8_t)SFC_PPU2001_Back))return;

	//计算当前偏移量
	const uint16_t scrollx = (uint16_t)ppu_.scroll[0] + (uint16_t)((ppu_.nametable_select & 1) << 8);
	const uint16_t scrolly = line + (uint16_t)ppu_.now_scrolly + (uint16_t)((ppu_.nametable_select & 2) ? 240 : 0);

	//由于Y是240一换，需要膜计算
	const uint16_t scrolly_index0 = scrolly / (uint16_t)240;
	const uint16_t scrolly_offset = scrolly % (uint16_t)240;

	//计算背景所使用的图样表
	const uint8_t* const pattern = ppu_.banks[ppu_.ctrl&SFC_PPU2000_BgTabl ? 4 : 0];
	//检测垂直偏移量确定使用图案表的前一半[8-9]还是后一半[10-11]
	const uint8_t* table[2];

	const int first_buck = 8 + ((scrolly_index0 & 1) << 1);
	table[0] = ppu_.banks[first_buck];
	table[1] = ppu_.banks[first_buck + 1];
	// 以16像素为单位扫描该行
	SFC_ALIGNAS(16) uint8_t aligned_buffer[SFC_WIDTH + 16 + 16];
	{
		const uint8_t realy = (uint8_t)scrolly_offset;
		// 保险起见扫描16+1次
		for (uint16_t i = 0; i != 17; ++i) {
			const uint16_t realx = scrollx + (i << 4);
			const uint8_t* nt = table[(realx >> 8) & 1];
			const uint8_t xunit = (realx & (uint16_t)0xF0) >> 4;
			// 获取32为单位的调色板索引字节
			const uint8_t attr = (nt + 32 * 30)[(realy >> 5 << 3) | (xunit >> 1)];
			// 获取属性表内位偏移
			const uint8_t aoffset = ((uint8_t)(xunit & 1) << 1) | ((realy & 0x10) >> 2);
			// 计算高两位
			const uint8_t high = (attr & (3 << aoffset)) >> aoffset << 3;
			// 计算图样表位置
			const uint8_t* too_young = nt + ((uint16_t)xunit << 1) + (uint16_t)(realy >> 3 << 5);
			const uint8_t too_young0 = too_young[0];
			const uint8_t too_young1 = too_young[1];
			// 渲染16个像素
			sfc_render_background_pixel16(
				high,
				pattern + too_young0 * 16 + (realy & 7),
				pattern + too_young1 * 16 + (realy & 7),
				aligned_buffer + (i << 4)
			);
		}
		// 将数据复制过去
		const uint8_t* const unaligned_buffer = aligned_buffer + (scrollx & 0x0f);
		memcpy(buffer, unaligned_buffer, SFC_WIDTH);
	}
	// 基于行的精灵0命中测试

	// 已经命中了
	if (ppu_.status & (uint8_t)SFC_PPU2002_Sp0Hit) return;
	// 没有必要测试
	const uint8_t hittest_data = sp0[line];
	if (!hittest_data) return;
	// 精灵#0的数据
	uint8_t* const unaligned_buffer = aligned_buffer + (scrollx & 0x0f);
	memset(unaligned_buffer + SFC_WIDTH, 0, 16);

	const uint8_t  xxxxx = ppu_.sprites[3];
	const uint8_t hittest = sfc_pack_bool8_into_byte(unaligned_buffer + xxxxx);
	if (hittest_data & hittest)
		ppu_.status |= (uint8_t)SFC_PPU2002_Sp0Hit;

}

void sfc_famicom_t::sfc_sprite0_hittest(uint8_t buffer[SFC_WIDTH]) {
	//先清空
	memset(buffer, 0, SFC_WIDTH);
	//关闭渲染
	enum { BOTH_BS = SFC_PPU2001_Back | SFC_PPU2001_Sprite };
	if ((ppu_.mask& (uint8_t)BOTH_BS) != (uint8_t)BOTH_BS)return;
	//获取数据以填充
	const uint8_t yyyyy = ppu_.sprites[0];
	//0xef以上就算了
	if (yyyyy >= 0xef) return;
	const auto iiiii = ppu_.sprites[1];
	const auto sp8x16 = ppu_.ctrl&SFC_PPU2000_Sp8x16;
	const auto* const spp = ppu_.banks[sp8x16 ?
		//偶数用0000 奇数1000
		(iiiii & 1 ? 4 : 0)
		:
		//检查SFC_PPU2000_SpTabl
		(ppu_.ctrl&SFC_PPU2000_SpTabl ? 4 : 0)];
	//attributes
	const auto aaaaa = ppu_.sprites[2];
	//获取平面数据
	const auto* const nowp0 = spp + iiiii * 16;
	const auto* const nowp1 = nowp0 + 8;
	auto *const buffer_write = buffer + yyyyy + 1;
	//8x16的情况
	const auto count = sp8x16 ? 16 : 8;

	//水平翻转
	if (aaaaa&(uint8_t)SFC_SPATTR_FlipH) {
		for (int i = 0; i != count; ++i) {
			const auto data = nowp0[i] | nowp1[i];
			buffer_write[i] = BitReverseTable256[data];
		}
	}
	//普通
	else {
		for (int i = 0; i != count; ++i) {
			const auto data = nowp0[i] | nowp1[i];
			buffer_write[i] = data;
		}
	}

	//垂直翻转
	if (aaaaa &(uint8_t)SFC_SPATTR_FlipV) {
		// 8x16
		if (sp8x16) {
			sfc_swap_byte(buffer_write + 0, buffer_write + 0xF);
			sfc_swap_byte(buffer_write + 1, buffer_write + 0xE);
			sfc_swap_byte(buffer_write + 2, buffer_write + 0xD);
			sfc_swap_byte(buffer_write + 3, buffer_write + 0xC);
			sfc_swap_byte(buffer_write + 4, buffer_write + 0xB);
			sfc_swap_byte(buffer_write + 5, buffer_write + 0xA);
			sfc_swap_byte(buffer_write + 6, buffer_write + 0x9);
			sfc_swap_byte(buffer_write + 7, buffer_write + 0x8);
		}
		else {
			sfc_swap_byte(buffer_write + 0, buffer_write + 7);
			sfc_swap_byte(buffer_write + 1, buffer_write + 6);
			sfc_swap_byte(buffer_write + 2, buffer_write + 5);
			sfc_swap_byte(buffer_write + 3, buffer_write + 4);
		}
	}
}