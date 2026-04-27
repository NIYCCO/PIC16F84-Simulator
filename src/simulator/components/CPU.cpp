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

    // Timer/Prescaler interner Zustand
    timerPrescalerCounter = 0;
    tmr0WrittenThisStep = false;

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

int CPU::getRP0() const {
    return (dataMemory.read(0x03) >> 5) & 0x01;
}

int CPU::normalizeFileAddress(int rawAddress) const {
    int a = rawAddress & 0xFF;

    // SFR-Spiegelungen Bank1 -> Bank0
    switch (a) {
        case 0x80: return 0x00; // INDF
        case 0x82: return 0x02; // PCL
        case 0x83: return 0x03; // STATUS
        case 0x84: return 0x04; // FSR
        case 0x8A: return 0x0A; // PCLATH
        case 0x8B: return 0x0B; // INTCON
        default: break;
    }

    // GPR-Spiegelung 0x8C..0xCF -> 0x0C..0x4F
    if (a >= 0x8C && a <= 0xCF) return a - 0x80;

    return a;
}

int CPU::resolveDirectAddress(int f) const {
    int bankOffset = getRP0() ? 0x80 : 0x00;
    int raw = bankOffset | (f & 0x7F);
    return normalizeFileAddress(raw);
}

int CPU::resolveWriteAddress(int f) const {
    int direct = resolveDirectAddress(f);

    if (direct != 0x00) {
        return direct;
    }

    // indirekt über INDF -> FSR
    int fsr = dataMemory.read(0x04) & 0xFF;
    return normalizeFileAddress(fsr);
}

int CPU::readFileRegister(int f) const {
    int direct = resolveDirectAddress(f);

    if (direct != 0x00) {
        return dataMemory.read(direct);
    }

    // INDF: indirekter Zugriff über FSR
    int fsr = dataMemory.read(0x04) & 0xFF;
    int indirect = normalizeFileAddress(fsr);

    // INDF auf INDF liefert 0
    if (indirect == 0x00) return 0;

    return dataMemory.read(indirect);
}

void CPU::writeFileRegister(int f, int value) {
    int v = value & 0xFF;
    int direct = resolveDirectAddress(f);

    if (direct != 0x00) {
        dataMemory.write(direct, v);

        // Schreibzugriff auf TMR0:
        // - Prescaler wird gelöscht
        // - für diesen Schritt kein zusätzlicher Timer-Tick
        if (direct == 0x01) {
            timerPrescalerCounter = 0;
            tmr0WrittenThisStep = true;
        }
        return;
    }

    // INDF: indirekter Zugriff über FSR
    int fsr = dataMemory.read(0x04) & 0xFF;
    int indirect = normalizeFileAddress(fsr);

    // INDF auf INDF: no-op
    if (indirect == 0x00) return;

    dataMemory.write(indirect, v);

    // Auch indirekter Zugriff auf TMR0 muss Prescaler löschen
    if (indirect == 0x01) {
        timerPrescalerCounter = 0;
        tmr0WrittenThisStep = true;
    }
}


void CPU::refreshPcFromPcl() {
    int pcl = dataMemory.read(0x02) & 0xFF;
    int pclath = dataMemory.read(0x0A) & 0x1F;
    pc = ((pclath << 8) | pcl) & 0x03FF;
}

bool CPU::isTimerClockInternal() const {
    // OPTION bit5 = T0CS; 0 => interner Instruktions-Takt
    int option = dataMemory.read(0x81);
    return ((option >> 5) & 0x01) == 0;
}

int CPU::getTimerPrescalerDivisor() const {
    // OPTION bits 2..0 = PS2..PS0
    // Für Timer0 gilt bei PSA=0: 000=>1:2 ... 111=>1:256
    int option = dataMemory.read(0x81);
    int ps = option & 0x07;
    return (1 << (ps + 1)); // 2,4,8,...,256
}

void CPU::incrementTimer0() {
    int tmr0 = dataMemory.read(0x01) & 0xFF;
    int next = (tmr0 + 1) & 0xFF;
    dataMemory.write(0x01, next);

    // Overflow: 0xFF -> 0x00 setzt T0IF (INTCON bit2)
    if (next == 0x00) {
        int intcon = dataMemory.read(0x0B);
        intcon |= (1 << 2); // T0IF
        dataMemory.write(0x0B, intcon);
    }
}

void CPU::tickTimer0() {
    // Wenn in diesem Schritt auf TMR0 geschrieben wurde:
    // kein zusätzlicher Tick im selben Schritt.
    if (tmr0WrittenThisStep) return;

    // Externer Takt (T0CS=1) wird in diesem Schritt noch nicht simuliert.
    if (!isTimerClockInternal()) return;

    int option = dataMemory.read(0x81);
    bool psa = ((option >> 3) & 0x01) != 0; // PSA=1 => Prescaler bei WDT

    if (psa) {
        // Timer ohne Prescaler -> pro CPU-Schritt ein Tick
        incrementTimer0();
    } else {
        // Prescaler gehört zum Timer
        timerPrescalerCounter++;
        if (timerPrescalerCounter >= getTimerPrescalerDivisor()) {
            timerPrescalerCounter = 0;
            incrementTimer0();
        }
    }
}

