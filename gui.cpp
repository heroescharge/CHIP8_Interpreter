#include "gui.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer.h"
#include <stdio.h>
#include <SDL.h>
#include <string>
#include <sstream>
#include <iostream>

#define TEXT_LABEL_COLOR IM_COL32(255, 0, 0, 255)
#define GREEN_COLOR IM_COL32(0, 255, 0, 255)
#define YELLOW_COLOR IM_COL32(255, 255, 0, 255)

GUI::GUI(Chip8 *chip8) {
    this->chip8 = chip8;
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) { throw Chip8::InitializationError(SDL_GetError()); }

    SDL_DisplayMode DM;
    if (SDL_GetCurrentDisplayMode(0, &DM) != 0) { throw Chip8::InitializationError(SDL_GetError()); }

    int COMPUTER_SCREEN_WIDTH = DM.w;
    int COMPUTER_SCREEN_HEIGHT = DM.h;

    // Create window with SDL_Renderer graphics context
    window = SDL_CreateWindow("CHIP8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
            COMPUTER_SCREEN_WIDTH, COMPUTER_SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
   

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO(); (void)io;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    io->FontGlobalScale = 1.5;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);
}

GUI::~GUI() {
    // Cleanup
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void GUI::renderGUI(float &clockSpeed) {
    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Create widgets
    createWidgets(clockSpeed);

    // Render
    ImGui::Render();
    SDL_RenderSetScale(renderer, io->DisplayFramebufferScale.x, io->DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(renderer);
}

// Converts c from hexadecimal to int
int hexToInt(char c) {
    if (c >= 'A') {
        return c - 'A' + 10;
    }
    return c - '0';
}

inline void GUI::createWidgets(float &clockSpeed) {
    const int WINDOW_WIDTH = io->DisplaySize.x;
    const int WINDOW_HEIGHT = io->DisplaySize.y;

    const int STACK_WIDTH = WINDOW_WIDTH / 15;
    const int INFO_WIDTH = 9 * WINDOW_WIDTH / 32;
    const int CPU_STATE_HEIGHT = WINDOW_HEIGHT / 3;

    const int DISPLAY_HEIGHT = (WINDOW_WIDTH - INFO_WIDTH - STACK_WIDTH) / 2;
    const int DISPLAY_WIDTH = 2 * DISPLAY_HEIGHT;

    const int GENERAL_WIDTH = 7 * DISPLAY_WIDTH / 10;

    // STACK
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(STACK_WIDTH, WINDOW_HEIGHT));        
    {
        bool stackWindowOpen = true;
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
        if (ImGui::Begin("Stack", &stackWindowOpen, flags)) {
            ImGui::PushStyleColor(ImGuiCol_Text, TEXT_LABEL_COLOR);
            ImGui::Text("SP");
            ImGui::SameLine();
            ImGui::PopStyleColor();
            ImGui::Text("%X", chip8->getStackPointer());
            ImGui::Text(""); // Newline

            for (int i = 0; i < 16; i++) {
                ImGui::PushStyleColor(ImGuiCol_Text, TEXT_LABEL_COLOR);
                ImGui::Text("%X", i);
                ImGui::SameLine();
                ImGui::PopStyleColor();
                ImGui::Text("0x%X", chip8->getStack(i));
                // Draw arrow on topmost stack element
                if (i == chip8->getStackPointer() - 1) {
                    ImGui::SameLine();
                    ImGui::Text(" <");
                }
            }
            ImGui::End();
        }
    }
    // CPU STATE
    ImGui::SetNextWindowPos(ImVec2(STACK_WIDTH, 0));
    ImGui::SetNextWindowSize(ImVec2(INFO_WIDTH, CPU_STATE_HEIGHT));        
    {
        bool registersWindowOpen = true;
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
        if (ImGui::Begin("CPU State", &registersWindowOpen, flags)) {
                

            if (ImGui::BeginTable("Registers1", 8)) {
                ImGui::PushStyleColor(ImGuiCol_Text, TEXT_LABEL_COLOR);

                for (int i = 0; i < 8; i++) {
                    std::ostringstream os;
                    os << "V" << i;
                    ImGui::TableSetupColumn(os.str().c_str());
                }
                ImGui::TableHeadersRow();
                
                ImGui::PopStyleColor();

                for (int i = 0; i < 8; i++) {
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", chip8->getRegister(i));
                }

                ImGui::EndTable();
            }
            if (ImGui::BeginTable("Registers2", 8)) {
                ImGui::PushStyleColor(ImGuiCol_Text, TEXT_LABEL_COLOR);

                for (int i = 8; i < 16; i++) {
                    std::ostringstream os;
                    os << "V" << i;
                    ImGui::TableSetupColumn(os.str().c_str());
                }
                ImGui::TableHeadersRow();

                ImGui::PopStyleColor();
                
                for (int i = 8; i < 16; i++) {
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", chip8->getRegister(i));
                }
                
                ImGui::EndTable();
            }

            ImGui::Text(""); // Newline
            ImGui::Text(""); // Newline

            ImGui::PushStyleColor(ImGuiCol_Text, GREEN_COLOR);
            ImGui::Text("PC:");
            ImGui::SameLine();
            ImGui::PopStyleColor();
            ImGui::Text("0x%X", chip8->getProgramCounter());
            ImGui::SameLine();

            ImGui::SetCursorPosX(INFO_WIDTH / 2);
            ImGui::PushStyleColor(ImGuiCol_Text, TEXT_LABEL_COLOR);
            ImGui::Text("DT:");
            ImGui::SameLine();
            ImGui::PopStyleColor();
            ImGui::Text("%d", chip8->getDelayTimer());

            ImGui::PushStyleColor(ImGuiCol_Text, YELLOW_COLOR);
            ImGui::Text("I:");
            ImGui::SameLine();
            ImGui::PopStyleColor();
            ImGui::Text("0x%X", chip8->getIndex());
            ImGui::SameLine();

            ImGui::SetCursorPosX(INFO_WIDTH / 2);
            ImGui::PushStyleColor(ImGuiCol_Text, TEXT_LABEL_COLOR);
            ImGui::Text("ST:");
            ImGui::SameLine();
            ImGui::PopStyleColor();
            ImGui::Text("%d", chip8->getSoundTimer());

            ImGui::PushStyleColor(ImGuiCol_Text, TEXT_LABEL_COLOR);
            ImGui::Text("OP:");
            ImGui::SameLine();
            ImGui::PopStyleColor();
            unsigned short currentOpcode = chip8->getMemory(chip8->getProgramCounter()) << 8 | 
                                           chip8->getMemory(1 + chip8->getProgramCounter());
            ImGui::Text("0x%X", currentOpcode);

            ImGui::End();
        }
    }
    // MEMORY
    ImGui::SetNextWindowPos(ImVec2(STACK_WIDTH, CPU_STATE_HEIGHT));
    ImGui::SetNextWindowSize(ImVec2(INFO_WIDTH, WINDOW_HEIGHT - CPU_STATE_HEIGHT));      
    {
        bool memoryDisplay = true;
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
        if (ImGui::Begin("Memory", &memoryDisplay, flags)) {
            for (int i = 0; i < 4096; i++) {
                unsigned short opcode = chip8->getMemory(i);
                // Color memory address green if programCounter is on it
                if (i == chip8->getProgramCounter()) {
                    ImGui::PushStyleColor(ImGuiCol_Text, GREEN_COLOR);
                }
                // Color memory address yellow if index is on it
                else if (i == chip8->getIndex()) {
                    ImGui::PushStyleColor(ImGuiCol_Text, YELLOW_COLOR);
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Text, TEXT_LABEL_COLOR);
                }
                ImGui::Text("0x%04X ", i);
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::Text("%02X", (opcode & 0xF000) >> 12);
                ImGui::SameLine();
                ImGui::Text("%02X", (opcode & 0x0F00) >> 8);
                ImGui::SameLine();
                ImGui::Text("%02X", (opcode & 0x00F0) >> 4);
                ImGui::SameLine();
                ImGui::Text("%02X", opcode & 0x000F);
                // Make two memory locations appear on same line
                if (i % 2 == 0) {
                    ImGui::SameLine();
                    ImGui::Text("\t");
                    ImGui::SameLine();
                }
            }

            ImGui::End();
        }
    }

    // DISPLAY
    ImGui::SetNextWindowPos(ImVec2(STACK_WIDTH + INFO_WIDTH, 0));
    ImGui::SetNextWindowSize(ImVec2(DISPLAY_WIDTH, DISPLAY_HEIGHT));      
    {
        bool displayOpen = true;
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
        if (ImGui::Begin("Display", &displayOpen, flags)) {
            const int SCALE = DISPLAY_WIDTH / 64;

            auto pos = ImGui::GetWindowPos();
            auto *drawList = ImGui::GetWindowDrawList();
            for (int x = 0; x < 64; x++) {
                for (int y = 0; y < 32; y++) {
                    if (chip8->getDisplay(64 * y + x)) {
                        drawList->AddRectFilled(ImVec2(pos[0] + x * SCALE, pos[1] + y * SCALE), 
                                                ImVec2(pos[0] + (x + 1) * SCALE, pos[1] + (y + 1) * SCALE), 
                                                0xFFFFFFFF);
                    }
                    else {
                        drawList->AddRectFilled(ImVec2(pos[0] + x * SCALE, pos[1] + y * SCALE), 
                                                ImVec2(pos[0] + (x + 1) * SCALE, pos[1] + (y + 1) * SCALE), 
                                                0x000000FF);
                    }
                }
            }

            ImGui::End();
        }
    }

    // GENERAL
    ImGui::SetNextWindowPos(ImVec2(STACK_WIDTH + INFO_WIDTH, DISPLAY_HEIGHT));
    ImGui::SetNextWindowSize(ImVec2(GENERAL_WIDTH, WINDOW_HEIGHT - DISPLAY_HEIGHT));      
    {
        bool displayOpen = true;
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
        if (ImGui::Begin("General", &displayOpen, flags)) {
            ImGui::PushStyleColor(ImGuiCol_Text, TEXT_LABEL_COLOR);
            ImGui::Text("Clock");
            ImGui::PopStyleColor();
            // Pause button
            if (ImGui::Button(chip8->isPaused() ? "Resume" : "Pause")) {
                chip8->togglePaused();
            }
            // Forward One Cycle Button
            if (ImGui::Button("Tick")) {
                forwardOneCycle();
            }
            // Clock speed
            ImGui::PushStyleColor(ImGuiCol_Text, TEXT_LABEL_COLOR);
            ImGui::Text("Clock Speed:");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::SliderFloat("float", &clockSpeed, 1.0, 1000.0);
            ImGui::SameLine();
            ImGui::Text("Hz");
            // FPS
            ImGui::PushStyleColor(ImGuiCol_Text, TEXT_LABEL_COLOR);
            ImGui::Text("FPS:");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::Text("%.2f", io->Framerate);

            ImGui::End();
        }
    }

    // Keypad
    ImGui::SetNextWindowPos(ImVec2(STACK_WIDTH + INFO_WIDTH + GENERAL_WIDTH, DISPLAY_HEIGHT));
    ImGui::SetNextWindowSize(ImVec2(DISPLAY_WIDTH - GENERAL_WIDTH, WINDOW_HEIGHT - DISPLAY_HEIGHT));      
    {
        bool displayOpen = true;
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
        if (ImGui::Begin("Keypad", &displayOpen, flags)) {
            std::string text = 
                "1  2  3  C\n"
                "4  5  6  D\n"
                "7  8  9  E\n"
                "A  0  B  F";

            float textWidth = ImGui::CalcTextSize(text.c_str()).x;
            float textHeight = ImGui::CalcTextSize(text.c_str()).y;

            ImGui::SetCursorPosX(((DISPLAY_WIDTH - GENERAL_WIDTH) - textWidth) * 0.5f);
            ImGui::SetCursorPosY(((WINDOW_HEIGHT - DISPLAY_HEIGHT) - textHeight) * 0.5f);

            for (int i = 0; i < 16; i++) {
                // Push color if key pressed
                if (chip8->getKey(hexToInt(chip8->chip8Keys[i]))) {
                    ImGui::PushStyleColor(ImGuiCol_Text, GREEN_COLOR);  
                }
                // If character is not last char in line
                if (i % 4 != 3) {
                    ImGui::Text("%c ", chip8->chip8Keys[i]);
                    ImGui::SameLine();
                }
                // If character is last char in line
                else {
                    ImGui::Text("%c", chip8->chip8Keys[i]);
                    ImGui::Text("");
                    ImGui::SetCursorPosX(((DISPLAY_WIDTH - GENERAL_WIDTH) - textWidth) * 0.5f);
                }
                // Pop color if key was pressed
                if (chip8->getKey(hexToInt(chip8->chip8Keys[i]))) {
                    ImGui::PopStyleColor();  
                }
            }
            ImGui::End();
        }
    }
}

int GUI::getWindowID() {
    return SDL_GetWindowID(window);
}

void GUI::forwardOneCycle() {
    chip8->emulateCycle();
}