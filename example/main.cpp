
#include "sql_test_event_listner.hpp"
#include <gtest/gtest.h>
#include "parse_argument.hpp"

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
    auto sql_output = ParseGtestSqlOutputArgument(&argc, argv);
    testing::InitGoogleTest(&argc, argv);
    if (sql_output.has_value()) {
        testing::TestEventListeners& listeners =
            testing::UnitTest::GetInstance()->listeners();
          auto* sql_listener = sql_output->type == SqlOutputType::SQLite
                                   ? new testing::SqlTestEventListener(
                                         std::filesystem::path(
                                             sql_output->connection))
                                   : new testing::SqlTestEventListener(
                                         sql_output->connection);
          listeners.Append(sql_listener);
    }
    return RUN_ALL_TESTS();
}
