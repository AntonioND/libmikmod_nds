#include <nds.h>
#include "drv_nds_sw7.h"
#include "drv_nds_hw7.h"

void MikMod7_ProcessCommand(u32 command)
{
	switch(command>>28)
	{
	case NDS_SW_CMD_INIT:
		NDS_SW7_Init((NDS_SW_IPC*)(command & 0xFFFFFFF));
		break;
		
	case NDS_SW_CMD_START:
		NDS_SW7_Start();
		break;
	
	case NDS_SW_CMD_UPDATE:
		NDS_SW7_Update();
		break;
		
	case NDS_SW_CMD_STOP:
		NDS_SW7_Stop();
		break;
		
	case NDS_SW_CMD_EXIT:
		NDS_SW7_Exit();
		break;
		
	case NDS_HW_CMD_INIT:
		NDS_HW7_Init((NDS_HW_IPC*)(command & 0xFFFFFFF));
		break;

	case NDS_HW_CMD_EXIT:
		NDS_HW7_Exit();
		break;

	case NDS_HW_CMD_PLAY_START:
		NDS_HW7_PlayStart();
		break;

	case NDS_HW_CMD_PLAY_STOP:
		NDS_HW7_PlayStop();
		break;

	case NDS_HW_CMD_RESET:
		NDS_HW7_Reset();
		break;

	case NDS_HW_CMD_VOICE_UPDATE:
		NDS_HW7_VoiceUpdate(command & 0xFF, (command >> 8) & 0xFF);
		break;

	default:
		break;
	}
}
