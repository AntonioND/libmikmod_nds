/*	MikMod sound library
	(c) 1998, 1999, 2000 Miodrag Vallat and others - see file AUTHORS for
	complete list.

	This library is free software; you can redistribute it and/or modify
	it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of
	the License, or (at your option) any later version.
 
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Library General Public License for more details.
 
	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA.
*/

/*==============================================================================

  $Id: drv_raw.c,v 1.2 2004/01/21 17:29:32 raph Exp $

  Driver for output on a Nintendo DS

==============================================================================*/

#include <unistd.h>
#include <string.h>

#include "mikmod_internals.h"

#include <nds.h>
#include "mikmod_drv_nds.h"
#include "mikmod9.h"

#define BUFFERSIZE 32768

static struct NDS_SW_IPC* ipc = NULL;

static void NDS_SW_CommandLine(CHAR *cmdline)
{
}

static BOOL NDS_SW_IsThere(void)
{
	return 1;
}

static BOOL NDS_SW_Init(void)
{
	md_mode|=DMODE_SOFT_MUSIC|DMODE_SOFT_SNDFX;
	md_mode &= ~DMODE_STEREO;

	ipc = (NDS_SW_IPC*)_mm_malloc(sizeof(NDS_SW_IPC));
	if (ipc == NULL) {
		MikMod_errno = MMERR_OUT_OF_MEMORY;
		return 1;
	}

	ipc->buffer = (SBYTE*)_mm_malloc(BUFFERSIZE);
	if (ipc->buffer == NULL) {
		_mm_free(ipc);
		ipc = NULL;
		MikMod_errno = MMERR_OUT_OF_MEMORY;
		return 1;
	}

	if (VC_Init()) {
		return 1;
	}

	ipc->bufferSize = BUFFERSIZE;
	ipc->sampleRate = md_mixfreq;
	ipc->format = (md_mode & DMODE_16BITS) ? 16 : 8;

	MikMod9_SendCommand(NDS_SW_CMD_INIT << 28 | (u32)ipc);
	
	return 0;
}

static void NDS_SW_Exit(void)
{
	MikMod9_SendCommand(NDS_SW_CMD_EXIT << 28);
	VC_Exit();
	_mm_free(ipc->buffer);
	ipc->buffer = NULL;
	_mm_free(ipc);
	ipc = NULL;
}

static void unsignedtosigned(void* data, size_t length)
{
	if(ipc->format == 16)
		return;

	UBYTE* ptr = (UBYTE*)data;
	int i;
	for(i = 0; i < length; i++)
	{
		*ptr++ ^= 0x80;
	}
}

static void NDS_SW_Update(void)
{
	DC_InvalidateRange(&ipc->readCursor, sizeof(ipc->readCursor));
	while(ipc->readCursor != ipc->writeCursor)
	{
		int todo = ipc->readCursor - ipc->writeCursor;
		if(todo < 0)
			todo = ipc->bufferSize - ipc->writeCursor;

		int written = VC_WriteBytes(ipc->buffer + ipc->writeCursor, todo);
		unsignedtosigned(ipc->buffer + ipc->writeCursor, written);
		
		int newCursor = ipc->writeCursor + written;
		if(ipc->writeCursor >= ipc->bufferSize)
			newCursor -= ipc->bufferSize;
		ipc->writeCursor = newCursor;
	}
	DC_FlushRange(&ipc->writeCursor, sizeof(ipc->writeCursor));
	
	// we send this afterwards to give arm7 as much time as possible
	// to update
	MikMod9_SendCommand(NDS_SW_CMD_UPDATE << 28);
}

static BOOL NDS_SW_Reset(void)
{
	return 0;
}

static BOOL NDS_SW_PlayStart(void)
{
	if(VC_PlayStart())
		return 1;
	
	Player_SetPosition(0);
	
	ipc->prevTimer = 0;
	ipc->writeCursor = 0;
	ipc->readCursor = 0;

	int written = VC_WriteBytes(ipc->buffer, ipc->bufferSize);
	unsignedtosigned(ipc->buffer, written);

	DC_FlushRange(ipc, sizeof(*ipc));
	MikMod9_SendCommand(NDS_SW_CMD_START << 28);
	
	return 0;
}

static void NDS_SW_PlayStop(void)
{
	VC_PlayStop();
	MikMod9_SendCommand(NDS_SW_CMD_STOP << 28);
}

MIKMODAPI MDRIVER drv_nds_sw={
	NULL,
	"NDS SW driver",
	"Nintendo DS Software driver v1.0",
	0,255,
	"nds_sw",
	NULL,
	NDS_SW_CommandLine,
	NDS_SW_IsThere,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	NDS_SW_Init,
	NDS_SW_Exit,
	NDS_SW_Reset,
	VC_SetNumVoices,
	NDS_SW_PlayStart,
	NDS_SW_PlayStop,
	NDS_SW_Update,
	NULL,
	VC_VoiceSetVolume,
	VC_VoiceGetVolume,
	VC_VoiceSetFrequency,
	VC_VoiceGetFrequency,
	VC_VoiceSetPanning,
	VC_VoiceGetPanning,
	VC_VoicePlay,
	VC_VoiceStop,
	VC_VoiceStopped,
	VC_VoiceGetPosition,
	VC_VoiceRealVolume
};

/* ex:set ts=4: */
