#pragma once

#include <GLFW/glfw3.h>

#include "imgui.h"

#include "imgui_memory_editor.h"
//#include "imfilebrowser.h"
#include "../ui/editor.h"

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

        bool isFirstLayout;
        bool showAboutPopup;

        bool initWindow();
        bool initImGui();
        void shutdown();

        void render();
        void setupDocking();
        void renderMenuBar();
        void renderPanels();
        void renderAboutPopup();
        void handleFileDialog();
};