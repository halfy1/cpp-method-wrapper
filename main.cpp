#include "wrapper.h"
#include <iostream>
#include <iomanip>

struct Calculator {
    int add(int a, int b) {
        return a + b;
    }

    int multiply(int a, int b) {
        return a * b;
    }

    int subtract(int a, int b) {
        return a - b;
    }

    double divide(double a, double b) {
        if (b == 0) {
            std::cout << "Ошибка: деление на 0" << std::endl;
            return 0.0;
        }
        return a / b;
    }

    int power(int base, int exp) {
        int result = 1;
        for (int i = 0; i < exp; ++i) {
            result *= base;
        }
        return result;
    }
};

int main() {
    Calculator calc;
    Engine engine;

    std::cout << std::fixed << std::setprecision(2);

    engine.register_command(make_wrapper(&calc, &Calculator::add), "add");
    std::cout << "add(10, 20) = " << engine.execute("add", {{"arg1", 10}, {"arg2", 20}}) << std::endl;

    engine.register_command(make_wrapper(&calc, &Calculator::multiply), "mul");
    std::cout << "multiply(5, 6) = " << engine.execute("mul", {{"arg1", 5}, {"arg2", 6}}) << std::endl;

    engine.register_command(make_wrapper(&calc, &Calculator::subtract), "sub");
    std::cout << "subtract(100, 30) = " << engine.execute("sub", {{"arg1", 100}, {"arg2", 30}}) << std::endl;

    engine.register_command(make_wrapper(&calc, &Calculator::divide), "div");
    std::cout << "divide(22, 7) = " << engine.execute("div", {{"arg1", 22.0}, {"arg2", 7.0}}) << std::endl;

    engine.register_command(make_wrapper(&calc, &Calculator::power), "pow");
    std::cout << "power(2, 10) = " << engine.execute("pow", {{"arg1", 2}, {"arg2", 10}}) << std::endl;

    engine.register_command(
        make_wrapper(&calc, &Calculator::add, {{"x", 100}, {"y", 200}}),
        "add_default"
    );
    std::cout << "add с дефолтами: " << engine.execute("add_default") << std::endl;
    std::cout << "add переопределение: " << engine.execute("add_default", {{"x", 5}, {"y", 3}}) << std::endl;

    return 0;
}
