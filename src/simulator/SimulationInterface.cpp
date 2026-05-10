#include "SimulationInterface.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "imfilebrowser.h"

#define TMR0    0x01
#define PCL     0x02
#define STATUS  0x03
#define FSR     0x04
#define PORTA   0x05
#define PORTB   0x06
#define EEDATA  0x08
#define EEADR   0x09
#define PCLATH  0x0A
#define INTCON  0x0B

// Bank 1
#define OPTION_REG 0x81
#define TRISA      0x85
#define TRISB      0x86
#define EECON1     0x88
#define EECON2     0x89

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
		ImGui::DockBuilderDockWindow("EEPROM", dock_id_right);
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
    //ImGui::ShowDemoWindow();
    auto renderEditableRegister = [this](const char* label, const char* id, uint8_t value, auto setter) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%s", label);
        ImGui::TableNextColumn();

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        uint8_t editedValue = value;
        ImGui::InputScalar(id, ImGuiDataType_U8, &editedValue, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            setter(editedValue);
        }
        ImGui::PopStyleColor();
    };
    auto renderRegisterBitEditor = [this](int address, int bit, const char* id) {
        uint8_t bitValue = static_cast<uint8_t>(getBit(pic.getDataMemory(address) & 0xFF, bit));
        char label[2] = { static_cast<char>('0' + bitValue), '\0' };
        char buttonId[64];
        snprintf(buttonId, sizeof(buttonId), "##%s", id);

        if (ImGui::Button(buttonId, ImVec2(24.0f, 0.0f))) {
            bitValue = 1 - bitValue;
            int registerValue = pic.getDataMemory(address) & 0xFF;
            if (bitValue == 1) {
                registerValue |= (1 << bit);
            } else {
                registerValue &= ~(1 << bit);
            }

            if (address == PORTA || address == PORTB) {
                pic.setExternalPortValue(address, registerValue);
            } else {
                pic.setDataMemory(address, registerValue);
            }
        }

        ImVec2 itemMin = ImGui::GetItemRectMin();
        ImVec2 itemMax = ImGui::GetItemRectMax();
        ImVec2 textSize = ImGui::CalcTextSize(label);
        ImVec2 textPos(
            itemMin.x + (itemMax.x - itemMin.x - textSize.x) * 0.5f,
            itemMin.y + (itemMax.y - itemMin.y - textSize.y) * 0.5f
        );
        ImGui::GetWindowDrawList()->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), label);
    };

    ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoMove);
        ImGui::BeginChild("Tris", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);
            if (ImGui::BeginTable("trisAtable", 9, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame)) {
                const char* headers[] = {"RA", "7", "6", "5", "4", "3", "2", "1", "0"};

                ImGui::TableNextRow();
                for (int i = 0; i < 9; i++) {
                    ImGui::TableSetColumnIndex(i);
                    ImGui::TextUnformatted(headers[i]);
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Tris");

                for (int i = 1; i < 9; i++) {
                    ImGui::TableSetColumnIndex(i);
                    ImGui::TextUnformatted("i");
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Pin");

                for (int bit = 7; bit >= 0; --bit) {
                    ImGui::TableSetColumnIndex(8 - bit);
                    char bitId[32];
                    snprintf(bitId, sizeof(bitId), "##porta_bit%d", bit);
                    renderRegisterBitEditor(PORTA, bit, bitId);
                }

                ImGui::EndTable();
            }

            if (ImGui::BeginTable("trisBtable", 9, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame)) {
                const char* headers[] = {"RB", "7", "6", "5", "4", "3", "2", "1", "0"};

                ImGui::TableNextRow();
                for (int i = 0; i < 9; i++) {
                    ImGui::TableSetColumnIndex(i);
                    ImGui::TextUnformatted(headers[i]);
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Tris");

                for (int i = 1; i < 9; i++) {
                    ImGui::TableSetColumnIndex(i);
                    ImGui::TextUnformatted("i");
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Pin");

                for (int bit = 7; bit >= 0; --bit) {
                    ImGui::TableSetColumnIndex(8 - bit);
                    char bitId[32];
                    snprintf(bitId, sizeof(bitId), "##portb_bit%d", bit);
                    renderRegisterBitEditor(PORTB, bit, bitId);
                }

                ImGui::EndTable();
            }
        ImGui::EndChild();
        //ImGui::SameLine();
        ImGui::BeginChild("Lauflicht", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);

            const float ledWidth = 35.0f;
            const float ledHeight = 35.0f;
            const float ledSpacing = 8.0f;
            const float rounding = 0.0f;
            const int portBValue = pic.getDataMemory(PORTB) & 0xFF;

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 startPos = ImGui::GetCursorScreenPos();

            for (int bit = 7; bit >= 0; --bit) {
                bool isOn = ((portBValue >> bit) & 0x01) != 0;
                float offset = static_cast<float>(7 - bit) * (ledWidth + ledSpacing);
                ImVec2 rectMin(startPos.x + offset, startPos.y);
                ImVec2 rectMax(rectMin.x + ledWidth, rectMin.y + ledHeight);

                ImU32 fillColor = isOn
                    ? IM_COL32(220, 40, 40, 255)
                    : IM_COL32(90, 20, 20, 255);
                ImU32 borderColor = isOn
                    ? IM_COL32(255, 120, 120, 255)
                    : IM_COL32(130, 50, 50, 255);

                drawList->AddRectFilled(rectMin, rectMax, fillColor, rounding);
            }

            ImGui::Dummy(ImVec2(8.0f * ledWidth + 7.0f * ledSpacing, ledHeight));
        ImGui::EndChild();

    ImGui::End();

    ImGui::Begin("EEPROM", nullptr, ImGuiWindowFlags_NoMove);
    ImGui::BeginChild("EEPROM", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);

        if (ImGui::BeginTable("tableEEPROM", 2, ImGuiTableFlags_SizingStretchSame)) {
            renderEditableRegister("EEADR", "##eeadr", static_cast<uint8_t>(pic.getDataMemory(EEADR) & 0x3F), [this](uint8_t value) {
                pic.setDataMemory(EEADR, value & 0x3F);
            });

            renderEditableRegister("EEDATA", "##eedata", static_cast<uint8_t>(pic.getDataMemory(EEDATA)), [this](uint8_t value) {
                pic.setDataMemory(EEDATA, value);
            });

            renderEditableRegister("EECON1", "##eecon1", static_cast<uint8_t>(pic.getDataMemory(EECON1) & 0x1F), [this](uint8_t value) {
                pic.setDataMemory(EECON1, value & 0x1F);
            });

            renderEditableRegister("EECON2", "##eecon2", static_cast<uint8_t>(pic.getDataMemory(EECON2)), [this](uint8_t value) {
                pic.setDataMemory(EECON2, value);
            });

            ImGui::EndTable();
        }

        ImGui::Spacing();

        if (ImGui::Button("EEPROM lesen", ImVec2(120.0f, 0.0f))) {
            int eecon1 = pic.getDataMemory(EECON1) & 0x1F;
            eecon1 |= 0x01; // RD
            pic.setDataMemory(EECON1, eecon1);
        }

        ImGui::SameLine();

        if (ImGui::Button("EEPROM schreiben", ImVec2(140.0f, 0.0f))) {
            int eecon1 = pic.getDataMemory(EECON1) & 0x1F;

            pic.setDataMemory(EECON2, 0x55);
            pic.setDataMemory(EECON2, 0xAA);

            eecon1 |= 0x04; // WREN
            pic.setDataMemory(EECON1, eecon1);

            eecon1 |= 0x02; // WR
            pic.setDataMemory(EECON1, eecon1);
        }

        ImGui::Spacing();
        ImGui::Text("Adresse: 0x%02X", pic.getDataMemory(EEADR) & 0x3F);
        ImGui::Text("Daten:   0x%02X", pic.getDataMemory(EEDATA) & 0xFF);
        ImGui::Text("EECON1:  0x%02X", pic.getDataMemory(EECON1) & 0x1F);

    ImGui::EndChild();
    ImGui::End();

    ImGui::Begin("Spezialfunktionsregister", nullptr, ImGuiWindowFlags_NoMove);

    ImGui::SeparatorText("sichtbar");
    ImGui::BeginChild("sichtbar", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
        if (ImGui::BeginTable("tablesichtbar", 2, ImGuiTableFlags_SizingStretchSame)) {
            renderEditableRegister("W-Reg", "##wreg", static_cast<uint8_t>(pic.getWRegister()), [this](uint8_t value) {
                pic.setWRegister(value);
            });
            renderEditableRegister("FSR", "##fsr", static_cast<uint8_t>(pic.getDataMemory(FSR)), [this](uint8_t value) {
                pic.setDataMemory(FSR, value);
            });
            renderEditableRegister("PCL", "##pcl", static_cast<uint8_t>(pic.getPC() & 0xFF), [this](uint8_t value) {
                pic.setDataMemory(PCL, value);
            });
            renderEditableRegister("PCLATH", "##pclath", static_cast<uint8_t>(pic.getDataMemory(PCLATH) & 0x1F), [this](uint8_t value) {
                pic.setDataMemory(PCLATH, value & 0x1F);
            });
            renderEditableRegister("Status", "##status", static_cast<uint8_t>(pic.getStatusRegister()), [this](uint8_t value) {
                pic.setDataMemory(STATUS, value);
            });

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
            ImGui::Text("0x%04X", pic.getPC() & 0x1FFF);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Stackpointer");
            ImGui::TableNextColumn();
            ImGui::Text("0x%02X", pic.getStackPointer());
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("VT");
            ImGui::TableNextColumn();
            ImGui::Text("0x%02X", pic.getVtCounter() & 0xFF);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("WDT aktiv");
            ImGui::TableNextColumn();
            bool wdtActive = pic.isWdtEnabled();
            if (ImGui::Checkbox("##WDT", &wdtActive)) {
                pic.setWdtEnabled(wdtActive);
            }
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("WDT");
            ImGui::TableNextColumn();
            ImGui::Text("%.3f / %.3f us",
                        pic.getWdtCounterUs(),
                        pic.getWdtTimeoutUs());

            ImGui::EndTable();
        }
    ImGui::EndChild();

    ImGui::SeparatorText("Stack");
    ImGui::BeginChild("Stack", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
        if (ImGui::BeginTable("tableStack", 2, ImGuiTableFlags_SizingStretchSame)) {
            const int stackPointer = pic.getStackPointer();
            for (int i = 0; i < 8; ++i) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("0x%02X", i);
                ImGui::TableNextColumn();
                ImGui::Text("0x%04X", pic.getStackValue(i) & 0x1FFF);
            }
            ImGui::EndTable();
        }
    ImGui::EndChild();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 3.0f);

    ImGui::BeginChild("GPR", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
        if (ImGui::BeginTable("tableGPR", 8, ImGuiTableFlags_SizingStretchSame)) {
            ImGui::TableSetupColumn("IRP");
            ImGui::TableSetupColumn("RP1");
            ImGui::TableSetupColumn("RP0");
            ImGui::TableSetupColumn("TO");
            ImGui::TableSetupColumn("PD");
            ImGui::TableSetupColumn("Z");
            ImGui::TableSetupColumn("DC");
            ImGui::TableSetupColumn("C");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            renderRegisterBitEditor(STATUS, 7, "##status_bit7");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(STATUS, 6, "##status_bit6");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(STATUS, 5, "##status_bit5");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(STATUS, 4, "##status_bit4");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(STATUS, 3, "##status_bit3");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(STATUS, 2, "##status_bit2");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(STATUS, 1, "##status_bit1");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(STATUS, 0, "##status_bit0");

            ImGui::EndTable();
        }
    ImGui::EndChild();

    ImGui::Text("OPTION: 0x%02X", (pic.getDataMemory(OPTION_REG) & 0xFF));

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
            renderRegisterBitEditor(OPTION_REG, 7, "##option_bit7");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(OPTION_REG, 6, "##option_bit6");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(OPTION_REG, 5, "##option_bit5");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(OPTION_REG, 4, "##option_bit4");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(OPTION_REG, 3, "##option_bit3");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(OPTION_REG, 2, "##option_bit2");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(OPTION_REG, 1, "##option_bit1");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(OPTION_REG, 0, "##option_bit0");

            ImGui::EndTable();
        }
    ImGui::EndChild();

    ImGui::Text("INTCON: 0x%02X", (pic.getDataMemory(INTCON) & 0xFF));

    ImGui::BeginChild("INTCON", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
        if (ImGui::BeginTable("tableINTCON", 8, ImGuiTableFlags_SizingStretchSame)) {
            ImGui::TableSetupColumn("GIE");
            ImGui::TableSetupColumn("EEIE");
            ImGui::TableSetupColumn("T0IE");
            ImGui::TableSetupColumn("INTE");
            ImGui::TableSetupColumn("RBIE");
            ImGui::TableSetupColumn("T0IF");
            ImGui::TableSetupColumn("INTF");
            ImGui::TableSetupColumn("RBIF");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            renderRegisterBitEditor(INTCON, 7, "##intcon_bit7");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(INTCON, 6, "##intcon_bit6");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(INTCON, 5, "##intcon_bit5");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(INTCON, 4, "##intcon_bit4");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(INTCON, 3, "##intcon_bit3");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(INTCON, 2, "##intcon_bit2");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(INTCON, 1, "##intcon_bit1");
            ImGui::TableNextColumn();
            renderRegisterBitEditor(INTCON, 0, "##intcon_bit0");

            ImGui::EndTable();
        }
    ImGui::EndChild();

    /* ImGui::SeparatorText("EEPROM");
    ImGui::BeginChild("EEPROM", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);

        if (ImGui::BeginTable("tableEEPROM", 2, ImGuiTableFlags_SizingStretchSame)) {
            renderEditableRegister("EEADR", "##eeadr", static_cast<uint8_t>(pic.getDataMemory(EEADR) & 0x3F), [this](uint8_t value) {
                pic.setDataMemory(EEADR, value & 0x3F);
            });

            renderEditableRegister("EEDATA", "##eedata", static_cast<uint8_t>(pic.getDataMemory(EEDATA)), [this](uint8_t value) {
                pic.setDataMemory(EEDATA, value);
            });

            renderEditableRegister("EECON1", "##eecon1", static_cast<uint8_t>(pic.getDataMemory(EECON1) & 0x1F), [this](uint8_t value) {
                pic.setDataMemory(EECON1, value & 0x1F);
            });

            renderEditableRegister("EECON2", "##eecon2", static_cast<uint8_t>(pic.getDataMemory(EECON2)), [this](uint8_t value) {
                pic.setDataMemory(EECON2, value);
            });

            ImGui::EndTable();
        }

        ImGui::Spacing();

        if (ImGui::Button("EEPROM lesen", ImVec2(120.0f, 0.0f))) {
            int eecon1 = pic.getDataMemory(EECON1) & 0x1F;
            eecon1 |= 0x01; // RD
            pic.setDataMemory(EECON1, eecon1);
        }

        ImGui::SameLine();

        if (ImGui::Button("EEPROM schreiben", ImVec2(140.0f, 0.0f))) {
            int eecon1 = pic.getDataMemory(EECON1) & 0x1F;

            pic.setDataMemory(EECON2, 0x55);
            pic.setDataMemory(EECON2, 0xAA);

            eecon1 |= 0x04; // WREN
            pic.setDataMemory(EECON1, eecon1);

            eecon1 |= 0x02; // WR
            pic.setDataMemory(EECON1, eecon1);
        }

        ImGui::Spacing();
        ImGui::Text("Adresse: 0x%02X", pic.getDataMemory(EEADR) & 0x3F);
        ImGui::Text("Daten:   0x%02X", pic.getDataMemory(EEDATA) & 0xFF);
        ImGui::Text("EECON1:  0x%02X", pic.getDataMemory(EECON1) & 0x1F);

    ImGui::EndChild(); */



    ImGui::End();


    editor.render(isRunning);

    if (editor.handleStepInRequest()) {
        pic.step();
        int line = pic.getLineForAddress(pic.getPC());
        editor.displayStepMarker(line);
    }

    if (editor.handleGoRequest()) {
        isRunning = !isRunning;
    }

    if (editor.handleResetRequest()) {
        pic.reset();
        int line = pic.getLineForAddress(pic.getPC());
        editor.displayStepMarker(line);
    }

    ImGuiWindowFlags mem_edit_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::Begin("Memory Editor", nullptr, mem_edit_flags);
    mem_edit.Cols = 8;
    mem_edit.OptShowAscii = false;
    mem_edit.OptAddrDigitsCount = 2;
    mem_edit.OptUpperCaseHex = true;
    mem_edit.OptShowOptions = false;
    mem_edit.DrawContents(pic.getDataMemory(), 256);
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

        ImGui::Separator();
        ImGui::Text("Dokumentation:");
        ImGui::TextLinkOpenURL("Klicken Sie hier", "https://github.com/NIYCCO/PIC16F84-Simulator/blob/main/README.md");

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
