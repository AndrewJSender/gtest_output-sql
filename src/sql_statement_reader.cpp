// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#include "sql_statement_reader.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace testing {
namespace {

std::filesystem::path SqlPath(const char* dialect, const char* file_name) {
#ifdef SQL_TEST_EVENT_LISTENER_SQL_DIR
  const std::filesystem::path sql_directory(SQL_TEST_EVENT_LISTENER_SQL_DIR);
#else
  const std::filesystem::path sql_directory("sql");
#endif
  return sql_directory / dialect / file_name;
}

std::string ReadSql(const char* dialect, const char* file_name) {
  const std::filesystem::path sql_path = SqlPath(dialect, file_name);
  std::ifstream file(sql_path);
  if (!file) {
    throw std::runtime_error("Unable to open SQL file '" + sql_path.string() +
                             "'");
  }
  return {std::istreambuf_iterator<char>(file),
          std::istreambuf_iterator<char>()};
}

}  // namespace

std::string ReadSqlSchema(const char* dialect, const char* file_name) {
  std::string sql = ReadSql(dialect, file_name);
  const std::size_t schema_end = sql.find("\n-- ", sql.find(';'));
  if (schema_end != std::string::npos) {
    sql.resize(schema_end);
  }
  return sql;
}

std::string ReadSqlStatement(const char* dialect, const char* file_name,
                             const char* marker) {
  const std::string sql = ReadSql(dialect, file_name);
  const std::string marker_text = std::string("-- ") + marker + "\n";
  const std::size_t marker_start = sql.find(marker_text);
  if (marker_start == std::string::npos) {
    throw std::runtime_error("SQL statement '" + std::string(marker) +
                             "' not found in '" + file_name + "'");
  }
  const std::size_t statement_start = marker_start + marker_text.size();
  const std::size_t statement_end = sql.find(';', statement_start);
  if (statement_end == std::string::npos) {
    throw std::runtime_error("SQL statement '" + std::string(marker) +
                             "' in '" + file_name + "' has no terminator");
  }
  return sql.substr(statement_start, statement_end - statement_start + 1);
}

}  // namespace testing
