#pragma once

#include "ProgramMemory.h"

class CPU {
private:
    const ProgramMemory& programMemory;

    int pc;
    int instructionRegister;
    int wRegister;
    int statusRegister;

    static const int STATUS_C  = 0;
    static const int STATUS_DC = 1;
    static const int STATUS_Z  = 2;

    void setStatusBit(int bit, bool value);
    bool getStatusBit(int bit) const;

    void updateZeroFlag(int value);

    void executeMovlw(int instruction);
    void executeAndlw(int instruction);
    void executeIorlw(int instruction);
    void executeXorlw(int instruction);
    void executeSublw(int instruction);
    void executeAddlw(int instruction);
    void executeGoto(int instruction);

    void decodeAndExecute(int instruction);

public:
    CPU(const ProgramMemory& pm);

    void reset();
    int fetch();
    void step();
    void printState() const;
    
    int getPC() const { return pc; }
    int getInstructionRegister() const { return instructionRegister; }
    int getWRegister() const { return wRegister & 0xFF; }
    int getStatusRegister() const { return statusRegister & 0xFF; }

    bool getZeroFlag() const { return getStatusBit(STATUS_Z); }
    bool getDigitCarryFlag() const { return getStatusBit(STATUS_DC); }
    bool getCarryFlag() const { return getStatusBit(STATUS_C); }

    void setWRegister(int value) { wRegister = value & 0xFF; }
};