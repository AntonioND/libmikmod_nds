#pragma once

#include <mikmod7.h>

void NDS_HW7_Init(NDS_HW_IPC* ipc);
void NDS_HW7_Exit(void);
void NDS_HW7_PlayStart(void);
void NDS_HW7_PlayStop(void);
void NDS_HW7_Reset(void);
void NDS_HW7_VoiceUpdate(u8 voice, u8 changes);
