#include "Stack.h"

Stack::Stack() {
    reset();
}

void Stack::reset() {
    stackPointer = 0;
    for (int i = 0; i < 8; i++) {
        stack[i] = 0;
    }
}

void Stack::push(int address) {
    stack[stackPointer] = address;
    stackPointer = (stackPointer + 1) % 8;  // Ringpuffer
}

int Stack::pop() {
    stackPointer = (stackPointer - 1 + 8) % 8;  // Ringpuffer
    return stack[stackPointer];
}