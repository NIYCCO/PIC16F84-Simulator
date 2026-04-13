#include "DataMemory.h"

DataMemory::DataMemory() {
    reset();
}

void DataMemory::reset() {
    for (int i = 0; i < 256; i++) {
        memory[i] = 0;
    }
}

int DataMemory::read(int address) const {
    if (address < 0 || address >= 256) return 0;
    return memory[address] & 0xFF;
}

void DataMemory::write(int address, int value) {
    if (address < 0 || address >= 256) return;
    memory[address] = value & 0xFF;
}