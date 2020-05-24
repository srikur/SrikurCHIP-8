#include "chip8.h"

CHIP8::CHIP8() {
	/* Initialize */
	pc = 0x200;
	i = 0;
	sp = 0;
	memset(memory, 0, 4096);
	memset(V, 0, 16);

	for (int i = 0; i < 80; i++) {
		memory[i] = fontset[i];
	}
}

bool CHIP8::loadGame(char* romName) {
	FILE* file = fopen(romName, "r");

	if (file == NULL) {
		cout << "Failed to open the file" << endl;
		return false;
	}

	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	char* buffer = (char*)malloc(length * sizeof(char));

	fseek(file, 0, SEEK_SET);
	fread(buffer, 1, 4096, file);
	fclose(file);

	for (int i = 0; i < length; i++) {
		memory[i + 0x200] = buffer[i];
	}

	return true;
}

void CHIP8::emulateCycle() {
	/* Fetch opcode */
	u16 instruction = memory[pc] << 8 | memory[pc + 1];
	decodeInstruction(instruction);
}

void CHIP8::decodeInstruction(u16 instruction) {
	switch (instruction & 0xF000) { /* Gets first four bits */
	case 0xA000:
		/* ANNN: Sets I to the address NNN */
		i = instruction & 0xFFF;
		pc += 2;
		break;
	case 0x1000:
		/* 1NNN: Go to address NNN */
		pc = instruction & 0x0FFF;
		break;
	case 0x0000:
		/* 0NNN, 00E0, and 00EE */
		switch (instruction) {

		}
		break;
	case 0x2000:
		/* 2NNN: Calls subroutine at NNN */
		break;
	case 0x3000:
		/* 3XNN: Skips the next instruction if VX equals NN */
		if (V[(instruction & 0xF00) >> 8] == (instruction & 0xFF)) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	case 0x4000:
		/* 4XNN: Skips the next instruction if VX does not equal NN */
		if (V[(instruction & 0xF00) >> 8] != (instruction & 0xFF)) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	case 0x5000:
		/* 5XY0: Skips the next instruction if VX = VY */
		if (V[(instruction & 0xF00) >> 8] == V[(instruction & 0xF0) >> 4]) {
			pc += 4;
		}
		else pc += 2;
		break;
	case 0x6000:
		/* 6XNN: Sets VX to NN */
		V[(instruction & 0xF00) >> 8] = instruction & 0xFF;
		pc += 2;
		break;
	case 0x7000:
		/* 7XNN: Add NN to VX */
		V[(instruction & 0xF00) >> 8] += (instruction & 0xFF);
		pc += 2;
		break;
	case 0x8000:
		switch (instruction) {

		}
		break;
	case 0xB000:
		/* BNNN: Jumps to the address NNN plus V0 */
		pc = V[0] + (instruction & 0xFFF);
		break;
	case 0xC000:
		/* CXNN: Sets VX to the result of a bitwise AND operation on a random number (0-255) and NN */
		srand((unsigned) time(0));
		int random = 1 + (rand() % 255);
		V[(instruction & 0xF00) >> 8] = random & 0xFF;
		pc += 2;
		break;
	case 0xD000:
		break;
	case 0xE000:
		break;
	case 0xF000:

		break;
	default:
		cout << "Unknown Opcode!" << endl;
		break;
	}
}