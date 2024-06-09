package org.srikur;

import java.util.Arrays;
import java.io.File;

public class CPU {

    private String romPath;
    private int[] memory;
    private int[] registers;
    private int programCounter;
    private int stackPointer;
    private int[] stack;
    private int indexRegister;

    private int delayTimer;
    private int soundTimer;

    private int[] keys;
    private int[] display;
    public boolean drawFlag;
    private boolean shiftQuirk;

    private final int SCREEN_WIDTH = 64;
    private final int SCREEN_HEIGHT = 32;
    private int instructionsPerFrame;

    private final int[] fontset = new int[] {
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

    public CPU(String romPath, int instructionsPerFrame, boolean shiftQuirk) {
        this.romPath = romPath;
        this.instructionsPerFrame = instructionsPerFrame;
        this.shiftQuirk = shiftQuirk;

        memory = new int[4096];
        registers = new int[16];
        programCounter = 0x200;
        stackPointer = 0;
        stack = new int[16];
        drawFlag = true;
        keys = new int[16];
        display = new int[SCREEN_WIDTH * SCREEN_HEIGHT];
        delayTimer = 0;
        soundTimer = 0;

        System.arraycopy(fontset, 0, memory, 0, fontset.length);

        loadROM();
    }

    private void loadROM() {
        // Load ROM into memory
        try {
            File file = new File(romPath);
            byte[] rom = new byte[(int) file.length()];
            java.io.FileInputStream fis = new java.io.FileInputStream(file);
            int res = fis.read(rom);
            if (res != rom.length) {
                throw new Exception("Failed to read ROM file");
            }
            fis.close();

            for (int i = 0; i < rom.length; i++) {
                memory[0x200 + i] = rom[i] & 0xFF;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public int[] getScreen() {
        return display;
    }

    public void setKey(int key, int value) {
        keys[key] = value;
    }

    public void emulateCycle() {
        for (int i = 0; i < instructionsPerFrame; i++) {
            int instruction = memory[programCounter] << 8 | memory[programCounter + 1];
            programCounter += 2;
            executeInstruction(instruction);

            if (delayTimer > 0) {
                delayTimer--;
            }

            if (soundTimer > 0) {
                soundTimer--;
            }

        }
    }

    public void executeInstruction(int instruction) {
        switch (instruction & 0xF000) {
            case 0x0000:
                if (instruction == 0x00E0) {
                    // Clear the screen
                    Arrays.fill(display, 0);
                } else if (instruction == 0x00EE) {
                    // Return from subroutine
                    programCounter = stack[stackPointer];
                    stackPointer--;
                }
                break;
            case 0x1000:
                // Jump to address NNN
                programCounter = instruction & 0x0FFF;
                break;
            case 0x2000:
                // Call subroutine at NNN
                stackPointer++;
                stack[stackPointer] = programCounter;
                programCounter = instruction & 0x0FFF;
                break;
            case 0x3000:
                // Skip next instruction if Vx == NN
                if (registers[(instruction & 0x0F00) >> 8] == (instruction & 0x00FF)) {
                    programCounter += 2;
                }
                break;
            case 0x4000:
                // Skip next instruction if Vx != NN
                if (registers[(instruction & 0x0F00) >> 8] != (instruction & 0x00FF)) {
                    programCounter += 2;
                }
                break;
            case 0x5000:
                // Skip next instruction if Vx == Vy
                if (registers[(instruction & 0x0F00) >> 8] == registers[(instruction & 0x00F0) >> 4]) {
                    programCounter += 2;
                }
                break;
            case 0x6000:
                // Set Vx = NN
                registers[(instruction & 0x0F00) >> 8] = instruction & 0x00FF;
                break;
            case 0x7000:
                // Set Vx = Vx + NN
                registers[(instruction & 0x0F00) >> 8] = (registers[(instruction & 0x0F00) >> 8] & 0xFF) + (instruction & 0x00FF) & 0xFF;
                break;
            case 0x8000:
                switch (instruction & 0x000F) {
                    case 0x0000:
                        // Set Vx = Vy
                        registers[(instruction & 0x0F00) >> 8] = registers[(instruction & 0x00F0) >> 4];
                        break;
                    case 0x0001:
                        // Set Vx = Vx OR Vy
                        registers[(instruction & 0x0F00) >> 8] = registers[(instruction & 0x0F00) >> 8] | registers[(instruction & 0x00F0) >> 4];
                        break;
                    case 0x0002:
                        // Set Vx = Vx AND Vy
                        registers[(instruction & 0x0F00) >> 8] = registers[(instruction & 0x0F00) >> 8] & registers[(instruction & 0x00F0) >> 4];
                        break;
                    case 0x0003:
                        // Set Vx = Vx XOR Vy
                        registers[(instruction & 0x0F00) >> 8] = registers[(instruction & 0x0F00) >> 8] ^ registers[(instruction & 0x00F0) >> 4];
                        break;
                    case 0x0004:
                        // Set Vx = Vx + Vy, set VF = carry
                        registers[(instruction & 0x0F00) >> 8] = (registers[(instruction & 0x0F00) >> 8] & 0xFF) + (registers[(instruction & 0x00F0) >> 4] & 0xFF);
                        if (registers[(instruction & 0x0F00) >> 8] > 0xFF) {
                            registers[0xF] = 1;
                        } else {
                            registers[0xF] = 0;
                        }
                        registers[(instruction & 0x0F00) >> 8] &= 0xFF;
                        break;
                    case 0x0005:
                        // Set Vx = Vx - Vy, set VF = NOT borrow
                        registers[(instruction & 0x0F00) >> 8] = registers[(instruction & 0x0F00) >> 8] - registers[(instruction & 0x00F0) >> 4];
                        if (registers[(instruction & 0x0F00) >> 8] < 0) {
                            registers[0xF] = 0;
                        } else {
                            registers[0xF] = 1;
                        }
                        registers[(instruction & 0x0F00) >> 8] &= 0xFF;
                        break;
                    case 0x0006:
                        // 8XY6: Shift the value of register X right by 1. Set VF to the least significant bit of register X before the shift.
                        int lastBit = registers[(instruction & 0x0F00) >> 8] & 0x1;
                        if (shiftQuirk) {
                            registers[(instruction & 0x0F00) >> 8] >>= 1;
                        } else {
                            registers[(instruction & 0x0F00) >> 8] = registers[(instruction & 0x0F00) >> 8] >> 1;
                        }
                        registers[0xF] = lastBit;
                        break;
                    case 0x0007:
                        // Set Vx = Vy - Vx, set VF = NOT borrow
                        registers[(instruction & 0x0F00) >> 8] = registers[(instruction & 0x00F0) >> 4] - registers[(instruction & 0x0F00) >> 8];
                        if (registers[(instruction & 0x0F00) >> 8] < 0) {
                            registers[0xF] = 0;
                        } else {
                            registers[0xF] = 1;
                        }
                        registers[(instruction & 0x0F00) >> 8] &= 0xFF;
                        break;
                    case 0x000E:
                        // Set Vx = Vx SHL 1
                        int firstBit = (registers[(instruction & 0x0F00) >> 8] & 0x80) >> 7;
                        if (shiftQuirk) {
                            registers[(instruction & 0x0F00) >> 8] = (registers[(instruction & 0x0F00) >> 8] << 1) & 0xFF;
                        } else {
                            registers[(instruction & 0x0F00) >> 8] = (registers[(instruction & 0x00F0) >> 4] << 1) & 0xFF;
                        }
                        registers[0xF] = firstBit;
                        break;
                    default:
                        break;
                }
                break;
            case 0x9000:
                // Skip next instruction if Vx != Vy
                if (registers[(instruction & 0x0F00) >> 8] != registers[(instruction & 0x00F0) >> 4]) {
                    programCounter += 2;
                }
                break;
            case 0xA000:
                // ANNN: Set index register to address NNN
                indexRegister = instruction & 0x0FFF;
                break;
            case 0xB000:
                // BNNN: Jump to address NNN + V0
                programCounter = (instruction & 0x0FFF) + registers[0];
                break;
            case 0xC000:
                // CXNN: Set Vx = random byte AND NN
                registers[(instruction & 0x0F00) >> 8] = (int) (Math.random() * 256) & (instruction & 0x00FF);
                break;
            case 0xD000:
                // DXYN: Draw a sprite at coordinate (Vx, Vy) that has a width of 8 pixels and a height of N pixels.
                // If any pixels are flipped from set to unset when the sprite is drawn, VF is set to 1. Otherwise, it is set to 0.
                int x = registers[(instruction & 0x0F00) >> 8];
                int y = registers[(instruction & 0x00F0) >> 4];
                registers[0xF] = 0;
                for (int i = 0; i < (instruction & 0x000F); i++) {
                    int spriteByte = memory[indexRegister + i];
                    for (int j = 0; j < 8; j++) {
                        int pixel = (spriteByte & (0x80 >> j)) >> (7 - j);
                        int displayIndex = (x + j) % SCREEN_WIDTH + ((y + i) % SCREEN_HEIGHT) * SCREEN_WIDTH;
                        if (pixel == 1) {
                            if (display[displayIndex] == 1) {
                                registers[0xF] = 1;
                            }
                            display[displayIndex] ^= 1;
                        }
                    }
                }
                drawFlag = true;
                break;
            case 0xE000:
                if ((instruction & 0x00FF) == 0x009E) {
                    // EX9E: Skip next instruction if key with the value of Vx is pressed
                    if (keys[registers[(instruction & 0x0F00) >> 8]] == 1) {
                        programCounter += 2;
                    }
                } else if ((instruction & 0x00FF) == 0x00A1) {
                    // EXA1: Skip next instruction if key with the value of Vx is not pressed
                    if (keys[registers[(instruction & 0x0F00) >> 8]] == 0) {
                        programCounter += 2;
                    }
                }
                break;
            case 0xF000:
                switch (instruction & 0x00FF) {
                    case 0x0007:
                        // Set Vx = delay timer value
                        registers[(instruction & 0x0F00) >> 8] = delayTimer;
                        break;
                    case 0x000A:
                        // FX0A: Wait for a key press, store the value of the key in Vx
                        boolean keyPressed = false;
                        for (int i = 0; i < keys.length; i++) {
                            if (keys[i] == 1) {
                                registers[(instruction & 0x0F00) >> 8] = i;
                                keyPressed = true;
                                break;
                            }
                        }
                        if (!keyPressed) {
                            programCounter -= 2;
                        }
                        break;
                    case 0x0015:
                        // Set delay timer = Vx
                        delayTimer = registers[(instruction & 0x0F00) >> 8] & 0xFF;
                        break;
                    case 0x0018:
                        // Set sound timer = Vx
                        soundTimer = registers[(instruction & 0x0F00) >> 8] & 0xFF;
                        break;
                    case 0x001E:
                        // Set index register = index register + Vx
                        indexRegister = (indexRegister + registers[(instruction & 0x0F00) >> 8]) & 0xFFFF;
                        break;
                    case 0x0029:
                        // Set index register to the location of the sprite for the character in Vx
                        indexRegister = registers[(instruction & 0x0F00) >> 8] * 5;
                        break;
                    case 0x0033:
                        // Store the binary-coded decimal representation of Vx at the addresses I, I+1, and I+2
                        memory[indexRegister] = registers[(instruction & 0x0F00) >> 8] / 100;
                        memory[indexRegister + 1] = (registers[(instruction & 0x0F00) >> 8] / 10) % 10;
                        memory[indexRegister + 2] = registers[(instruction & 0x0F00) >> 8] % 10;
                        break;
                    case 0x0055:
                        // Store registers V0 through Vx in memory starting at location I.
                        for (int i = 0; i <= ((instruction & 0x0F00) >> 8); i++) {
                            memory[(indexRegister + i) & 0xFFFF] = registers[i];
                        }
                        indexRegister = (indexRegister & 0xFFFF) + (((instruction & 0x0F00) >> 8) + 1) & 0xFFFF;
                        break;
                    case 0x0065:
                        // Read registers V0 through Vx from memory starting at location I
                        for (int i = 0; i <= ((instruction & 0x0F00) >> 8); i++) {
                            registers[i] = memory[indexRegister + i] & 0xFF;
                        }
                        indexRegister = indexRegister + (((instruction & 0x0F00) >> 8) + 1) & 0xFFFF;
                        break;
                    default:
                        break;
                }
        }
    }
}
