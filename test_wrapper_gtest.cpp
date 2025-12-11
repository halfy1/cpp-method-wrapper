#include "wrapper.h"
#include <gtest/gtest.h>
#include <stdexcept>

class TestClass {
public:
    int add(int a, int b) {
        return a + b;
    }

    int multiply(int a, int b, int c) {
        return a * b * c;
    }

    double divide(double a, double b) {
        if (b == 0) throw std::runtime_error("Деление на 0");
        return a / b;
    }

    void print_message(int code) {
        std::cout << "Message code: " << code << std::endl;
    }

    int single_arg(int x) {
        return x * 2;
    }

    int five_args(int a, int b, int c, int d, int e) {
        return a + b + c + d + e;
    }
};


TEST(WrapperBasicTest, IntAddition) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::add), "add");
    Value result = engine.execute("add", {{"arg1", 10}, {"arg2", 20}});

    EXPECT_EQ(std::get<int>(result), 30);
}

TEST(WrapperBasicTest, ThreeArguments) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::multiply), "multiply");
    Value result = engine.execute("multiply", {{"arg1", 2}, {"arg2", 3}, {"arg3", 4}});

    EXPECT_EQ(std::get<int>(result), 24);
}

TEST(WrapperBasicTest, DoubleOperation) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::divide), "divide");
    Value result = engine.execute("divide", {{"arg1", 10.0}, {"arg2", 4.0}});

    EXPECT_DOUBLE_EQ(std::get<double>(result), 2.5);
}

TEST(WrapperBasicTest, VoidReturnType) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::print_message), "print");
    Value result = engine.execute("print", {{"arg1", 42}});

    EXPECT_EQ(std::get<int>(result), 0);
}

TEST(WrapperBasicTest, SingleArgument) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::single_arg), "single");
    Value result = engine.execute("single", {{"arg1", 21}});

    EXPECT_EQ(std::get<int>(result), 42);
}

TEST(WrapperBasicTest, FiveArguments) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::five_args), "five");
    Value result = engine.execute("five", {
        {"arg1", 1}, {"arg2", 2}, {"arg3", 3}, {"arg4", 4}, {"arg5", 5}
    });

    EXPECT_EQ(std::get<int>(result), 15);
}


TEST(WrapperErrorTest, WrongArgumentCount) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::add), "add");

    EXPECT_THROW({
        engine.execute("add", {{"arg1", 10}});
    }, std::invalid_argument);

    EXPECT_THROW({
        engine.execute("add", {{"arg1", 10}, {"arg2", 20}, {"arg3", 30}});
    }, std::invalid_argument);
}

TEST(WrapperErrorTest, MissingArgument) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::add), "add");

    EXPECT_THROW({
        engine.execute("add", {{"wrong1", 10}, {"wrong2", 20}});
    }, std::runtime_error);
}

TEST(WrapperErrorTest, CommandNotFound) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::add), "add");

    EXPECT_THROW({
        engine.execute("nonexistent", {{"arg1", 10}, {"arg2", 20}});
    }, std::runtime_error);
}


TEST(WrapperAdvancedTest, CommandOverwrite) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::add), "cmd");
    Value result1 = engine.execute("cmd", {{"arg1", 10}, {"arg2", 20}});
    EXPECT_EQ(std::get<int>(result1), 30);

    engine.register_command(make_wrapper(&obj, &TestClass::multiply), "cmd");
    Value result2 = engine.execute("cmd", {{"arg1", 2}, {"arg2", 3}, {"arg3", 4}});
    EXPECT_EQ(std::get<int>(result2), 24);
}

TEST(WrapperAdvancedTest, MultipleCommands) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::add), "add");
    engine.register_command(make_wrapper(&obj, &TestClass::multiply), "mul");
    engine.register_command(make_wrapper(&obj, &TestClass::single_arg), "double");

    Value r1 = engine.execute("add", {{"arg1", 5}, {"arg2", 7}});
    Value r2 = engine.execute("mul", {{"arg1", 2}, {"arg2", 3}, {"arg3", 4}});
    Value r3 = engine.execute("double", {{"arg1", 21}});

    EXPECT_EQ(std::get<int>(r1), 12);
    EXPECT_EQ(std::get<int>(r2), 24);
    EXPECT_EQ(std::get<int>(r3), 42);
}

TEST(WrapperAdvancedTest, NegativeNumbers) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::add), "add");
    Value result = engine.execute("add", {{"arg1", -10}, {"arg2", -20}});

    EXPECT_EQ(std::get<int>(result), -30);
}

TEST(WrapperAdvancedTest, ZeroValues) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::multiply), "mul");
    Value result = engine.execute("mul", {{"arg1", 0}, {"arg2", 100}, {"arg3", 200}});

    EXPECT_EQ(std::get<int>(result), 0);
}

TEST(WrapperAdvancedTest, TypeConversion) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::divide), "divide");

    Value result = engine.execute("divide", {{"arg1", 20}, {"arg2", 5}});
    EXPECT_DOUBLE_EQ(std::get<double>(result), 4.0);
}


TEST(WrapperEdgeCaseTest, LargeNumbers) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::add), "add");
    Value result = engine.execute("add", {{"arg1", 1000000}, {"arg2", 2000000}});

    EXPECT_EQ(std::get<int>(result), 3000000);
}

TEST(WrapperEdgeCaseTest, SmallDoubles) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::divide), "divide");
    Value result = engine.execute("divide", {{"arg1", 0.001}, {"arg2", 0.1}});

    EXPECT_NEAR(std::get<double>(result), 0.01, 1e-9);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}