#pragma once

#include "components/CPU.h"
#include "components/ProgramMemory.h"

class PIC16F84 {
    public:
        PIC16F84();
        ~PIC16F84();

        void loadProgram(const std::string& path);
        
        void run();

    private:
        CPU cpu;
        ProgramMemory programMemory;

};