#include <gtest/gtest.h>

#include "scheduler.h"

class TaskSchedulerTest : public ::testing::Test {
   protected:
    TTaskScheduler scheduler;
};

TEST_F(TaskSchedulerTest, DifferentDataTypes) {
    auto int_id = scheduler.add([] { return 42; });
    auto float_id = scheduler.add([] { return 3.14f; });
    auto string_id = scheduler.add([] { return std::string("hello"); });

    EXPECT_EQ(42, scheduler.getResult<int>(int_id));
    EXPECT_FLOAT_EQ(3.14f, scheduler.getResult<float>(float_id));
    EXPECT_EQ("hello", scheduler.getResult<std::string>(string_id));
}
TEST_F(TaskSchedulerTest, CustomTypeWithFuture) {
    struct Point {
        int x, y;
        Point operator+(const Point& other) const {
            return {x + other.x, y + other.y};
        }
    };

    auto p1 = scheduler.add([] { return Point{1, 2}; });
    auto p2 = scheduler.add([] { return Point{3, 4}; });
    auto sum = scheduler.add(
        [](Point a, Point b) { return a + b; },
        scheduler.getFutureResult<Point>(p1),
        scheduler.getFutureResult<Point>(p2));

    auto result = scheduler.getResult<Point>(sum);
    EXPECT_EQ(4, result.x);
    EXPECT_EQ(6, result.y);
}