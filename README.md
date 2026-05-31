[Русский](README_RU.md)

# TTaskScheduler

A header-only C++17 task scheduler that executes a graph of data-dependent computations. Tasks declare their inputs upfront — including results of tasks that haven't run yet — and the scheduler resolves the dependency graph automatically, computing each task at most once.

## How it works

The central idea is `getFutureResult<T>(id)`: a typed forward reference to the output of a task that will be evaluated later. When `getResult` or `executeAll` is called, the scheduler resolves each `FutureResult` by computing its source task on demand (lazy evaluation with result caching).

**Example — quadratic equation roots:**

```cpp
float a = 1, b = -2, c = 0;

TTaskScheduler scheduler;

// -4ac
auto id1 = scheduler.add([](float a, float c) { return -4 * a * c; }, a, c);

// b² + (-4ac)
auto id2 = scheduler.add([](float b, float v) { return b * b + v; },
                          b, scheduler.getFutureResult<float>(id1));

// (-b + sqrt(D)) and (-b - sqrt(D))
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

`sqrt(D)` is computed once even though both `id5` and `id6` depend on it.

## API

```cpp
// Register a task. Returns a task handle.
auto id = scheduler.add(callable, arg1, arg2);  // 0–2 arguments

// Get a lazy reference to a task's future result (use as an argument to another task).
auto future = scheduler.getFutureResult<T>(id);

// Retrieve the result of a task, computing it (and its dependencies) if needed.
T result = scheduler.getResult<T>(id);

// Execute all registered tasks in dependency order.
scheduler.executeAll();
```

**Supported callable types:** lambdas, function pointers, member function pointers.  
**Maximum arguments per task:** 2 (the instance counts as an argument for member functions).

## Implementation

The scheduler uses **type erasure** to store heterogeneous callables in a single container. Each task is wrapped behind an `ICallable` interface with three specializations:

| Class | Arguments |
|-------|-----------|
| `Derived0<F, R>` | none |
| `Derived1<F, R, A>` | one (with `FutureResult` unwrapping) |
| `Derived2<F, R, A, B>` | two (with `FutureResult` unwrapping for either argument) |

`FutureResult<T>` arguments are detected at compile time via `is_future_result_v` and resolved by calling `getResult` on the referenced task before invoking the callable.

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Run the quadratic equation example:

```bash
./bin/main
```

Run tests:

```bash
ctest --output-on-failure
```

## Tests

Test suite built with **Google Test**, covering:

- Primitive data types as task arguments/results
- `FutureResult` forward references and dependency chains
- Object references (member function pointers)
- Execution ordering guarantees
- Container types as task values

## Requirements

- C++17
- CMake 3.14+
- Google Test (fetched automatically via CMake FetchContent)
