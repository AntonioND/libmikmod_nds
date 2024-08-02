#include <nds.h>
#include "drv_nds_hw7.h"

static NDS_HW_IPC* ipc = NULL;

void NDS_HW7_Init(NDS_HW_IPC* _ipc)
{
	ipc = _ipc;
}

void NDS_HW7_Exit(void)
{
	ipc = NULL;
}

void NDS_HW7_PlayStart(void)
{
}

void NDS_HW7_PlayStop(void)
{
	int i;
	for(i = 0; i < NDS_HW_MAXVOICES; i++) {
		SCHANNEL_CR(i) = 0;
	}
}

void NDS_HW7_Reset(void)
{
}

void NDS_HW7_VoiceUpdate(u8 voice, u8 changes)
{
	NDS_HW_VOICE* v = ipc->voices + voice;

	if(changes & NDS_HW_CHANGE_VOLUME) {
		// seems v->volume ranges from 0-256 (not 0-255)
		// nds wants 0-127
		SCHANNEL_VOL(voice) = ((v->volume<255) ? v->volume >> 1 : 127);
	}

	if(changes & NDS_HW_CHANGE_PANNING) {
		SCHANNEL_PAN(voice) = v->panning >> 1;
	}

	if(changes & NDS_HW_CHANGE_FREQUENCY) {
		SCHANNEL_TIMER(voice) = SOUND_FREQ((s32)v->frequency);
	}
	
	if(changes & NDS_HW_CHANGE_STOP) {
		SCHANNEL_CR(voice) = 0;
	}

	if(changes & NDS_HW_CHANGE_START) {	
		// nds sound hardware wants number of 32-bit words
		// there are two 16-bit samples or four 8-bit samples per word
		int shift = (v->flags & SF_16BITS) ? 1 : 2;
		
		// set start
		SCHANNEL_SOURCE(voice) = ((u32)ipc->samples[v->handle]) + (v->start << shift);
		
		// set end and repeat point
		if(v->flags & SF_LOOP) {
			SCHANNEL_LENGTH(voice) = (v->loopend - v->loopstart) >> shift;
			SCHANNEL_REPEAT_POINT(voice) = (v->loopstart - v->start) >> shift;
		} else {
			SCHANNEL_LENGTH(voice) = (v->length - v->start) >> shift;
			SCHANNEL_REPEAT_POINT(voice) = 0;
		}
		
		// stop old sound
		SCHANNEL_CR(voice) = 0;
		
		// start sound
		SCHANNEL_CR(voice) =
			SOUND_VOL((v->volume<255) ? v->volume >> 1 : 127) |
			SOUND_PAN(v->panning >> 1) |
			((v->flags & SF_LOOP) ? SOUND_REPEAT : SOUND_ONE_SHOT) |
			((v->flags & SF_16BITS) ? SOUND_FORMAT_16BIT : SOUND_FORMAT_8BIT) |
			SCHANNEL_ENABLE;
	}
}
