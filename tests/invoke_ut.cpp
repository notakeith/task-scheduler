#include <gtest/gtest.h>
#include "scheduler.h"

class TaskSchedulerTest : public ::testing::Test {
   protected:
    TTaskScheduler scheduler;
};

TEST_F(TaskSchedulerTest, SingleCalculationCount) {
    int counter = 0;
    auto task = [&counter] {
        counter++;
        return 42;
    };

    auto id = scheduler.add(task);
    scheduler.getResult<int>(id);
    scheduler.getResult<int>(id);

    EXPECT_EQ(1, counter);
}

TEST_F(TaskSchedulerTest, DependentTasksCalculationCount) {
    int counter1 = 0, counter2 = 0;

    auto task1 = [&counter1] {
        counter1++;
        return 10;
    };

    auto task2 = [&counter2](int x) {
        counter2++;
        return x * 2;
    };

    auto id1 = scheduler.add(task1);
    auto id2 = scheduler.add(task2, scheduler.getFutureResult<int>(id1));

    scheduler.getResult<int>(id2);
    scheduler.getResult<int>(id2);

    EXPECT_EQ(1, counter1);
    EXPECT_EQ(1, counter2);
}

TEST_F(TaskSchedulerTest, ReusedResultCalculationCount) {
    int counter = 0;
    auto task = [&counter] {
        counter++;
        return 100;
    };

    auto id = scheduler.add(task);

    auto id1 = scheduler.add([](int x) { return x; }, scheduler.getFutureResult<int>(id));
    auto id2 = scheduler.add([](int x) { return x; }, scheduler.getFutureResult<int>(id));
    auto id3 = scheduler.add([](int x) { return x; }, scheduler.getFutureResult<int>(id));

    scheduler.getResult<int>(id1);
    scheduler.getResult<int>(id2);
    scheduler.getResult<int>(id3);

    EXPECT_EQ(1, counter);
}

TEST_F(TaskSchedulerTest, LazyEvaluationCount) {
    int counter = 0;
    auto task = [&counter] {
        counter++;
        return 55;
    };

    auto id = scheduler.add(task);

    EXPECT_EQ(0, counter);

    scheduler.getResult<int>(id);
    EXPECT_EQ(1, counter);
}

TEST_F(TaskSchedulerTest, TaskUpdateCount) {
    int counter = 0;
    auto task = [&counter] {
        counter++;
        return counter;
    };

    auto id = scheduler.add(task);
    auto first = scheduler.getResult<int>(id);

    id = scheduler.add(task);
    auto second = scheduler.getResult<int>(id);

    EXPECT_EQ(1, first);
    EXPECT_EQ(2, second);
    EXPECT_EQ(2, counter);
}
