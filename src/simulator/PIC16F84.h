#pragma once

#include "components/CPU.h"
#include "components/ProgramMemory.h"

class PIC16F84 {
    public:
        PIC16F84();
        ~PIC16F84();

        void loadProgram(const std::string& path);
        
        void run();
        void step();
        void reset();

        int getPC() const;
        int getInstructionRegister() const;
        int getWRegister() const;
        int getStatusRegister() const;

        void setWRegister(int value) { cpu.setWRegister(value); }

        int getLineForAddress(int address) const;

    private:
        ProgramMemory programMemory;
        CPU cpu;

};