#include "PIC16F84.h"

PIC16F84::PIC16F84() : 
programMemory(), dataMemory(), cpu(programMemory, dataMemory) {
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

int PIC16F84::getDataMemory(int address) const {
    return dataMemory.read(address);
}

uint64_t PIC16F84::getExecutedCycles() const {
    return cpu.getExecutedCycles();
}

double PIC16F84::getExecutedTimeUs() const {
    return cpu.getExecutedTimeUs();
}

double PIC16F84::getWdtCounterUs() const {
    return cpu.getWdtCounterUs();
}

double PIC16F84::getWdtTimeoutUs() const {
    return cpu.getWdtTimeoutUs();
}

int PIC16F84::getVtCounter() const {
    return cpu.getVtCounter();
}

bool PIC16F84::isWdtEnabled() const {
    return cpu.isWdtEnabled();
}

bool PIC16F84::isSleeping() const {
    return cpu.isSleeping();
}
