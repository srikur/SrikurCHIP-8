//
//  CPU.swift
//  SrikurCHIP-8
//
//  Created by Srikur Kanuparthy on 6/10/24.
//

import Foundation

class CPU {
    private var stackPointer: Int = 0
    private var programCounter: Int = 0x200
    private var delayTimer: Int = 0
    private var soundTimer: Int = 0
    private var indexRegister: Int = 0
    private var registers: [Int] = Array(repeating: 0, count: 16)
    private var stack: [Int] = Array(repeating: 0, count: 16)
    private var keys: [Int] = Array(repeating: 0, count: 16)
    private var screen: [UInt8] = Array(repeating: 0, count: 2048)
    private var memory: [Int] = Array(repeating: 0, count: 4096)
    private var _drawFlag: Bool = true
    var drawFlag: Bool {
        get {
            return _drawFlag
        }
        set {
            _drawFlag = newValue
        }
    }
    private var _shiftQuirk: Bool = false
    var shiftQuirk: Bool {
        get {
            _shiftQuirk
        }
        set {
            _shiftQuirk = newValue
        }
    }
    private var ipf: Int = 10
    
    static let SCREEN_WIDTH = 64
    static let SCREEN_HEIGHT = 32
    
    private let fontset: [Int] = [
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
    ]
    
    let instructionTable: [(CPU) -> (Int) -> ()]
    
    init(ipf: Int, romPath: URL) {
        self.ipf = ipf
        self.instructionTable = [
            CPU.i00EX, CPU.i1NNN, CPU.i2NNN, CPU.i3XNN, CPU.i4XNN, CPU.i5XY0, CPU.i6XNN, CPU.i7XNN,
            CPU.i8XYN, CPU.i9XY0, CPU.iANNN, CPU.iBNNN, CPU.iCXNN, CPU.iDXY0, CPU.iEXNN, CPU.iFXNN
        ]
        
        // Load fontset
        let fontsetAddress = 0x00
        for i in 0..<fontset.count {
            let value = fontset[i]
            memory[fontsetAddress + i] = value
        }
        
        self.loadRom(romPath)
    }
    
    func loadRom(_ romPath: URL) {
        print("Loading ROM: \(romPath)")
        let romData = try! Data(contentsOf: romPath)
        for i in 0..<romData.count {
            memory[i + 0x200] = Int(romData[i])
        }
        print("Loaded \(romData.count) bytes")
    }
    
    func setKey(_ key: Int, _ value: Bool) {
        keys[key] = value ? 1 : 0
    }
    
    func getScreen() -> [UInt8] {
        return screen
    }
    
    func emulateCycle() {
        for _ in 1...ipf {
            // Fetch
            let instruction = (memory[programCounter] << 8) | memory[programCounter + 1]
            programCounter += 2
            
            // Decode
            let function = (instruction & 0xF000) >> 12
            let functionPointer = instructionTable[function]
            
            // Execute
            functionPointer(self)(instruction)
            
            // Update Timers
            if delayTimer > 0 {
                delayTimer -= 1
            }
            if soundTimer > 0 {
                soundTimer -= 1
                print("BEEP")
            }
        }
    }
    
    // Instructions List
    func i00EX(instruction: Int) {
        if instruction == 0x00E0 {
            // 0E00: Clear the screen. Set all pixels to 0
            screen.withUnsafeMutableBufferPointer({ buffer in
                for i in 0..<buffer.count {
                    buffer[i] = 0
                }
            })
            drawFlag = true
        } else if instruction == 0x00EE {
            // 00EE: Return from a subroutine. Set the program counter to the address at the top of the stack
            stackPointer -= 1
            programCounter = stack[stackPointer]
        }
    }
    
    func i1NNN(instruction: Int) {
        // 1NNN: Jump to address NNN
        programCounter = instruction & 0x0FFF
    }
    
    func i2NNN(instruction: Int) {
        // 2NNN: Call subroutine at NNN. We place the current program counter on the stack so we can return to it later when returning from the subroutine
        stack[stackPointer] = programCounter
        stackPointer += 1
        programCounter = instruction & 0x0FFF
    }
    
    func i3XNN(instruction: Int) {
        // 3XNN: Skip the next instruction if register X equals NN
        if registers[(instruction & 0x0F00) >> 8] == (instruction & 0x00FF) {
            programCounter += 2
        }
    }
    
    func i4XNN(instruction: Int) {
        // 4XNN: Skip the next instruction if register X does not equal NN
        if registers[(instruction & 0x0F00) >> 8] != (instruction & 0x00FF) {
            programCounter += 2
        }
    }
    
