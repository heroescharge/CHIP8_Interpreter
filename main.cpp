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
        chip8.loadGame("roms/pong2.rom");

        const float timersCycleDuration = 1000 / 60; // 60 Hz timers
        float clockSpeed = 500; // Clock speed in Hertz

        SDL_Event e;
        auto lastCycleTime = std::chrono::high_resolution_clock::now();
        auto lastTimersTime = lastCycleTime;
        while (true){
            float cycleDuration = (1  / clockSpeed) * 1e3; // Duration of a cycle

            // Check if user quits out of window
            if (SDL_PollEvent(&e) == 1 && e.type == SDL_QUIT){
                return EXIT_SUCCESS;
            }
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE && e.window.windowID == gui.getWindowID()) {
                return EXIT_SUCCESS;
            }
           
           
            auto currentTime = std::chrono::high_resolution_clock::now();
		    float dtCycle = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();
		    float dtTimer = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastTimersTime).count();
            // Run next emulation cycle
            if (dtCycle > cycleDuration && !chip8.isPaused()) {
                lastCycleTime = currentTime;

                chip8.emulateCycle();
            }
            // Decrement timers
            if (dtTimer > timersCycleDuration && !chip8.isPaused()) {
                lastTimersTime = currentTime;
                chip8.updateTimers();
            }
            
            gui.renderGUI(clockSpeed); // Pass clockSpeed by reference so that GUI can display it
        }
    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}