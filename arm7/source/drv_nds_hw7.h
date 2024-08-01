#pragma once

#include <mikmod7.h>

void NDS_HW7_Init(NDS_HW_IPC* ipc);
void NDS_HW7_Exit();
void NDS_HW7_PlayStart();
void NDS_HW7_PlayStop();
void NDS_HW7_Reset();
void NDS_HW7_VoiceUpdate(u8 voice, u8 changes);