bool CPU::isGlobalInterruptEnabled() const {
    // INTCON bit7 = GIE
    return (dataMemory.read(0x0B) & (1 << 7)) != 0;
}

bool CPU::isTimer0InterruptEnabled() const {
    // INTCON bit5 = T0IE
    return (dataMemory.read(0x0B) & (1 << 5)) != 0;
}

bool CPU::isTimer0InterruptFlagSet() const {
    // INTCON bit2 = T0IF
    return (dataMemory.read(0x0B) & (1 << 2)) != 0;
}

bool CPU::shouldTriggerTimer0Interrupt() const {
    return isGlobalInterruptEnabled()
        && isTimer0InterruptEnabled()
        && isTimer0InterruptFlagSet();
}

void CPU::enterInterrupt() {
    // Rückkehradresse sichern (nächster Befehl)
    stack.push(pc & 0x3FF);

    // GIE löschen (PIC sperrt weitere Interrupts beim Eintritt)
    int intcon = dataMemory.read(0x0B);
    intcon &= ~(1 << 7);
    dataMemory.write(0x0B, intcon);

    // ISR-Einsprungadresse
    pc = 0x0004;
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
    int f = instruction & 0x7F;
    int destination = resolveWriteAddress(f);

    writeFileRegister(f, wRegister & 0xFF);

    if (destination == 0x02) {
        refreshPcFromPcl();
    }
}


void CPU::executeMovf(int instruction) {
    int f = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int value = readFileRegister(f);

    updateZeroFlag(value);

    if (d == 0) {
        wRegister = value & 0xFF;
    } else {
        int destination = resolveWriteAddress(f);
        writeFileRegister(f, value);
        if (destination == 0x02) {
            refreshPcFromPcl();
        }
    }
}


void CPU::executeClrf(int instruction) {
    int f = instruction & 0x7F;
    writeFileRegister(f, 0);
    setStatusBit(STATUS_Z, true);
}


void CPU::executeClrw(int instruction) {
    wRegister = 0;
    setStatusBit(STATUS_Z, true);
}

void CPU::executeAddwf(int instruction) {
    int f = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int fileVal = readFileRegister(f);
    int w = wRegister & 0xFF;

    int result = fileVal + w;
    int result8 = result & 0xFF;

    updateZeroFlag(result8);
    setStatusBit(STATUS_C, result > 0xFF);
    setStatusBit(STATUS_DC, ((fileVal & 0x0F) + (w & 0x0F)) > 0x0F);

    if (d == 0) {
        wRegister = result8;
    } else {
        int destination = resolveWriteAddress(f);
        writeFileRegister(f, result8);

        if (destination == 0x02) {
            refreshPcFromPcl();
        }
    }
}


void CPU::executeSubwf(int instruction) {
    int f = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int fileVal = readFileRegister(f);
    int w = wRegister & 0xFF;

    int result = fileVal - w;
    int result8 = result & 0xFF;

    updateZeroFlag(result8);
    setStatusBit(STATUS_C, fileVal >= w);
    setStatusBit(STATUS_DC, (fileVal & 0x0F) >= (w & 0x0F));

    if (d == 0) wRegister = result8;
    else writeFileRegister(f, result8);
}

void CPU::executeComf(int instruction) {
    int f = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int result = (~readFileRegister(f)) & 0xFF;

    updateZeroFlag(result);

    if (d == 0) {
        wRegister = result;
    } else {
        int destination = resolveWriteAddress(f);
        writeFileRegister(f, result);
        if (destination == 0x02) {
            refreshPcFromPcl();
        }
    }
}

void CPU::executeAndwf(int instruction) {
    int f = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int result = (readFileRegister(f) & (wRegister & 0xFF)) & 0xFF;

    updateZeroFlag(result);

    if (d == 0) {
        wRegister = result;
    } else {
        int destination = resolveWriteAddress(f);
        writeFileRegister(f, result);
        if (destination == 0x02) {
            refreshPcFromPcl();
        }
    }
}


void CPU::executeIorwf(int instruction) {
    int f = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int result = (readFileRegister(f) | (wRegister & 0xFF)) & 0xFF;

    updateZeroFlag(result);

    if (d == 0) {
        wRegister = result;
    } else {
        int destination = resolveWriteAddress(f);
        writeFileRegister(f, result);
        if (destination == 0x02) {
            refreshPcFromPcl();
        }
    }
}

