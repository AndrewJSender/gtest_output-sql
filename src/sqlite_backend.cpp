// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#include "sqlite_backend.hpp"

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
       {"program.sql", "environment.sql", "suite.sql", "test.sql"}) {
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

std::int64_t SqliteBackend::InsertRow(
    const char* file_name, const char* marker,
    const std::vector<std::string>& values,
    const std::vector<std::int64_t>& integers) {
  ExecuteStatement(ReadSqlStatement(kDialect, file_name, marker), values,
                   integers);
  return sqlite3_last_insert_rowid(m_db);
}

}  // namespace testing
