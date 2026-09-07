
#include "sql_test_event_listner.hpp"
#include <gtest/gtest.h>
#include "parse_argument.hpp"

TEST(SqlTestEventListener, TestProgramStart) {
    // This test is just to trigger the OnTestProgramStart event.
    // The actual output will be printed by the SqlTestEventListener.
    sleep(3);
    ASSERT_TRUE(true);
}

int main(int argc, char* argv[]) {
    auto db_path = ParseGtestSqlOutputArgument(&argc, argv);
    testing::InitGoogleTest(&argc, argv);
    if (db_path.has_value()) {
        testing::TestEventListeners& listeners =
            testing::UnitTest::GetInstance()->listeners();
        auto* sql_listener =
            new testing::SqlTestEventListener(*db_path);
        listeners.Append(sql_listener);
    }
    return RUN_ALL_TESTS();
}
