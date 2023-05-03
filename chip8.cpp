#include <SDL.h>
#include <exception>
#include <string>
#include <cstring>
#include <fstream>
#include <random>
#include "chip8.h"

#include <iostream>

Chip8::InitializationError::InitializationError(std::string errorMsg) {
    this->errorMsg = "Initialization error: " + errorMsg;
}

const char * Chip8::InitializationError::what() const noexcept {
    return errorMsg.c_str();
}

Chip8::Chip8() {
    // Load fontset into memory
    unsigned char chip8_fontset[80] = { 
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

    for (int i = 0; i < 80; i++) {
        memory[i + FONTSET_START_ADDRESS] = chip8_fontset[i];
    }
}

Chip8::~Chip8() {
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
}

void Chip8::loadGame(std::string fileName) {
    std::ifstream fin(fileName);
    if (!fin.is_open()) {
        throw Chip8::InitializationError("Unable to open game file");
    }

    unsigned char nextCode;
    int counter = 0;
    while (fin >> nextCode) {
        memory[counter + PROGRAM_START_ADDRESS] = nextCode;
        counter++;
    }

    fin.close();
}

void Chip8::initializeInput() {

}

void Chip8::emulateCycle() {
    unsigned short opcode = memory[programCounter] << 8 | memory[programCounter + 1];
    programCounter += 2;

    currentOpcode = opcode;

    int x, y, n; // Used to store (x, y) coords for drawing commands, and the size of sprite in n
    unsigned short sum; // Used to store sums for some cases
    short int difference; // Used to store differences for some cases
    
    switch(opcode & 0xF000) {
        case 0x0000:
            switch(opcode & 0x000F) {
                case 0x0000: // 0x00E0: Clears display
                    clearScreen();
                    break;
                case 0x000E: // 0x00EE: Returns from subroutine
                    programCounter = stack[--stackPointer];
                    break;
            }
            break;
        case 0x1000: // 0x1nnn: Set program counter to nnn
            programCounter = opcode & 0x0FFF;
            break;
        case 0x2000: // 0x2nnn: Calls subroutine at nnn
            stack[stackPointer++] = programCounter;
            programCounter = opcode & 0x0FFF;
            break;
        case 0x3000: // 0x3xkk: Skip next instruction if register Vx == kk
            if (registers[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
                programCounter += 2;
            }
            break;
        case 0x4000: // 0x4xkk: Skip next instruction if register Vx != kk
            if (registers[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
                programCounter += 2;
            }
            break;
        case 0x5000: // 0x5xy0: Skip next instruction if Vx == Vy
            if (registers[(opcode & 0x0F00) >> 8] == registers[(opcode & 0x0F0) >> 4]) {
                programCounter += 2;
            }
            break;
        case 0x6000: // 0x6xkk: Set Vx = kk
            registers[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            break;
        case 0x7000: // 0x7xkk: Increment Vx by kk
            registers[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            break;
        case 0x8000:
            switch(opcode & 0x000F) {
                case 0x0: // 0x8xy0: Set Vx = Vy
                    registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4];
                    break;
                case 0x1: // 0x8xy1: Set Vx = Vx OR Vy
                    registers[(opcode & 0x0F00) >> 8] |= registers[(opcode & 0x00F0) >> 4];
                    break;
                case 0x2: // 0x8xy2: Set Vx = Vx AND Vy
                    registers[(opcode & 0x0F00) >> 8] &= registers[(opcode & 0x00F0) >> 4];
                    break;
                case 0x3: // 0x8xy3: Set Vx = Vx XOR Vy
                    registers[(opcode & 0x0F00) >> 8] ^= registers[(opcode & 0x00F0) >> 4];
                    break;
                case 0x4: // 0x8xy4: Set Vx = Vx + Vy, and VF = carry
                    sum = registers[(opcode & 0x0F00) >> 8] + registers[(opcode & 0x00F0) >> 4];
                    registers[(opcode & 0x0F00) >> 8] = sum & 0x00FF;
                    registers[0xF] = (sum) > 0x00FF ? 1 : 0;
                    break;
                case 0x5: // 0x8xy5: Set Vx = Vx - Vy, and VF = NOT borrow
                    difference = registers[(opcode & 0x0F00) >> 8] - registers[(opcode & 0x00F0) >> 4];
                    registers[(opcode & 0x0F00) >> 8] = (unsigned char) difference;
                    registers[0xF] = (difference > 0) ? 1 : 0;
                    break;
                case 0x6: // 0x8xy6: If LSb of Vx is 1, Set VF = 1; Set Vx = Vx >> 1
                    registers[0xF] = ((registers[(opcode & 0x0F00) >> 8] & 0x01) == 1) ? 1 : 0;
                    registers[(opcode & 0x0F00) >> 8] >>= 1;
                    break;
                case 0x7: // 0x8xy7: Set Vx - Vy - Vx, and VF = NOT borrow
                    difference = registers[(opcode & 0x00F0) >> 4] - registers[(opcode & 0x0F00) >> 8];
                    registers[(opcode & 0x0F00) >> 8] = (unsigned char) difference;
                    registers[0xF] = difference > 0 ? 1 : 0;
                    break;
                case 0xE: // 0x8xyE: If MSb of Vx is 1, Set VF = 1; Set Vx = Vx << 1
                    registers[0xF] = ((registers[(opcode & 0x0F00) >> 8] & 0x80) == 0x80) ? 1 : 0;
                    registers[(opcode & 0x0F00) >> 8] >>= 1;
                    break;
            }
            break;
        case 0x9000: // 0x9xy0: Skip next instruction if Vx != Vy
            if (registers[(opcode & 0x0F00) >> 8] != registers[(opcode & 0x0F0) >> 4]) {
                programCounter += 2;
            }
            break;
        case 0xA000: // 0xAnnn: Set index register = nnn
            index = opcode & 0x0FFF;
            break;
        case 0xB000: // 0xBnnn: Jump to location nnn + V0
            programCounter = registers[0] + (opcode & 0x0FFF);
            break;
        case 0xC000: // 0xCxkk: Set Vx = random byte AND kk
            registers[(opcode & 0x0F00) >> 8] = (rand() % 255) & (opcode & 0x00FF);
            break;
        case 0xD000: // 0xDxyn: Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
            x = registers[(opcode & 0x0F00) >> 8] % 64;
            y = registers[(opcode & 0x00F0) >> 4] % 32;
            n = opcode & 0x000F;

            for (int j = 0; j < n; j++) {
                unsigned char sprite = memory[index + j];
                for (int i = 0; i < 8; i++) {
                    bool isWhite = (sprite & (0x80 >> i)) > 0;
                    if (isWhite) {
                        unsigned short pixelIndex = 64 * ((y + j) % 32) + ((x + i) % 64);
                        registers[0xF] = display[pixelIndex] ? 1 : 0;
                        display[pixelIndex] = display[pixelIndex] != isWhite;
                    }
                }
            }

            break;
        case 0xE000:
            switch (opcode & 0x000F) {
                case 0xE: // 0xEx9E: Skip next instruction if key with value Vx is pressed
                    if (keys[(opcode & 0x0F00) >> 8]) {
                        programCounter += 2;
                    }
                    break;
                case 0x1: // 0xExA1: Skip next instruction if key with value Vx is not pressed.
                    if (!keys[(opcode & 0x0F00) >> 8]) {
                        programCounter += 2;
                    }
                    break;
            }
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x07: // 0xFx07: Set Vx to delay timer value
                    registers[(opcode & 0x0F00) >> 8] = delayTimer;
                    break;
                case 0x0A: // 0xFx0A: Wait for a key press, store the value of the key in Vx
                    /* 
                    
                        NEED TO IMPLEMENT
                    
                    */
                    break;
                case 0x15: // 0xFx15: Set delay timer = Vx
                    delayTimer = registers[(opcode & 0x0F00) >> 8];
                    break;
                case 0x18: // 0xFx18: Set sound timer = Vx
                    soundTimer = registers[(opcode & 0x0F00) >> 8];
                    break;
                case 0x1E: // 0xFx1E: Set index register += Vx
                    index += registers[(opcode & 0x0F00) >> 8];
                    break;
                case 0x29: // 0xFx29: Set index register = location of sprite for digit Vx.
                    index = memory[0x050 + 5 * (registers[(opcode & 0x0F00) >> 8] & 0xF)];
                    break;
                case 0x33: // 0xFx33: Store BCD representation of Vx in mem locations index, index+1, and index+2
                    memory[index] = registers[(opcode & 0x0F00) >> 8] / 100;
                    memory[index + 1] = (registers[(opcode & 0x0F00) >> 8] % 100) / 10;
                    memory[index + 2] = registers[(opcode & 0x0F00) >> 8] % 10;
                    break;
                case 0x55: // 0xFx55: Copy V0 to Vx to memory, starting from mem location index
                    for (int i = 0; i <= (opcode & 0x0F00) >> 8; i++) {
                        memory[i + index] = registers[i];
                    }
                    break;
                case 0x65: // 0xFx65: Copy values from memory into V0 to Vx, starting from mem location index
                    for (int i = 0; i <= (opcode & 0x0F00) >> 8; i++) {
                        registers[i] = memory[i + index];
                    }
                    break;
            }
            break;

        setKeys(); // Update key binds
    }
}

void Chip8::clearScreen() {
    for (int i = 0; i < 64 * 32; i++) {
        display[i] = false;
    }
}

void Chip8::setKeys() {

}

// Getters for chip8

unsigned short Chip8::getMemory(unsigned short i) {
    return memory[i];
}
unsigned char Chip8::getRegister(int i) {
    return registers[i];
}
bool Chip8::getDisplay(int i) {
    return display[i];
}
bool Chip8::getKey(int i) {
    return keys[i];
}
unsigned short Chip8::getStack(int i) {
    return stack[i];
}
unsigned short Chip8::getSoundTimer() {
    return soundTimer;
}
unsigned short Chip8::getDelayTimer() {
    return delayTimer;
}
unsigned short Chip8::getIndex() {
    return index;
}
unsigned short Chip8::getProgramCounter() {
    return programCounter;
}
unsigned char Chip8::getStackPointer() {
    return stackPointer;
}
unsigned short Chip8::getCurrentOpcode() {
    return currentOpcode;
}
