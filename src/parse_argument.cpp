// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#include "parse_argument.hpp"

#include <string>

std::optional<std::filesystem::path> ParseGtestSqlOutputArgument(int* argc,
                                                                  char* argv[]) {
  constexpr char sqlite_output_flag[] = "--gtest_output";
  constexpr char sqlite_output_prefix[] = "sqlite:";

  for (int i = 1; i < *argc; ++i) {
    const std::string argument(argv[i]);
    std::string output_value;
    int arguments_to_remove = 0;

    if (argument.rfind("--gtest_output=", 0) == 0) {
      output_value = argument.substr(std::string("--gtest_output=").length());
      arguments_to_remove = 1;
    } else if (argument == sqlite_output_flag && i + 1 < *argc) {
      output_value = argv[i + 1];
      arguments_to_remove = 2;
    } else {
      continue;
    }

    if (output_value.rfind(sqlite_output_prefix, 0) != 0) {
      continue;
    }

    std::filesystem::path db_path = output_value.substr(sizeof(sqlite_output_prefix) - 1);
    for (int j = i; j + arguments_to_remove < *argc; ++j) {
      argv[j] = argv[j + arguments_to_remove];
    }
    *argc -= arguments_to_remove;
    argv[*argc] = nullptr;
    return db_path;
  }

  return std::nullopt;
}
