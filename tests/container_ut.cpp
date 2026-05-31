#include <gtest/gtest.h>

#include "scheduler.h"

class TaskSchedulerTest : public ::testing::Test {
   protected:
    TTaskScheduler scheduler;
};

TEST_F(TaskSchedulerTest, VectorOperations) {
    auto create = scheduler.add([] {
        return std::vector<int>{1, 2, 3};
    });
    auto transform = scheduler.add(
        [](const std::vector<int>& v) {
            std::vector<int> result;
            for (int x : v) result.push_back(x * 2);
            return result;
        },
        scheduler.getFutureResult<std::vector<int>>(create));

    auto result = scheduler.getResult<std::vector<int>>(transform);
    ASSERT_EQ(3, result.size());
    EXPECT_EQ(2, result[0]);
    EXPECT_EQ(4, result[1]);
    EXPECT_EQ(6, result[2]);
}

TEST_F(TaskSchedulerTest, MapOperations) {
    std::map<int, int> input_map = {{1, 10}, {2, 20}, {3, 30}};
    auto id = scheduler.add([](const std::map<int, int>& m) {
        int sum = 0;
        for (const auto& [key, value] : m) {
            sum += value;
        }
        return sum;
    }, input_map);

    EXPECT_EQ(scheduler.getResult<int>(id), 60);
}
