#include <iostream>
#include <exception>
#include <fstream>
#include <sstream>

#include "imgui.h"
#include "editor.h"

static const char* defaultText = 
"// Welcome to the PIC16F84 Simulator!\n"
"// Open a .lst file\n";

Editor::Editor() {
    editor.SetText(defaultText);
    editor.SetReadOnlyEnabled(true);
    editor.SetShowScrollbarMiniMapEnabled(false);

    editor.SetLineDecorator(18.0f, [this](TextEditor::Decorator &decorator) {
        const int line = decorator.line;
        const bool hasBreakpoint = breakpoints.find(line) != breakpoints.end();

        const float size = decorator.height - 2.0f;
        const ImVec2 buttonSize(size, size);

        ImVec2 pos = ImGui::GetCursorScreenPos();

        ImGui::InvisibleButton("##breakpoint", buttonSize);

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            toggleBreakpoint(line);
        }

        auto* drawList = ImGui::GetWindowDrawList();
        const ImVec2 center(pos.x + size * 0.5f, pos.y + size * 0.5f);
        const float radius = size * 0.35f;

        if (hasBreakpoint) {
            drawList->AddCircleFilled(center, radius, IM_COL32(220, 50, 50, 255));
        } else if (ImGui::IsItemHovered()) {
            drawList->AddCircle(center, radius, IM_COL32(120, 120, 120, 180), 0, 1.5f);
            ImGui::SetTooltip("Toggle breakpoint at line %d", line+1);
        }

    });
}

void Editor::openFile(const std::string &path) {
    try {
        clearBreakpoints();
        addressToLine.clear();
        lineToAddress.clear();

        std::ifstream stream(path.c_str());
        std::string text;

        stream.seekg(0, std::ios::end);
        text.reserve(stream.tellg());
        stream.seekg(0, std::ios::beg);

        text.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        stream.close();

        editor.SetText(text);

        std::istringstream iss(text);
        std::string line;
        int lineNumber = 0;

        while (std::getline(iss, line)) {
            if (line.length() >= 4) {
                try {
                    std::string address_str = line.substr(0, 4);
                    int address = std::stoi(address_str, nullptr, 16);

                    addressToLine[address] = lineNumber;
                    lineToAddress[lineNumber] = address;
                } catch (...) {
                    // Keine echte Befehlszeile, z. B. Kommentar oder Leerzeile
                }
            }
            lineNumber++;
        }

    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

void Editor::render() {
    ImGui::Begin("Text Editor", nullptr, ImGuiWindowFlags_NoMove);
    if (ImGui::Button("Step in", ImVec2(80, 0))) {
        std::cout << "Step in button clicked!" << std::endl;
        stepInRequested = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Step out", ImVec2(80, 0))) {
        std::cout << "Step out button clicked!" << std::endl;
    }
    ImGui::SameLine();
    if (ImGui::Button("Step over", ImVec2(80, 0))) {
        std::cout << "Step over button clicked!" << std::endl;
    }
    ImGui::SameLine();
    if (ImGui::Button("Go", ImVec2(80, 0))) {
        std::cout << "Go button clicked!" << std::endl;
        goRequested = true;
    }
    ImGui::Spacing();
    ImGui::BeginChild("TextEditor", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None);
    editor.Render("TextEditor");
    ImGui::EndChild();
    ImGui::End();
}

void Editor::setBreakpoint(int line) {
    breakpoints.insert(line);
}

void Editor::removeBreakpoint(int line) {
    breakpoints.erase(line);
}

void Editor::toggleBreakpoint(int line) {
    if (breakpoints.find(line) != breakpoints.end()) {
        breakpoints.erase(line);
    } else {
        breakpoints.insert(line);
    }
}

void Editor::displayStepMarkerForAddress(int address) {
    editor.ClearMarkers();

    auto it = addressToLine.find(address);
    if (it != addressToLine.end()) {
        int line = it->second;
        editor.AddMarker(line, 0, IM_COL32(128, 0, 32, 128), "", "");
    }
}

bool Editor::consumeStepInRequest() {
    if (stepInRequested) {
        stepInRequested = false;
        return true;
    }
    return false;
}

std::unordered_set<int> Editor::getBreakpointAddresses() const {
    std::unordered_set<int> result;

    for (int line : breakpoints) {
        auto it = lineToAddress.find(line);
        if (it != lineToAddress.end()) {
            result.insert(it->second);
        }
    }

    return result;
}

bool Editor::consumeGoRequest() {
    if (goRequested) {
        goRequested = false;
        return true;
    }
    return false;
}