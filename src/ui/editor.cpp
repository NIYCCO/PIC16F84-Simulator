#include <iostream>
#include <exception>
#include <fstream>

#include "imgui.h"
#include "editor.h"

static const char* defaultText = 
"// Welcome to the PIC16F84 Simulator!\n"
"// Open a .lst file\n";

Editor::Editor(PIC16F84 &pic) : pic(&pic) {
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
    ImGui::SameLine();
    if (ImGui::Button("Reset", ImVec2(80, 0))) {
        std::cout << "Reset button clicked!" << std::endl;
        pic->reset();
    }

    const char* maxText = "Laufzeit: 00.000,00 \xC2\xB5s";
    ImVec2 maxTextSize = ImGui::CalcTextSize(maxText);
    float fixedButtonWidth = maxTextSize.x + ImGui::GetStyle().FramePadding.x * 2.0f;

    float aktuelleLaufzeit = 0.0f;
    char laufzeitText[64];
    snprintf(laufzeitText, sizeof(laufzeitText), "Laufzeit: %.2f \xC2\xB5s", aktuelleLaufzeit);

    float rightEdgeX = ImGui::GetWindowWidth() - fixedButtonWidth - ImGui::GetStyle().WindowPadding.x;
    ImGui::SameLine(rightEdgeX);

    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_Button));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_Button));

    ImGui::Button(laufzeitText, ImVec2(fixedButtonWidth, 0.0f));

    ImGui::PopStyleColor(2);

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

void Editor::displayStepMarker(int line) {
    editor.ClearMarkers();
    editor.AddMarker(line, 0, IM_COL32( 32,  96, 160, 255), "", "");
    editor.ScrollToLine(line, TextEditor::Scroll::alignMiddle);
}

bool Editor::handleStepInRequest() {
    if (stepInRequested) {
        stepInRequested = false;
        return true;
    }
    return false;
}

bool Editor::handleGoRequest() {
    if (goRequested) {
        goRequested = false;
        return true;
    }
    return false;
}