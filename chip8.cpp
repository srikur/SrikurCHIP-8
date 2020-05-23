#include "chip8.h"

CHIP8::CHIP8() {
	/* Initialize */
	pc = 0x200;
	i = 0;
	sp = 0;
	memset(memory, 0, 4096);
	memset(registers, 0, 16);

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
		i = instruction & 0x0FFF;
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
	default:
		cout << "Unknown Opcode!" << endl;
		break;
	}
}