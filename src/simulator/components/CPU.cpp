#include "CPU.h"
#include <iostream>
#include <iomanip>

#include "instructions.h"

CPU::CPU(const ProgramMemory& pm) : programMemory(pm) {
    reset();
}

void CPU::reset() {
    pc = 0;
    instructionRegister = 0;
    wRegister = 0;
    statusRegister = 0;
}

int CPU::fetch() {
    instructionRegister = programMemory.getInstruction(pc);
    return instructionRegister;
}

void CPU::setStatusBit(int bit, bool value) {
    if (value) {
        statusRegister |= (1 << bit);
    } else {
        statusRegister &= ~(1 << bit);
    }
}

bool CPU::getStatusBit(int bit) const {
    return (statusRegister & (1 << bit)) != 0;
}

void CPU::updateZeroFlag(int value) {
    setStatusBit(STATUS_Z, (value & 0xFF) == 0);
}

void CPU::executeMovlw(int instruction) {
    int literal = instruction & 0x00FF;
    wRegister = literal & 0xFF;
}

void CPU::executeAndlw(int instruction) {
    int literal = instruction & 0x00FF;
    wRegister = (wRegister & literal) & 0xFF;
    updateZeroFlag(wRegister);
}

void CPU::executeIorlw(int instruction) {
    int literal = instruction & 0x00FF;
    wRegister = (wRegister | literal) & 0xFF;
    updateZeroFlag(wRegister);
}

void CPU::executeXorlw(int instruction) {
    int literal = instruction & 0x00FF;
    wRegister = (wRegister ^ literal) & 0xFF;
    updateZeroFlag(wRegister);
}

void CPU::executeSublw(int instruction) {
    int literal = instruction & 0x00FF;
    int oldW = wRegister & 0xFF;

    int result = literal - oldW;
    int result8 = result & 0xFF;

    wRegister = result8;

    updateZeroFlag(result8);

    // PIC-Verhalten bei SUBLW:
    // C gesetzt, wenn literal >= W
    // DC gesetzt, wenn unteres Nibble kein Borrow erzeugt
    setStatusBit(STATUS_C, literal >= oldW);
    setStatusBit(STATUS_DC, (literal & 0x0F) >= (oldW & 0x0F));
}

void CPU::executeAddlw(int instruction) {
    int literal = instruction & 0x00FF;
    int oldW = wRegister & 0xFF;

    int result = oldW + literal;
    int result8 = result & 0xFF;

    wRegister = result8;

    updateZeroFlag(result8);
    setStatusBit(STATUS_C, result > 0xFF);
    setStatusBit(STATUS_DC, ((oldW & 0x0F) + (literal & 0x0F)) > 0x0F);
}

void CPU::executeGoto(int instruction) {
    int target = instruction & 0x07FF;
    pc = target % 1024;
}

void CPU::decodeAndExecute(int instruction) {
    if ((instruction & 0x3F00) == Instruction::MOVLW) {
        executeMovlw(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::ANDLW) {
        executeAndlw(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::IORLW) {
        executeIorlw(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::XORLW) {
        executeXorlw(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::SUBLW) {
        executeSublw(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::ADDLW) {
        executeAddlw(instruction);
        pc++;
    }
    else if ((instruction & 0x3800) == Instruction::GOTO) {
        executeGoto(instruction);
    }
    else {
        std::cout << "Unbekannter oder noch nicht implementierter Befehl: 0x"
                  << std::uppercase << std::hex
                  << std::setw(4) << std::setfill('0') << instruction
                  << std::endl;
        pc++;
    }

    pc %= 1024;
}

void CPU::step() {
    fetch();

    std::cout << "PC=0x"
              << std::uppercase << std::hex
              << std::setw(4) << std::setfill('0') << pc
              << "  INST=0x"
              << std::setw(4) << instructionRegister
              << std::endl;

    decodeAndExecute(instructionRegister);

    std::cout << "    W=0x"
              << std::setw(2) << (wRegister & 0xFF)
              << "  STATUS(Z DC C)= "
              << getStatusBit(STATUS_Z) << " "
              << getStatusBit(STATUS_DC) << " "
              << getStatusBit(STATUS_C)
              << std::endl;
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
              << "\nSTATUS = 0x"
              << std::setw(2) << (statusRegister & 0xFF)
              << std::endl;
}