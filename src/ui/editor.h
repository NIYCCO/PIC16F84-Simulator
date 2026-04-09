#pragma once

#include <string>
#include <unordered_set>

#include "TextEditor.h"

class Editor {
    public:
        Editor();

        void openFile(const std::string &path);
        void render();

        std::unordered_set<int> getBreakpoints() const { return breakpoints; }
        
        void displayStepMarker(int line);

    private:
        TextEditor editor;
        std::unordered_set<int> breakpoints;

        void setBreakpoint(int line);
        void removeBreakpoint(int line);
        void toggleBreakpoint(int line);
        void clearBreakpoints() { breakpoints.clear(); }

};