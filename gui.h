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

    /*
    Creates widgets on GUI
    */
    void createWidgets(float &clockSpeed);

public:
    GUI(Chip8 *chip8);
    ~GUI();

    /*
    Updates GUI using state of CHIP 8
    */
    void renderGUI(float &clockSpeed);

    /*
    Get numerical id of SDL window
    */
    int getWindowID();

    /*
    Forwards Chip 8 by one emulation cycle
    */
    void forwardOneCycle(); 
    
};

#endif