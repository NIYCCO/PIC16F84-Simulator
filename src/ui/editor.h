#pragma once

#include <string>
#include <unordered_set>

#include "../simulator/PIC16F84.h"
#include "TextEditor.h"


class Editor {
    public:
        Editor(PIC16F84 &pic);

        void openFile(const std::string &path);
        void render(bool simulationRunning);

        std::unordered_set<int> getBreakpoints() const { return breakpoints; }
        
        void displayStepMarker(int line);

        bool handleGoRequest();
        bool handleResetRequest();
        bool goRequested = false;
        bool handleStepInRequest();

    private:
        PIC16F84 *pic;
        TextEditor editor;
        std::unordered_set<int> breakpoints;
        bool stepInRequested = false;
        bool resetRequested = false;
        double quartzFrequencyMHz = 4.0;

        void setBreakpoint(int line);
        void removeBreakpoint(int line);
        void toggleBreakpoint(int line);
        void clearBreakpoints() { breakpoints.clear(); }

};
