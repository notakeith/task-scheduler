#include <gtest/gtest.h>

#include "scheduler.h"

class TaskSchedulerTest : public ::testing::Test {
   protected:
    TTaskScheduler scheduler;
};

TEST_F(TaskSchedulerTest, TasksExecutionOrderWithReturnValues) {
    std::vector<int> execution_order;

    auto task1 = scheduler.add([&execution_order]() -> int {
        execution_order.push_back(1);
        return 1; 
    });
    auto task2 = scheduler.add([&execution_order]() -> int {
        execution_order.push_back(2);
        return 2; 
    });
    auto task3 = scheduler.add([&execution_order]() -> int {
        execution_order.push_back(3);
        return 3; 
    });
    
    scheduler.executeAll();
    
    EXPECT_EQ(execution_order.size(), 3);
    EXPECT_EQ(execution_order[0], 1); 
    EXPECT_EQ(execution_order[1], 2); 
    EXPECT_EQ(execution_order[2], 3); 

}