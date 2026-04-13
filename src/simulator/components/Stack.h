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
};  