# Task Scheduler

> [English version](README.md)

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?logo=cplusplus&logoColor=white)](https://en.cppreference.com/w/cpp/17)
[![CMake](https://img.shields.io/badge/CMake-3.14%2B-064F8C?logo=cmake&logoColor=white)](https://cmake.org/)
[![GTest](https://img.shields.io/badge/tested_with-GTest-4285F4?logo=google&logoColor=white)](https://github.com/google/googletest)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

Header-only планировщик задач на C++17, выполняющий граф взаимозависимых вычислений. Задачи заранее объявляют свои входные данные — в том числе результаты ещё не выполненных задач — а планировщик автоматически разрешает граф зависимостей, вычисляя каждую задачу не более одного раза.

## Принцип работы

Ключевая идея — `getFutureResult<T>(id)`: типизированная ссылка на результат задачи, которая будет вычислена позже. При вызове `getResult` или `executeAll` планировщик разрешает каждый `FutureResult`, вычисляя задачу-источник по требованию (ленивые вычисления с кэшированием результатов).

**Пример — корни квадратного уравнения:**

```cpp
float a = 1, b = -2, c = 0;

TTaskScheduler scheduler;

auto id1 = scheduler.add([](float a, float c) { return -4 * a * c; }, a, c);

auto id2 = scheduler.add([](float b, float v) { return b * b + v; },
                          b, scheduler.getFutureResult<float>(id1));

auto id3 = scheduler.add([](float b, float d) { return -b + std::sqrt(d); },
                          b, scheduler.getFutureResult<float>(id2));
auto id4 = scheduler.add([](float b, float d) { return -b - std::sqrt(d); },
                          b, scheduler.getFutureResult<float>(id2));

auto id5 = scheduler.add([](float a, float v) { return v / (2 * a); },
                          a, scheduler.getFutureResult<float>(id3));
auto id6 = scheduler.add([](float a, float v) { return v / (2 * a); },
                          a, scheduler.getFutureResult<float>(id4));

scheduler.executeAll();

std::cout << scheduler.getResult<float>(id5) << "\n"; // x₁
std::cout << scheduler.getResult<float>(id6) << "\n"; // x₂
```

`sqrt(D)` вычисляется один раз, несмотря на то что от него зависят и `id5`, и `id6`.

## API

```cpp
auto id = scheduler.add(callable, arg1, arg2);  // 0–2 аргумента
auto future = scheduler.getFutureResult<T>(id);
T result = scheduler.getResult<T>(id);
scheduler.executeAll();
```

**Поддерживаемые callable:** лямбды, указатели на функции, указатели на методы класса.  
**Максимум аргументов на задачу:** 2.

## Реализация

Планировщик использует **стирание типов (type erasure)** для хранения разнотипных callable в одном контейнере. Каждая задача оборачивается за интерфейсом `ICallable` с тремя специализациями:

| Класс | Аргументы |
|-------|-----------|
| `Derived0<F, R>` | нет |
| `Derived1<F, R, A>` | один (с разворачиванием `FutureResult`) |
| `Derived2<F, R, A, B>` | два (с разворачиванием `FutureResult` для любого аргумента) |

## Сборка

```bash
mkdir build && cd build
cmake ..
cmake --build .
./bin/main          # пример с квадратным уравнением
ctest --output-on-failure
```

## Требования

- C++17
- CMake 3.14+
- Google Test (загружается автоматически через CMake FetchContent)
