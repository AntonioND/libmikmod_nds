/*
    MikMod sound library

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

  Driver for output on a Nintendo DS

==============================================================================*/

#include <unistd.h>
#include <string.h>

#include "mikmod_internals.h"

#include <nds.h>
#include "mikmod9.h"

#define ASSERT(x) if(!(x)) {\
    printf("assertion failed %s:%i\n", __FILE__, __LINE__);\
    for(;;);\
}

static NDS_HW_IPC* ipc = NULL;

static void NDS_HW_CommandLine(const CHAR *cmdline)
{
    (void)cmdline;
}

static BOOL NDS_HW_IsPresent(void)
{
    return 1;
}

static SWORD NDS_HW_SampleLoad(struct SAMPLOAD* sload, int type)
{
    ASSERT(sload != NULL);

    (void)type;

    SAMPLE *s = sload->sample;
    int handle;

    /* Find empty slot to put sample address in */
    for (handle = 0; handle < NDS_HW_MAXSAMPLES; handle++)
    {
        if (ipc->samples[handle] == 0)
            break;
    }

    if (handle == MAXSAMPLEHANDLES)
    {
        _mm_errno = MMERR_OUT_OF_HANDLES;
        return -1;
    }

    /* Reality check for loop settings */
    if (s->loopend > s->length)
        s->loopend = s->length;
    if (s->loopstart >= s->loopend)
        s->flags &= ~SF_LOOP;

    SL_SampleSigned(sload);

    ipc->samples[handle] = MikMod_malloc(s->length << ((s->flags & SF_16BITS) ? 1 : 0));
    if (ipc->samples[handle] == NULL)
    {
        _mm_errno = MMERR_SAMPLE_TOO_BIG;
        return -1;
    }

    /* read sample into buffer */
    if (SL_Load(ipc->samples[handle], sload, s->length))
        return -1;

    DC_FlushRange(ipc->samples[handle], s->length << ((s->flags & SF_16BITS) ? 1 : 0));

    return handle;
}

static void NDS_HW_SampleUnload(SWORD handle)
{
    ASSERT(handle >= 0);
    ASSERT(handle < NDS_HW_MAXSAMPLES);

    MikMod_free(ipc->samples[handle]);
    ipc->samples[handle] = NULL;
}

static ULONG NDS_HW_FreeSampleSpace(int type)
{
    (void)type;

    /* TODO what is this supposed to do? */
    return 0;
}

static ULONG NDS_HW_RealSampleLength(int type, SAMPLE* s)
{
    ASSERT(s != NULL);

    (void)type;

    /* TODO why +16? */
    return s->length * ((s->flags & SF_16BITS) ? 2 : 1) + 16;
}

static BOOL NDS_HW_Init(void)
{
    ipc = (NDS_HW_IPC*)MikMod_malloc(sizeof(NDS_HW_IPC));
    if (ipc == NULL)
        return 1;

    memset(ipc, 0, sizeof(NDS_HW_IPC));

    MikMod9_SendCommand((NDS_HW_CMD_INIT << 28) | (u32)ipc);

    return 0;
}

static void NDS_HW_Exit(void)
{
    MikMod9_SendCommand(NDS_HW_CMD_EXIT << 28);
    MikMod_free(ipc);
    ipc = NULL;
}

static void NDS_HW_Update(void)
{
    /* should be called md_bpm*0.4 times per second */
    DC_FlushRange(ipc, sizeof(*ipc));
    for (int i = 0; i < NDS_HW_MAXVOICES; i++)
    {
        NDS_HW_VOICE* v = ipc->voices + i;
        if (v->changes != 0)
        {
            MikMod9_SendCommand(
                (NDS_HW_CMD_VOICE_UPDATE << 28) |
                (v->changes << 8) |
                i);
            v->changes = 0;
        }
    }
    md_player();
}

static BOOL NDS_HW_Reset(void)
{
    /* TODO what do we need to do here? */
    MikMod9_SendCommand(NDS_HW_CMD_RESET << 28);
    return 0;
}

static BOOL NDS_HW_SetNumVoices(void)
{
    return 0;
}

static BOOL NDS_HW_PlayStart(void)
{
    Player_SetPosition(0);
    MikMod9_SendCommand(NDS_HW_CMD_PLAY_START << 28);
    return 0;
}

static void NDS_HW_PlayStop(void)
{
    for (int i = 0; i < NDS_HW_MAXVOICES; i++)
        ipc->voices[i].playing = false;

    MikMod9_SendCommand(NDS_HW_CMD_PLAY_STOP << 28);
}

