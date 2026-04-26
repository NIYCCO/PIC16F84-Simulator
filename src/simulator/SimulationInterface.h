#pragma once

#include <string>
#include <GLFW/glfw3.h>

#include "imgui.h"

#include "imgui_memory_editor.h"
//#include "imfilebrowser.h"
#include "../ui/editor.h"

#include "PIC16F84.h"

namespace ImGui {
    class FileBrowser;
}

class SimulationInterface {
    public:
        SimulationInterface();
        ~SimulationInterface();

        void run();
    private:
        GLFWwindow* window;
        ImGui::FileBrowser* fileDialog;
        Editor editor;
        MemoryEditor mem_edit;

        PIC16F84 pic;

        bool isFirstLayout;
        bool showAboutPopup;

        bool initWindow();
        bool initImGui();
        bool isRunning = false;
        void shutdown();

        void render();
        void setupDocking();
        void renderMenuBar();
        void renderPanels();
        void renderAboutPopup();
        void handleFileDialog();

        std::string wregToText(int wreg);

        int getBit(int value, int bit) const {
            return (value >> bit) & 0x01;
        }
};