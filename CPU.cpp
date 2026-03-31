#include "CPU.h"
#include <iostream>
#include <iomanip>

CPU::CPU(const ProgramMemory& pm) : programMemory(pm) {
    reset();
}

void CPU::reset() {
    pc = 0;
    instructionRegister = 0;
    wRegister = 0;
}

int CPU::fetch() {
    instructionRegister = programMemory.getInstruction(pc);
    return instructionRegister;
}

void CPU::step() {
    fetch();

    std::cout << "PC = 0x"
              << std::uppercase << std::hex
              << std::setw(4) << std::setfill('0') << pc
              << "   Instruction = 0x"
              << std::setw(4) << instructionRegister
              << std::endl;

    pc = (pc + 1) % 1024;
}

void CPU::printState() const {
    std::cout << "CPU-Zustand:\n";
    std::cout << "PC = 0x"
              << std::uppercase << std::hex
              << std::setw(4) << std::setfill('0') << pc
              << "\nIR = 0x"
              << std::setw(4) << instructionRegister
              << "\nW  = 0x"
              << std::setw(2) << (wRegister & 0xFF)
              << std::endl;
}