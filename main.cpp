/* 
Emulates Chip 8 system and runs the ROM located at the provided file path
*/
#include <iostream>
#include "chip8.h"
#include "gui.h"
#include "imgui_impl_sdl2.h"



int main(int argc, char **argv) {
    try{
        Chip8 chip8;
        GUI gui(&chip8);

        chip8.initializeInput();
        chip8.loadGame("roms/logo.rom");

        float clockSpeed = 50;
        float cycleDuration = (1  / clockSpeed) * 1e3;

        SDL_Event e;
        auto lastCycleTime = std::chrono::high_resolution_clock::now();
        while (true){
            if (SDL_PollEvent(&e) == 1 && e.type == SDL_QUIT){
                return EXIT_SUCCESS;
            }
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE && e.window.windowID == gui.getWindowID()) {
                return EXIT_SUCCESS;
            }
           
            auto currentTime = std::chrono::high_resolution_clock::now();
		    float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();
            if (dt > cycleDuration && !gui.isPaused()) {
                lastCycleTime = currentTime;

                chip8.emulateCycle();
            }
            
            gui.renderGUI();
        }
    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}