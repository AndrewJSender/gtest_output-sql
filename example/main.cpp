
// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#include <gtest/gtest.h>

#include "sql_test_event_listner.hpp"

#include "register_listener.hpp"

TEST(SqlTestEventListener, Test1) {
    GTEST_SUCCEED() << "Success 1";
    sleep(1);
    EXPECT_TRUE(true);
    
}

TEST(SqlTestEventListener, Test2) {
    GTEST_SUCCEED() << "Success 2";
    sleep(1);
    EXPECT_TRUE(true);
}

TEST(SqlTestEventListener, DISABLED_disabled) {
    GTEST_SUCCEED() << "Should not happen";
    sleep(1);
    EXPECT_TRUE(true);
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    RegisterSqlTestEventListenerIfRequested(&argc, argv);
    return RUN_ALL_TESTS();
}
