#pragma once
#include "includes.h"

constexpr int screen_width = 64;
constexpr int screen_height = 32;

using namespace std;

class CHIP8 {
public:

	CHIP8();

	bool loadGame(const char* romName);
	void emulateCycle();

	u8 memory[4096];
	u8 V[16];

	u16 i;
	u16 pc;
	bool drawFlag = true;
	u8 delay_timer;
	u8 sound_timer;

	u16 stack[16];
	u8 keys[16];
	u16 sp;

	int shift_quirk;

	u8 graphics[64 * 32];

	u8 fontset[80] = {
	  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	  0x20, 0x60, 0x20, 0x20, 0x70, // 1
	  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

private:

	u16 instruction;

	void (CHIP8::* const Chip8_8XYN_Instructions[16])(void) = {
		&CHIP8::cpu8XY0, &CHIP8::cpu8XY1, &CHIP8::cpu8XY2, &CHIP8::cpu8XY3,
		&CHIP8::cpu8XY4, &CHIP8::cpu8XY5, &CHIP8::cpu8XY6,&CHIP8::cpu8XY7, NULL, NULL, NULL, NULL, NULL, NULL, &CHIP8::cpu8XYE, NULL
	};

	void (CHIP8::* const Chip8Table[17])(void) = {
		&CHIP8::cpu0NNN, &CHIP8::cpu1NNN, &CHIP8::cpu2NNN, &CHIP8::cpu3XNN, &CHIP8::cpu4XNN, &CHIP8::cpu5XY0, &CHIP8::cpu6XNN, &CHIP8::cpu7XNN,
		&CHIP8::cpu8XYN, &CHIP8::cpu9XY0, &CHIP8::cpuANNN, &CHIP8::cpuBNNN, &CHIP8::cpuCXNN, &CHIP8::cpuDXYN, &CHIP8::cpuEXNN, &CHIP8::cpuFXNN
	};

	/* Chip 8 Table */
	void cpu0NNN();
	void cpu1NNN();
	void cpu2NNN();
	void cpu3XNN();
	void cpu4XNN();
	void cpu5XY0();
	void cpu6XNN();
	void cpu7XNN();
	void cpu8XYN();
	void cpu9XY0();
	void cpuANNN();
	void cpuBNNN();
	void cpuCXNN();
	void cpuDXYN();
	void cpuEXNN();
	void cpuFXNN();

	/* Arithmetic Instructions */
	void cpu8XY0();
	void cpu8XY1();
	void cpu8XY2();
	void cpu8XY3();
	void cpu8XY4();
	void cpu8XY5();
	void cpu8XY6();
	void cpu8XY7();
	void cpu8XYE();
};