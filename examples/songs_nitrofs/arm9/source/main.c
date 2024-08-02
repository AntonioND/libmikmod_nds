// SPDX-License-Identifier: CC0-1.0
//
// SPDX-FileContributor: Antonio Niño Díaz, 2024

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>
#include <filesystem.h>

#include <mikmod9.h>

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

    if (!nitroFSInit(NULL))
    {
        printf("Failed to initialize NitroFS\n");
        wait_forever();
    }

    soundEnable();

    printf("Select mixer:\n");
    printf("\n");
    printf("A: Hardware mixer\n");
    printf("B: Software mixer\n");
    printf("\n");

    bool software_mixer;

    while (1)
    {
        swiWaitForVBlank();

        scanKeys();
        uint32_t keys_down = keysDown();
        if (keys_down & KEY_A)
        {
            MikMod_RegisterDriver(&drv_nds_hw);
            software_mixer = false;
            break;
        }
        else if (keys_down & KEY_B)
        {
            MikMod_RegisterDriver(&drv_nds_sw);
            software_mixer = true;
            break;
        }
    }

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

    printf("\n");
    printf("Loading module from NitroFS\n");

    MODULE *module = Player_Load("module.xm", 64, 0);
    if (module == NULL)
    {
        printf("Could not load module, reason: \n%s\n",
               MikMod_strerror(MikMod_errno));
        wait_forever();
    }

    // Ensure that all the memory used by the module is visible by the ARM7
    DC_FlushAll();

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

        // When using the software mixer it is needed to call this once per
        // frame to mix audio.
        if (software_mixer)
            MikMod_Update();

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
