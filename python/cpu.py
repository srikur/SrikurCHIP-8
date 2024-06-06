import random

class CPU:
    memory = [0] * 0xFFF
    registers: list[int] = [0] * 16
    stack = [0] * 16
    stack_pointer = 0
    program_counter = 0
    delay_timer = 0
    sound_timer = 0
    keys = [0] * 16
    screen = [0] * 0x800
    draw_flag = False
    shift_quirk = False

    screen_width = 64
    screen_height = 32

    def __init__(self):
        pass

    def load_rom(self, path: str):
        with open(path, "rb") as file:
            data = file.read()
            for i in range(len(data)):
                self.memory[i + 0x200] = data[i]

    def emulateCycle(self):
        instruction = self.memory[self.program_counter] << 8 | self.memory[self.program_counter + 1]
        # print("Opcode: " + hex(instruction))
        self.program_counter += 2

        self.executeInstruction(instruction)

        if self.delay_timer > 0:
            self.delay_timer -= 1

        if self.sound_timer > 0:
            if self.sound_timer == 1:
                print("BEEP!")
            self.sound_timer -= 1

    def setKeys(self, keys: list):
        self.keys = keys

    def getScreen(self) -> list:
        return self.screen
    
    def executeInstruction(self, instruction: int):
        # Instruction Set
        match instruction & 0xF000:
            case 0x0000:
                if instruction == 0x00E0:
                    # 0E00: Clear the screen. Set all pixels to 0
                    self.screen = [0] * 0x800
                    self.draw_flag = True
                elif instruction == 0x00EE:
                    # 00EE: Return from a subroutine. Set the program counter to the address at the top of the stack
                    self.stack_pointer -= 1
                    self.program_counter = self.stack[self.stack_pointer]
            case 0x1000:
                # 1NNN: Jump to address NNN
                self.program_counter = instruction & 0x0FFF
            case 0x2000:
                # 2NNN: Call subroutine at NNN. We place the current program counter on the stack so we can return to it later when returning from the subroutine
                self.stack[self.stack_pointer] = self.program_counter
                self.stack_pointer += 1
                self.program_counter = instruction & 0x0FFF
            case 0x3000:
                # 3XNN: Skip the next instruction if register X equals NN
                if self.registers[(instruction & 0x0F00) >> 8] == (instruction & 0x00FF):
                    self.program_counter += 2
            case 0x4000:
                # 4XNN: Skip the next instruction if register X does not equal NN
                if self.registers[(instruction & 0x0F00) >> 8] != (instruction & 0x00FF):
                    self.program_counter += 2
            case 0x5000:
                # 5XY0: Skip the next instruction if register X equals register Y
                if self.registers[(instruction & 0x0F00) >> 8] == self.registers[(instruction & 0x00F0) >> 4]:
                    self.program_counter += 2
            case 0x6000:
                # 6XNN: Set register X to NN
                self.registers[(instruction & 0x0F00) >> 8] = (instruction & 0x00FF)
            case 0x7000:
                # 7XNN: Add NN to register X
                self.registers[(instruction & 0x0F00) >> 8] = ((self.registers[(instruction & 0x0F00) >> 8] & 0xFF) + (instruction & 0xFF)) & 0xFF
            case 0x8000:
                match instruction & 0x000F:
                    case 0x0000:
                        # 8XY0: Set register X to the value of register Y
                        self.registers[(instruction & 0x0F00) >> 8] = (self.registers[(instruction & 0x00F0) >> 4])
                    case 0x0001:
                        # 8XY1: Set register X to the value of register X OR register Y
                        self.registers[(instruction & 0x0F00) >> 8] |= (self.registers[(instruction & 0x00F0) >> 4])
                    case 0x0002:
                        # 8XY2: Set register X to the value of register X AND register Y
                        self.registers[(instruction & 0x0F00) >> 8] &= (self.registers[(instruction & 0x00F0) >> 4])
                    case 0x0003:
                        # 8XY3: Set register X to the value of register X XOR register Y
                        self.registers[(instruction & 0x0F00) >> 8] ^= (self.registers[(instruction & 0x00F0) >> 4])
                    case 0x0004:
                        # 8XY4: Add the value of register Y to register X. Set VF to 1 if there is a carry, 0 otherwise
                        self.registers[(instruction & 0x0F00) >> 8] = (self.registers[(instruction & 0x0F00) >> 8] & 0xFF) \
                                                                    + (self.registers[(instruction & 0x00F0) >> 4] & 0xFF)
                        if self.registers[(instruction & 0x0F00) >> 8] > 0xFF:
                            self.registers[0xF] = 1
                        else:
                            self.registers[0xF] = 0
                        self.registers[(instruction & 0x0F00) >> 8] &= 0xFF
                    case 0x0005:
                        # 8XY5: Subtract the value of register Y from register X. Set VF to 0 if there is a borrow, 1 otherwise
                        self.registers[(instruction & 0x0F00) >> 8] = (self.registers[(instruction & 0x0F00) >> 8] & 0xFF) \
                                                                    - (self.registers[(instruction & 0x00F0) >> 4] & 0xFF)
                        if self.registers[(instruction & 0x0F00) >> 8] < 0:
                            self.registers[0xF] = 0
                        else:
                            self.registers[0xF] = 1
                        self.registers[(instruction & 0x0F00) >> 8] &= 0xFF
                    case 0x0006:
                        # 8XY6: Shift the value of register X right by 1. Set VF to the least significant bit of X before the shift
                        last_bit = self.registers[(instruction & 0x0F00) >> 8] & 0xFF & 0x01
                        print(f"VF: {self.registers[0xF]}")
                        if self.shift_quirk:
                            self.registers[(instruction & 0x0F00) >> 8] >>= 1
                        else:
                            self.registers[(instruction & 0x0F00) >> 8] = (self.registers[(instruction & 0x00F0) >> 4] & 0xFF) >> 1
                        self.registers[0xF] = last_bit
                    case 0x0007:
                        # 8XY7: Set register X to the value of register Y minus register X. Set VF to 0 if there is a borrow, 1 otherwise
                        self.registers[(instruction & 0x0F00) >> 8] = (self.registers[(instruction & 0x00F0) >> 4] & 0xFF) \
                                                                    - (self.registers[(instruction & 0x0F00) >> 8] & 0xFF)
                        if self.registers[(instruction & 0x0F00) >> 8] < 0:
                            self.registers[0xF] = 0
                        else:
                            self.registers[0xF] = 1
                        self.registers[(instruction & 0x0F00) >> 8] &= 0xFF
                    case 0x000E:
                        # 8XYE: Shift the value of register X left by 1. Set VF to the most significant bit of X before the shift
                        first_bit = (self.registers[(instruction & 0x0F00) >> 8] & 0x80) >> 7
                        if self.shift_quirk:
                            self.registers[(instruction & 0x0F00) >> 8] = ((self.registers[(instruction & 0x0F00) >> 8] & 0xFF) << 1) & 0xFF
                        else:
                            self.registers[(instruction & 0x0F00) >> 8] = ((self.registers[(instruction & 0x00F0) >> 4] & 0xFF) << 1) & 0xFF
                        self.registers[0xF] = first_bit
            case 0x9000:
                # 9XY0: Skip the next instruction if register X does not equal register Y
                if self.registers[(instruction & 0x0F00) >> 8] != self.registers[(instruction & 0x00F0) >> 4]:
                    self.program_counter += 2
            case 0xA000:
                # ANNN: Set the index register to the address NNN
                self.index_register = instruction & 0x0FFF
            case 0xB000:
                # BNNN: Jump to the address NNN plus the value of register 0
                self.program_counter = (instruction & 0x0FFF) + self.registers[0]
            case 0xC000:
                # CXNN: Set register X to a random number AND NN
                random.seed()
                self.registers[(instruction & 0x0F00) >> 8] = random.randint(0, 255) & (instruction & 0x00FF)
            case 0xD000:
                # DXYN: Draw a sprite at coordinate VX, VY using N bytes of sprite data starting at the address stored in the index register
                # If any set pixels are unset, set VF to 1. Otherwise, set VF to 0
                self.registers[0xF] = 0
                x = self.registers[(instruction & 0x0F00) >> 8]
                y = self.registers[(instruction & 0x00F0) >> 4]
                for i in range(instruction & 0x000F):
                    pixel = self.memory[self.index_register + i]
                    for j in range(8):
                        if (pixel & (0x80 >> j)) != 0:
                            index = (x + j) % self.screen_width + ((y + i) % self.screen_height) * self.screen_width
                            if self.screen[index] == 1:
                                self.registers[0xF] = 1
                            self.screen[index] = self.screen[index] ^ 1
                self.draw_flag = True
            case 0xE000:
                match instruction & 0x00FF:
                    case 0x009E:
                        # EX9E: Skip the next instruction if the key stored in register X is pressed
                        if self.keys[self.registers[(instruction & 0x0F00) >> 8]] != 0:
                            self.program_counter += 2
                    case 0x00A1:
                        # EXA1: Skip the next instruction if the key stored in register X is not pressed
                        if self.keys[self.registers[(instruction & 0x0F00) >> 8]] == 0:
                            self.program_counter += 2
            case 0xF000:
                match instruction & 0x00FF:
                    case 0x0007:
                        # FX07: Set register X to the value of the delay timer
                        self.registers[(instruction & 0x0F00) >> 8] = self.delay_timer
                    case 0x000A:
                        # FX0A: Wait for a key press and store the result in register X
                        key_pressed = False
                        for i in range(16):
                            if self.keys[i] != 0:
                                self.registers[(instruction & 0x0F00) >> 8] = i
                                key_pressed = True
                                break
                        if not key_pressed:
                            self.program_counter -= 2
                    case 0x0015:
                        # FX15: Set the delay timer to the value of register X
                        self.delay_timer = self.registers[(instruction & 0x0F00) >> 8] & 0xFF
                    case 0x0018:
                        # FX18: Set the sound timer to the value of register X
                        self.sound_timer = self.registers[(instruction & 0x0F00) >> 8] & 0xFF
                    case 0x001E:
                        # FX1E: Add the value of register X to the index register
                        # self.index_register += (self.registers[(instruction & 0x0F00) >> 8])
                        self.index_register = ((self.index_register & 0xFFF) + self.registers[(instruction & 0x0F00) >> 8]) & 0xFFFF
                    case 0x0029:
                        # FX29: Set the index register to the location of the sprite for the character in register X
                        self.index_register = self.registers[(instruction & 0x0F00) >> 8] * 5
                    case 0x0033:
                        # FX33: Store the binary-coded decimal representation of the value of register X at the addresses I, I+1, and I+2
                        self.memory[self.index_register] = self.registers[(instruction & 0x0F00) >> 8] // 100
                        self.memory[self.index_register + 1] = (self.registers[(instruction & 0x0F00) >> 8] // 10) % 10
                        self.memory[self.index_register + 2] = self.registers[(instruction & 0x0F00) >> 8] % 10
                    case 0x0055:
                        # FX55: Store the values of registers V0 to VX in memory starting at address I
                        for i in range(((instruction & 0x0F00) >> 8) + 1):
                            self.memory[(self.index_register + i) & 0xFFFF] = self.registers[i]
                        self.index_register = (self.index_register & 0xFFFF) + ((((instruction & 0x0F00) >> 8) + 1) & 0xFFFF) & 0xFFFF
                    case 0x0065:
                        # FX65: Fill registers V0 to VX with values from memory starting at address I
                        for i in range(((instruction & 0x0F00) >> 8) + 1):
                            self.registers[i] = self.memory[self.index_register + i] & 0xFF
                        self.index_register = (self.index_register & 0xFFFF) + ((((instruction & 0x0F00) >> 8) + 1) & 0xFFFF) & 0xFFFF