#include <gtest/gtest.h>

#include "scheduler.h"

class TaskSchedulerTest : public ::testing::Test {
   protected:
    TTaskScheduler scheduler;
};

TEST_F(TaskSchedulerTest, ExecuteAllOnEmptyScheduler) {
    EXPECT_NO_THROW(scheduler.executeAll());
}