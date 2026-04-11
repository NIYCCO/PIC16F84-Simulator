#include "PIC16F84.h"

PIC16F84::PIC16F84() : programMemory(), cpu(programMemory) {
}

PIC16F84::~PIC16F84() {
}

void PIC16F84::loadProgram(const std::string& path) {
    programMemory.loadFromFile(path);
    programMemory.printMemory();
    cpu.reset();
}

void PIC16F84::run() {
    while (programMemory.isUsed(cpu.getPC())) {
        cpu.step();
    }
}

void PIC16F84::step() {
    if (programMemory.isUsed(cpu.getPC())) {
        cpu.step();
    }
}

void PIC16F84::reset() {
    cpu.reset();
}

int PIC16F84::getPC() const {
    return cpu.getPC();
}

int PIC16F84::getInstructionRegister() const {
    return cpu.getInstructionRegister();
}

int PIC16F84::getWRegister() const {
    return cpu.getWRegister();
}

int PIC16F84::getStatusRegister() const {
    return cpu.getStatusRegister();
}

int PIC16F84::getLineForAddress(int address) const {
    return programMemory.getLineForAddress(address);
}