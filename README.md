# lolSnes
A SNES emulator for the Nintendo DS, aiming to make rendering the most accurate possible.

Considering the DS isn't powerful enough to handle a line-accurate software renderer AND emulate the CPU and other
funny things at the same time, I have to emulate graphics using the DS hardware. Reproducing the functions it
doesn't support (like per-tile priority or funky color effects) will be the challenge. How they will be emulated
will also depend on how games use them.

## Features
### What is currently supported

- Large ROMs (via intelligent ROM cache system)
- Regular joypad (for player 1)
- Linear audio interpolation (the SNES does Gaussian interpolation, but the DS isn't powerful enough)
- SRAM with auto-saving

### WIP Emulation report

- **CPU:** 99% (all opcodes emulated; may miss a few unimportant bits about timing)
- **PPU:** ~50% (2bpp, 4bpp and 8bpp graphics, modes 0-4 and 7, mosaic, master brightness)
- **SPC700:** 90% (most important stuff emulated)
- **DSP:** ~80% (code taken from SNemulDS. Emulates BRR sound with envelopes. Seems to lack noise and echo.)
- **DMA::** 50% (HDMA not supported yet, and DMA method is inefficient)

### What is NOT supported

- Expansion chips
- Multiplayer

### What will never be emulated perfectly

- **Color subtract:** The Nintendo DS hardware has no such feature
- **Offset per tile:** The Nintendo DS hardware has no such feature
- **Per-tile priority:** layer priority is changed per-scanline based on gross approximation, though

100% accurate graphics emulation is impossible, but we'll do our best.

## Building
If you don't want to compile yourself but you still want to get the latest build, please use our [TWLBot github repository](https://github.com/TWLBot/Builds/blob/master/extras/lolSnes.7z)

In order to compile this application on your own, you will need [devkitPro](https://devkitpro.org/) with the devkitARM toolchain, plus the necessary tools and libraries. devkitPro includes `dkp-pacman` for easy installation of all components:

```
 $ dkp-pacman -Syu devkitARM general-tools dstools ndstool libnds libfat-nds
```

Once everything is downloaded and installed, `git clone` this repository, navigate to the folder in which it was cloned, and run `make` to compile the application. If there is an error, let us know.

## How to use

1. Place `lolsnes.nds` where ever you like.
2. Create a folder titled `snes` in the same folder as `lolsnes.nds`.
3. Place your SNES ROMs in the `snes` folder you have just created

lolSnes is able to properly detect the ROM type in most cases. Headered and headerless ROMs are supported, both
LoROM and HiROM.

## Credits

- **Arisotura** - Original creator of lolSnes
- **RocketRobz** - Implementing argument support for ROM launching
- **FlameKat53** - Setting up the auto-compiling system

## lolSnes community

Would you like to join our DS Homebrew Discord community where we talk about lolSnes? Do you want to report bugs and glitches?
If so, here's an invite link: https://discord.gg/yqSut8c
