#pragma once

class DataMemory {
private:
    int memory[256];

public:
    DataMemory();

    void reset();

    int read(int address) const;
    void write(int address, int value);
};
