// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#ifndef PARSE_ARGUMENT_HPP_
#define PARSE_ARGUMENT_HPP_

#include <filesystem>
#include <optional>
#include <string>

enum class SqlOutputType {
  SQLite,
  PostgreSQL,
};

struct SqlOutput {
  SqlOutputType type;
  std::string connection;
};

std::optional<SqlOutput> ParseGtestSqlOutputArgument(int* argc, char* argv[]);

#endif  // PARSE_ARGUMENT_HPP_
