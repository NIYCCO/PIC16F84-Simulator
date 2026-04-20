#pragma once

#include "components/CPU.h"
#include "components/ProgramMemory.h"
#include "components/DataMemory.h"    

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
        int getDataMemory(int address) const;   

        void setWRegister(int value) { cpu.setWRegister(value); }

        int getLineForAddress(int address) const;

        uint8_t* getDataMemory() const { return cpu.getDataMemory(); }

    private:
        ProgramMemory programMemory;
        DataMemory dataMemory;      
        CPU cpu;
};