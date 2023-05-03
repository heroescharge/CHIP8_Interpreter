/*
Driver for Chip8 emulator; simulates a Chip 8 machine
*/

#ifndef CHIP8_H_INCLUDED
#define CHIP8_H_INCLUDED

#define FONTSET_START_ADDRESS 0x50
#define PROGRAM_START_ADDRESS 0x200

#include <SDL.h>
#include <exception>
#include <string>

class Chip8 {
private:
    // 0x000-0x1FF stores Chip-8 interpreter
    // 0x050-0x0A0 - Used for built in 4x5 pixel font set (0-F)
    // 0x200-0xFFF - Program ROM and work RAM
    unsigned char memory[4096] = {};
    // Registers labeled V0 - VF
    // VF used as carry flag
    unsigned char registers[16] = {};
    // Stores index for holding memory addresses (sometimes iterating through strings, arrays, etc)
    unsigned short index = 0;
    // Stores mem address of next instruction; starts at 0x200
    unsigned short programCounter = PROGRAM_START_ADDRESS;
    //B&W screen, 64 x 32 pixels
    bool display[64 * 32] = {};
    // Timers count down from 0 when positive
    // Buzzer sound will play as long as sound timer is positive
    unsigned short delayTimer = 0;
    unsigned short soundTimer = 0;
    // Stores state of key input: keys are 0x0 to 0xF
    unsigned char keys[16] = {};
    // Stack to store addresses that interpreter should interpret when finished w/ subroutine
    unsigned short stack[16] = {};
    // Stack pointer to point to top of stack (points to one element above the top)
    unsigned char stackPointer = 0;
    // Most recent opcode
    unsigned short currentOpcode;
    //SDL Window
    SDL_Window *window;
    SDL_Renderer *renderer;

public:
    // Custom error to handle problems occurring during initialization of Chip 8
    class InitializationError : public std::exception {
    private:
        std::string errorMsg;
    public:
        /*
        Initialize the error message for this exception
        */
        InitializationError(std::string errorMsg);

        /*
        Override what() method from std::exception class
        */
        const char *what() const noexcept;
    };

    /*
    Constructor for CHIP-8 system
    */
    Chip8();
    
    /*
    Destructor for CHIP-8 system
    */
    ~Chip8();

    /*
    Loads the game ROM
    Args:
        - fileName: A pointer to a string containing the path of the file to load
    */
    void loadGame(std::string fileName);

    /*
    Initializes the keyboard interface
    */
    void initializeInput();

    /* 
    Runs one emulation cycle
    */
    void emulateCycle();


    /*
    Updates key inputs from user
    */
    void setKeys();

    /*
    Clears the screen, coloring all pixels black
    */
    void clearScreen();

    /*
    Getters for all state variables
    */
    unsigned short getMemory(unsigned short i);
    unsigned char getRegister(int i);
    bool getDisplay(int i);
    bool getKey(int i);
    unsigned short getStack(int i);
    unsigned short getSoundTimer();
    unsigned short getDelayTimer();
    unsigned short getIndex();
    unsigned short getProgramCounter();
    unsigned char getStackPointer();
    unsigned short getCurrentOpcode();
};

#endif