#include "ProgramMemory.h"
#include "CPU.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Bitte LST-Datei angeben!\n";
        return 1;
    }

    ProgramMemory pm;
    pm.loadFromFile(argv[1]);

    std::cout << "============================\n";
    std::cout << "PROGRAM MEMORY\n";
    std::cout << "============================\n";
    pm.printMemory();

    std::cout << "\n============================\n";
    std::cout << "CPU TEST\n";
    std::cout << "============================\n";

    CPU cpu(pm);
    cpu.printState();

    std::cout << "\nFuehre 7 Schritte aus:\n";
    for (int i = 0; i < 7; i++) {
        cpu.step();
    }

    std::cout << "\nEndzustand:\n";
    cpu.printState();

    return 0;
}