#include "CPU.h"
#include <iostream>
#include <iomanip>

#include "instructions.h"

CPU::CPU(const ProgramMemory& pm, DataMemory& dm)
    : programMemory(pm), dataMemory(dm), stack() {
    reset();
}

void CPU::reset() {
    pc = 0;
    instructionRegister = 0;
    wRegister = 0;
    dataMemory.write(0x03, 0);
    stack.reset();   
}

int CPU::fetch() {
    instructionRegister = programMemory.getInstruction(pc);
    return instructionRegister;
}

void CPU::setStatusBit(int bit, bool value) {
    int status = dataMemory.read(0x03);
    if (value) {
        status |= (1 << bit);
    } else {
        status &= ~(1 << bit);
    }
    dataMemory.write(0x03, status);
}

bool CPU::getStatusBit(int bit) const {
    return (dataMemory.read(0x03) & (1 << bit)) != 0;
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

void CPU::executeMovwf(int instruction) {
    int address = instruction & 0x7F;
    dataMemory.write(address, wRegister & 0xFF);

    if (address == 0x02) {
        pc = (dataMemory.read(0x0A) << 8) | (wRegister & 0xFF);
        pc %= 1024;
    }
}

void CPU::executeMovf(int instruction) {
    int address = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int value = dataMemory.read(address);
    updateZeroFlag(value);
    if (d == 0) {
        wRegister = value;
    } else {
        dataMemory.write(address, value);
    }
}

void CPU::executeClrf(int instruction) {
    int address = instruction & 0x7F;
    dataMemory.write(address, 0);
    setStatusBit(STATUS_Z, true);
}

void CPU::executeClrw(int instruction) {
    wRegister = 0;
    setStatusBit(STATUS_Z, true);
}

void CPU::executeAddwf(int instruction) {
    int address = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int f = dataMemory.read(address);
    int w = wRegister & 0xFF;

    int result = f + w;
    int result8 = result & 0xFF;

    updateZeroFlag(result8);
    setStatusBit(STATUS_C, result > 0xFF);
    setStatusBit(STATUS_DC, ((f & 0x0F) + (w & 0x0F)) > 0x0F);

    if (d == 0) wRegister = result8;
    else dataMemory.write(address, result8);

    if (d == 1 && address == 0x02) {
        pc = (dataMemory.read(0x0A) << 8) | result8;
        pc %= 1024;
    }
}

void CPU::executeSubwf(int instruction) {
    int address = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int f = dataMemory.read(address);
    int w = wRegister & 0xFF;

    int result = f - w;
    int result8 = result & 0xFF;

    updateZeroFlag(result8);
    // PIC-Sonderverhalten: kein Invertieren des Carry
    setStatusBit(STATUS_C, f >= w);
    setStatusBit(STATUS_DC, (f & 0x0F) >= (w & 0x0F));

    if (d == 0) wRegister = result8;
    else dataMemory.write(address, result8);
}

void CPU::executeComf(int instruction) {
    int address = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int result = (~dataMemory.read(address)) & 0xFF;

    updateZeroFlag(result);

    if (d == 0) wRegister = result;
    else dataMemory.write(address, result);
}

void CPU::executeAndwf(int instruction) {
    int address = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int result = (dataMemory.read(address) & (wRegister & 0xFF)) & 0xFF;

    updateZeroFlag(result);

    if (d == 0) wRegister = result;
    else dataMemory.write(address, result);
}

void CPU::executeIorwf(int instruction) {
    int address = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int result = (dataMemory.read(address) | (wRegister & 0xFF)) & 0xFF;

    updateZeroFlag(result);

    if (d == 0) wRegister = result;
    else dataMemory.write(address, result);
}

void CPU::executeXorwf(int instruction) {
    int address = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int result = (dataMemory.read(address) ^ (wRegister & 0xFF)) & 0xFF;

    updateZeroFlag(result);

    if (d == 0) wRegister = result;
    else dataMemory.write(address, result);
}

void CPU::executeSwapf(int instruction) {
    int address = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int value = dataMemory.read(address);
    int result = ((value & 0x0F) << 4) | ((value & 0xF0) >> 4);


    if (d == 0) wRegister = result;
    else dataMemory.write(address, result);
}

void CPU::executeDecf(int instruction) {
    int address = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int result = (dataMemory.read(address) - 1) & 0xFF;

    updateZeroFlag(result);

    if (d == 0) wRegister = result;
    else dataMemory.write(address, result);
}

void CPU::executeIncf(int instruction) {
    int address = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int result = (dataMemory.read(address) + 1) & 0xFF;

    updateZeroFlag(result);

    if (d == 0) wRegister = result;
    else dataMemory.write(address, result);
}

void CPU::executeDecfsz(int instruction) {
    int address = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int result = (dataMemory.read(address) - 1) & 0xFF;

    if (d == 0) wRegister = result;
    else dataMemory.write(address, result);

    if (result == 0) pc++;  
}

void CPU::executeIncfsz(int instruction) {
    int address = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int result = (dataMemory.read(address) + 1) & 0xFF;

    if (d == 0) wRegister = result;
    else dataMemory.write(address, result);

    if (result == 0) pc++; 
}

void CPU::executeRlf(int instruction) {
    int address = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int value = dataMemory.read(address);
    int oldCarry = getStatusBit(STATUS_C) ? 1 : 0;

    int newCarry = (value >> 7) & 0x01;
    int result = ((value << 1) | oldCarry) & 0xFF;

    setStatusBit(STATUS_C, newCarry);

    if (d == 0) wRegister = result;
    else dataMemory.write(address, result);
}

void CPU::executeRrf(int instruction) {
    int address = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int value = dataMemory.read(address);
    int oldCarry = getStatusBit(STATUS_C) ? 1 : 0;

    int newCarry = value & 0x01;
    int result = ((value >> 1) | (oldCarry << 7)) & 0xFF;

    setStatusBit(STATUS_C, newCarry);

    if (d == 0) wRegister = result;
    else dataMemory.write(address, result);
}

void CPU::executeBcf(int instruction) {
    int address = instruction & 0x7F;
    int bit = (instruction >> 7) & 0x07;
    int value = dataMemory.read(address);
    dataMemory.write(address, value & ~(1 << bit));
}

void CPU::executeBsf(int instruction) {
    int address = instruction & 0x7F;
    int bit = (instruction >> 7) & 0x07;
    int value = dataMemory.read(address);
    dataMemory.write(address, value | (1 << bit));
}

void CPU::executeBtfsc(int instruction) {
    int address = instruction & 0x7F;
    int bit = (instruction >> 7) & 0x07;
    int value = dataMemory.read(address);
    if (((value >> bit) & 0x01) == 0) pc++; 
}

void CPU::executeBtfss(int instruction) {
    int address = instruction & 0x7F;
    int bit = (instruction >> 7) & 0x07;
    int value = dataMemory.read(address);
    if (((value >> bit) & 0x01) == 1) pc++; 
}

void CPU::executeCall(int instruction) {
    int target = instruction & 0x07FF;
    stack.push(pc + 1); 
    pc = target % 1024;
}

void CPU::executeReturn(int instruction) {
    pc = stack.pop();
}

void CPU::executeRetlw(int instruction) {
    int literal = instruction & 0x00FF;
    wRegister = literal & 0xFF;
    pc = stack.pop();
}

void CPU::executeRetfie(int instruction) {
    pc = stack.pop();
    int intcon = dataMemory.read(0x0B);
    dataMemory.write(0x0B, intcon | 0x80);
}

void CPU::executeNop() {
    // macht nichts
}

void CPU::executeSleep() {
    int status = dataMemory.read(0x03);
    status &= ~(1 << 3);  // PD = 0
    status |=  (1 << 4);  // TO = 1
    dataMemory.write(0x03, status);
    // TODO: Simulator in Sleep-Zustand versetzen
}

void CPU::executeClrwdt() {
    int status = dataMemory.read(0x03);
    status |= (1 << 4);  // TO = 1
    status |= (1 << 3);  // PD = 1
    dataMemory.write(0x03, status);
    // TODO: Watchdog-Zähler zurücksetzen wenn Watchdog implementiert
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
    else if ((instruction & 0x3F80) == Instruction::MOVWF) {
        executeMovwf(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::MOVF) {
        executeMovf(instruction);
        pc++;
    }
    else if ((instruction & 0x3F80) == Instruction::CLRF) {
        executeClrf(instruction);
        pc++;
    }
    else if ((instruction & 0x3F80) == Instruction::CLRW) {
        executeClrw(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::ADDWF) {
        executeAddwf(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::SUBWF) {
        executeSubwf(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::COMF) {
        executeComf(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::ANDWF) {
        executeAndwf(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::IORWF) {
        executeIorwf(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::XORWF) {
        executeXorwf(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::SWAPF) {
        executeSwapf(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::DECF) {
        executeDecf(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::INCF) {
        executeIncf(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::DECFSZ) {
        executeDecfsz(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::INCFSZ) {
        executeIncfsz(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::RLF) {
        executeRlf(instruction);
        pc++;
    }
    else if ((instruction & 0x3F00) == Instruction::RRF) {
        executeRrf(instruction);
        pc++;
    }
    else if ((instruction & 0x3C00) == Instruction::BCF) {
        executeBcf(instruction);
        pc++;
    }
    else if ((instruction & 0x3C00) == Instruction::BSF) {
        executeBsf(instruction);
        pc++;
    }
    else if ((instruction & 0x3C00) == Instruction::BTFSC) {
        executeBtfsc(instruction);
        pc++;
    }
    else if ((instruction & 0x3C00) == Instruction::BTFSS) {
        executeBtfss(instruction);
        pc++;
    }
    else if ((instruction & 0x3800) == Instruction::CALL) {
        executeCall(instruction);
    }
    else if (instruction == Instruction::RETURN) {
        executeReturn(instruction);
    }
    else if ((instruction & 0x3F00) == Instruction::RETLW) {
        executeRetlw(instruction);
    }
    else if (instruction == Instruction::RETFIE) {
        executeRetfie(instruction);
    }
    else if (instruction == Instruction::NOP) {
        executeNop();
        pc++;
    }
    else if (instruction == Instruction::SLEEP) {
        executeSleep();
        pc++;
    }
    else if (instruction == Instruction::CLRWDT) {
        executeClrwdt();
        pc++;
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

    dataMemory.write(0x02, pc & 0xFF);

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
              << std::setw(2) << (dataMemory.read(0x03) & 0xFF)
              << std::endl;
}