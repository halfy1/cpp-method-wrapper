#pragma once

#include <any>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <vector>

class ExecuteResult {
private:
    std::any value_;
    std::function<std::string()> to_string_;

public:
    template<typename T>
    explicit ExecuteResult(T&& value) 
        : value_(std::forward<T>(value))
        , to_string_([v = value_]() -> std::string {
            if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
                return std::any_cast<std::string>(v);
            } else if constexpr (std::is_arithmetic_v<std::decay_t<T>>) {
                return std::to_string(std::any_cast<std::decay_t<T>>(v));
            } else {
                return "<custom type>";
            }
        }) {}

    ExecuteResult() 
        : value_(std::any())
        , to_string_([]() { return "<void>"; }) {}

    template<typename T>
    T get() const {
        try {
            return std::any_cast<T>(value_);
        } catch (const std::bad_any_cast&) {
            throw std::runtime_error("Невозможно преобразовать результат к запрошенному типу");
        }
    }

    std::string to_string() const { return to_string_(); }

    friend std::ostream& operator<<(std::ostream& os, const ExecuteResult& result) {
        return os << result.to_string();
    }
};

class WrapperBase {
public:
    virtual ~WrapperBase() = default;
    virtual ExecuteResult execute(const std::unordered_map<std::string, std::any>& args) = 0;
};

template<typename Class, typename Ret, typename... Args>
class Wrapper : public WrapperBase {
private:
    Class* obj_;
    Ret (Class::*method_)(Args...);
    std::vector<std::string> arg_names_;
    std::unordered_map<std::string, std::any> default_values_;
    std::vector<std::type_index> expected_types_;

    template<std::size_t... I>
    static std::vector<std::string> generate_default_names(std::index_sequence<I...>) {
        return {("arg" + std::to_string(I + 1))...};
    }

    static std::vector<std::type_index> init_expected_types() {
        if constexpr (sizeof...(Args) > 0) {
            return {std::type_index(typeid(Args))...};
        }
        return {};
    }

    template<std::size_t I>
    auto extract_arg(const std::unordered_map<std::string, std::any>& args) {
        using ArgType = std::tuple_element_t<I, std::tuple<Args...>>;
        const std::string& name = arg_names_[I];

        auto it = args.find(name);
        if (it != args.end()) {
            if (std::type_index(it->second.type()) != std::type_index(typeid(ArgType))) {
                std::ostringstream oss;
                oss << "Неверный тип для аргумента '" << name << "': "
                    << "ожидается " << typeid(ArgType).name() 
                    << ", передан " << it->second.type().name();
                throw std::runtime_error(oss.str());
            }
            return std::any_cast<ArgType>(it->second);
        }

        auto def_it = default_values_.find(name);
        if (def_it != default_values_.end()) {
            return std::any_cast<ArgType>(def_it->second);
        }

        throw std::runtime_error("Отсутствует обязательный аргумент: " + name);
    }

    template<std::size_t... Is>
    ExecuteResult invoke_method(const std::unordered_map<std::string, std::any>& args,
                                std::index_sequence<Is...>) {
        if constexpr (std::is_void_v<Ret>) {
            (obj_->*method_)(extract_arg<Is>(args)...);
            return ExecuteResult{};  // пустой конструктор для void
        } else {
            Ret result = (obj_->*method_)(extract_arg<Is>(args)...);
            return ExecuteResult{std::move(result)};
        }
    }

public:
    Wrapper(Class* obj, 
            Ret (Class::*method)(Args...),
            std::vector<std::pair<std::string, std::any>> arg_defaults = {})
        : obj_(obj)
        , method_(method)
        , expected_types_(init_expected_types())
    {
        if (arg_defaults.empty() && sizeof...(Args) > 0) {
            arg_names_ = generate_default_names(std::make_index_sequence<sizeof...(Args)>{});
        } else {
            if (arg_defaults.size() != sizeof...(Args)) {
                std::ostringstream oss;
                oss << "Несоответствие количества аргументов: "
                    << "функция ожидает " << sizeof...(Args) 
                    << ", передано " << arg_defaults.size();
                throw std::invalid_argument(oss.str());
            }

            for (size_t i = 0; i < arg_defaults.size(); ++i) {
                const auto& [name, value] = arg_defaults[i];
                
                if (std::type_index(value.type()) != expected_types_[i]) {
                    std::ostringstream oss;
                    oss << "Неверный тип для аргумента '" << name << "' (позиция " << i << "): "
                        << "ожидается " << expected_types_[i].name()
                        << ", передан " << value.type().name();
                    throw std::invalid_argument(oss.str());
                }

                arg_names_.push_back(name);
                default_values_[name] = value;
            }
        }
    }

    ExecuteResult execute(const std::unordered_map<std::string, std::any>& args) override {
        return invoke_method(args, std::index_sequence_for<Args...>{});
    }

    const std::vector<std::string>& get_arg_names() const { return arg_names_; }
};

class Engine {
private:
    std::unordered_map<std::string, std::unique_ptr<WrapperBase>> commands_;

public:
    Engine() = default;
    ~Engine() = default;

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    Engine(Engine&&) = default;
    Engine& operator=(Engine&&) = default;

    void register_command(std::unique_ptr<WrapperBase> wrapper, 
                         const std::string& command_name) {
        if (commands_.count(command_name)) {
            throw std::invalid_argument("Команда '" + command_name + "' уже зарегистрирована");
        }
        commands_[command_name] = std::move(wrapper);
    }

    ExecuteResult execute(const std::string& command_name, 
                         const std::unordered_map<std::string, std::any>& args = {}) {
        auto it = commands_.find(command_name);
        if (it == commands_.end()) {
            throw std::runtime_error("Команда не найдена: " + command_name);
        }
        return it->second->execute(args);
    }

    bool has_command(const std::string& command_name) const {
        return commands_.count(command_name) > 0;
    }

    size_t command_count() const { return commands_.size(); }
};

template<typename Class, typename Ret, typename... Args>
auto make_wrapper(Class* obj, 
                  Ret (Class::*method)(Args...),
                  std::vector<std::pair<std::string, std::any>> arg_defaults = {}) {
    return std::make_unique<Wrapper<Class, Ret, Args...>>(obj, method, std::move(arg_defaults));
}
