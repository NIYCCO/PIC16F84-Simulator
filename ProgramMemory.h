#ifndef PROGRAMMEMORY_H
#define PROGRAMMEMORY_H

#include <string>

class ProgramMemory {
private:
    int memory[1024];
    bool used[1024];

public:
    ProgramMemory();

    void loadFromFile(const std::string& filename);
    void printMemory() const;

    int getInstruction(int address) const;
    bool isUsed(int address) const;
};

#endif