void CPU::executeXorwf(int instruction) {
    int f = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int result = (readFileRegister(f) ^ (wRegister & 0xFF)) & 0xFF;

    updateZeroFlag(result);

    if (d == 0) {
        wRegister = result;
    } else {
        int destination = resolveWriteAddress(f);
        writeFileRegister(f, result);
        if (destination == 0x02) {
            refreshPcFromPcl();
        }
    }
}

void CPU::executeSwapf(int instruction) {
    int f = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int value = readFileRegister(f);
    int result = ((value & 0x0F) << 4) | ((value & 0xF0) >> 4);

    if (d == 0) {
        wRegister = result;
    } else {
        int destination = resolveWriteAddress(f);
        writeFileRegister(f, result);
        if (destination == 0x02) {
            refreshPcFromPcl();
        }
    }
}

void CPU::executeDecf(int instruction) {
    int f = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int result = (readFileRegister(f) - 1) & 0xFF;

    updateZeroFlag(result);

    if (d == 0) {
        wRegister = result;
    } else {
        int destination = resolveWriteAddress(f);
        writeFileRegister(f, result);
        if (destination == 0x02) {
            refreshPcFromPcl();
        }
    }
}

void CPU::executeIncf(int instruction) {
    int f = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int result = (readFileRegister(f) + 1) & 0xFF;

    updateZeroFlag(result);

    if (d == 0) {
        wRegister = result;
    } else {
        int destination = resolveWriteAddress(f);
        writeFileRegister(f, result);
        if (destination == 0x02) {
            refreshPcFromPcl();
        }
    }
}

void CPU::executeDecfsz(int instruction) {
    int f = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int result = (readFileRegister(f) - 1) & 0xFF;

    if (d == 0) {
        wRegister = result;
    } else {
        int destination = resolveWriteAddress(f);
        writeFileRegister(f, result);
        if (destination == 0x02) {
            refreshPcFromPcl();
        }
    }

    if (result == 0) pc++;
}

void CPU::executeIncfsz(int instruction) {
    int f = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int result = (readFileRegister(f) + 1) & 0xFF;

    if (d == 0) {
        wRegister = result;
    } else {
        int destination = resolveWriteAddress(f);
        writeFileRegister(f, result);
        if (destination == 0x02) {
            refreshPcFromPcl();
        }
    }

    if (result == 0) pc++;
}

void CPU::executeRlf(int instruction) {
    int f = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int value = readFileRegister(f);
    int oldCarry = getStatusBit(STATUS_C) ? 1 : 0;

    int newCarry = (value >> 7) & 0x01;
    int result = ((value << 1) | oldCarry) & 0xFF;

    setStatusBit(STATUS_C, newCarry);

    if (d == 0) {
        wRegister = result;
    } else {
        int destination = resolveWriteAddress(f);
        writeFileRegister(f, result);
        if (destination == 0x02) {
            refreshPcFromPcl();
        }
    }
}


void CPU::executeRrf(int instruction) {
    int f = instruction & 0x7F;
    int d = (instruction >> 7) & 0x01;
    int value = readFileRegister(f);
    int oldCarry = getStatusBit(STATUS_C) ? 1 : 0;

    int newCarry = value & 0x01;
    int result = ((value >> 1) | (oldCarry << 7)) & 0xFF;

    setStatusBit(STATUS_C, newCarry);

    if (d == 0) {
        wRegister = result;
    } else {
        int destination = resolveWriteAddress(f);
        writeFileRegister(f, result);
        if (destination == 0x02) {
            refreshPcFromPcl();
        }
    }
}

void CPU::executeBcf(int instruction) {
    int f = instruction & 0x7F;
    int bit = (instruction >> 7) & 0x07;
    int value = readFileRegister(f);
    int destination = resolveWriteAddress(f);

    writeFileRegister(f, value & ~(1 << bit));

    if (destination == 0x02) {
        refreshPcFromPcl();
    }
}

void CPU::executeBsf(int instruction) {
    int f = instruction & 0x7F;
    int bit = (instruction >> 7) & 0x07;
    int value = readFileRegister(f);
    int destination = resolveWriteAddress(f);

    writeFileRegister(f, value | (1 << bit));

    if (destination == 0x02) {
        refreshPcFromPcl();
    }
}


void CPU::executeBtfsc(int instruction) {
    int f = instruction & 0x7F;
    int bit = (instruction >> 7) & 0x07;
    int value = readFileRegister(f);

    if (((value >> bit) & 0x01) == 0) pc++;
}

void CPU::executeBtfss(int instruction) {
    int f = instruction & 0x7F;
    int bit = (instruction >> 7) & 0x07;
    int value = readFileRegister(f);

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
    tmr0WrittenThisStep = false;
    decodeAndExecute(instructionRegister);
    tickTimer0();

    if (shouldTriggerTimer0Interrupt()) {
        enterInterrupt();
    }

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