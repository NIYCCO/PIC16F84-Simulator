#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>

#include "TextEditor.h"


class Editor {
    public:
        Editor();

        void openFile(const std::string &path);
        void render();

        std::unordered_set<int> getBreakpoints() const { return breakpoints; }
        std::unordered_set<int> getBreakpointAddresses() const;
        
        void displayStepMarkerForAddress(int address);

        bool consumeGoRequest();
        bool goRequested = false;
        bool consumeStepInRequest();

    private:
        TextEditor editor;
        std::unordered_set<int> breakpoints;
        std::unordered_map<int, int> addressToLine;
        std::unordered_map<int, int> lineToAddress;
        bool stepInRequested = false;

        void setBreakpoint(int line);
        void removeBreakpoint(int line);
        void toggleBreakpoint(int line);
        void clearBreakpoints() { breakpoints.clear(); }

};