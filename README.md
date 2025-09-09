# Visual Apple 1

An Apple 1 emulator written in C utilizing the raylib library

## Build Instructions

Both a (limited) terminal and visual application are provided

### Terminal

Use `gcc 6502emu.c -o apple` to build the Windows version

Use `gcc 6502emuLinux.c -o apple` to build the Linux version

You will need to copy the monitor.rom and basic.rom files into the directory of the executable for the emulator to work

### Visual

You must have raylib installed to build this!

Use `gcc main.c 6502emu.c assemble.c sprite.c -o apple -lraylib` to build the executable

## Special Controls (Visual)

### Assembler

Drag a 6502 assembly source file onto the execuatable to have it directly loaded into RAM

### Function keys

F1: Toggle text between white/greem

F2: Reset machine

F3: Clear screen

F4/F5: Make screen smaller/larger

F6: Load selected file

F7: Save a block of memory

F8: Toggle Debug Info

### Saving/Loading

Files are catalogued through a tapes.txt file, each line represents one data file and should be formatted as such

`file.rom lowaddress highaddress`

where lowaddress and highaddress are 4 nibble address (i.e. 03E5)

You can toggle which file is selected with the up and down arrow keys (The first entry is tape #0, the second is tape #1, etc.)

The selected file is loaded in with F6


To save a block of memory, press F7 and enter in the low and high address of the range you want to save.

It will be saved to tape.rom.
