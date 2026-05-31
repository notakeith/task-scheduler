#include <gtest/gtest.h>

#include "scheduler.h"

int zero_args() {
    return 42;
}

int incr(int i) {
    return i + 1;
}

int sum(int a, int b) {
    return a + b;
}

struct Functor {
    int operator()(int a, int b) const {
        return a * b;
    }
};

class TaskSchedulerTest : public ::testing::Test {
   protected:
    TTaskScheduler scheduler;
};

TEST_F(TaskSchedulerTest, ZeroArgsTest) {
    auto id = scheduler.add(zero_args);
    EXPECT_EQ(42, scheduler.getResult<int>(id));
}

TEST_F(TaskSchedulerTest, IncrTest) {
    auto id = scheduler.add(incr, 5);
    EXPECT_EQ(6, scheduler.getResult<int>(id));
}

TEST_F(TaskSchedulerTest, SumTest) {
    auto id = scheduler.add(sum, 3, 4);
    EXPECT_EQ(7, scheduler.getResult<int>(id));
}

TEST_F(TaskSchedulerTest, FunctorTest) {
    Functor f;
    auto id = scheduler.add(f, 3, 4);
    EXPECT_EQ(12, scheduler.getResult<int>(id));
}

TEST_F(TaskSchedulerTest, LambdaTest) {
    float a = 10;
    auto id = scheduler.add([](float a) { return 5; }, a);
    EXPECT_EQ(5, scheduler.getResult<int>(id));
}

TEST_F(TaskSchedulerTest, MultipleGetResultTest) {
    float a = 10;
    float c = 4;
    auto id = scheduler.add(sum, a, c);

    EXPECT_EQ(14, scheduler.getResult<int>(id));
    EXPECT_EQ(14, scheduler.getResult<int>(id));
}

TEST_F(TaskSchedulerTest, FutureResultTest) {
    auto id1 = scheduler.add([](float a) { return 5; }, 10.0f);
    auto id2 = scheduler.add(incr, scheduler.getFutureResult<int>(id1));

    EXPECT_EQ(6, scheduler.getResult<int>(id2));
}

TEST_F(TaskSchedulerTest, ChainedTasks) {
    auto id1 = scheduler.add([] { return 2; });
    auto id2 = scheduler.add([](int x) { return x * 3; }, scheduler.getFutureResult<int>(id1));
    auto id3 = scheduler.add([](int x) { return x + 5; }, scheduler.getFutureResult<int>(id2));

    EXPECT_EQ(11, scheduler.getResult<int>(id3));
}