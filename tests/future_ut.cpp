#include <gtest/gtest.h>
#include "scheduler.h"

class TaskSchedulerTest : public ::testing::Test {
   protected:
    TTaskScheduler scheduler;
};

TEST_F(TaskSchedulerTest, GetResultFromFutureResult) {
    auto future_id = scheduler.add([]() { return 100; });
    auto future_result = scheduler.getFutureResult(future_id);

    EXPECT_EQ(future_result.getResult(), 100);
}

TEST_F(TaskSchedulerTest, FutureResultAsFirstArg) {
    auto future_id = scheduler.add([]() { return 10; });
    auto future_result = scheduler.getFutureResult(future_id);

    auto task_id = scheduler.add([](int x, int y) { return x + y; }, future_result, 20);
    EXPECT_EQ(scheduler.getResult<int>(task_id), 30);
}

TEST_F(TaskSchedulerTest, FutureResultAsSecondArg) {
    auto future_id = scheduler.add([]() { return 20; });
    auto future_result = scheduler.getFutureResult(future_id);

    auto task_id = scheduler.add([](int x, int y) { return x + y; }, 10, future_result);
    EXPECT_EQ(scheduler.getResult<int>(task_id), 30);
}

TEST_F(TaskSchedulerTest, TwoFutureResultsAsArgs) {
    auto future_id1 = scheduler.add([]() { return 10; });
    auto future_result1 = scheduler.getFutureResult(future_id1);

    auto future_id2 = scheduler.add([]() { return 20; });
    auto future_result2 = scheduler.getFutureResult(future_id2);

    auto task_id = scheduler.add([](int x, int y) { return x + y; }, future_result1, future_result2);
    EXPECT_EQ(scheduler.getResult<int>(task_id), 30);
}

TEST_F(TaskSchedulerTest, FutureResultNotCalculatedUntilGetResult) {
    bool executed = false;
    auto future_id = scheduler.add([&executed]() {
        executed = true;
        return 42;
    });
    auto future_result = scheduler.getFutureResult(future_id);

    EXPECT_FALSE(executed);
    EXPECT_EQ(future_result.getResult(), 42);
    EXPECT_TRUE(executed);
}
