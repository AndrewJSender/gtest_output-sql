
#include "sql_test_event_listner.hpp"
#include <gtest/gtest.h>

TEST(SqlTestEventListener, TestProgramStart) {
    // This test is just to trigger the OnTestProgramStart event.
    // The actual output will be printed by the SqlTestEventListener.
    ASSERT_TRUE(true);
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    testing::TestEventListeners& listeners = testing::UnitTest::GetInstance()->listeners();
    std::filesystem::path db_path("/Users/sender/Downloads/output.db");
    auto* sql_listener = new testing::SqlTestEventListener(db_path);
    listeners.Append(sql_listener);
    return RUN_ALL_TESTS();
}
