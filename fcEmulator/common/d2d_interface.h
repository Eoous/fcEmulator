#pragma once
#include "../sfc_famicom_t.h"
#ifndef SFC_NOEXCEPT
#ifdef __cplusplus
#define SFC_EXTERN_C extern "C" 
#define SFC_NOEXCEPT noexcept
#else
#define SFC_EXTERN_C extern
#define SFC_NOEXCEPT
#endif
#endif

SFC_EXTERN_C void main_cpp(sfc_famicom_t& famicom) SFC_NOEXCEPT;
SFC_EXTERN_C void main_render(void* rgba,sfc_famicom_t& famicom) SFC_NOEXCEPT;
SFC_EXTERN_C void user_input(int index, unsigned char data) SFC_NOEXCEPT;
SFC_EXTERN_C int sub_render(void* rgba) SFC_NOEXCEPT;
SFC_EXTERN_C void qsave() SFC_NOEXCEPT;
SFC_EXTERN_C void qload() SFC_NOEXCEPT;