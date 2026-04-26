#pragma once

class Stack {
private:
    int stack[8];
    int stackPointer;

public:
    Stack();

    void reset();
    void push(int address);
    int pop();

    int getStackPointer() const {
        return stackPointer;
    }

    int getStackValue(int index) const {
        if (index < 0 || index >= 8) {
            return 0;
        }
        return stack[index];
    }
};  