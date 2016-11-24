#include "gtest/gtest.h"

#include <algorithm>

#include "../include/spica.h"
using namespace spica;

TEST(StackTest, Instance) {
    Stack<double> stk;
    ASSERT_DEATH(stk.pop(), "");
}

TEST(StackTest, PushAndPop) {
    Stack<double> stk;
    stk.push(1.0);
    stk.push(2.0);
    stk.push(3.0);
    EXPECT_EQ(stk.pop(), 3.0);
    EXPECT_EQ(stk.pop(), 2.0);
    EXPECT_EQ(stk.pop(), 1.0);
    ASSERT_DEATH(stk.pop(), "");
}

TEST(StackTest, ManyPush) {
    Stack<int> stk;
    for (int i = 0; i < 10000; i++) {
        stk.push(i);
    }

    for (int i = 9999; i >= 0; i--) {
        EXPECT_EQ(stk.pop(), i);
    }
}

TEST(StackTest, Clear) {
    Stack<float> stk;
    stk.push(1.0f);
    stk.push(2.0f);
    stk.push(3.0f);
    stk.clear();
    ASSERT_DEATH(stk.pop(), "");
}