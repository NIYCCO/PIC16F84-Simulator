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
        uint64_t getExecutedCycles() const;
        double getExecutedTimeUs() const;
        double getWdtCounterUs() const;
        double getWdtTimeoutUs() const;
        int getVtCounter() const;
        bool isWdtEnabled() const;
        bool isSleeping() const;

        int getStackPointer() const { return cpu.getStackPointer(); }
        int getStackValue(int index) const { return cpu.getStackValue(index); }

        void setWRegister(int value) { cpu.setWRegister(value); }
        void setDataMemory(int address, int value) { cpu.setDataMemoryValue(address, value); }
        void setWdtEnabled(bool enabled) { cpu.setWdtEnabled(enabled); }
        void setQuartzFrequencyMHz(double mhz) { cpu.setQuartzFrequencyMHz(mhz); }
        double getQuartzFrequencyMHz() const { return cpu.getQuartzFrequencyMHz(); }

        int getLineForAddress(int address) const;

        uint8_t* getDataMemory() const { return cpu.getDataMemory(); }

    private:
        ProgramMemory programMemory;
        DataMemory dataMemory;      
        CPU cpu;
};
