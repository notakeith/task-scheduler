#include <gtest/gtest.h>

#include "scheduler.h"

class TaskSchedulerTest : public ::testing::Test {
   protected:
    TTaskScheduler scheduler;
};

struct AddNumber {
    float add(float a) {
        return a + number;
    }

    float number;
};

TEST_F(TaskSchedulerTest, ClassMethodWithConstObject) {
    const AddNumber add{.number = 3};
    // auto id = scheduler.add(&AddNumber::add, add, 2.0f);
    // EXPECT_FLOAT_EQ(5.0f, scheduler.getResult<float>(id));
}
