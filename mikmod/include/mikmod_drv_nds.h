#pragma once

typedef struct NDS_SW_IPC
{
    s8 *buffer;
	int prevTimer;
    int writeCursor;
	int readCursor;
	int bufferSize;		// in bytes
	int sampleRate;
	int format;			// 8 or 16
	int playing;
} NDS_SW_IPC;

#define NDS_HW_MAXSAMPLES 128
#define NDS_HW_MAXVOICES 16

#define NDS_HW_CHANGE_START     (1<<0)
#define NDS_HW_CHANGE_STOP      (1<<1)
#define NDS_HW_CHANGE_VOLUME    (1<<2)
#define NDS_HW_CHANGE_FREQUENCY (1<<3)
#define NDS_HW_CHANGE_PANNING   (1<<4)

typedef struct NDS_HW_VOICE
{
	s16 handle;
	u32 start;
	u32 length;
	u32 loopstart;
	u32 loopend;
	u16 flags;
	u16 volume;
	u32 frequency;
	u32 panning;
	int playing;
	u8 changes;
} NDS_HW_VOICE;

typedef struct NDS_HW_IPC
{
	void* samples[NDS_HW_MAXSAMPLES];
	NDS_HW_VOICE voices[NDS_HW_MAXVOICES];
} NDS_HW_IPC;

enum NDS_COMMAND
{
	NDS_SW_CMD_INIT = 1,
	NDS_SW_CMD_START,
	NDS_SW_CMD_UPDATE,
	NDS_SW_CMD_STOP,
	NDS_SW_CMD_EXIT,
	NDS_HW_CMD_INIT,
	NDS_HW_CMD_EXIT,
	NDS_HW_CMD_PLAY_START,
	NDS_HW_CMD_PLAY_STOP,
	NDS_HW_CMD_RESET,
	NDS_HW_CMD_VOICE_UPDATE
};
