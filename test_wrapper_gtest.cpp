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
    ExecuteResult result = engine.execute("add", {{"arg1", 10}, {"arg2", 20}});

    EXPECT_EQ(result.get<int>(), 30);
}

TEST(WrapperBasicTest, ThreeArguments) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::multiply), "multiply");
    ExecuteResult result = engine.execute("multiply", {{"arg1", 2}, {"arg2", 3}, {"arg3", 4}});

    EXPECT_EQ(result.get<int>(), 24);
}

TEST(WrapperBasicTest, DoubleOperation) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::divide), "divide");
    ExecuteResult result = engine.execute("divide", {{"arg1", 10.0}, {"arg2", 4.0}});

    EXPECT_DOUBLE_EQ(result.get<double>(), 2.5);
}

TEST(WrapperBasicTest, VoidReturnType) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::print_message), "print");
    ExecuteResult result = engine.execute("print", {{"arg1", 42}});

    EXPECT_EQ(result.to_string(), "<void>");
}

TEST(WrapperBasicTest, SingleArgument) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::single_arg), "single");
    ExecuteResult result = engine.execute("single", {{"arg1", 21}});

    EXPECT_EQ(result.get<int>(), 42);
}

TEST(WrapperBasicTest, FiveArguments) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::five_args), "five");
    ExecuteResult result = engine.execute("five", {
        {"arg1", 1}, {"arg2", 2}, {"arg3", 3}, {"arg4", 4}, {"arg5", 5}
    });

    EXPECT_EQ(result.get<int>(), 15);
}

TEST(WrapperDefaultValuesTest, UseAllDefaults) {
    TestClass obj;
    Engine engine;

    engine.register_command(
        make_wrapper(&obj, &TestClass::add, {{"x", 100}, {"y", 200}}),
        "add"
    );
    ExecuteResult result = engine.execute("add");

    EXPECT_EQ(result.get<int>(), 300);
}

TEST(WrapperDefaultValuesTest, OverrideOneDefault) {
    TestClass obj;
    Engine engine;

    engine.register_command(
        make_wrapper(&obj, &TestClass::add, {{"x", 100}, {"y", 200}}),
        "add"
    );
    ExecuteResult result = engine.execute("add", {{"x", 50}});

    EXPECT_EQ(result.get<int>(), 250);
}

TEST(WrapperDefaultValuesTest, OverrideAllDefaults) {
    TestClass obj;
    Engine engine;

    engine.register_command(
        make_wrapper(&obj, &TestClass::add, {{"x", 100}, {"y", 200}}),
        "add"
    );
    ExecuteResult result = engine.execute("add", {{"x", 10}, {"y", 20}});

    EXPECT_EQ(result.get<int>(), 30);
}

TEST(WrapperErrorTest, WrongArgumentCount) {
    TestClass obj;

    EXPECT_THROW({
        make_wrapper(&obj, &TestClass::add, {{"arg1", 0}});
    }, std::invalid_argument);

    EXPECT_THROW({
        make_wrapper(&obj, &TestClass::add, {{"arg1", 0}, {"arg2", 0}, {"arg3", 0}});
    }, std::invalid_argument);
}

TEST(WrapperErrorTest, WrongArgumentType) {
    TestClass obj;

    EXPECT_THROW({
        make_wrapper(&obj, &TestClass::add, {{"x", 1.5}, {"y", 2.5}});
    }, std::invalid_argument);

    EXPECT_THROW({
        make_wrapper(&obj, &TestClass::divide, {{"x", 10}, {"y", 5}});
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

TEST(WrapperErrorTest, TypeMismatchAtRuntime) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::add), "add");

    EXPECT_THROW({
        engine.execute("add", {{"arg1", 10.5}, {"arg2", 20}});
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

TEST(WrapperErrorTest, DuplicateCommand) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::add), "cmd");

    EXPECT_THROW({
        engine.register_command(make_wrapper(&obj, &TestClass::multiply), "cmd");
    }, std::invalid_argument);
}

TEST(WrapperAdvancedTest, MultipleCommands) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::add), "add");
    engine.register_command(make_wrapper(&obj, &TestClass::multiply), "mul");
    engine.register_command(make_wrapper(&obj, &TestClass::single_arg), "double");

    ExecuteResult r1 = engine.execute("add", {{"arg1", 5}, {"arg2", 7}});
    ExecuteResult r2 = engine.execute("mul", {{"arg1", 2}, {"arg2", 3}, {"arg3", 4}});
    ExecuteResult r3 = engine.execute("double", {{"arg1", 21}});

    EXPECT_EQ(r1.get<int>(), 12);
    EXPECT_EQ(r2.get<int>(), 24);
    EXPECT_EQ(r3.get<int>(), 42);
}

TEST(WrapperAdvancedTest, NegativeNumbers) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::add), "add");
    ExecuteResult result = engine.execute("add", {{"arg1", -10}, {"arg2", -20}});

    EXPECT_EQ(result.get<int>(), -30);
}

TEST(WrapperAdvancedTest, ZeroValues) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::multiply), "mul");
    ExecuteResult result = engine.execute("mul", {{"arg1", 0}, {"arg2", 100}, {"arg3", 200}});

    EXPECT_EQ(result.get<int>(), 0);
}

TEST(WrapperEngineTest, HasCommand) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::add), "add");

    EXPECT_TRUE(engine.has_command("add"));
    EXPECT_FALSE(engine.has_command("mul"));
}

TEST(WrapperEngineTest, CommandCount) {
    TestClass obj;
    Engine engine;

    EXPECT_EQ(engine.command_count(), 0);

    engine.register_command(make_wrapper(&obj, &TestClass::add), "add");
    EXPECT_EQ(engine.command_count(), 1);

    engine.register_command(make_wrapper(&obj, &TestClass::multiply), "mul");
    EXPECT_EQ(engine.command_count(), 2);
}

TEST(WrapperEdgeCaseTest, LargeNumbers) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::add), "add");
    ExecuteResult result = engine.execute("add", {{"arg1", 1000000}, {"arg2", 2000000}});

    EXPECT_EQ(result.get<int>(), 3000000);
}

TEST(WrapperEdgeCaseTest, SmallDoubles) {
    TestClass obj;
    Engine engine;

    engine.register_command(make_wrapper(&obj, &TestClass::divide), "divide");
    ExecuteResult result = engine.execute("divide", {{"arg1", 0.001}, {"arg2", 0.1}});

    EXPECT_NEAR(result.get<double>(), 0.01, 1e-9);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