    func i5XY0(instruction: Int) {
        // 5XY0: Skip the next instruction if register X equals register Y
        if registers[(instruction & 0x0F00) >> 8] == registers[(instruction & 0x00F0) >> 4] {
            programCounter += 2
        }
    }
    
    func i6XNN(instruction: Int) {
        // 6XNN: Set register X to NN
        registers[(instruction & 0x0F00) >> 8] = instruction & 0x00FF
    }
    
    func i7XNN(instruction: Int) {
        // 7XNN: Add NN to register X
        registers[(instruction & 0x0F00) >> 8] = ((registers[(instruction & 0x0F00) >> 8] & 0xFF) + (instruction & 0x00FF)) & 0xFF
    }
    
    func i8XYN(instruction: Int) {
        switch instruction & 0x000F {
        case 0x0000:
            // 8XY0: Set register X to the value of register Y
            registers[(instruction & 0x0F00) >> 8] = registers[(instruction & 0x00F0) >> 4]
        case 0x0001:
            // 8XY1: Set register X to the value of register X OR register Y
            registers[(instruction & 0x0F00) >> 8] |= registers[(instruction & 0x00F0) >> 4]
            
        case 0x0002:
            // 8XY2: Set register X to the value of register X AND register Y
            registers[(instruction & 0x0F00) >> 8] &= registers[(instruction & 0x00F0) >> 4]
            
        case 0x0003:
            // 8XY3: Set register X to the value of register X XOR register Y
            registers[(instruction & 0x0F00) >> 8] ^= registers[(instruction & 0x00F0) >> 4]
            
        case 0x0004:
            // 8XY4: Add the value of register Y to register X. Set VF to 1 if there is a carry, 0 otherwise
            registers[(instruction & 0x0F00) >> 8] = (registers[(instruction & 0x0F00) >> 8] + registers[(instruction & 0x00F0) >> 4])
            if registers[(instruction & 0x0F00) >> 8] > 0xFF {
                registers[0xF] = 1
            } else {
                registers[0xF] = 0
            }
            registers[(instruction & 0x0F00) >> 8] &= 0xFF
        
        case 0x0005:
            // 8XY5: Subtract the value of register Y from register X. Set VF to 0 if there is a borrow, 1 otherwise
            registers[(instruction & 0x0F00) >> 8] = registers[(instruction & 0x0F00) >> 8] - registers[(instruction & 0x00F0) >> 4]
            if registers[(instruction & 0x0F00) >> 8] < 0 {
                registers[0xF] = 0
            } else {
                registers[0xF] = 1
            }
            registers[(instruction & 0x0F00) >> 8] &= 0xFF
            
        case 0x0006:
            // 8XY6: Shift the value of register X right by 1. Set VF to the least significant bit of X before the shift
            let lastBit = registers[(instruction & 0x0F00) >> 8] & 0x1
            if shiftQuirk {
                registers[(instruction & 0x0F00) >> 8] >>= 1
            } else {
                registers[(instruction & 0x0F00) >> 8] = registers[(instruction & 0x00F0) >> 4] >> 1
            }
            registers[0xF] = lastBit
            
        case 0x0007:
            // 8XY7: Set register X to the value of register Y minus register X. Set VF to 0 if there is a borrow, 1 otherwise
            registers[(instruction & 0x0F00) >> 8] = registers[(instruction & 0x00F0) >> 4] - registers[(instruction & 0x0F00) >> 8]
            if registers[(instruction & 0x0F00) >> 8] < 0 {
                registers[0xF] = 0
            } else {
                registers[0xF] = 1
            }
            registers[(instruction & 0x0F00) >> 8] &= 0xFF
            
        case 0x000E:
            // 8XYE: Shift the value of register X left by 1. Set VF to the most significant bit of X before the shift
            let firstBit = (registers[(instruction & 0x0F00) >> 8] & 0x80) >> 7
            if shiftQuirk {
                registers[(instruction & 0x0F00) >> 8] = ((registers[(instruction & 0x0F00) >> 8] & 0xFF) << 1) & 0xFF
            } else {
                registers[(instruction & 0x0F00) >> 8] = ((registers[(instruction & 0x00F0) >> 4] & 0xFF) << 1) & 0xFF
            }
            registers[0xF] = firstBit
            
        default:
            break
        }
    }
    
    func i9XY0(instruction: Int) {
        // 9XY0: Skip the next instruction if register X does not equal register Y
        if registers[(instruction & 0x0F00) >> 8] != registers[(instruction & 0x00F0) >> 4] {
            programCounter += 2
        }
    }
    
    func iANNN(instruction: Int) {
        // ANNN: Set the index register to the address NNN
        indexRegister = instruction & 0x0FFF
    }
    
