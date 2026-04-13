#include "SimulationInterface.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "imfilebrowser.h"

SimulationInterface::SimulationInterface() : window(nullptr), isFirstLayout(true), showAboutPopup(false), editor(pic) {
    fileDialog = new ImGui::FileBrowser();
    fileDialog->SetTitle("Open LST File");
    fileDialog->SetTypeFilters({ ".lst", ".LST" });
}

SimulationInterface::~SimulationInterface() {
    delete fileDialog;
    shutdown();
}

bool SimulationInterface::initWindow() {
    if (!glfwInit()) return false;

    GLFWimage icons[2];
    icons[0].pixels = stbi_load("assets/icon_16.png", &icons[0].width, &icons[0].height, nullptr, 4);
    icons[1].pixels = stbi_load("assets/icon_32.png", &icons[1].width, &icons[1].height, nullptr, 4);

    if (!icons[0].pixels || !icons[1].pixels) {
        std::cerr << "Failed to load icons" << std::endl;
        return false;
    }
    
    window = glfwCreateWindow(1280, 720, "PIC16F84 Simulator", nullptr, nullptr);
    glfwSetWindowIcon(window, 2, icons);
    
    if (icons[0].pixels) stbi_image_free(icons[0].pixels);
    if (icons[1].pixels) stbi_image_free(icons[1].pixels);

    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    return true;
}

bool SimulationInterface::initImGui() {
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

    return true;
}

void SimulationInterface::run() {
    if (!initWindow()) return;
    initImGui();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        render();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }
}

void SimulationInterface::render() {
    bool open = true;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("MainDockSpaceWindow", &open, window_flags);
	ImGui::PopStyleVar();

	ImGui::PopStyleVar(2);

	setupDocking();
    renderMenuBar();
    renderPanels();
    handleFileDialog();
    renderAboutPopup();
}

void SimulationInterface::setupDocking() {
    if (ImGui::DockBuilderGetNode(ImGui::GetID("MyDockspace")) == NULL) {
		ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::DockBuilderRemoveNode(dockspace_id);
		ImGui::DockBuilderAddNode(dockspace_id, 0);

		ImGuiID dock_main_id = dockspace_id;
		ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);
		ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.20f, NULL, &dock_main_id);
		ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.50f, NULL, &dock_main_id);

        ImGuiID dock_id_left_bottom = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.50f, NULL, &dock_id_left);

        ImGui::DockBuilderDockWindow("Memory Editor", dock_id_left);
        ImGui::DockBuilderDockWindow("Spezialfunktionsregister", dock_id_left_bottom);


		ImGui::DockBuilderDockWindow("Properties", dock_main_id);
		ImGui::DockBuilderDockWindow("Stats", dock_id_right);
		ImGui::DockBuilderDockWindow("Viewport", dock_main_id);
        ImGui::DockBuilderDockWindow("Text Editor", dock_id_bottom);
		ImGui::DockBuilderFinish(dockspace_id);
	}

	ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
	ImGui::End();
}

