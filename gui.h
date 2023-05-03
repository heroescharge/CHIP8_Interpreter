/* 
GUI for Chip 8 system
*/

#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

#include "chip8.h"
#include "imgui.h"

class GUI {
private:
    Chip8 *chip8;
    SDL_Window *window;
    SDL_Renderer *renderer;
    ImGuiIO *io;

    bool paused = true;
    /*
    Creates widgets on GUI
    */
    void createWidgets();

public:
    GUI(Chip8 *chip8);
    ~GUI();

    /*
    Updates GUI using state of CHIP 8
    */
    void renderGUI();

    /*
    Get numerical id of SDL window
    */
    int getWindowID();

    /*
    Check if GUI is paused
    */
    bool isPaused();

    /*
    Forwards Chip 8 by one emulation cycle
    */
    void forwardOneCycle(); 
    
};

#endif