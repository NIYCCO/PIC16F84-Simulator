#include <iostream>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "imfilebrowser.h"
#include "ui/editor.h"

int main() {
    if (!glfwInit()) return -1;

    GLFWimage icons[2];
    icons[0].pixels = stbi_load("assets/icon_16.png", &icons[0].width, &icons[0].height, nullptr, 4);
    icons[1].pixels = stbi_load("assets/icon_32.png", &icons[1].width, &icons[1].height, nullptr, 4);

    if (!icons[0].pixels || !icons[1].pixels) {
        std::cerr << "Failed to load icons" << std::endl;
        return -1;
    }
    
    GLFWwindow* window = glfwCreateWindow(1280, 720, "PIC16F84 Simulator", nullptr, nullptr);
    glfwSetWindowIcon(window, 2, icons);
    
    if (icons[0].pixels) stbi_image_free(icons[0].pixels);
    if (icons[1].pixels) stbi_image_free(icons[1].pixels);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImGui::FileBrowser fileDialog;

    fileDialog.SetTitle("Open LST File");
    fileDialog.SetTypeFilters({ ".lst", ".LST" });

    Editor editor;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open LST File", "Ctrl+O")) fileDialog.Open();
                ImGui::Separator();
                if (ImGui::MenuItem("Quit", "Alt+F4")) glfwSetWindowShouldClose(window, true);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Settings")) {
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Info")) {
                ImGui::OpenPopup("About PIC16F84 Simulator");
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        fileDialog.Display();

        if(fileDialog.HasSelected()) {
            std::cout << "Selected filename: " << fileDialog.GetSelected().string() << std::endl;
            editor.openFile(fileDialog.GetSelected().string());
            fileDialog.ClearSelected();
        }

        bool show_demo_window = true;
        ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::Begin("Scene Hierarchy");
        ImGui::Text("Hier stehen deine Entities");
        ImGui::End();

        ImGui::Begin("Properties");
        ImGui::Text("Details zum ausgewählten Objekt");
        ImGui::End();

        ImGui::Begin("Stats");
        ImGui::Text("Renderer Stats:");
        ImGui::Text("Draw Calls: 0");
        ImGui::End();

        ImGui::Begin("Content Browser");
        ImGui::Text("folder/");
        ImGui::Text("main.asm");
        ImGui::End();

        ImGui::Begin("Viewport");
        ImGui::Text("Hier könnte dein Simulator-Output hin");
        ImGui::End();

        editor.render();

        if (ImGui::BeginPopupModal("About PIC16F84 Simulator", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("PIC16F84 Simulator - Version 1.0");
            ImGui::Separator();
            ImGui::Text("Created by:");
            ImGui::TextLinkOpenURL("Nico Rigsinger", "https://github.com/NIYCCO");
            ImGui::TextLinkOpenURL("Louis Wunsch", "https://github.com/DerPowerbauer");
            if (ImGui::Button("OK")) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    return 0;
}