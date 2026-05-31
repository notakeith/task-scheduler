[English](README.md)

# TTaskScheduler

Header-only планировщик задач на C++17, выполняющий граф взаимозависимых вычислений. Задачи заранее объявляют свои входные данные — в том числе результаты ещё не выполненных задач — а планировщик автоматически разрешает граф зависимостей, вычисляя каждую задачу не более одного раза.

## Принцип работы

Ключевая идея — `getFutureResult<T>(id)`: типизированная ссылка на результат задачи, которая будет вычислена позже. При вызове `getResult` или `executeAll` планировщик разрешает каждый `FutureResult`, вычисляя задачу-источник по требованию (ленивые вычисления с кэшированием результатов).

**Пример — корни квадратного уравнения:**

```cpp
float a = 1, b = -2, c = 0;

TTaskScheduler scheduler;

// -4ac
auto id1 = scheduler.add([](float a, float c) { return -4 * a * c; }, a, c);

// b² + (-4ac)
auto id2 = scheduler.add([](float b, float v) { return b * b + v; },
                          b, scheduler.getFutureResult<float>(id1));

// (-b + sqrt(D)) и (-b - sqrt(D))
auto id3 = scheduler.add([](float b, float d) { return -b + std::sqrt(d); },
                          b, scheduler.getFutureResult<float>(id2));
auto id4 = scheduler.add([](float b, float d) { return -b - std::sqrt(d); },
                          b, scheduler.getFutureResult<float>(id2));

// x₁ = id3 / 2a,  x₂ = id4 / 2a
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
// Зарегистрировать задачу. Возвращает дескриптор задачи.
auto id = scheduler.add(callable, arg1, arg2);  // 0–2 аргумента

// Получить ленивую ссылку на будущий результат задачи (используется как аргумент другой задачи).
auto future = scheduler.getFutureResult<T>(id);

// Получить результат задачи, вычислив его (и зависимости) при необходимости.
T result = scheduler.getResult<T>(id);

// Выполнить все зарегистрированные задачи в порядке зависимостей.
scheduler.executeAll();
```

**Поддерживаемые типы callable:** лямбды, указатели на функции, указатели на методы класса.  
**Максимум аргументов на задачу:** 2 (экземпляр класса считается аргументом для указателей на методы).

## Реализация

Планировщик использует **стирание типов (type erasure)** для хранения разнотипных callable-объектов в одном контейнере. Каждая задача оборачивается за интерфейсом `ICallable` с тремя специализациями:

| Класс | Аргументы |
|-------|-----------|
| `Derived0<F, R>` | нет |
| `Derived1<F, R, A>` | один (с разворачиванием `FutureResult`) |
| `Derived2<F, R, A, B>` | два (с разворачиванием `FutureResult` для любого аргумента) |

Аргументы типа `FutureResult<T>` определяются на этапе компиляции через `is_future_result_v` и разрешаются вызовом `getResult` на задаче-источнике перед запуском callable.

## Сборка

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Запустить пример с квадратным уравнением:

```bash
./bin/main
```

Запустить тесты:

```bash
ctest --output-on-failure
```

## Тесты

Набор тестов на **Google Test**, охватывающий:

- Примитивные типы данных в качестве аргументов и результатов задач
- Ссылки `FutureResult` и цепочки зависимостей
- Ссылки на объекты (указатели на методы классов)
- Гарантии порядка выполнения
- Контейнерные типы как значения задач

## Требования

- C++17
- CMake 3.14+
- Google Test (загружается автоматически через CMake FetchContent)
