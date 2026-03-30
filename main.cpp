#include "ProgramMemory.h"
#include <iostream>

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cout << "Bitte LST-Datei angeben!\n";
        return 1;
    }

    ProgramMemory pm;

    pm.loadFromFile(argv[1]);  // ← Datei kommt aus Terminal
    pm.printMemory();

    return 0;
}