    func iBNNN(instruction: Int) {
        // BNNN: Jump to the address NNN plus the value of register 0
        programCounter = (instruction & 0x0FFF) + registers[0]
    }
    
    func iCXNN(instruction: Int) {
        // CXNN: Set register X to a random number AND NN
        let randomNumber = Int.random(in: 0..<256)
        registers[(instruction & 0x0F00) >> 8] = randomNumber & (instruction & 0x00FF)
    }
    
    func iDXY0(instruction: Int) {
        // DXYN: Draw a sprite at coordinate VX, VY using N bytes of sprite data starting at the address stored in the index register
        // If any set pixels are unset, set VF to 1. Otherwise, set VF to 0
        registers[0xF] = 0
        let x = registers[(instruction & 0x0F00) >> 8]
        let y = registers[(instruction & 0x00F0) >> 4]
        let height = instruction & 0x000F

        for i in 0..<height {
            let spriteByte = memory[indexRegister + i]
            for j in 0..<8 {
                let pixel = (spriteByte & (0x80 >> j)) >> (7 - j)
                if pixel == 1 {
                    let xCoord = (x + j) % CPU.SCREEN_WIDTH
                    let yCoord = (y + i) % CPU.SCREEN_HEIGHT
                    let index = xCoord + (yCoord * CPU.SCREEN_WIDTH)
                    
                    if screen[index] == 1 {
                        registers[0xF] = 1
                    }
                    
                    screen[index] ^= 1
                }
            }
        }
        drawFlag = true
    }
    
    func iEXNN(instruction: Int) {
        switch instruction & 0x00FF {
        case 0x009E:
            // # EX9E: Skip the next instruction if the key stored in register X is pressed
            if keys[registers[(instruction & 0x0F00) >> 8]] != 0 {
                programCounter += 2
            }
        case 0x00A1:
            // EXA1: Skip the next instruction if the key stored in register X is not pressed
            if keys[registers[(instruction & 0x0F00) >> 8]] == 0 {
                programCounter += 2
            }
        default:
            break
        }
    }
    
    func iFXNN(instruction: Int) {
        switch instruction & 0x00FF {
            
        case 0x0007:
            // FX07: Set register X to the value of the delay timer
            registers[(instruction & 0x0F00) >> 8] = delayTimer
            
        case 0x000A:
            // FX0A: Wait for a key press and store the result in register X
            if let keyIndex = keys.firstIndex(where: { $0 != 0 }) {
                registers[(instruction & 0x0F00) >> 8] = keys[keyIndex]
                keys[keyIndex] = 0
            } else {
                programCounter -= 2
            }
            
        case 0x0015:
            // FX15: Set the delay timer to the value of register X
            delayTimer = registers[(instruction & 0x0F00) >> 8] & 0xFF
            
        case 0x0018:
            // FX18: Set the sound timer to the value of register
            soundTimer = registers[(instruction & 0x0F00) >> 8] & 0xFF
            
        case 0x001E:
            // FX1E: Add the value of register X to the index
            indexRegister = (indexRegister + registers[(instruction & 0x0F00) >> 8]) & 0xFFFF
            
        case 0x0029:
            // FX29: Set the index register to the location of the sprite for the character in register X
            indexRegister = registers[(instruction & 0x0F00) >> 8] * 5
            
        case 0x0033:
            // FX33: Store the binary-coded decimal representation of the value of register X at the addresses I, I+1, and I+
            memory[indexRegister] = registers[(instruction & 0x0F00) >> 8] / 100
            memory[indexRegister + 1] = (registers[(instruction & 0x0F00) >> 8] / 10) % 10
            memory[indexRegister + 2] = registers[(instruction & 0x0F00) >> 8] % 10
            
        case 0x0055:
            // FX55: Store the values of registers V0 to VX in memory starting at address I
            for i in 0...(instruction & 0x0F00) >> 8 {
                memory[(indexRegister + i) & 0xFFFF] = registers[i]
            }
            indexRegister = ((indexRegister & 0xFFFF) + ((((instruction & 0x0F00) >> 8) + 1) & 0xFFFF)) & 0xFFFF
            
        case 0x0065:
            // FX65: Fill registers V0 to VX with values from memory starting at address I
            for i in 0...(instruction & 0x0F00) >> 8 {
                registers[i] = memory[indexRegister + i] & 0xFF
            }
            indexRegister = ((indexRegister & 0xFFFF) + ((((instruction & 0x0F00) >> 8) + 1) & 0xFFFF)) & 0xFFFF
        default:
            break
        }
    }
}
