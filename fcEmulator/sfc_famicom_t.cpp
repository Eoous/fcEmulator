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
	FILE *const file = fopen("mario.nes", "rb");

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

//std::shared_ptr<sfc_famicom_t> sfc_famicom_t::getInstance(void* arg) {
//	if (singleFamicom==nullptr) {
//		singleFamicom = std::shared_ptr<sfc_famicom_t>(new sfc_famicom_t(arg));
//	}
//	return singleFamicom;
//}

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
		cpu_.cpu_cycle_count, buf,
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
//line = 当前渲染的扫面线行号
void sfc_famicom_t::sfc_render_background_scanline(uint16_t line, const uint8_t sp0[SFC_HEIGHT + (16)], uint8_t * buffer) {
	//取消背景显示
	if (!(ppu_.mask&(uint8_t)SFC_PPU2001_Back))return;

	//计算当前偏移量
	//读取滚动偏移量x和y
	//这个滚动偏移应该是窗口的位置
	const uint16_t scrollx = (uint16_t)ppu_.scroll[0] + (uint16_t)((ppu_.nametable_select & 1) << 8);
	const uint16_t scrolly = line + (uint16_t)ppu_.now_scrolly + (uint16_t)((ppu_.nametable_select & 2) ? 240 : 0);

	//由于Y是240一换，需要模计算
	//index0 = ppu_.nametable_select & 2?
	//offset
	//y轴只有240个像素点 多出来的是对应面的镜像，直接算出来偏移量就可以了
	//offset像是真实的y坐标
	const uint16_t scrolly_index0 = scrolly / (uint16_t)240;
	const uint16_t scrolly_offset = scrolly % (uint16_t)240;

	//计算背景所使用的图样表
	//cpu中$2000 D4位
	//如果为1，背景从$1000开始。
	//如果为0，背景从$0000开始
	const auto* const pattern = ppu_.banks[ppu_.ctrl&SFC_PPU2000_BgTabl ? 4 : 0];

	//检测垂直偏移量确定使用图案表的前一半[8-9]还是后一半[10-11]
	//8 - 00
	//9 - 01
	//10 - 10
	//11 - 11
	const uint8_t* table[2];

	const int first_buck = 8 + ((scrolly_index0 & 1) << 1);
	table[0] = ppu_.banks[first_buck];
	table[1] = ppu_.banks[first_buck + 1];

	//以16像素为单位扫描该行 每次渲染16像素 == 每次渲染2个tiles
	//1个tile 在某行有8像素，16像素则为2个tiles
	//这里去掉了 alignas(16) ，原作者应该是想16像素对齐的，加上不知道有什么意义
	uint8_t aligned_buffer[SFC_WIDTH + 16 + 16];
	//std::cout << unsigned(&aligned_buffer) << std::endl;
	//std::cout << unsigned(&aligned_buffer[16]) << std::endl;
	//std::cout << unsigned(&aligned_buffer[32]) << std::endl;
	//std::cout << &aligned_buffer[3] << std::endl;

{
	const uint8_t realy = (uint8_t)scrolly_offset;
	// 保险起见扫描16+1次
	for (uint16_t i = 0; i != 17; ++i) {
		//realx 当前以16像素为单位渲染的x坐标
		//realy							y坐标
		const uint16_t realx = scrollx + (i << 4);
		//选择名称表
		//scrollx=1，i=16时候 当前渲染像素则是01页的第一个像素
		//realx=257   256>>8=1
		//00页由第一个名称表控制
		//01页由第二个名称表控制
		const uint8_t* nt = table[(realx >> 8) & 1];//(realx >> 8) & 1
		//横向32个tiles 每2个tiles一组，共16组 用坐标高4位来判断是哪一组
		const uint8_t xunit = (realx & (uint16_t)0xf0) >> 4;
		// 获取32为单位的调色板索引字节
		//
		const uint8_t attr = (nt + 32 * 30)[(realy >> 5 << 3) | (xunit >> 1)];
		// 获取属性表内位偏移
		//xunit & 1 如果是0 在第1列	realy & 0x10 如果是0000 0000 在第1行 
		//			如果是1 在第2列				 如果是0001 0000 在第2行
		//y在前 x在后
		const uint8_t aoffset = ((uint8_t)(xunit & 1) << 1) | ((realy & 0x10) >> 2);

		// 计算高两位
		// 因为是32色 所以最后<< 3 如果是16色 最后<< 2
		const uint8_t high = (attr & (0x03 << aoffset)) >> aoffset << 3;

/*		计算图样表位置
		xuint_t << 1 
		以16像素为单位渲染 每个目标的对应行在图样表x偏移量
		realy >> 3 << 5 
		 << 5 是因为每行有32个tiles，如果realy>>3 表示对应行	*/
		const uint8_t* pt = nt + ((uint16_t)xunit << 1) + (uint16_t)(realy >> 3 << 5);
		//渲染的第1个tile在pt中位置(更觉得是以tile为单位的偏移量)
		const uint8_t pt0 = pt[0];
		//渲染的第2个tile在pt中位置
		const uint8_t pt1 = pt[1];

		// 渲染16个像素
		sfc_render_background_pixel16(
			high,
			pattern + pt0 * 16 + (realy & 7),
			pattern + pt1 * 16 + (realy & 7),
			aligned_buffer + (i << 4)
		);
	}
	//printf("%d\n", scrollx & 0x0f);
	// 将数据复制过去
	// +(scrollx & 0xf) 可能是因为要绘制的部分后面多出来16像素
	const uint8_t* const unaligned_buffer = aligned_buffer + (scrollx & 0x0f);
	memcpy(buffer, unaligned_buffer, SFC_WIDTH+(scrollx & 0x0f));
}
	// 基于行的精灵0命中测试

	// 已经命中(在sp0之后)了
	// 一开始status没有Sp0Hit 命中之后会进行修改 后面的就不会计算
	if (ppu_.status & (uint8_t)SFC_PPU2002_Sp0Hit) return;

	// 没有必要测试
	//sp0[line] 对应行数如果有sp0 那么hittest_data是它的颜色
	const uint8_t hittest_data = sp0[line];

	// hittest_data=0 即不属于sp0像素(在sp0之前)的时候 直接返回
	// hittest_data=1 即属于sp0内像素的时候 计算下面的
	 if (!hittest_data) return;

	// 精灵#0的数据
	// uint8_t* const unaligned_buffer = aligned_buffer + (scrollx & 0x0f);
	uint8_t* const unaligned_buffer = aligned_buffer;

	// memset(unaligned_buffer + SFC_WIDTH, 0, 16);		// 原语句
	// memset(unaligned_buffer, 0, 16);					// 00页的这一行前16个像素全楚数据 实测没用

	// xxxxx 是sp0的x坐标
	const uint8_t xxxxx = ppu_.sprites[3];
	const uint8_t hittest = sfc_pack_bool8_into_byte(unaligned_buffer + xxxxx);

	//PPU 的内置硬件把位于内存位置 0 的 Sprite 特殊对待。当这一 Sprite 生成时，一旦其和背景的可见部分重叠在一起，一个被称为 Sprite0 flag 的比特位就被置 1。
	if (hittest_data & hittest)
		ppu_.status |= (uint8_t)SFC_PPU2002_Sp0Hit;

}


