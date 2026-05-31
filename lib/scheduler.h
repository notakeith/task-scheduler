#include <iostream>
#include <memory>
#include <type_traits>

template<typename T>struct FutureResult;
class Task;

template<typename> 
inline constexpr bool is_future_result_v = false;

template<typename T> 
inline constexpr bool is_future_result_v<FutureResult<T>> = true;

template <typename U>
struct ExtractValueType {using T = U;};

template <typename U>
struct ExtractValueType<FutureResult<U>> {using T = U;};

template<typename T, typename R, typename Arg>
class MethodWrapper {
    T& obj;
    R(T::*method)(Arg);
public:
    MethodWrapper(T& obj, R(T::*method)(Arg)): obj(obj), method(method) {}
    R operator()(Arg arg) {return (obj.*method)(arg);}
};
template<typename T, typename R, typename Arg>
MethodWrapper<T, R, Arg> make_method_wrapper(T& obj, R(T::*method)(Arg)) {
    return MethodWrapper<T, R, Arg>(obj, method);
}

template<typename T>
struct Result {
    using value_type = T;
    std::vector<Task>* tasks_;
    int taskid_;
    T getResult() {
        if (taskid_ < 0 || taskid_ >= static_cast<int>(tasks_[0].size()))
            throw std::out_of_range("Invalid task ID");
        return tasks_[0][taskid_].template get<T>();
    }
    Result(int taskid, std::vector<Task>* tasks) : taskid_(taskid), tasks_(tasks) {};
};

template<typename T>
struct FutureResult {
    using value_type = T;
    int taskid_;
    std::vector<Task>* tasks_;
    T getResult() {return tasks_[0][taskid_].template get<T>();}
    FutureResult() = default;
    FutureResult(int taskid, std::vector<Task>* tasks) : taskid_(taskid), tasks_(tasks) {};
};

class AnyResult {
public:
    class Base {
    public:
        virtual ~Base() = default;
    };
    template<typename T>
    class Derived : public Base {
    public:
        Derived(T value) : value_(std::move(value)) {}

        T& get() {
            return value_;
        }
    private:
        T value_;
    };
private:
    std::shared_ptr<Base> storage_;
    bool calculated_ = false;
public:
    template<typename T>
    AnyResult(T value) : storage_(new Derived<T>(std::move(value))), calculated_(true) {}
    AnyResult() : storage_(nullptr), calculated_(false) {}

    template<typename T>
    T& get() {
        return static_cast<Derived<T>&>(*storage_).get();
    }

    operator bool() {
        return calculated_;
    }
};

class Task {
public:
    using value_type = int;
    class Base {
    public:
        virtual ~Base() = default;
        virtual AnyResult get() = 0;
    };

    template<typename Func>
    class Derived0 : public Base {
        struct ICallable {
            virtual AnyResult invoke() = 0;
            virtual ~ICallable() = default;
        };

        template<typename TFunc>
        struct Callable : public ICallable {
            Callable(TFunc func) : f(func) {}
            AnyResult invoke() override { 
                return f(); 
            }
            TFunc f;
        };

    public:
        Derived0() = default;

        template<typename TFunc>
        Derived0(TFunc func) : callable_(new Callable<TFunc>(func)) {}

        AnyResult get() override {
            return callable_->invoke();
        }

    private:
        std::shared_ptr<ICallable> callable_;
    };

    template<typename Func, typename Arg>
    class Derived1 : public Base {
        struct ICallable {
            virtual AnyResult invoke(Arg arg) = 0;
            virtual ~ICallable() = default;
        };

        template<typename TFunc>
        struct Callable : public ICallable {
            Callable(TFunc func, Arg arg) : f(func), arg_(arg) {}
            AnyResult invoke(Arg) override {
                if constexpr (is_future_result_v<Arg>) {
                    return f(static_cast<typename Arg::value_type>(arg_.getResult()));
                } else {
                    return f(arg_);
                }
            }
            TFunc f;
            Arg arg_;
        };

    public:
        Derived1() = default;

        template<typename TFunc>
        Derived1(TFunc func, Arg arg) : callable_(new Callable<TFunc>(func, arg)) {}

        AnyResult get() override {
            return callable_->invoke(arg_);
        }

    private:
        std::shared_ptr<ICallable> callable_;
        Arg arg_;
    };

    template<typename Func, typename Arg1, typename Arg2>
    class Derived2 : public Base {
        struct ICallable {
            virtual AnyResult invoke(Arg1 arg1, Arg2 arg2) = 0;
            virtual ~ICallable() = default;
        };

