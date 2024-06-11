#pragma once
#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <SDL.h>

constexpr int screen_width = 64;
constexpr int screen_height = 32;

using namespace std;

class CHIP8 {
public:

	CHIP8();

	bool loadGame(const char* romName);
	void emulateCycle();

	uint8_t memory[4096];
	uint8_t V[16];

	uint16_t i;
	uint16_t programCounter;
	bool drawFlag = true;
	uint8_t delay_timer;
	uint8_t sound_timer;

	uint16_t stack[16];
	uint8_t keys[16];
	uint16_t sp;

	int shift_quirk;

	uint8_t graphics[64 * 32];

	uint8_t fontset[80] = {
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

	uint16_t instruction;

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