// 实测发现 貌似这部分只是绘制sp0而已，并不进行检测
void sfc_famicom_t::sfc_sprite0_hittest(uint8_t buffer[SFC_WIDTH]) {
	//先清空
	memset(buffer, 0, SFC_WIDTH);
	//关闭渲染
	enum { BOTH_BS = SFC_PPU2001_Back | SFC_PPU2001_Sprite };
	if ((ppu_.mask& (uint8_t)BOTH_BS) != (uint8_t)BOTH_BS)return;
	//获取sp0的数据以填充
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

// the sprites overflow test
uint16_t sfc_famicom_t::sfc_sprite_overflow_test(){
	//完全关闭渲染
	enum{BOTH_BS = SFC_PPU2001_Back|SFC_PPU2001_Sprite};
	if (!(ppu_.mask&(uint8_t)BOTH_BS))return SFC_HEIGHT;
	//正式处理
	uint8_t buffer[256 + 16];
	memset(buffer, 0, 256);
	// 8 x 16
	const int height = ppu_.ctrl&SFC_PPU2000_Sp8x16 ? 16 : 8;
	for (int i = 0; i != SFC_SPRITE_COUNT; ++i) {
		const uint8_t y = ppu_.sprites[i * 4];
		for (int i = 0; i != height; ++i) {
			buffer[y + i]++;
		}
	}
	//搜索第一个超过8的
	uint16_t line;
	for (line = 0; line != SFC_HEIGHT; ++line) {
		if (buffer[line] > 8)break;
	}

	return line;
}

void sfc_famicom_t::sfc_sprite_expand_8_on(uint8_t p0, uint8_t p1, uint8_t high, uint8_t * output){
	// 0 - D7
	const uint8_t low0 = ((p0 & (uint8_t)0x80) >> 6) | ((p1 & (uint8_t)0x80) >> 5);
	if (low0) output[0] = high | low0;
	// 1 - D6
	const uint8_t low1 = ((p0 & (uint8_t)0x40) >> 5) | ((p1 & (uint8_t)0x40) >> 4);
	if (low1) output[1] = high | low1;
	// 2 - D5
	const uint8_t low2 = ((p0 & (uint8_t)0x20) >> 4) | ((p1 & (uint8_t)0x20) >> 3);
	if (low2) output[2] = high | low2;
	// 3 - D4
	const uint8_t low3 = ((p0 & (uint8_t)0x10) >> 3) | ((p1 & (uint8_t)0x10) >> 2);
	if (low3) output[3] = high | low3;
	// 4 - D3
	const uint8_t low4 = ((p0 & (uint8_t)0x08) >> 2) | ((p1 & (uint8_t)0x08) >> 1);
	if (low4) output[4] = high | low4;
	// 5 - D2
	const uint8_t low5 = ((p0 & (uint8_t)0x04) >> 1) | ((p1 & (uint8_t)0x04) >> 0);
	if (low5) output[5] = high | low5;
	// 6 - D1
	const uint8_t low6 = ((p0 & (uint8_t)0x02) >> 0) | ((p1 & (uint8_t)0x02) << 1);
	if (low6) output[6] = high | low6;
	// 7 - D0
	const uint8_t low7 = ((p0 & (uint8_t)0x01) << 1) | ((p1 & (uint8_t)0x01) << 2);
	if (low7) output[7] = high | low7;
}

void sfc_famicom_t::sfc_sprite_expand_8_op(uint8_t p0, uint8_t p1, uint8_t high, uint8_t * output){
	// 0 - D7
	const uint8_t low0 = ((p0 & (uint8_t)0x80) >> 6) | ((p1 & (uint8_t)0x80) >> 5);
	if (~output[0] & 1) output[0] = high | low0;
	// 1 - D6
	const uint8_t low1 = ((p0 & (uint8_t)0x40) >> 5) | ((p1 & (uint8_t)0x40) >> 4);
	if (~output[1] & 1) output[1] = high | low1;
	// 2 - D5
	const uint8_t low2 = ((p0 & (uint8_t)0x20) >> 4) | ((p1 & (uint8_t)0x20) >> 3);
	if (~output[2] & 1) output[2] = high | low2;
	// 3 - D4
	const uint8_t low3 = ((p0 & (uint8_t)0x10) >> 3) | ((p1 & (uint8_t)0x10) >> 2);
	if (~output[3] & 1) output[3] = high | low3;
	// 4 - D3
	const uint8_t low4 = ((p0 & (uint8_t)0x08) >> 2) | ((p1 & (uint8_t)0x08) >> 1);
	if (~output[4] & 1) output[4] = high | low4;
	// 5 - D2
	const uint8_t low5 = ((p0 & (uint8_t)0x04) >> 1) | ((p1 & (uint8_t)0x04) >> 0);
	if (~output[5] & 1) output[5] = high | low5;
	// 6 - D1
	const uint8_t low6 = ((p0 & (uint8_t)0x02) >> 0) | ((p1 & (uint8_t)0x02) << 1);
	if (~output[6] & 1) output[6] = high | low6;
	// 7 - D0
	const uint8_t low7 = ((p0 & (uint8_t)0x01) << 1) | ((p1 & (uint8_t)0x01) << 2);
	if (~output[7] & 1) output[7] = high | low7;
}

void sfc_famicom_t::sfc_sprite_expand_8_rn(uint8_t p0, uint8_t p1, uint8_t high, uint8_t * output){
	// 7 - D7
	const uint8_t low0 = ((p0 & (uint8_t)0x80) >> 6) | ((p1 & (uint8_t)0x80) >> 5);
	if (low0) output[7] = high | low0;
	// 6 - D6
	const uint8_t low1 = ((p0 & (uint8_t)0x40) >> 5) | ((p1 & (uint8_t)0x40) >> 4);
	if (low1) output[6] = high | low1;
	// 5 - D5
	const uint8_t low2 = ((p0 & (uint8_t)0x20) >> 4) | ((p1 & (uint8_t)0x20) >> 3);
	if (low2) output[5] = high | low2;
	// 4 - D4
	const uint8_t low3 = ((p0 & (uint8_t)0x10) >> 3) | ((p1 & (uint8_t)0x10) >> 2);
	if (low3) output[4] = high | low3;
	// 3 - D3
	const uint8_t low4 = ((p0 & (uint8_t)0x08) >> 2) | ((p1 & (uint8_t)0x08) >> 1);
	if (low4) output[3] = high | low4;
	// 2 - D2
	const uint8_t low5 = ((p0 & (uint8_t)0x04) >> 1) | ((p1 & (uint8_t)0x04) >> 0);
	if (low5) output[2] = high | low5;
	// 1 - D1
	const uint8_t low6 = ((p0 & (uint8_t)0x02) >> 0) | ((p1 & (uint8_t)0x02) << 1);
	if (low6) output[1] = high | low6;
	// 0 - D0
	const uint8_t low7 = ((p0 & (uint8_t)0x01) << 1) | ((p1 & (uint8_t)0x01) << 2);
	if (low7) output[0] = high | low7;
}

void sfc_famicom_t::sfc_sprite_expand_8_rp(uint8_t p0, uint8_t p1, uint8_t high, uint8_t * output){
		// 7 - D7
		const uint8_t low0 = ((p0 & (uint8_t)0x80) >> 6) | ((p1 & (uint8_t)0x80) >> 5);
		if (~output[7] & 1) output[7] = high | low0;
		// 6 - D6
		const uint8_t low1 = ((p0 & (uint8_t)0x40) >> 5) | ((p1 & (uint8_t)0x40) >> 4);
		if (~output[6] & 1) output[6] = high | low1;
		// 5 - D5
		const uint8_t low2 = ((p0 & (uint8_t)0x20) >> 4) | ((p1 & (uint8_t)0x20) >> 3);
		if (~output[5] & 1) output[5] = high | low2;
		// 4 - D4
		const uint8_t low3 = ((p0 & (uint8_t)0x10) >> 3) | ((p1 & (uint8_t)0x10) >> 2);
		if (~output[4] & 1) output[4] = high | low3;
		// 3 - D3
		const uint8_t low4 = ((p0 & (uint8_t)0x08) >> 2) | ((p1 & (uint8_t)0x08) >> 1);
		if (~output[3] & 1) output[3] = high | low4;
		// 2 - D2
		const uint8_t low5 = ((p0 & (uint8_t)0x04) >> 1) | ((p1 & (uint8_t)0x04) >> 0);
		if (~output[2] & 1) output[2] = high | low5;
		// 1 - D1
		const uint8_t low6 = ((p0 & (uint8_t)0x02) >> 0) | ((p1 & (uint8_t)0x02) << 1);
		if (~output[1] & 1) output[1] = high | low6;
		// 0 - D0
		const uint8_t low7 = ((p0 & (uint8_t)0x01) << 1) | ((p1 & (uint8_t)0x01) << 2);
		if (~output[0] & 1) output[0] = high | low7;
}

void sfc_famicom_t::sfc_render_sprites(uint8_t * buffer){
	//8x16
	const uint8_t sp8x16 = (ppu_.ctrl&(uint8_t)SFC_PPU2000_Sp8x16) >> 2;
	//精灵用图样
	const uint8_t* sppbuffer[2];
	sppbuffer[0]=ppu_.banks[
		(sp8x16) ? 0 : (ppu_.ctrl&SFC_PPU2000_SpTabl ? 4 : 0)];
	sppbuffer[1] = sp8x16 ? ppu_.banks[4] : sppbuffer[0] + 16;
	
	//遍历所有精灵
	for (int index = 0; index != SFC_SPRITE_COUNT; ++index) {
		const auto* const base = ppu_.sprites + (SFC_SPRITE_COUNT - 1 - index) * 4;
		const auto yyyy = base[0];
		if (yyyy >= (uint8_t)0xef) {
			continue;
		}
		const auto iiii = base[1];
		const auto aaaa = base[2];
		const auto xxxx = base[3];
		const auto high = ((aaaa & 3) | 4) << 3;

		const auto* const nowp0 = sppbuffer[iiii & 1] + (iiii&(uint8_t)0xfe) * 16;
		const auto* const nowp1 = nowp0 + 8;
		auto* const write = buffer + xxxx + (yyyy + 1)*SFC_WIDTH;

		//hVHP
		switch (((uint8_t)(aaaa >> 5) | sp8x16)&(uint8_t)0x0f) {
		case 0x8:
			//1000 : 8x16前
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_on(nowp0[j + 16], nowp1[j + 16], high, write + SFC_WIDTH * (j + 8));
			sfc_fallthrough;
		case 0x0:
			// 0000: 前
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_on(nowp0[j], nowp1[j], high, write + SFC_WIDTH * j);
			break;
		case 0x9:
			// 1001: 8x16 后
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_op(nowp0[j + 16], nowp1[j + 16], high, write + SFC_WIDTH * (j + 8));
			sfc_fallthrough;
		case 0x1:
			// 0001: 后
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_op(nowp0[j], nowp1[j], high, write + SFC_WIDTH * j);
			break;
		case 0xA:
			// 1010: 8x16 水平翻转 前 
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_rn(nowp0[j + 16], nowp1[j + 16], high, write + SFC_WIDTH * (j + 8));
			sfc_fallthrough;
		case 0x2:
			// 0010: 水平翻转 前 
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_rn(nowp0[j], nowp1[j], high, write + SFC_WIDTH * j);
			break;
		case 0xB:
			// 1011: 8x16 水平翻转 后
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_rp(nowp0[j + 16], nowp1[j + 16], high, write + SFC_WIDTH * (j + 8));
			sfc_fallthrough;
		case 0x3:
			// 0011: 水平翻转 后
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_rp(nowp0[j], nowp1[j], high, write + SFC_WIDTH * j);
			break;
		case 0xC:
			// 1100: 8x16 垂直翻转 前
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_on(nowp0[j + 16], nowp1[j + 16], high, write + SFC_WIDTH * (7 - j));
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_on(nowp0[j], nowp1[j], high, write + SFC_WIDTH * (15 - j));
			break;
		case 0x4:
			// 0100: 垂直翻转 前 
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_on(nowp0[j], nowp1[j], high, write + SFC_WIDTH * (7 - j));
			break;
		case 0xD:
			// 1101: 8x16 垂直翻转 后
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_op(nowp0[j + 16], nowp1[j + 16], high, write + SFC_WIDTH * (7 - j));
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_op(nowp0[j], nowp1[j], high, write + SFC_WIDTH * (15 - j));
			break;
		case 0x5:
			// 0101: 垂直翻转 后
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_op(nowp0[j], nowp1[j], high, write + SFC_WIDTH * (7 - j));
			break;
		case 0xE:
			// 1110: 8x16 垂直翻转 水平翻转 前 
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_rn(nowp0[j + 16], nowp1[j + 16], high, write + SFC_WIDTH * (7 - j));
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_rn(nowp0[j], nowp1[j], high, write + SFC_WIDTH * (15 - j));
			break;
		case 0x6:
			// 0110: 8x16 垂直翻转 水平翻转 前 
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_rn(nowp0[j], nowp1[j], high, write + SFC_WIDTH * (7 - j));
			break;
		case 0xF:
			// 1111: 8x16 垂直翻转 水平翻转 后
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_rp(nowp0[j + 16], nowp1[j + 16], high, write + SFC_WIDTH * (7 - j));
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_rp(nowp0[j], nowp1[j], high, write + SFC_WIDTH * (15 - j));
			break;
		case 0x7:
			// 0111: 垂直翻转 水平翻转 后
			for (int j = 0; j != 8; ++j)
				sfc_sprite_expand_8_rp(nowp0[j], nowp1[j], high, write + SFC_WIDTH * (7 - j));
			break;
		}
	}
}

void sfc_famicom_t::sfc_render_frame_easy(uint8_t * buffer){
	enum{SCAN_LINE_COUNT=SFC_HEIGHT};
	auto* const data = buffer;
	const auto vblank_line = config_.get_vb_s();
	const uint32_t per_scanline = config_.get_m_cps();
	uint32_t end_cycle_count = 0;

	//精灵0用命中测试缓存
	//sprite zero hits
	/*
	貌似这个只是用来告诉cpu ppu绘制的图片已经进行到的位置
	The PPU detects this condition and sets bit 6 of PPUSTATUS ($2002) to 1 starting at this pixel, letting the CPU know how far along the PPU is in drawing the picture.
	https://wiki.nesdev.com/w/index.php?title=PPU_OAM&redirect=no#Sprite_zero_hits
	The PPU sets bit 6 of PPUSTATUS if an opaque pixel of the sprite at OAM index 0 intersects an opaque background pixel. 
	It sets this flag as soon as it draws the intersecting pixel, so by putting sprite 0 at a clever location, a game can detect when the PPU has drawn a part of the picture.
	https://retrocomputing.stackexchange.com/questions/1898/how-can-i-create-a-split-scroll-effect-in-an-nes-game
	*/
	uint8_t sp0_hittest_buffer[SFC_WIDTH];
	sfc_sprite0_hittest(sp0_hittest_buffer);
	//精灵溢出测试
	//返回的时候溢出的行(貌似)
	const auto sp_overflow_line = sfc_sprite_overflow_test();
	//关闭渲染则输出背景色?
	if (!(ppu_.mask&(uint8_t)SFC_PPU2001_Back)) {
		memset(buffer, 0, SFC_WIDTH*SFC_HEIGHT);
	}
	//渲染 以扫描线为单位
	//SCAN_LINE_COUNT=240,一共240条扫描线
	{
		for (uint16_t i = 0; i != (uint16_t)SCAN_LINE_COUNT; ++i) {
			end_cycle_count += per_scanline;

			//end_cycle_count_this_round
			//同步cycle_count 如果cycle_count超过end_this_round则不再执行
			//即每条扫描线最多执行113个cpu周期
			//对应下面的 - 执行CPU部分 -
			const uint32_t end_cycle_count_this_round = end_cycle_count / MASTER_CYCLE_PER_CPU;
			auto* const count = &cpu_.cpu_cycle_count;
			// 渲染背景
			sfc_render_background_scanline(i, sp0_hittest_buffer, buffer);
			// 溢出测试
			if (i == sp_overflow_line)
				ppu_.status |= (uint8_t)SFC_PPU2002_SpOver;
			// 执行CPU
			while(*count < end_cycle_count_this_round) {
				//sfc_before_execute();
				cpu_.sfc_cpu_execute_one();
			}
			//printf("%d,%d,%d - 1", end_cycle_count, per_scanline, end_cycle_count_this_round);


			buffer += SFC_WIDTH;
			// 执行HBlank
		}
	}
	// 渲染精灵
	//240行结束后一次性渲染所有精灵
	if (ppu_.mask & (uint8_t)SFC_PPU2001_Sprite) {
		sfc_render_sprites(data);
	}

	// 让CPU执行一段时间
	// 后渲染
	{
		end_cycle_count += per_scanline;
		const uint32_t end_cycle_count_this_round = end_cycle_count / MASTER_CYCLE_PER_CPU;
		uint32_t* const count = &cpu_.cpu_cycle_count;
		while(*count < end_cycle_count_this_round) {
			//sfc_before_execute();
			cpu_.sfc_cpu_execute_one();
		}
		//printf("%d,%d,%d - 2", end_cycle_count, per_scanline, end_cycle_count_this_round);
	}
	//大部分都渲染完了
	//进入垂直空白期间

	// 开始
	// VBlank开始标志 或运算让status知道VBlank开始了
	ppu_.status |= (uint8_t)SFC_PPU2002_VBlank;
	if (ppu_.ctrl & (uint8_t)SFC_PPU2000_NMIGen) {
		cpu_.sfc_operation_NMI();
	}
	// CPU执行一段时间
	for (uint16_t i = 0; i != vblank_line; ++i) {
		end_cycle_count += per_scanline;
		const uint32_t end_cycle_count_this_round = end_cycle_count / MASTER_CYCLE_PER_CPU;
		uint32_t* const count = &cpu_.cpu_cycle_count;
		for (; *count < end_cycle_count_this_round;) {
			//sfc_before_execute();
			cpu_.sfc_cpu_execute_one();
		}
		//printf("%d,%d,%d - 3", end_cycle_count, per_scanline, end_cycle_count_this_round);
	}
	// 结束
	ppu_.status = 0;
	// 垂直滚动仅对下帧有效
	ppu_.now_scrolly = ppu_.scroll[1];

	// 预渲染
	end_cycle_count += per_scanline * 2;
	// 最后一次保证是偶数(DMA使用)
	const uint32_t end_cycle_count_last_round =
		(uint32_t)(end_cycle_count / MASTER_CYCLE_PER_CPU) & ~(uint32_t)1;
	{
		uint32_t* const count = &cpu_.cpu_cycle_count;
		for (; *count < end_cycle_count_last_round;) {
			//sfc_before_execute();
			cpu_.sfc_cpu_execute_one();
		}
		//printf("%d,%d,%d - 4", end_cycle_count, per_scanline);
	}

	// 重置计数器(32位整数太短了)
	cpu_.cpu_cycle_count -= end_cycle_count_last_round;
}




