#include "chip8.h"

CHIP8::CHIP8() {
	/* Initialize */
	pc = 0x200;
	i = 0;
	sp = 0;
	delay_timer = 0;
	sound_timer = 0;
	memset(memory, 0, 4096);
	memset(V, 0, 16);
	memset(graphics, 0, 2048);
	memset(keys, 0, 16);

	for (int i = 0; i < 80; i++) {
		memory[i] = fontset[i];
	}
}

bool CHIP8::loadGame(const char* romName) {
	FILE* file = fopen(romName, "rb");

	if (file == NULL) {
		cout << "Failed to open the file" << endl;
		return false;
	}

	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	printf("File length: %i\n", length);
	fseek(file, 0, SEEK_SET);
	char* buffer = (char*)malloc(length * sizeof(char));

	if (buffer == NULL) {
		printf("Failed to open the file!\n");
		return false;
	}

	long result = fread(buffer, 1, length, file);
	if (result != length) {
		printf("Error reading the file!\n");
		printf("Bytes read: %d\n", result);
		return false;
	}

	if (length < (4096 - 512)) {
		for (int i = 0; i < length; i++) {
			memory[i + 0x200] = buffer[i];
		}
	}
	else {
		printf("ROM size too large for memory!\n");
		return false;
	}

	fclose(file);
	free(buffer);

	return true;
}

void CHIP8::emulateCycle() {
	/* Fetch opcode */
	instruction = memory[pc] << 8 | memory[pc + 1];
	printf("Currently calling instruction: %.4X\n", instruction);
	printf("%X\n", (instruction & 0xF000) >> 12);

	void(CHIP8:: * opcode_fn)() = Chip8Table[(instruction & 0xF000) >> 12];
	(this->*opcode_fn)();

	if (delay_timer > 0) {
		delay_timer--;
	}

	if (sound_timer > 0) {
		if (sound_timer == 1) {
			/* SOUND */
		}
		sound_timer--;
	}
}

void CHIP8::cpu0NNN() {
	switch (instruction & 0xF) {
	case 0x0:
		/* 00E0: Clear the screen */
		memset(graphics, 0xFF000000, screen_height * screen_width);
		drawFlag = true;
		pc += 2;
		break;
	case 0xE:
		/* 00EE: Returns from a subroutine */
		--sp;
		pc = stack[sp];
		pc += 2;
		break;
	}
}

void CHIP8::cpu1NNN() {
	/* 1NNN: Go to address NNN */
	printf("Function called!\n");
	pc = instruction & 0x0FFF;
}

void CHIP8::cpu2NNN() {
	/* 2NNN: Calls subroutine at NNN */
	stack[sp] = pc;
	++sp;
	pc = (instruction & 0xFFF);
}

void CHIP8::cpu3XNN() {
	/* 3XNN: Skips the next instruction if VX equals NN */
	if (V[(instruction & 0xF00) >> 8] == (instruction & 0xFF)) {
		pc += 4;
	}
	else {
		pc += 2;
	}
}

void CHIP8::cpu4XNN() {
	/* 4XNN: Skips the next instruction if VX does not equal NN */
	if (V[(instruction & 0xF00) >> 8] != (instruction & 0xFF)) {
		pc += 4;
	}
	else {
		pc += 2;
	}
}

void CHIP8::cpu5XY0() {
	/* 5XY0: Skips the next instruction if VX = VY */
	if (V[(instruction & 0xF00) >> 8] == V[(instruction & 0xF0) >> 4]) {
		pc += 4;
	}
	else pc += 2;
}

void CHIP8::cpu6XNN() {
	/* 6XNN: Sets VX to NN */
	V[(instruction & 0xF00) >> 8] = (instruction & 0xFF);
	pc += 2;
}

void CHIP8::cpu7XNN() {
	/* 7XNN: Add NN to VX */
	V[(instruction & 0xF00) >> 8] += (instruction & 0xFF);
	pc += 2;
}

void CHIP8::cpu8XYN() {
	void(CHIP8:: * opcode_fn)() = Chip8_8XYN_Instructions[instruction & 0xF];
	(this->*opcode_fn)();
}

