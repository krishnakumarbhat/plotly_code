#include "arithmetic.h"

int ArithmeticOperations::Calculate(const SharedData* data) {
    switch (data->operation) {
    case '+':
        return data->num1 + data->num2;
    case '-':
        return data->num1 - data->num2;
    case '*':
        return data->num1 * data->num2;
    case '/':
        return (data->num2 != 0) ? (data->num1 / data->num2) : 0;
    default:
        return 0;
    }
}
