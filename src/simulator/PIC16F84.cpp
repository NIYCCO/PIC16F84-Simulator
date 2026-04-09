#include "PIC16F84.h"

PIC16F84::PIC16F84() : cpu(programMemory) {

}

PIC16F84::~PIC16F84() {

}

void PIC16F84::loadProgram(const std::string& path) {
    programMemory.loadFromFile(path);
    programMemory.printMemory();
}