void CHIP8::cpu8XY0() {
	/* 8XY0: Sets value of VX to the value of VY */
	V[(instruction & 0xF00) >> 8] = V[(instruction & 0xF0) >> 4];
	pc += 2;
}

void CHIP8::cpu8XY1() {
	/* 8XY1: Sets VX to VX OR VY (Bitwise OR operation) */
	V[(instruction & 0xF00) >> 8] |= V[(instruction & 0xF0) >> 4];
	pc += 2;
}

void CHIP8::cpu8XY2() {
	/* 8XY2: Sets VX to VX AND VY (Bitwise AND operation) */
	V[(instruction & 0xF00) >> 8] &= V[(instruction & 0xF0) >> 4];
	pc += 2;
}

void CHIP8::cpu8XY3() {
	/* 8XY3: Sets VX to VX XOR VY (Bitwise XOR operation) */
	V[(instruction & 0xF00) >> 8] ^= V[(instruction & 0xF0) >> 4];
	pc += 2;
}

void CHIP8::cpu8XY4() {
	/* 8XY4: Add VY to VX. VF is set to 1 when there is a
				carry, and zero when there is not. */
	if (V[(instruction & 0xF0) >> 4] > (0xFF - V[(instruction & 0xF00) >> 8])) {
		V[0xF] = 1;
	}
	else {
		V[0xF] = 0;
	}
	V[(instruction & 0xF00) >> 8] += V[(instruction & 0xF0) >> 4];
	pc += 2;
}

void CHIP8::cpu8XY5() {
	/* 8XY5: VY is subtracted from VX. Set VF to 0 if a borrow occurs. Set VF to 1 if a borrow does not occur. */
	if (V[(instruction & 0xF0) >> 4] > V[(instruction & 0xF00) >> 8]) {
		V[0xF] = 0;
	}
	else {
		V[0xF] = 1;
	}
	V[(instruction & 0xF00) >> 8] -= V[(instruction & 0xF0) >> 4];
	pc += 2;
}

void CHIP8::cpu8XY6() {
	/* 8XY6: Store the value of register VY shifted right one bit in register VX. Set register VF to the least signficant bit prior to the shift. */
	V[0xF] = (V[(instruction & 0xF00) >> 8]) & 0x01;
	if (shift_quirk) {
		V[(instruction & 0xF00) >> 8] >>= 1;
	}
	else {
		V[(instruction & 0xF00) >> 8] = V[(instruction & 0xF0) >> 4] >> 1;
	}
	pc += 2;
}

void CHIP8::cpu8XY7() {
	/* 8XY7: Set register VX to the value of VY minus VX. Set VF to 0 if a borrow occurs. Set VF to 1 if a borrow does not occur. */
	if (V[(instruction & 0x0F00) >> 8] > V[(instruction & 0x00F0) >> 4]) {
		V[0xF] = 0;
	}
	else {
		V[0xF] = 1;
	}
	V[(instruction & 0xF00) >> 8] = V[(instruction & 0x00F0) >> 4] - V[(instruction & 0x0F00) >> 8];
	pc += 2;
}

void CHIP8::cpu8XYE() {
	/* 8XYE: Store the value of register VY shifted left one bit in register VX. Set register VF to the most significant bit prior to the shift. */
	V[0xF] = !!((V[(instruction & 0xF00) >> 8]) & 0x80);
	if (shift_quirk) {
		V[(instruction & 0xF00) >> 8] <<= 1;
	}
	else {
		V[(instruction & 0xF00) >> 8] = V[(instruction & 0xF0) >> 4] << 1;
	}
	pc += 2;
}

void CHIP8::cpu9XY0() {
	/* 9XY0: Skip the following instruction if the value of register VX is not equal to the value of register VY */
	if (V[(instruction & 0xF00) >> 8] != V[(instruction & 0xF0) >> 4]) {
		pc += 4;
	}
	else {
		pc += 2;
	}
}

void CHIP8::cpuANNN() {
	i = instruction & 0xFFF;
	pc += 2;
}

void CHIP8::cpuBNNN() {
	/* BNNN: Jumps to the address NNN plus V0 */
	pc = V[0] + (instruction & 0xFFF);
}

void CHIP8::cpuCXNN() {
	/* CXNN: Sets VX to the result of a bitwise AND operation on a random number (0-255) and NN */
	srand((unsigned)time(0));
	V[(instruction & 0x0F00) >> 8] = (rand() % (0xFF + 1)) & (instruction & 0x00FF);
	pc += 2;
}

