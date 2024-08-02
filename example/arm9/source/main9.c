#include <nds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mikmod9.h>

#include "module_bin.h"

// Assign FIFO_USER_07 channel to libmikmod
#define FIFO_LIBMIKMOD (FIFO_USER_07)

// Called by the drivers in mikmod library
void MikMod9_SendCommand(u32 command)
{
    fifoSendValue32(FIFO_LIBMIKMOD, command);
}

void TimerInterrupt(void)
{
    // Player tick
    MikMod_Update();

    // Update BPM in case it has changed in the middle of the song
    TIMER0_DATA = TIMER_FREQ_256(md_bpm * 50 / 125);
}

void wait_forever(void)
{
    printf("\n");
    printf("Press START to exit to loader\n");

    while (1)
    {
        swiWaitForVBlank();

        scanKeys();
        uint32_t keys_down = keysDown();
        if (keys_down & KEY_START)
            exit(0);
    }
}

int main(int argc, char *argv[])
{
    consoleDemoInit();

    soundEnable();

    // Register hardware or software mixer
    MikMod_RegisterDriver(&drv_nds_hw);
    // MikMod_RegisterDriver(&drv_nds_sw);

    // If we don't know what kind of module we're going to load we can register
    // all loaders, but that will result in a larger binary.
    MikMod_RegisterAllLoaders();
    // This will only load the loader of MOD files instead.
    //MikMod_RegisterLoader(&load_mod);

    printf("Initializing library\n");
    if (MikMod_Init(""))
    {
        printf("Could not initialize sound, reason: \n%s\n",
               MikMod_strerror(MikMod_errno));
        wait_forever();
    }

    printf("\nLoading module\n");

    // Player_LoadMemory() loads a module directly from memory it could be
    // possible to use Player_Load() to load from FAT or NitroFS.
    MODULE *module = Player_LoadMemory(module_bin, module_bin_size, 64, 0);
    if (module == NULL)
    {
        printf("Could not load module, reason: \n%s\n",
               MikMod_strerror(MikMod_errno));
        wait_forever();
    }

    printf("Title:    %s\n", module->songname);
    printf("Channels: %u\n", module->numchn);

    printf("\n");
    printf("Starting module\n");

    Player_Start(module);

    printf("\n");
    printf("Press B to stop\n");

    // Call update with correct timing
    timerStart(0, ClockDivider_256, TIMER_FREQ_256(md_bpm * 50 / 125),
               TimerInterrupt);

    // Save cursor position
    printf("\e[s");

    while (Player_Active())
    {
        swiWaitForVBlank();

        scanKeys();
        uint32_t keys_down = keysDown();
        if (keys_down & KEY_B)
            break;

        // When using the software driver we could call update here instead.
        // When using the hardware driver there is no need to call anything.
        //MikMod_Update();

        printf("Time: %lu:%02lu:%02lu\e[u", module->sngtime / 60000,
                module->sngtime / 1000 % 60, module->sngtime / 10 % 100);
    }

    printf("\nStopping module\n");
    Player_Stop();
    Player_Free(module);

    printf("\nExit library\n");
    MikMod_Exit();

    soundDisable();

    wait_forever();

    return 0;
}
