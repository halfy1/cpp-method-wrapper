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
    
    engine.register_command(
        make_wrapper(&calc, &Calculator::add<int, int>, {{"arg1", 0}, {"arg2", 0}}),
        "add"
    );
    Value r1 = engine.execute("add", {{"arg1", 10}, {"arg2", 20}});
    std::cout << "add(10, 20) = " << std::get<int>(r1) << std::endl;
    
    Value r2 = engine.execute("add", {{"arg2", 30}, {"arg1", 15}});
    std::cout << "add(arg2=30, arg1=15) = " << std::get<int>(r2) << std::endl;
    
    Value r3 = engine.execute("add", {{"arg1", 100}});
    std::cout << "add(100, default=0) = " << std::get<int>(r3) << std::endl;
    
    Value r4 = engine.execute("add", {});
    std::cout << "add(default=0, default=0) = " << std::get<int>(r4) << std::endl << std::endl;
    

    engine.register_command(
        make_wrapper(&calc, &Calculator::multiply<int, int>, {{"x", 1}, {"y", 1}}),
        "mul"
    );
    Value m1 = engine.execute("mul", {{"x", 5}, {"y", 6}});
    std::cout << "multiply(x=5, y=6) = " << std::get<int>(m1) << std::endl;
    
    Value m2 = engine.execute("mul", {{"y", 10}});
    std::cout << "multiply(x=default=1, y=10) = " << std::get<int>(m2) << std::endl << std::endl;
    

    engine.register_command(
        make_wrapper(&calc, &Calculator::add<int, int, int>, 
                    {{"first", 0}, {"second", 0}, {"third", 0}}),
        "add3"
    );
    Value a1 = engine.execute("add3", {{"first", 10}, {"second", 20}, {"third", 30}});
    std::cout << "add(10, 20, 30) = " << std::get<int>(a1) << std::endl;
    
    Value a2 = engine.execute("add3", {{"third", 5}, {"first", 1}, {"second", 2}});
    std::cout << "add(first=1, second=2, third=5) = " << std::get<int>(a2) << std::endl;
    
    Value a3 = engine.execute("add3", {{"first", 100}, {"third", 50}});
    std::cout << "add(100, default=0, 50) = " << std::get<int>(a3) << std::endl << std::endl;
    

    engine.register_command(
        make_wrapper(&calc, &Calculator::divide, {{"dividend", 1.0}, {"divisor", 1.0}}),
        "div"
    );
    Value d1 = engine.execute("div", {{"dividend", 22.0}, {"divisor", 7.0}});
    std::cout << "divide(22, 7) = " << std::get<double>(d1) << std::endl;
    
    Value d2 = engine.execute("div", {{"divisor", 2.0}});
    std::cout << "divide(default=1.0, 2.0) = " << std::get<double>(d2) << std::endl;
    
    return 0;
}
