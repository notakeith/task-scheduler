#include <gtest/gtest.h>

#include "scheduler.h"

class TaskSchedulerTest : public ::testing::Test {
   protected:
    TTaskScheduler scheduler;
};

TEST_F(TaskSchedulerTest, TaskThrowsException) {
    auto id = scheduler.add([] {
        throw std::runtime_error("error");
        return 0;
    });

    EXPECT_THROW(scheduler.getResult<int>(id), std::runtime_error);
}

TEST_F(TaskSchedulerTest, ExceptionPropagation) {
    auto throws = scheduler.add([] {
        throw std::runtime_error("error");
        return 0;
    });
    auto depends = scheduler.add(
        [](int x) { return x + 1; },
        scheduler.getFutureResult<int>(throws));

    EXPECT_THROW(scheduler.getResult<int>(throws), std::runtime_error);
    EXPECT_THROW(scheduler.getResult<int>(depends), std::runtime_error);
}
