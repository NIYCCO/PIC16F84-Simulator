#pragma once

#include <cstdint>

class DataMemory {
private:
    uint8_t memory[256];

public:
    DataMemory();

    void reset();

    int read(int address) const;
    void write(int address, int value);

    //int* getMemory() { return memory; }

    uint8_t* getMemory() { return memory; }
    const uint8_t* getMemory() const { return memory; }
};
