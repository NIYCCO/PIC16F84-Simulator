#ifndef CPU_H
#define CPU_H

#include "ProgramMemory.h"

class CPU {
private:
    const ProgramMemory& programMemory;

    int pc;
    int instructionRegister;
    int wRegister;

public:
    CPU(const ProgramMemory& pm);

    void reset();
    int fetch();
    void step();
    void printState() const;
};

#endif