void SimulationInterface::renderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open LST File", "Ctrl+O")) {
                fileDialog->Open();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "Alt+F4")) glfwSetWindowShouldClose(window, true);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Settings")) {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Info")) {
            showAboutPopup = true;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void SimulationInterface::renderPanels() {
    ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoMove);
    ImGui::Text("Details zum ausgewählten Objekt");
    if (ImGui::Button("TEST")) {
        std::cout << "Test clicked!" << std::endl;
        std::cout << "\n=== TEST ===" << std::endl;
        std::cout << "PC vorher: 0x" << std::hex << std::uppercase << pic.getPC() << std::endl;
        std::cout << "IR vorher: 0x" << std::hex << std::uppercase << pic.getInstructionRegister() << std::endl;
        std::cout << "W vorher: 0x" << std::hex << std::uppercase << pic.getWRegister() << std::endl;
        std::cout << "STATUS vorher: 0x" << std::hex << std::uppercase << pic.getStatusRegister() << std::endl;

        pic.step();

        std::cout << "PC nachher: 0x" << std::hex << std::uppercase << pic.getPC() << std::endl;
        std::cout << "IR nachher: 0x" << std::hex << std::uppercase << pic.getInstructionRegister() << std::endl;
        std::cout << "W nachher: 0x" << std::hex << std::uppercase << pic.getWRegister() << std::endl;
        std::cout << "STATUS nachher: 0x" << std::hex << std::uppercase << pic.getStatusRegister() << std::endl;
    }
    ImGui::End();

    ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoMove);
    ImGui::Text("Renderer Stats:\nDraw Calls: 0");
    ImGui::End();

    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoMove);
    ImGui::Text("Hier könnte dein Simulator-Output hin");
    ImGui::End();

    ImGui::Begin("Spezialfunktionsregister", nullptr, ImGuiWindowFlags_NoMove);

    ImGui::SeparatorText("sichtbar");
    ImGui::BeginChild("sichtbar", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
        if (ImGui::BeginTable("tablesichtbar", 2, ImGuiTableFlags_SizingStretchSame)) {

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("W-Reg");
            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            // Hier InputText (Für W-Reg)
            uint8_t wRegVal = (uint8_t)pic.getWRegister();
            ImGui::InputScalar("##wreg", ImGuiDataType_U8, &wRegVal, NULL, NULL, "%02X", ImGuiInputTextFlags_CharsHexadecimal);

            // 3. Prüfen, ob die Eingabe abgeschlossen wurde
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                pic.setWRegister(wRegVal);
            }
            ImGui::PopStyleColor();
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("FSR");
            ImGui::TableNextColumn();
            ImGui::Text("0x00");
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("PCL");
            ImGui::TableNextColumn();
            ImGui::Text("0x00");
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("PCLATH");
            ImGui::TableNextColumn();
            ImGui::Text("0x00");
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Status");
            ImGui::TableNextColumn();
            ImGui::Text("0x00");

            ImGui::EndTable();
        }
        
    ImGui::EndChild();

    ImGui::SeparatorText("versteckt");
    ImGui::BeginChild("versteckt", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
        if (ImGui::BeginTable("tableversteckt", 2, ImGuiTableFlags_SizingStretchSame)) {

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("PC");
            ImGui::TableNextColumn();
            ImGui::Text("0x%02X", pic.getPC());
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Stackpointer");
            ImGui::TableNextColumn();
            ImGui::Text("0x00");
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("VT");
            ImGui::TableNextColumn();
            ImGui::Text("0x00");
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("WDT aktiv");
            ImGui::TableNextColumn();
            bool wdtActive = false; // Hier solltest du den tatsächlichen Status der WDT abfragen
            ImGui::Checkbox("##WDT", &wdtActive);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("WDT");
            ImGui::TableNextColumn();
            ImGui::Text("0x00");

            ImGui::EndTable();
        }
    ImGui::EndChild();

    ImGui::SeparatorText("Stack");
    ImGui::BeginChild("Stack", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
        if (ImGui::BeginTable("tableStack", 2, ImGuiTableFlags_SizingStretchSame)) {
            for (int i = 0; i < 8; ++i) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("0x%02X", i);
                ImGui::TableNextColumn();
                ImGui::Text("0x0000");
            }
            ImGui::EndTable();
        }
    ImGui::EndChild();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 3.0f);

    ImGui::BeginChild("GPR", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
        if (ImGui::BeginTable("tableGPR", 8, ImGuiTableFlags_SizingStretchSame)) {
            ImGui::TableSetupColumn("IRP");
            ImGui::TableSetupColumn("RP");
            ImGui::TableSetupColumn("RP0");
            ImGui::TableSetupColumn("TO");
            ImGui::TableSetupColumn("PD");
            ImGui::TableSetupColumn("Z");
            ImGui::TableSetupColumn("DC");
            ImGui::TableSetupColumn("C");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");

            ImGui::EndTable();
        }
    ImGui::EndChild();

    ImGui::BeginChild("Option", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
        if (ImGui::BeginTable("tableOption", 8, ImGuiTableFlags_SizingStretchSame)) {
            ImGui::TableSetupColumn("RBP");
            ImGui::TableSetupColumn("IntEdg");
            ImGui::TableSetupColumn("T0CS");
            ImGui::TableSetupColumn("T0SE");
            ImGui::TableSetupColumn("PSA");
            ImGui::TableSetupColumn("PS2");
            ImGui::TableSetupColumn("PS1");
            ImGui::TableSetupColumn("PS0");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");

            ImGui::EndTable();
        }
    ImGui::EndChild();

    ImGui::BeginChild("INTCON", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
        if (ImGui::BeginTable("tableINTCON", 8, ImGuiTableFlags_SizingStretchSame)) {
            ImGui::TableSetupColumn("GIE");
            ImGui::TableSetupColumn("PIE");
            ImGui::TableSetupColumn("T0IE");
            ImGui::TableSetupColumn("INTE");
            ImGui::TableSetupColumn("RBIE");
            ImGui::TableSetupColumn("T0IF");
            ImGui::TableSetupColumn("INTF");
            ImGui::TableSetupColumn("RBIF");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");
            ImGui::TableNextColumn();
            ImGui::Text("0");

            ImGui::EndTable();
        }
    ImGui::EndChild();

    ImGui::End();


    editor.render();

    if (editor.handleStepInRequest()) {
        pic.step();
        int line = pic.getLineForAddress(pic.getPC());
        editor.displayStepMarker(line);
    }

    if (editor.handleGoRequest()) {
        isRunning = !isRunning;
    }

    static uint8_t data[256];
    size_t data_size = sizeof(data);
    ImGuiWindowFlags mem_edit_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::Begin("Memory Editor", nullptr, mem_edit_flags);
    mem_edit.Cols = 8;
    mem_edit.OptShowAscii = false;
    mem_edit.OptAddrDigitsCount = 2;
    mem_edit.OptUpperCaseHex = true;
    mem_edit.OptShowOptions = false;
    mem_edit.DrawContents(data, data_size);
    ImGui::End();

    if (isRunning) {
    auto breakpoints = editor.getBreakpoints();

    int currentLine = pic.getLineForAddress(pic.getPC());

    if (breakpoints.find(currentLine) != breakpoints.end()) {
        isRunning = false;
    } else {
        pic.step();
        int line = pic.getLineForAddress(pic.getPC());
        editor.displayStepMarker(line);
    }
}
}

void SimulationInterface::handleFileDialog() {
    fileDialog->Display();

    if(fileDialog->HasSelected()) {
        std::cout << "Selected filename: " << fileDialog->GetSelected().string() << std::endl;
        editor.openFile(fileDialog->GetSelected().string());
        pic.loadProgram(fileDialog->GetSelected().string());
        int line = pic.getLineForAddress(pic.getPC());
        editor.displayStepMarker(line);
        fileDialog->ClearSelected();
    }
}

void SimulationInterface::renderAboutPopup() {
    if (showAboutPopup) {
        ImGui::OpenPopup("About PIC16F84 Simulator");
        showAboutPopup = false;
    }

    if (ImGui::BeginPopupModal("About PIC16F84 Simulator", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("PIC16F84 Simulator - Version 1.0");
        ImGui::Separator();
        ImGui::Text("Created by:");
        ImGui::TextLinkOpenURL("Nico Rigsinger", "https://github.com/NIYCCO");
        ImGui::TextLinkOpenURL("Louis Wunsch", "https://github.com/DerPowerbauer");
        
        if (ImGui::Button("OK")) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

void SimulationInterface::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

std::string SimulationInterface::wregToText(int wreg) {
    char buffer[5];
    snprintf(buffer, sizeof(buffer), "0x%02X", wreg);
    return std::string(buffer);
}