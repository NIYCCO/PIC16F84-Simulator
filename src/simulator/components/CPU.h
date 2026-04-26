#pragma once

#include "ProgramMemory.h"
#include "DataMemory.h"
#include "Stack.h"   

class CPU {
private:
    const ProgramMemory& programMemory;
    DataMemory& dataMemory;       

    int pc;
    int instructionRegister;
    int wRegister;

    Stack stack;  

    

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
    void executeMovwf(int instruction);   
    void executeMovf(int instruction);
    void executeClrf(int instruction);
    void executeClrw(int instruction);   
    void executeAddwf(int instruction);
    void executeSubwf(int instruction);
    void executeComf(int instruction);
    void executeAndwf(int instruction);
    void executeIorwf(int instruction);
    void executeXorwf(int instruction);
    void executeSwapf(int instruction);
    void executeDecf(int instruction);
    void executeIncf(int instruction);
    void executeDecfsz(int instruction);
    void executeIncfsz(int instruction);
    void executeRlf(int instruction);
    void executeRrf(int instruction);
    void executeBcf(int instruction);
    void executeBsf(int instruction);
    void executeBtfsc(int instruction);
    void executeBtfss(int instruction);
    void executeCall(int instruction);
    void executeReturn(int instruction);
    void executeRetlw(int instruction);
    void executeRetfie(int instruction);
    void executeNop();
    void executeSleep();
    void executeClrwdt();

    void decodeAndExecute(int instruction);

    int getRP0() const;
    int normalizeFileAddress(int rawAddress) const;
    int resolveDirectAddress(int f) const;
    int resolveWriteAddress(int f) const;
    int readFileRegister(int f) const;
    void writeFileRegister(int f, int value);
    void refreshPcFromPcl();



public:
    CPU(const ProgramMemory& pm, DataMemory& dm);   

    void reset();
    int fetch();
    void step();
    void printState() const;

    int getPC() const { return pc; }
    int getInstructionRegister() const { return instructionRegister; }
    int getWRegister() const { return wRegister & 0xFF; }
    int getStatusRegister() const { return dataMemory.read(0x03); }  // NEU

    int getStackPointer() const { return stack.getStackPointer(); }
    int getStackValue(int index) const { return stack.getStackValue(index); }

    bool getZeroFlag() const { return getStatusBit(STATUS_Z); }
    bool getDigitCarryFlag() const { return getStatusBit(STATUS_DC); }
    bool getCarryFlag() const { return getStatusBit(STATUS_C); }

    void setWRegister(int value) { wRegister = value & 0xFF; }

    uint8_t* getDataMemory() const { return dataMemory.getMemory(); }
};