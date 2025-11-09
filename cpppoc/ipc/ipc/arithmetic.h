#pragma once

struct SharedData {
    int num1;
    int num2;
    int result;
    char operation;
};

class ArithmeticOperations {
public:
    static int Calculate(const SharedData* data);
};
