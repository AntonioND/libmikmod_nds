#pragma once

#include <mikmod.h>
#include <mikmod_drv_nds.h>
#include <mikmod_memoryloader.h>

#ifdef __cplusplus
extern "C" {
#endif

extern u16 md_bpm;
void MikMod9_SendCommand(u32 command);

#ifdef __cplusplus
}
#endif
