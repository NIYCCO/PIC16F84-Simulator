#include "ProgramMemory.h"
#include <iostream>
#include <fstream>
#include <iomanip>

#include "instructions.h"

ProgramMemory::ProgramMemory() {
    for (int i = 0; i < 1024; i++) {
        memory[i] = 0;
        used[i] = false;
    }
}

void ProgramMemory::loadFromFile(const std::string& filename) {
    std::ifstream file(filename.c_str());
    printf("Lade Programm aus Datei: %s\n", filename.c_str());

    if (!file.is_open()) {
        std::cout << "Fehler beim Oeffnen der Datei!\n";
        return;
    }

    std::string line;

    while (std::getline(file, line)) {
        if (line.length() < 9) {
            continue;
        }

        try {
            std::string address_str = line.substr(0, 4);
            std::string command_str = line.substr(5, 4);

            int address = std::stoi(address_str, nullptr, 16);
            int command = std::stoi(command_str, nullptr, 16);

            if (address >= 0 && address < 1024) {
                memory[address] = command;
                used[address] = true;
            }
        } catch (...) {
            // ungueltige Zeilen ignorieren
        }
    }

    file.close();
}

void ProgramMemory::printMemory() const {
    std::cout << "Eingelesene Befehle:\n";

    for (int i = 0; i < 1024; i++) {
        if (used[i]) {
            std::cout << "Addr 0x"
                      << std::uppercase << std::hex
                      << std::setw(4) << std::setfill('0') << i
                      << " : 0x"
                      << std::setw(4) << std::setfill('0') << memory[i]
                      << std::endl;
        }
    }
}

int ProgramMemory::getInstruction(int address) const {
    if (address < 0 || address >= 1024) {
        return 0;
    }
    return memory[address];
}

bool ProgramMemory::isUsed(int address) const {
    if (address < 0 || address >= 1024) {
        return false;
    }
    return used[address];
}