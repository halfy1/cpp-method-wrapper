#include "wrapper.h"
#include <iostream>
#include <iomanip>

struct Calculator {
    template<typename... Args>
    int add(Args... args) {
        return (args + ...);
    }

    template<typename... Args>
    int multiply(Args... args) {
        return (args * ...);
    }

    template<typename First, typename... Rest>
    int subtract(First first, Rest... rest) {
        return first - (rest + ...);
    }

    template<typename... Args>
    double average(Args... args) {
        return (args + ...) / static_cast<double>(sizeof...(args));
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

    engine.register_command(make_wrapper(&calc, &Calculator::add<int, int>), "add2");
    Value r2 = engine.execute("add2", {{"arg1", 10}, {"arg2", 20}});
    std::cout << "add(10, 20) = " << std::get<int>(r2) << std::endl;

    engine.register_command(make_wrapper(&calc, &Calculator::add<int, int, int>), "add3");
    Value r3 = engine.execute("add3", {{"arg1", 10}, {"arg2", -20}, {"arg3", 30}});
    std::cout << "add(10, -20, 30) = " << std::get<int>(r3) << std::endl;

    engine.register_command(make_wrapper(&calc, &Calculator::add<int, int, int, int, int>), "add5");
    Value r5 = engine.execute("add5", {
        {"arg1", 1}, {"arg2", 2}, {"arg3", 3}, {"arg4", 4}, {"arg5", 5}
    });
    std::cout << "add(1, 2, 3, 4, 5) = " << std::get<int>(r5) << std::endl;

    engine.register_command(make_wrapper(&calc, &Calculator::multiply<int, int>), "mul2");
    Value m2 = engine.execute("mul2", {{"arg1", 5}, {"arg2", 6}});
    std::cout << "multiply(5, 6) = " << std::get<int>(m2) << std::endl;

    engine.register_command(make_wrapper(&calc, &Calculator::multiply<int, int, int>), "mul3");
    Value m3 = engine.execute("mul3", {{"arg1", 2}, {"arg2", 3}, {"arg3", 4}});
    std::cout << "multiply(2, 3, 4) = " << std::get<int>(m3) << std::endl;

    engine.register_command(make_wrapper(&calc, &Calculator::subtract<int, int>), "sub2");
    Value s2 = engine.execute("sub2", {{"arg1", 100}, {"arg2", 30}});
    std::cout << "subtract(100, 30) = " << std::get<int>(s2) << std::endl;

    engine.register_command(make_wrapper(&calc, &Calculator::subtract<int, int, int>), "sub3");
    Value s3 = engine.execute("sub3", {{"arg1", 100}, {"arg2", 20}, {"arg3", 30}});
    std::cout << "subtract(100, 20, 30) = " << std::get<int>(s3) << std::endl;

    engine.register_command(make_wrapper(&calc, &Calculator::average<int, int, int>), "avg3");
    Value avg = engine.execute("avg3", {{"arg1", 80}, {"arg2", 90}, {"arg3", 100}});
    std::cout << "average(80, 90, 100) = " << std::get<double>(avg) << std::endl;

    engine.register_command(make_wrapper(&calc, &Calculator::divide), "div");
    Value div_result = engine.execute("div", {{"arg1", 22.0}, {"arg2", 7.0}});
    std::cout << "divide(22, 7) = " << std::get<double>(div_result) << std::endl;

    engine.register_command(make_wrapper(&calc, &Calculator::power), "pow");
    Value pow_result = engine.execute("pow", {{"arg1", 2}, {"arg2", 10}});
    std::cout << "power(2, 10) = " << std::get<int>(pow_result) << std::endl;

    return 0;
}
