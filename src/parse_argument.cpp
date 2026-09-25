// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#include "parse_argument.hpp"

#include "sql_test_event_listner.hpp"

#include <gtest/gtest.h>
#include <string>

namespace {

constexpr char kGtestOutputFlag[] = "--gtest_output";
constexpr char kSqliteOutputPrefix[] = "sqlite:";
constexpr char kPostgresqlOutputPrefix[] = "postgresql://";

}  // namespace

std::optional<SqlOutput> ParseGtestSqlOutputArgument(int* argc, char* argv[]) {
  for (int i = 1; i < *argc; ++i) {
    const std::string argument(argv[i]);
    std::string output_value;
    int arguments_to_remove = 0;

    if (argument.rfind(std::string(kGtestOutputFlag) + '=', 0) == 0) {
      output_value =
          argument.substr(std::string(kGtestOutputFlag).length() + 1);
      arguments_to_remove = 1;
    } else if (argument == kGtestOutputFlag && i + 1 < *argc) {
      output_value = argv[i + 1];
      arguments_to_remove = 2;
    } else {
      continue;
    }

    SqlOutputType type;
    std::size_t connection_start = std::string::npos;
    if (output_value.rfind(kSqliteOutputPrefix, 0) == 0) {
      type = SqlOutputType::SQLite;
      connection_start = sizeof(kSqliteOutputPrefix) - 1;
    } else if (output_value.rfind(kPostgresqlOutputPrefix, 0) == 0) {
      type = SqlOutputType::PostgreSQL;
      connection_start = 0;
    }
    if (connection_start == std::string::npos ||
        output_value.size() == connection_start) {
      continue;
    }

    for (int j = i; j + arguments_to_remove < *argc; ++j) {
      argv[j] = argv[j + arguments_to_remove];
    }
    *argc -= arguments_to_remove;
    argv[*argc] = nullptr;
    return SqlOutput{type, output_value.substr(connection_start)};
  }

  return std::nullopt;
}