        template<typename TFunc>
        struct Callable : public ICallable {
            Callable(TFunc func, Arg1 arg1, Arg2 arg2) : f(func), arg1_(arg1), arg2_(arg2) {}
            AnyResult invoke(Arg1, Arg2) override {
                if constexpr (is_future_result_v<Arg1> && is_future_result_v<Arg2>) {
                    return f(static_cast<typename Arg1::value_type>(arg1_.getResult()),
                           static_cast<typename Arg2::value_type>(arg2_.getResult()));
                } else if constexpr (is_future_result_v<Arg1>) {
                    return f(static_cast<typename Arg1::value_type>(arg1_.getResult()),
                           arg2_);
                } else if constexpr (is_future_result_v<Arg2>) {
                    return f(arg1_,
                           static_cast<typename Arg2::value_type>(arg2_.getResult()));
                } else {
                    return f(arg1_, arg2_);
                }
            }
            TFunc f;
            Arg1 arg1_;
            Arg2 arg2_;
        };

    public:
        Derived2() = default;

        template<typename TFunc>
        Derived2(TFunc func, Arg1 arg1, Arg2 arg2)
            : callable_(new Callable<TFunc>(func, arg1, arg2)) {}

        AnyResult get() override {
            return callable_->invoke(arg1_, arg2_);
        }

    private:
        std::shared_ptr<ICallable> callable_;
        Arg1 arg1_;
        Arg2 arg2_;
    };

private:
    std::shared_ptr<Base> task_;
    AnyResult result_;

public:
    template<typename Func>
    Task(Func func) : task_(new Derived0<Func>(std::move(func))) {}

    template<typename Func, typename Arg>
    Task(Func func, Arg arg) : task_(new Derived1<Func, Arg>(std::move(func), arg)) {}
    
    template<typename Func, typename Arg1, typename Arg2>
    Task(Func func, Arg1 arg1, Arg2 arg2)
        : task_(new Derived2<Func, Arg1, Arg2>(std::move(func), arg1, arg2)) {}

    template<typename T>
    T get() {
        if (result_)
            return result_.get<T>();
        result_ = task_->get();
        return result_.get<T>();
    }
    void calc() {
        if (result_)
            return;
        result_ = task_->get();
    }
    bool isCalculated() {
        return result_;
    }
};

class TTaskScheduler {
    std::vector<Task> tasks_;
    int nextTaskId = 0;
public:
    template<typename Func>
    auto add(Func func) -> Result<decltype(func())>  {
        tasks_.emplace_back(std::move(func));
        return Result<decltype(func())>{nextTaskId++, &tasks_};
    }
    template <typename Func, typename Arg>
    auto add(Func func, Arg arg) -> Result<decltype(func(std::declval<typename ExtractValueType<Arg>::T>()))> {
        tasks_.emplace_back(std::move(func), arg);
        return Result<decltype(func(std::declval<typename ExtractValueType<Arg>::T>()))>{nextTaskId++, &tasks_};
    }
    template <typename Func, typename Arg1, typename Arg2>
    auto add(Func func, Arg1 arg1, Arg2 arg2) -> Result<decltype(func(std::declval<typename ExtractValueType<Arg1>::T>(),std::declval<typename ExtractValueType<Arg2>::T>()))> {
        tasks_.emplace_back(std::move(func), arg1,arg2);
        return Result<decltype(func(std::declval<typename ExtractValueType<Arg1>::T>(),std::declval<typename ExtractValueType<Arg2>::T>()))>{nextTaskId++, &tasks_};
    }
    template<typename T, typename R, typename Arg>
    auto add(R(T::*method)(Arg), T obj, Arg arg) -> Result<R> {
        return add(make_method_wrapper(obj, method), arg);
    }
    template<typename T, typename R, typename Arg>
    auto add(R(T::*method)(Arg), T obj, FutureResult<Arg> arg) -> Result<R> {
        return add(make_method_wrapper(obj, method), arg);
    }
    template<typename T>
    FutureResult<T> getFutureResult(Result<T> taskid) {
        return FutureResult<T>{taskid.taskid_, &tasks_};
    }
    template<typename T>
    T getResult(Result<T> task) {
        if (task.taskid_ < 0 || task.taskid_ >= tasks_.size()) {
            throw std::out_of_range("Invalid task ID");
        }
        return tasks_[task.taskid_].template get<T>();
    }
    void executeAll() {
        for (auto& task : tasks_) {
            task.calc();
        }
    }
};