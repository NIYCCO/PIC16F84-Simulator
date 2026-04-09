#include <iostream>
#include <exception>
#include <fstream>

#include "imgui.h"
#include "editor.h"

static const char* defaultText = 
"// Welcome to the PIC16F84 Simulator!\n"
"// Open a .lst file\n";

Editor::Editor() {
    editor.SetText(defaultText);
    editor.SetReadOnlyEnabled(true);

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
        std::ifstream stream(path.c_str());
        std::string text;

        stream.seekg(0, std::ios::end);
        text.reserve(stream.tellg());
        stream.seekg(0, std::ios::beg);

        text.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        stream.close();

        editor.SetText(text);
    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

void Editor::render() {
    ImGui::Begin("Text Editor", nullptr, ImGuiWindowFlags_NoMove);
    editor.Render("TextEditor");
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

void Editor::displayStepMarker(int line) {
    editor.ClearMarkers();
    editor.AddMarker(line-1, 0, IM_COL32(128, 0, 32, 128), "", "");
}