// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#include "register_listener.hpp"

#include "parse_argument.hpp"
#include "sql_test_event_listner.hpp"

#include <gtest/gtest.h>

void RegisterSqlTestEventListenerIfRequested(std::optional<SqlOutput> sql_output) {
  if (!sql_output.has_value()) {
    return;
  }

  testing::TestEventListeners& listeners =
      testing::UnitTest::GetInstance()->listeners();
  auto* sql_listener = sql_output->type == SqlOutputType::SQLite
                           ? new testing::SqlTestEventListener(
                                 std::filesystem::path(sql_output->connection))
                           : new testing::SqlTestEventListener(
                                 testing::SqlTestEventListener::PostgreSqlTag{},
                                 sql_output->connection);
  listeners.Append(sql_listener);
}
