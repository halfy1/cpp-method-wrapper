# C++ Method Wrapper

## Установка зависимостей

```bash
sudo apt update
sudo apt install libgtest-dev
```


## Быстрый старт

```bash
cmake -S . -B build
cmake --build build
./build/wrapper_demo
./build/test_wrapper
```

## Основные компоненты

### `ExecuteResult`
Обёртка для результата выполнения команды. Поддерживает любой тип через `std::any`.


### `Wrapper<Class, Ret, Args...>`
Шаблонный класс для обёртывания методов с автоматическим выводом типов.


### `Engine`
Движок для управления командами.

### `make_wrapper()`
Фабричная функция с автоматическим выводом типов.