static void NDS_HW_VoiceSetVolume(UBYTE voice, UWORD volume)
{
    ASSERT(voice < NDS_HW_MAXVOICES);

    ipc->voices[voice].volume = volume;
    ipc->voices[voice].changes |= NDS_HW_CHANGE_VOLUME;
}

static UWORD NDS_HW_VoiceGetVolume(UBYTE voice)
{
    ASSERT(voice < NDS_HW_MAXVOICES);

    return ipc->voices[voice].volume;
}

static void NDS_HW_VoiceSetFrequency(UBYTE voice, ULONG frequency)
{
    ASSERT(voice < NDS_HW_MAXVOICES);

    ipc->voices[voice].frequency = frequency;
    ipc->voices[voice].changes |= NDS_HW_CHANGE_FREQUENCY;
}

static ULONG NDS_HW_VoiceGetFrequency(UBYTE voice)
{
    ASSERT(voice < NDS_HW_MAXVOICES);

    return ipc->voices[voice].frequency;
}

static void NDS_HW_VoiceSetPanning(UBYTE voice, ULONG panning)
{
    ASSERT(voice < NDS_HW_MAXVOICES);

    ipc->voices[voice].panning = panning;
    ipc->voices[voice].changes |= NDS_HW_CHANGE_PANNING;
}

static ULONG NDS_HW_VoiceGetPanning(UBYTE voice)
{
    ASSERT(voice < NDS_HW_MAXVOICES);

    return ipc->voices[voice].panning;
}

static void NDS_HW_VoicePlay(
    UBYTE voice,
    SWORD handle,
    ULONG start,
    ULONG length,
    ULONG loopstart,
    ULONG loopend,
    UWORD flags)
{
    ASSERT(voice < NDS_HW_MAXVOICES);
    ASSERT(handle >= 0);
    ASSERT(handle < NDS_HW_MAXSAMPLES);

    NDS_HW_VOICE* v = ipc->voices + voice;
    v->handle = handle;
    v->start = start;
    v->length = length;
    v->loopstart = loopstart;
    v->loopend = loopend;
    v->flags = flags;
    v->playing = TRUE;
    v->changes |= NDS_HW_CHANGE_START;
}

static void NDS_HW_VoiceStop(UBYTE voice)
{
    ASSERT(voice < NDS_HW_MAXVOICES);

    ipc->voices[voice].playing = FALSE;
    ipc->voices[voice].changes |= NDS_HW_CHANGE_STOP;
}

static BOOL NDS_HW_VoiceStopped(UBYTE voice)
{
    ASSERT(voice < NDS_HW_MAXVOICES);

    return ipc->voices[voice].playing;
}

static SLONG NDS_HW_VoiceGetPosition(UBYTE voice)
{
    ASSERT(voice < NDS_HW_MAXVOICES);

    (void)voice;

    return -1;
}

static ULONG NDS_HW_VoiceRealVolume(UBYTE voice)
{
    ASSERT(voice < NDS_HW_MAXVOICES);

    (void)voice;

    return 0;
}

MIKMODAPI MDRIVER drv_nds_hw = {
    NULL,
    "NDS HW driver",
    "Nintendo DS Hardware driver v1.0",
    NDS_HW_MAXVOICES, NDS_HW_MAXVOICES,
    "nds_hw",
    NULL,
    NDS_HW_CommandLine,
    NDS_HW_IsPresent,
    NDS_HW_SampleLoad,
    NDS_HW_SampleUnload,
    NDS_HW_FreeSampleSpace,
    NDS_HW_RealSampleLength,
    NDS_HW_Init,
    NDS_HW_Exit,
    NDS_HW_Reset,
    NDS_HW_SetNumVoices,
    NDS_HW_PlayStart,
    NDS_HW_PlayStop,
    NDS_HW_Update,
    NULL,
    NDS_HW_VoiceSetVolume,
    NDS_HW_VoiceGetVolume,
    NDS_HW_VoiceSetFrequency,
    NDS_HW_VoiceGetFrequency,
    NDS_HW_VoiceSetPanning,
    NDS_HW_VoiceGetPanning,
    NDS_HW_VoicePlay,
    NDS_HW_VoiceStop,
    NDS_HW_VoiceStopped,
    NDS_HW_VoiceGetPosition,
    NDS_HW_VoiceRealVolume
};
