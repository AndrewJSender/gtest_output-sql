// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#include "parse_argument.hpp"

#include <string>

std::optional<SqlOutput> ParseGtestSqlOutputArgument(int* argc, char* argv[]) {
  constexpr char gtest_output_flag[] = "--gtest_output";
  constexpr char sqlite_output_prefix[] = "sqlite:";
  constexpr char postgresql_output_prefix[] = "postgresql://";

  for (int i = 1; i < *argc; ++i) {
    const std::string argument(argv[i]);
    std::string output_value;
    int arguments_to_remove = 0;

    if (argument.rfind(std::string(gtest_output_flag) + '=', 0) == 0) {
      output_value =
          argument.substr(std::string(gtest_output_flag).length() + 1);
      arguments_to_remove = 1;
    } else if (argument == gtest_output_flag && i + 1 < *argc) {
      output_value = argv[i + 1];
      arguments_to_remove = 2;
    } else {
      continue;
    }

    SqlOutputType type;
    std::size_t connection_start = std::string::npos;
    if (output_value.rfind(sqlite_output_prefix, 0) == 0) {
      type = SqlOutputType::SQLite;
      connection_start = sizeof(sqlite_output_prefix) - 1;
    } else if (output_value.rfind(postgresql_output_prefix, 0) == 0) {
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
