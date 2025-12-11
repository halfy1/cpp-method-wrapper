#pragma once

#include <string>
#include <variant>
#include <stdexcept>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <utility>

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
    Wrapper(Class* obj, Ret (Class::*method)(FuncArgs...), const std::vector<std::pair<std::string, Value>>& defaults)
        : obj_(obj), method_(method) {
        
        if (defaults.size() != sizeof...(FuncArgs)) {
            throw std::invalid_argument(
                "Ожидалось " + std::to_string(sizeof...(FuncArgs)) +
                " дефолтных аргументов, получено " + std::to_string(defaults.size())
            );
        }
        
        arg_names_.reserve(defaults.size());
        for (const auto& [key, value] : defaults) {
            arg_names_.push_back(key);
            default_args_[key] = value;
        }
    }

    Wrapper(const Wrapper&) = delete;
    Wrapper& operator=(const Wrapper&) = delete;

    Wrapper(Wrapper&&) = default;
    Wrapper& operator=(Wrapper&&) = default;

    Value execute(const Args& user_args) override {
        Args merged_args = default_args_;
        
        for (const auto& [key, value] : user_args) {
            if (default_args_.find(key) == default_args_.end()) {
                throw std::runtime_error(
                    "Неизвестный аргумент: " + key + 
                    ". Доступные: " + get_available_args()
                );
            }
            merged_args[key] = value;
        }
        
        return call_method(merged_args, std::index_sequence_for<FuncArgs...>{});
    }

    std::vector<std::string> get_arg_names() const {
        return arg_names_;
    }

private:
    Class* obj_;
    Ret (Class::*method_)(FuncArgs...);
    Args default_args_;
    std::vector<std::string> arg_names_;

    std::string get_available_args() const {
        std::string result;
        for (size_t i = 0; i < arg_names_.size(); ++i) {
            result += arg_names_[i];
            if (i < arg_names_.size() - 1) result += ", ";
        }
        return result;
    }

    template<std::size_t I>
    auto get_arg(const Args& merged_args) {
        const std::string& arg_name = arg_names_[I];
        auto it = merged_args.find(arg_name);
        
        if (it == merged_args.end()) {
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

    template<std::size_t... I>
    Value call_method(const Args& merged_args, std::index_sequence<I...>) {
        static_assert(std::is_void_v<Ret> ||
                     std::is_same_v<Ret, int> ||
                     std::is_same_v<Ret, double>,
                     "Тип возврата должен быть int, double или void");

        if constexpr (std::is_void_v<Ret>) {
            (obj_->*method_)(get_arg<I>(merged_args)...);
            return Value{0};
        } else {
            Ret result = (obj_->*method_)(get_arg<I>(merged_args)...);
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
auto make_wrapper(Class* obj, Ret (Class::*method)(FuncArgs...), const std::vector<std::pair<std::string, Value>>& defaults) {
    return std::make_unique<Wrapper<Class, Ret, FuncArgs...>>(obj, method, defaults);
}
