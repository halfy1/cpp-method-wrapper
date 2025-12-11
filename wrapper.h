#pragma once

#include <string>
#include <variant>
#include <stdexcept>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

using Value = std::variant<int, double>;
using Args = std::unordered_map<std::string, Value>;

class WrapperBase {
public:
    virtual ~WrapperBase() = default;
    virtual Value execute(const Args& args) = 0;
};

template<typename Class, typename Ret, typename... FuncArgs>
class Wrapper : public WrapperBase {
public:
    Wrapper(Class* obj, Ret (Class::*method)(FuncArgs...))
        : obj_(obj), method_(method) {
        arg_names_ = generate_arg_names(std::make_index_sequence<sizeof...(FuncArgs)>{});
    }

    Wrapper(const Wrapper&) = delete;
    Wrapper& operator=(const Wrapper&) = delete;

    Wrapper(Wrapper&&) = default;
    Wrapper& operator=(Wrapper&&) = default;

    Value execute(const Args& args) override {
        if (args.size() != sizeof...(FuncArgs)) {
            throw std::invalid_argument(
                "Ожидалось " + std::to_string(sizeof...(FuncArgs)) + 
                " аргументов, получено " + std::to_string(args.size())
            );
        }
        
        return call_method(args, std::index_sequence_for<FuncArgs...>{});
    }

    std::vector<std::string> get_arg_names() const {
        return arg_names_;
    }

private:
    Class* obj_;
    Ret (Class::*method_)(FuncArgs...);
    std::vector<std::string> arg_names_;

    template<std::size_t... I>
    std::vector<std::string> generate_arg_names(std::index_sequence<I...>) {
        return {("arg" + std::to_string(I + 1))...};
    }

    template<std::size_t I>
    auto get_arg(const Args& args) {
        const std::string& arg_name = arg_names_[I];

        auto it = args.find(arg_name);
        if (it == args.end()) {
            throw std::runtime_error("Пропущен аргумент: " + arg_name);
        }

        using ArgType = typename std::tuple_element<I, std::tuple<FuncArgs...>>::type;

        if (std::holds_alternative<int>(it->second)) {
            return static_cast<ArgType>(std::get<int>(it->second));
        } else if (std::holds_alternative<double>(it->second)) {
            return static_cast<ArgType>(std::get<double>(it->second));
        }

        throw std::runtime_error("Неверный тип: " + arg_name);
    }

    template<std::size_t... Is>
    Value call_method(const Args& args, 
                      std::index_sequence<Is...>) {
        static_assert(std::is_void_v<Ret> || 
                     std::is_same_v<Ret, int> || 
                     std::is_same_v<Ret, double>,
                     "Тип должен быть int, double, void");

        if constexpr (std::is_void_v<Ret>) {
            (obj_->*method_)(get_arg<Is>(args)...);
            return Value{0};
        } else {
            Ret result = (obj_->*method_)(get_arg<Is>(args)...);
            return Value{result};
        }
    }
};

class Engine {
public:
    Engine() = default;
    ~Engine() = default;

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    Engine(Engine&&) = default;
    Engine& operator=(Engine&&) = default;

    void register_command(std::unique_ptr<WrapperBase> wrapper, 
                         const std::string& command_name) {
        commands_[command_name] = std::move(wrapper);
    }

    Value execute(const std::string& command_name, const Args& args) {
        auto it = commands_.find(command_name);
        if (it == commands_.end()) {
            throw std::runtime_error("Команда не найдена: " + command_name);
        }
        return it->second->execute(args);
    }

private:
    std::unordered_map<std::string, std::unique_ptr<WrapperBase>> commands_;
};

template<typename Class, typename Ret, typename... FuncArgs>
auto make_wrapper(Class* obj, Ret (Class::*method)(FuncArgs...)) {
    return std::make_unique<Wrapper<Class, Ret, FuncArgs...>>(obj, method);
}
