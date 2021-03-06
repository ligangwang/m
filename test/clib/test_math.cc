/*
 * Copyright (C) 2020 Ligang Wang <ligangwangs@gmail.com>
 *
 * unit test for utility functions
 */
#include "clib/math.h"
#include "gtest/gtest.h"

TEST(testMath, testPow)
{
    ASSERT_EQ(1, pow_int(2, 0));
    ASSERT_EQ(2, pow_int(2, 1));
    ASSERT_EQ(4, pow_int(2, 2));
    ASSERT_EQ(16, pow_int(2, 4));
    ASSERT_EQ(32, pow_int(2, 5));
    ASSERT_EQ(0, pow_int(2, -1));
}
