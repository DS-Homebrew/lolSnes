/*
	Copyright 2013 Mega-Mario

	This file is part of lolSnes.

	lolSnes is free software: you can redistribute it and/or modify it under
	the terms of the GNU General Public License as published by the Free
	Software Foundation, either version 3 of the License, or (at your option)
	any later version.

	lolSnes is distributed in the hope that it will be useful, but WITHOUT ANY 
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
	FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along 
	with lolSnes. If not, see http://www.gnu.org/licenses/.
*/

#include <nds.h>
#include <stdio.h>
#include <dirent.h>

#ifdef NITROFS_ROM
#include <filesystem.h>
#else
#include <fat.h>
#endif

#include "cpu.h"
#include "memory.h"
#include "ppu.h"

#include "lolsnes_screen.h"


#define VERSION "v1.1"


void doSplashscreen()
{
	int i;

	u16* pal = (u16*)0x05000000;
	for (i = 0; i < (lolsnes_screenPalLen >> 1); i++)
		pal[i] = lolsnes_screenPal[i];

	videoBgEnable(0);

	int mapblock = ((lolsnes_screenTilesLen + 0x7FF) >> 11);
	*(vu16*)0x04000008 = (mapblock << 8) | 0x0080;

	u32* tiles = (u32*)0x06000000;
	for (i = 0; i < (lolsnes_screenTilesLen >> 2); i++)
		tiles[i] = lolsnes_screenTiles[i];

	u16* map = (u16*)(0x06000000 + (mapblock << 11));
	for (i = 0; i < (lolsnes_screenMapLen >> 1); i++)
		map[i] = lolsnes_screenMap[i];

	*(vu16*)0x04000010 = 0;
	*(vu16*)0x04000012 = 0;
}


bool running = false;


void arm7print(u32 value32, void* userdata)
{
	iprintf(IPC->Dbg_String);
}


void stop() {
	while (1) {
		swiWaitForVBlank();
	}
}


void sleepMode()
{
	// turn off
	IPC->Pause |= 1;
	
	u32 powerstate = *(vu32*)0x04000304;
	*(vu32*)0x04000304 = (powerstate & 0x8001);
	
	while (IPC->Pause & 2);
	
	// turn back on
	*(vu32*)0x04000304 = powerstate;
	
	IPC->Pause &= ~1;
	while (*(vu16*)0x04000006 != 192);
}


char* filelist;
int nfiles;


bool debug_on = false;

u32 framecount = 0;

ITCM_CODE void vblank()
{
	if (IPC->Pause & 2)
		sleepMode();
		
	PPU_VBlank();
	
	// every 8 frames, check if SRAM needs to be saved
	framecount++;
	if (!(framecount & 0x7)) Mem_SaveSRAM();
}

void vblank_idle()
{
	scanKeys();
	int pressed = keysHeld();

	if ((pressed & KEY_L) && (pressed & KEY_R) && (pressed & KEY_B) && (pressed & KEY_DOWN)) {
		toggleConsole(!debug_on);
	}
}


int menusel = 0;
int menuscroll = 0;

void setMenuSel(int sel)
{
	*(vu16*)0x04001040 = 0x00FE;
	*(vu16*)0x04001044 = (sel << 11) | ((sel+1) << 3);
}

void menuPrint(int x, int y, char* str)
{
	u16* menu = (u16*)0x06201800;
	int i;

	menu += (y << 5) + x;

	for (i = 0; str[i] != '\0'; i++)
	{
		menu[i] = 0xF000 | str[i];
	}
}

void makeMenu()
{
	menuPrint(0, 0, "- lolSnes " VERSION " -                ");
	menuPrint(0, 1, "________________________________");

	int i;
	int maxfile;

	if ((nfiles - menuscroll) <= 22) maxfile = (nfiles - menuscroll);
	else maxfile = 22;

	for (i = 0; i < maxfile; i++) {
		menuPrint(0, 2+i, "                                ");
		menuPrint(2, 2+i, &filelist[(menuscroll+i) << 8]);
		if ((menuscroll+i) == menusel)
			menuPrint(0, 2+i, "\x10");
	}

	menuPrint(31, 2, "\x1E");
	menuPrint(31, 23, "\x1F");
	for (i = 3; i < 23; i++)
		menuPrint(31, i, "|");

	if ((nfiles - menuscroll) <= 22)
		menuPrint(31, 22, "\x08");
	else
		menuPrint(31, 3 + ((menuscroll * 20) / (nfiles - 22)), "\x08");

	setMenuSel(2 + menusel - menuscroll);
}


char fullpath[PATH_MAX+255];

int main(int argc, char **argv)
{
	defaultExceptionHandler();

	irqEnable(IRQ_VBLANK);
	irqEnable(IRQ_HBLANK);

	irqSet(IRQ_VBLANK, vblank_idle);

	fifoSetValue32Handler(FIFO_USER_02, arm7print, NULL);

	//vramSetBankA(VRAM_A_LCD);
	videoSetMode(MODE_0_2D);
	*(vu8*)0x04000240 = 0x81;
	doSplashscreen();

	// map some VRAM
	// bank C to ARM7, bank H for subscreen graphics
	*(vu8*)0x04000242 = 0x82;
	*(vu8*)0x04000248 = 0x81;

	videoSetModeSub(MODE_0_2D);
	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 2, 0, false, true);

	*(vu16*)0x0400100A = 0x0300;

	setBackdropColorSub(0x7C00);
	
	// configure BLDCNT so that backdrop becomes black
	*(vu16*)0x04001050 = 0x00E0;
	*(vu8*)0x04001054 = 16;
	
	// enable window 0 and disable color effects inside it
	*(vu16*)0x04001000 |= 0x2000;
	*(vu16*)0x04001048 = 0x001F;
	*(vu16*)0x0400104A = 0x003F;

	keysSetRepeat(25, 5);

	consoleDemoInit();
#ifdef NITROFS_ROM
	if (!nitroFSInit(NULL))
#else
	if (!fatInitDefault())
#endif
	{
		iprintf("FAT init failed\n");
		return -1;
	}

	makeMenu();

	iprintf("lolSnes " VERSION ", by Arisotura\n");
	
	if (argc <= 0) {
		iprintf("No ARGV argument passed");
		stop();
	}

	char* filename = argv[1];

	if (!Mem_LoadROM(filename)) {
		iprintf("ROM loading failed\n");
		stop();
	}

	*(vu16*)0x04001000 &= 0xDFFF;
	iprintf("ROM loaded, running\n");

	CPU_Reset();
	fifoSendValue32(FIFO_USER_01, 1);

	swiWaitForVBlank();
	fifoSendValue32(FIFO_USER_01, 2);

	irqSet(IRQ_VBLANK, vblank);
	irqSet(IRQ_HBLANK, PPU_HBlank);

	swiWaitForVBlank();
	CPU_Run();

	return 0;
}

void printvar()
{
	asm("stmdb sp!, {r12}");
	//iprintf("printvar %04X\n", IPC->_debug);
	asm("ldmia sp!, {r12}");
}

void printstuff(u32 foo, u32 bar, u32 blarg)
{
	asm("stmdb sp!, {r0-r3, r12}");
	iprintf("printstuff %08X %08X %08X\n", foo, bar, blarg);
	asm("ldmia sp!, {r0-r3, r12}");
}
