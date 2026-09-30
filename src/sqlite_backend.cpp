// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#include "sqlite_backend.hpp"

#include <memory>
#include <stdexcept>
#include <utility>

#include "sql_statement_reader.hpp"

namespace testing {
namespace {

constexpr char kDialect[] = "sqlite";

void CheckSqlite(int result, sqlite3* db, const char* operation) {
  if (result != SQLITE_OK && result != SQLITE_DONE && result != SQLITE_ROW) {
    throw std::runtime_error(std::string(operation) + ": " +
                             sqlite3_errmsg(db));
  }
}

}  // namespace

SqliteBackend::SqliteBackend(std::filesystem::path db_path) {
  const int result = sqlite3_open_v2(
      db_path.string().c_str(), &m_db,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
  if (result != SQLITE_OK) {
    const std::string error =
        m_db != nullptr ? sqlite3_errmsg(m_db) : "unable to allocate SQLite handle";
    if (m_db != nullptr) {
      sqlite3_close(m_db);
      m_db = nullptr;
    }
    throw std::runtime_error("Unable to open SQLite database '" +
                             db_path.string() + "': " + error);
  }

  ExecuteStatement("PRAGMA foreign_keys = ON;", {}, {});
  ExecuteStatement("PRAGMA user_version = 1;", {}, {});
  for (const char* file_name :
       {"program.sql", "iteration.sql", "environment.sql", "suite.sql",
        "test.sql", "test_result_part.sql"}) {
    ExecuteScript(file_name);
  }
}

SqliteBackend::~SqliteBackend() {
  if (m_db != nullptr) {
    sqlite3_close(m_db);
  }
}

void SqliteBackend::ExecuteScript(const char* file_name) {
  const std::string sql = ReadSqlSchema(kDialect, file_name);
  char* error_message = nullptr;
  const int result =
      sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &error_message);
  if (result != SQLITE_OK) {
    const std::string error =
        error_message != nullptr ? error_message : "unknown SQLite error";
    sqlite3_free(error_message);
    throw std::runtime_error("Unable to execute SQL file '" +
                             std::string(file_name) + "': " + error);
  }
}

void SqliteBackend::ExecuteStatement(
    const std::string& sql, const std::vector<std::string>& values,
    const std::vector<std::int64_t>& integers) {
  sqlite3_stmt* statement = nullptr;
  CheckSqlite(sqlite3_prepare_v2(m_db, sql.c_str(), -1, &statement, nullptr),
              m_db, "Unable to prepare SQLite statement");
  int parameter = 1;
  for (const auto& value : values) {
    CheckSqlite(sqlite3_bind_text(statement, parameter++, value.c_str(), -1,
                                  SQLITE_TRANSIENT),
                m_db, "Unable to bind SQLite text");
  }
  for (std::int64_t integer : integers) {
    CheckSqlite(sqlite3_bind_int64(statement, parameter++, integer), m_db,
                "Unable to bind SQLite integer");
  }
  CheckSqlite(sqlite3_step(statement), m_db,
              "Unable to execute SQLite statement");
  sqlite3_finalize(statement);
}

void SqliteBackend::Execute(const char* file_name, const char* marker,
                            const std::vector<std::string>& values,
                            const std::vector<std::int64_t>& integers) {
  ExecuteStatement(ReadSqlStatement(kDialect, file_name, marker), values,
                   integers);
}

std::vector<std::vector<std::string>> SqliteBackend::Query(
    const char* file_name, const char* marker,
    const std::vector<std::string>& values,
    const std::vector<std::int64_t>& integers) {
  const std::string sql = ReadSqlStatement(kDialect, file_name, marker);
  sqlite3_stmt* raw_statement = nullptr;
  CheckSqlite(sqlite3_prepare_v2(m_db, sql.c_str(), -1, &raw_statement, nullptr),
              m_db, "Unable to prepare SQLite query");
  const auto finalize = [](sqlite3_stmt* statement) {
    if (statement != nullptr) {
      sqlite3_finalize(statement);
    }
  };
  const std::unique_ptr<sqlite3_stmt, decltype(finalize)> statement(
      raw_statement, finalize);

  int parameter = 1;
  for (const auto& value : values) {
    CheckSqlite(sqlite3_bind_text(statement.get(), parameter++, value.c_str(),
                                  -1, SQLITE_TRANSIENT),
                m_db, "Unable to bind SQLite text");
  }
  for (std::int64_t integer : integers) {
    CheckSqlite(sqlite3_bind_int64(statement.get(), parameter++, integer), m_db,
                "Unable to bind SQLite integer");
  }

  std::vector<std::vector<std::string>> rows;
  int result = SQLITE_OK;
  while ((result = sqlite3_step(statement.get())) == SQLITE_ROW) {
    std::vector<std::string> row;
    const int column_count = sqlite3_column_count(statement.get());
    row.reserve(static_cast<std::size_t>(column_count));
    for (int column = 0; column < column_count; ++column) {
      const auto* text = sqlite3_column_text(statement.get(), column);
      const int length = sqlite3_column_bytes(statement.get(), column);
      row.emplace_back(text != nullptr
                           ? std::string(reinterpret_cast<const char*>(text),
                                         static_cast<std::size_t>(length))
                           : std::string());
    }
    rows.push_back(std::move(row));
  }
  if (result != SQLITE_DONE) {
    CheckSqlite(result, m_db, "Unable to execute SQLite query");
  }
  return rows;
}

std::int64_t SqliteBackend::InsertRow(
    const char* file_name, const char* marker,
    const std::vector<std::string>& values,
    const std::vector<std::int64_t>& integers) {
  ExecuteStatement(ReadSqlStatement(kDialect, file_name, marker), values,
                   integers);
  return sqlite3_last_insert_rowid(m_db);
}

}  // namespace testing