void CHIP8::cpuDXYN() {
	/* DXYN: Draw a sprite at position VX, VY with N bytes of sprite data starting at the address stored in I. Set VF to 1 if any pixels are changed to unset, and 0 otherwise. */

	V[0xF] = 0;
	for (int y = 0; y < (instruction & 0xF); y++) {
		for (int x = 0; x < 8; x++) {
			u8 pixel = memory[i + y];
			if (pixel & (0x80 >> x)) {
				int index = (V[(instruction & 0xF00) >> 8] + x) % screen_width +
					((V[(instruction & 0xF0) >> 4] + y) % screen_height) * screen_width;
				if (graphics[index] == 1) {
					V[0xF] = 1;
				}
				graphics[index] ^= 1;
			}
		}
	}

	drawFlag = true;
	pc += 2;
}

void CHIP8::cpuEXNN() {
	switch (instruction & 0xFF) {
	case 0x9E:
		/* EX9E: Skip the following instruction if the key corresponding to the hex value currently stored in register VX is pressed */
		if (keys[V[(instruction & 0xF00) >> 8]] != 0) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	case 0xA1:
		/* EXA1: Skip the following instruction if the key corresponding to the hex value currently stored in register VX is not pressed*/
		if (keys[V[(instruction & 0xF00) >> 8]] == 0) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	}
}

void CHIP8::cpuFXNN() {
	switch (instruction & 0xFF) {
	case 0x07:
		/* FX07: Store the current value of the delay timer in register VX */
		V[(instruction & 0xF00) >> 8] = delay_timer;
		pc += 2;
		break;
	case 0x0A: {
		/* FX0A: Wait for a keypress and store the result in register VX */
		bool keyPressed = false;
		for (int i = 0; i < 16; i++) {
			if (keys[i] != 0) {
				V[(instruction & 0xF00) >> 8] = i;
				keyPressed = true;
			}
		}

		if (!keyPressed) {
			return;
		}

		pc += 2;
	}
			 break;

	case 0x15:
		/* FX15: Set the delay timer to the value of register VX */
		delay_timer = V[(instruction & 0xF00) >> 8];
		pc += 2;
		break;
	case 0x18:
		/* Sets the sound timer to the value of register VX */
		sound_timer = V[(instruction & 0xF00) >> 8];
		pc += 2;
		break;
	case 0x1E:
		/* FX1E: Add the value stored in register VX to register I */
		if (i + V[(instruction & 0xF00) >> 8] > 0xFFF) {
			V[0xF] = 1;
		}
		else {
			V[0xF] = 0;
		}
		i += V[(instruction & 0xF00) >> 8];
		pc += 2;
		break;
	case 0x29:
		/* Set I to the memory address of the sprite data corresponding to the hexadecimal digit stored in register VX */
		i = V[(instruction & 0xF00) >> 8] * 5;
		pc += 2;
		break;
	case 0x33:
		/* FX33: Store the binary-coded decimnal equivalent of the value stored in register VX at addresses I, I + 1, and I + 2 */
		memory[i] = (V[(instruction & 0x0F00) >> 8] % 1000) / 100;
		memory[i + 1] = (V[(instruction & 0x0F00) >> 8] % 100) / 10;
		memory[i + 2] = (V[(instruction & 0x0F00) >> 8] % 10);
		pc += 2;
		break;
	case 0x55:
		/* FX55: Store the values of registers V0 through VX inclusive in memory starting at address I. I is set to I + X + 1 after the operation */
		for (int j = 0; j <= ((instruction & 0xF00) >> 8); ++j) {
			memory[i + j] = V[j];
		}
		i += ((instruction & 0xF00) >> 8) + 1;
		pc += 2;
		break;
	case 0x65:
		/* FX65: Fill registers V0 through VX inclusive with the values stored in memory starting at address I. I is set to I + X + 1 after the operation */
		for (int j = 0; j <= ((instruction & 0xF00) >> 8); ++j) {
			V[j] = memory[i + j];
		}
		i += ((instruction & 0xF00) >> 8) + 1;
		pc += 2;
		break;
	}
}