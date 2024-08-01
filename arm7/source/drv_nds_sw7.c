#include <nds.h>
#include "drv_nds_sw7.h"

static struct NDS_SW_IPC* ipc = 0;

void NDS_SW7_Init(NDS_SW_IPC* _ipc)
{
	ipc = _ipc;
}

void NDS_SW7_Start()
{
	int period = (1<<24)/ipc->sampleRate;

	TIMER0_DATA = 0x10000 - (period * 2);
	TIMER0_CR = TIMER_ENABLE | TIMER_DIV_1;

	TIMER1_DATA = 0;
	TIMER1_CR = TIMER_ENABLE | TIMER_CASCADE | TIMER_DIV_1;

	SCHANNEL_TIMER(0) = 0x10000 - period;
	SCHANNEL_SOURCE(0) = (u32)ipc->buffer;
	SCHANNEL_REPEAT_POINT(0) = 0;
	SCHANNEL_LENGTH(0) = ipc->bufferSize >> 2;
	SCHANNEL_CR(0) =
		SCHANNEL_ENABLE |
		SOUND_REPEAT |
		SOUND_VOL(127) |
		SOUND_PAN(64) |
		((ipc->format == 8) ? SOUND_8BIT : SOUND_16BIT);

	ipc->playing = true;
}

void NDS_SW7_Stop()
{
	ipc->playing = false;

	TIMER0_DATA = 0;
	TIMER0_CR = 0;
	TIMER1_DATA = 0;
	TIMER1_CR = 0;

	SCHANNEL_CR(0) = 0;
}

void NDS_SW7_Update()
{
	if(!ipc->playing)
		return;

	int time = TIMER1_DATA;
	int samples = time - ipc->prevTimer;

	if(samples < 0) samples += 65536;

	ipc->prevTimer = time;

	int newCursor = ipc->readCursor +
		samples * ((ipc->format == 8) ? 1 : 2);
	if(newCursor > ipc->bufferSize)
		newCursor -= ipc->bufferSize;

	ipc->readCursor = newCursor;
}

void NDS_SW7_Exit()
{
	NDS_SW7_Stop();
	ipc = 0;
}
