#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mikmod.h>

typedef struct MMEMORYREADER {
	MREADER core;
	const UBYTE* buffer;
	size_t size;
	const UBYTE* cursor;
} MMEMORYREADER;

static BOOL MemoryReader_Eof(MREADER* reader)
{
	MMEMORYREADER* mreader = (MMEMORYREADER*)reader;
	return mreader->cursor >= mreader->buffer + mreader->size;
}

static BOOL MemoryReader_Read(MREADER* reader, void* ptr, size_t size)
{
	MMEMORYREADER* mreader = (MMEMORYREADER*)reader;
	if(mreader->cursor + size >= mreader->buffer + mreader->size)
		return 0;

	memcpy(ptr, mreader->cursor, size);
	mreader->cursor += size;

	return 1;
}

static int MemoryReader_Get(MREADER* reader)
{
	MMEMORYREADER* mreader = (MMEMORYREADER*)reader;
	if(mreader->cursor >= mreader->buffer + mreader->size)
		return EOF;

	return *mreader->cursor++;
}

static BOOL MemoryReader_Seek(MREADER* reader, long offset, int whence)
{
	MMEMORYREADER* mreader = (MMEMORYREADER*)reader;
	const UBYTE* newcursor = 0;

	switch(whence)
	{
	case SEEK_SET:
		newcursor = mreader->buffer + offset;
		break;

	case SEEK_CUR:
		newcursor = mreader->cursor + offset;
		break;

	case SEEK_END:
		newcursor = mreader->buffer + mreader->size + offset;
		break;
	}

	if(newcursor < mreader->buffer)
		return -1;

	if(newcursor >= mreader->buffer + mreader->size)
		return -1;

	mreader->cursor = newcursor;

	return 0;
}

static long MemoryReader_Tell(MREADER* reader)
{
	MMEMORYREADER* mreader = (MMEMORYREADER*)reader;
	return mreader->cursor - mreader->buffer;
}

MIKMODAPI extern MODULE* Player_LoadMemory(const void* buffer, size_t size, int maxchan, BOOL curious)
{
	MODULE* module = 0;

	MMEMORYREADER* reader=(MMEMORYREADER*)malloc(sizeof(MMEMORYREADER));
	if (reader) {
		reader->core.Eof  = &MemoryReader_Eof;
		reader->core.Read = &MemoryReader_Read;
		reader->core.Get  = &MemoryReader_Get;
		reader->core.Seek = &MemoryReader_Seek;
		reader->core.Tell = &MemoryReader_Tell;
		reader->buffer = (const UBYTE*)buffer;
		reader->size = size;

		module = Player_LoadGeneric((MREADER*)reader, maxchan, curious);
		free(reader);
	}

	return module;
}
