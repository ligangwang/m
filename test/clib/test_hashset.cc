/*
 * Copyright (C) 2020 Ligang Wang <ligangwangs@gmail.com>
 *
 * unit test for clib hashset functions
 */
#include "gtest/gtest.h"

#include "clib/hashset.h"
#include "clib/object.h"
#include "clib/string.h"

TEST(testHashset, TestAddAndCheck)
{
    hashset hs;
    hashset_init(&hs);
    const char* str1 = "hello hs1";
    const char* str2 = "hello hs2";
    const char* str3 = "no existing";
    hashset_set(&hs, str1);
    hashset_set(&hs, str2);
    ASSERT_EQ(2, hashset_size(&hs));
    ASSERT_TRUE(hashset_in(&hs, str1));
    ASSERT_TRUE(hashset_in(&hs, str2));
    ASSERT_FALSE(hashset_in(&hs, str3));
    hashset_deinit(&hs);
}
