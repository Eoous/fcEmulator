#pragma once
#include "../Famicom/Famicom.h"
#ifndef SFC_NOEXCEPT
#ifdef __cplusplus
#define SFC_EXTERN_C extern "C" 
#define SFC_NOEXCEPT noexcept
#else
#define SFC_EXTERN_C extern
#define SFC_NOEXCEPT
#endif
#endif

SFC_EXTERN_C void main_cpp(Famicom& famicom) SFC_NOEXCEPT;
SFC_EXTERN_C void main_render(void* rgba,Famicom& famicom) SFC_NOEXCEPT;
SFC_EXTERN_C void user_input(int index, unsigned char data) SFC_NOEXCEPT;
SFC_EXTERN_C int sub_render(void* rgba) SFC_NOEXCEPT;
SFC_EXTERN_C void qsave() SFC_NOEXCEPT;
SFC_EXTERN_C void qload() SFC_NOEXCEPT;