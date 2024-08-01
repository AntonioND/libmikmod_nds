#include "nds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mikmod9.h>

#include "module_bin.h"

// called by the drivers in mikmod library
void MikMod9_SendCommand(u32 command)
{
	while (REG_IPC_FIFO_CR & IPC_FIFO_SEND_FULL);
	REG_IPC_FIFO_TX = command;
}

void TimerInterrupt()
{
	// player tick
	MikMod_Update();
	
	// the bpm can change in the middle of the song
	TIMER0_DATA = TIMER_FREQ_256(md_bpm * 50 / 125);
}

int main(void) {
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR;

	consoleDemoInit();
	irqInit();
	irqEnable(IRQ_VBLANK); // needed for swiWaitForVBlank()

	// register hardware or software mixer
	MikMod_RegisterDriver(&drv_nds_hw);
	//MikMod_RegisterDriver(&drv_nds_sw);

	// if we don't know what kind of module we're going to load we can register
	// all loaders, but that will result in a larger binary
	MikMod_RegisterAllLoaders();
	//MikMod_RegisterLoader(&load_mod);

	printf("Initializing library\n");
	if (MikMod_Init("")) {
		printf("Could not initialize sound, reason: \n%s\n",
			MikMod_strerror(MikMod_errno));
		return 1;
	}

	printf("\nLoading module\n");
	// Player_LoadMemory() loads a module directly from memory
	// it could be possible to use Player_Load() to load from FAT,
	// but I've never tried this
	MODULE* module = Player_LoadMemory(module_bin, module_bin_size, 64, 0);
	if (module) {
		printf("Title:    %s\n", module->songname);
		printf("Channels: %u\n", module->numchn);

		printf("\nStarting module\n");
		Player_Start(module);
		
		// call update with correct timing
		TIMER0_CR = 0;
		irqSet(IRQ_TIMER0, TimerInterrupt);
		irqEnable(IRQ_TIMER0);
		TIMER0_DATA = TIMER_FREQ_256(md_bpm * 50 / 125);
		TIMER0_CR = TIMER_DIV_256 | TIMER_IRQ_REQ | TIMER_ENABLE;

		// save cursor position
		printf("\e[s");

		while (Player_Active()) {
			// when using the software driver we could call update
			// here instead
			//MikMod_Update();
			printf(
				"Time: %u:%02u:%02u\e[u",
				module->sngtime/60000,
				module->sngtime/1000%60,
				module->sngtime/10%100);
			swiWaitForVBlank();
		}

		printf("\nStopping module\n");
		Player_Stop();
		Player_Free(module);
	} else {
		printf("Could not load module, reason: \n%s\n",
			MikMod_strerror(MikMod_errno));
	}

	printf("\nExit library\n");
	MikMod_Exit();
	return 0;
}
