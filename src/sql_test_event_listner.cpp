// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#include "sql_test_event_listner.hpp"

#include <chrono>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace testing {
namespace {

using Type = SqlTestEventListener::Type;

std::filesystem::path SqlPath(Type type, const char* file_name) {
#ifdef SQL_TEST_EVENT_LISTENER_SQL_DIR
  const std::filesystem::path sql_directory(SQL_TEST_EVENT_LISTENER_SQL_DIR);
#else
  const std::filesystem::path sql_directory("sql");
#endif
  return sql_directory /
         (type == Type::SQLite ? "sqlite" : "postgresql") / file_name;
}

std::string ReadSql(Type type, const char* file_name) {
  const std::filesystem::path sql_path = SqlPath(type, file_name);
  std::ifstream file(sql_path);
  if (!file) {
    throw std::runtime_error("Unable to open SQL file '" + sql_path.string() +
                             "'");
  }
  return {std::istreambuf_iterator<char>(file),
          std::istreambuf_iterator<char>()};
}

std::string Schema(Type type, const char* file_name) {
  std::string sql = ReadSql(type, file_name);
  const std::size_t schema_end = sql.find("\n-- ", sql.find(';'));
  if (schema_end != std::string::npos) {
    sql.resize(schema_end);
  }
  return sql;
}

std::string ReadStatement(Type type, const char* file_name,
                          const char* marker) {
  const std::string sql = ReadSql(type, file_name);
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

void CheckSqlite(int result, sqlite3* db, const char* operation) {
  if (result != SQLITE_OK && result != SQLITE_DONE && result != SQLITE_ROW) {
    throw std::runtime_error(std::string(operation) + ": " + sqlite3_errmsg(db));
  }
}

void ExecuteSqliteScript(sqlite3* db, const std::string& sql,
                         const char* file_name) {
  char* error_message = nullptr;
  const int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr,
                                  &error_message);
  if (result != SQLITE_OK) {
    const std::string error =
        error_message != nullptr ? error_message : "unknown SQLite error";
    sqlite3_free(error_message);
    throw std::runtime_error("Unable to execute SQL file '" +
                             std::string(file_name) + "': " + error);
  }
}

void CheckPostgreSQLConnection(PGconn* db) {
  if (PQstatus(db) != CONNECTION_OK) {
    const std::string error = PQerrorMessage(db);
    PQfinish(db);
    throw std::runtime_error("Unable to connect to PostgreSQL: " + error);
  }
}

std::string DecodeUrlComponent(const std::string& value) {
  std::string decoded;
  decoded.reserve(value.size());
  for (std::size_t i = 0; i < value.size(); ++i) {
    if (value[i] != '%') {
      decoded += value[i] == '+' ? ' ' : value[i];
      continue;
    }
    if (i + 2 >= value.size()) {
      throw std::runtime_error("Invalid percent-encoded PostgreSQL URL");
    }
    const auto hex_value = [](char character) -> int {
      if (character >= '0' && character <= '9') {
        return character - '0';
      }
      if (character >= 'a' && character <= 'f') {
        return character - 'a' + 10;
      }
      if (character >= 'A' && character <= 'F') {
        return character - 'A' + 10;
      }
      return -1;
    };
    const int high = hex_value(value[i + 1]);
    const int low = hex_value(value[i + 2]);
    if (high < 0 || low < 0) {
      throw std::runtime_error("Invalid percent-encoded PostgreSQL URL");
    }
    decoded += static_cast<char>((high << 4) | low);
    i += 2;
  }
  return decoded;
}

std::string QueryValue(const std::string& query, const std::string& name) {
  std::size_t start = 0;
  while (start < query.size()) {
    const std::size_t end = query.find('&', start);
    const std::string parameter =
        query.substr(start, end == std::string::npos ? end : end - start);
    const std::size_t equals = parameter.find('=');
    if (parameter.substr(0, equals) == name) {
      return equals == std::string::npos ? "" : parameter.substr(equals + 1);
    }
    if (end == std::string::npos) {
      break;
    }
    start = end + 1;
  }
  return "";
}

PGconn* ConnectPostgreSQL(const std::string& url) {
  constexpr char postgresql_prefix[] = "postgresql://";
  const std::string endpoint = url.substr(sizeof(postgresql_prefix) - 1);
  const std::size_t query_start = endpoint.find('?');
  if (query_start == std::string::npos) {
    return PQconnectdb(url.c_str());
  }
  const std::string query = endpoint.substr(query_start + 1);
  const std::string user = QueryValue(query, "DBUser");
  if (QueryValue(query, "Action") != "connect" || user.empty()) {
    return PQconnectdb(url.c_str());
  }

  std::string authority = endpoint.substr(0, query_start);
  if (!authority.empty() && authority.back() == '/') {
    authority.pop_back();
  }
  const std::size_t port_separator = authority.rfind(':');
  const std::string host = authority.substr(0, port_separator);
  const std::string port = port_separator == std::string::npos
                               ? "5432"
                               : authority.substr(port_separator + 1);
  if (host.empty() || port.empty()) {
    throw std::runtime_error("Invalid PostgreSQL IAM authentication URL");
  }
  const std::string decoded_user = DecodeUrlComponent(user);
  const char* keywords[] = {"host", "port", "user", "password", "sslmode",
                            nullptr};
  const char* values[] = {host.c_str(), port.c_str(), decoded_user.c_str(),
                          endpoint.c_str(), "require", nullptr};
  return PQconnectdbParams(keywords, values, 0);
}

void CheckPostgreSQLResult(PGresult* result, ExecStatusType expected,
                           PGconn* db, const char* operation) {
  if (result == nullptr) {
    throw std::runtime_error(std::string(operation) + ": " + PQerrorMessage(db));
  }
  if (PQresultStatus(result) != expected) {
    const std::string error = PQerrorMessage(db);
    PQclear(result);
    throw std::runtime_error(std::string(operation) + ": " + error);
  }
}

void ExecutePostgreSQLScript(PGconn* db, const std::string& sql,
                             const char* file_name) {
  PGresult* result = PQexec(db, sql.c_str());
  CheckPostgreSQLResult(result, PGRES_COMMAND_OK, db,
                        ("Unable to execute SQL file '" +
                         std::string(file_name) + "'").c_str());
  PQclear(result);
}

void MigratePostgreSQLTimestampColumns(PGconn* db) {
  constexpr char migration[] = R"SQL(
DO $$
DECLARE
  target_table TEXT;
  target_column TEXT;
BEGIN
  FOREACH target_table IN ARRAY ARRAY['program', 'environment', 'suite', 'test']
  LOOP
    FOREACH target_column IN ARRAY ARRAY['start_timestamp', 'end_timestamp']
    LOOP
      IF EXISTS (
          SELECT 1
          FROM information_schema.columns
          WHERE table_schema = current_schema()
            AND table_name = target_table
            AND column_name = target_column
            AND data_type <> 'bigint') THEN
        EXECUTE format(
            'ALTER TABLE %I ALTER COLUMN %I TYPE BIGINT',
            target_table, target_column);
      END IF;
    END LOOP;
  END LOOP;
END $$;
)SQL";
  ExecutePostgreSQLScript(db, migration, "PostgreSQL timestamp migration");
}

std::vector<std::string> Parameters(const std::vector<std::string>& values,
                                    const std::vector<sqlite3_int64>& integers) {
  std::vector<std::string> parameters = values;
  parameters.reserve(values.size() + integers.size());
  for (sqlite3_int64 integer : integers) {
    parameters.push_back(std::to_string(integer));
  }
  return parameters;
}

sqlite3_int64 InsertRowSqlite(sqlite3* db, const std::string& sql,
                              const std::vector<std::string>& values,
                              const std::vector<sqlite3_int64>& integers) {
  sqlite3_stmt* statement = nullptr;
  CheckSqlite(sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, nullptr), db,
              "Unable to prepare SQLite statement");
  int parameter = 1;
  for (const auto& value : values) {
    CheckSqlite(sqlite3_bind_text(statement, parameter++, value.c_str(), -1,
                                  SQLITE_TRANSIENT),
                db, "Unable to bind SQLite text");
  }
  for (sqlite3_int64 integer : integers) {
    CheckSqlite(sqlite3_bind_int64(statement, parameter++, integer), db,
                "Unable to bind SQLite integer");
  }
  CheckSqlite(sqlite3_step(statement), db, "Unable to insert SQLite row");
  sqlite3_finalize(statement);
  return sqlite3_last_insert_rowid(db);
}

sqlite3_int64 InsertRowPostgreSQL(
    PGconn* db, const std::string& sql, const std::vector<std::string>& values,
    const std::vector<sqlite3_int64>& integers) {
  const std::vector<std::string> parameters = Parameters(values, integers);
  std::vector<const char*> parameter_values;
  parameter_values.reserve(parameters.size());
  for (const auto& parameter : parameters) {
    parameter_values.push_back(parameter.c_str());
  }
  PGresult* result =
      PQexecParams(db, sql.c_str(), static_cast<int>(parameter_values.size()),
                   nullptr, parameter_values.data(), nullptr, nullptr, 0);
  CheckPostgreSQLResult(result, PGRES_TUPLES_OK, db,
                        "Unable to insert PostgreSQL row");
  if (PQntuples(result) != 1 || PQnfields(result) != 1) {
    PQclear(result);
    throw std::runtime_error(
        "Unable to insert PostgreSQL row: expected one returned identifier");
  }
  const std::string id = PQgetvalue(result, 0, 0);
  PQclear(result);
  try {
    return std::stoll(id);
  } catch (const std::invalid_argument&) {
    throw std::runtime_error(
        "Unable to insert PostgreSQL row: invalid returned identifier");
  } catch (const std::out_of_range&) {
    throw std::runtime_error(
        "Unable to insert PostgreSQL row: returned identifier out of range");
  }
}

void ExecuteSqlite(sqlite3* db, const std::string& sql,
                   const std::vector<std::string>& values,
                   const std::vector<sqlite3_int64>& integers) {
  sqlite3_stmt* statement = nullptr;
  CheckSqlite(sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, nullptr), db,
              "Unable to prepare SQLite statement");
  int parameter = 1;
  for (const auto& value : values) {
    CheckSqlite(sqlite3_bind_text(statement, parameter++, value.c_str(), -1,
                                  SQLITE_TRANSIENT),
                db, "Unable to bind SQLite text");
  }
  for (sqlite3_int64 integer : integers) {
    CheckSqlite(sqlite3_bind_int64(statement, parameter++, integer), db,
                "Unable to bind SQLite integer");
  }
  CheckSqlite(sqlite3_step(statement), db, "Unable to execute SQLite statement");
  sqlite3_finalize(statement);
}

void ExecutePostgreSQL(PGconn* db, const std::string& sql,
                       const std::vector<std::string>& values,
                       const std::vector<sqlite3_int64>& integers) {
  const std::vector<std::string> parameters = Parameters(values, integers);
  std::vector<const char*> parameter_values;
  parameter_values.reserve(parameters.size());
  for (const auto& parameter : parameters) {
    parameter_values.push_back(parameter.c_str());
  }
  PGresult* result =
      PQexecParams(db, sql.c_str(), static_cast<int>(parameter_values.size()),
                   nullptr, parameter_values.data(), nullptr, nullptr, 0);
  CheckPostgreSQLResult(result, PGRES_COMMAND_OK, db,
                        "Unable to execute PostgreSQL statement");
  PQclear(result);
}

sqlite3_int64 NowMillis() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

std::string TestKey(const TestInfo& test_info) {
  return std::string(test_info.test_suite_name()) + '\0' + test_info.name();
}
}  // namespace

SqlTestEventListener::SqlTestEventListener(std::filesystem::path db_path)
    : m_type(Type::SQLite) {
  const int result = sqlite3_open_v2(
      db_path.string().c_str(), &m_sqlite_db,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
  if (result != SQLITE_OK) {
    const std::string error = m_sqlite_db != nullptr
                                  ? sqlite3_errmsg(m_sqlite_db)
                                  : "unable to allocate SQLite handle";
    if (m_sqlite_db != nullptr) {
      sqlite3_close(m_sqlite_db);
      m_sqlite_db = nullptr;
    }
    throw std::runtime_error("Unable to open SQLite database '" +
                             db_path.string() + "': " + error);
  }
  ExecuteSqlite(m_sqlite_db, "PRAGMA foreign_keys = ON;", {}, {});
  ExecuteSqlite(m_sqlite_db, "PRAGMA user_version = 1;", {}, {});
  for (const char* file_name : {"program.sql", "environment.sql", "suite.sql",
                                "test.sql"}) {
    ExecuteSqliteScript(m_sqlite_db, Schema(m_type, file_name), file_name);
  }
}

SqlTestEventListener::SqlTestEventListener(std::string url)
    : m_type(Type::PostgreSQL), m_postgresql_db(ConnectPostgreSQL(url)) {
  if (m_postgresql_db == nullptr) {
    throw std::runtime_error("Unable to connect to PostgreSQL");
  }
  CheckPostgreSQLConnection(m_postgresql_db);
  for (const char* file_name : {"program.sql", "environment.sql", "suite.sql",
                                "test.sql"}) {
    ExecutePostgreSQLScript(m_postgresql_db, Schema(m_type, file_name),
                            file_name);
  }
  MigratePostgreSQLTimestampColumns(m_postgresql_db);
}

SqlTestEventListener::~SqlTestEventListener() {
  if (m_sqlite_db != nullptr) {
    sqlite3_close(m_sqlite_db);
  }
  if (m_postgresql_db != nullptr) {
    PQfinish(m_postgresql_db);
  }
}

void SqlTestEventListener::OnTestProgramStart(const UnitTest& unit_test) {
  m_program_start_timestamp = unit_test.start_timestamp();
  const std::string sql = ReadStatement(m_type, "program.sql", "program_insert");
  m_program_id = m_type == Type::SQLite
                     ? InsertRowSqlite(m_sqlite_db, sql, {"program", "running"},
                                       {m_program_start_timestamp})
                     : InsertRowPostgreSQL(m_postgresql_db, sql,
                                            {"program", "running"},
                                            {m_program_start_timestamp});
}

void SqlTestEventListener::OnTestIterationStart(const UnitTest&, int) {}
void SqlTestEventListener::OnEnvironmentsSetUpStart(const UnitTest&) {
  m_environment_start_timestamp = NowMillis();
  const std::string sql =
      ReadStatement(m_type, "environment.sql", "environment_insert");
  m_environment_id =
      m_type == Type::SQLite
          ? InsertRowSqlite(m_sqlite_db, sql, {"environment", "running"},
                            {m_environment_start_timestamp, m_program_id})
          : InsertRowPostgreSQL(m_postgresql_db, sql, {"environment", "running"},
                                {m_environment_start_timestamp, m_program_id});
}
void SqlTestEventListener::OnEnvironmentsSetUpEnd(const UnitTest&) {
  const std::string sql =
      ReadStatement(m_type, "environment.sql", "environment_setup_update");
  if (m_type == Type::SQLite) {
    ExecuteSqlite(m_sqlite_db, sql, {"setup_complete"}, {m_environment_id});
  } else {
    ExecutePostgreSQL(m_postgresql_db, sql, {"setup_complete"},
                      {m_environment_id});
  }
}
void SqlTestEventListener::OnTestSuiteStart(const TestSuite& test_suite) {
  const auto start_timestamp = NowMillis();
  m_suite_start_timestamps[test_suite.name()] = start_timestamp;
  const std::string sql = ReadStatement(m_type, "suite.sql", "suite_insert");
  m_suite_ids[test_suite.name()] =
      m_type == Type::SQLite
          ? InsertRowSqlite(m_sqlite_db, sql, {test_suite.name(), "running"},
                            {start_timestamp, m_program_id})
          : InsertRowPostgreSQL(m_postgresql_db, sql,
                                {test_suite.name(), "running"},
                                {start_timestamp, m_program_id});
}
#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
void SqlTestEventListener::OnTestCaseStart(const TestCase&) {}
#endif
void SqlTestEventListener::OnTestStart(const TestInfo& test_info) {
  const auto suite_id = m_suite_ids.at(test_info.test_suite_name());
  const auto start_timestamp = NowMillis();
  m_test_start_timestamps[TestKey(test_info)] = start_timestamp;
  const std::string sql = ReadStatement(m_type, "test.sql", "test_insert");
  m_test_ids[TestKey(test_info)] =
      m_type == Type::SQLite
          ? InsertRowSqlite(m_sqlite_db, sql, {test_info.name(), "running"},
                            {start_timestamp, suite_id})
          : InsertRowPostgreSQL(m_postgresql_db, sql,
                                {test_info.name(), "running"},
                                {start_timestamp, suite_id});
}
void SqlTestEventListener::OnTestDisabled(const TestInfo& test_info) {
  const auto suite_id = m_suite_ids.at(test_info.test_suite_name());
  const std::string sql =
      ReadStatement(m_type, "test.sql", "test_disabled_insert");
  m_test_ids[TestKey(test_info)] =
      m_type == Type::SQLite
          ? InsertRowSqlite(m_sqlite_db, sql, {test_info.name(), "disabled"},
                            {suite_id})
          : InsertRowPostgreSQL(m_postgresql_db, sql,
                                {test_info.name(), "disabled"}, {suite_id});
}
void SqlTestEventListener::OnTestPartResult(const TestPartResult&) {}
void SqlTestEventListener::OnTestEnd(const TestInfo& test_info) {
  const TestResult* result = test_info.result();
  const char* result_name = result->Skipped() ? "skipped"
                            : result->Failed() ? "failed"
                                               : "passed";
  const std::string sql = ReadStatement(m_type, "test.sql", "test_update");
  const std::vector<sqlite3_int64> integers = {
      m_test_start_timestamps.at(TestKey(test_info)) + result->elapsed_time(),
      m_test_ids.at(TestKey(test_info))};
  if (m_type == Type::SQLite) {
    ExecuteSqlite(m_sqlite_db, sql, {result_name}, integers);
  } else {
    ExecutePostgreSQL(m_postgresql_db, sql, {result_name}, integers);
  }
}
void SqlTestEventListener::OnTestSuiteEnd(const TestSuite& test_suite) {
  const int incomplete_count = test_suite.total_test_count() -
                               test_suite.successful_test_count() -
                               test_suite.failed_test_count() -
                               test_suite.skipped_test_count();
  const std::string sql = ReadStatement(m_type, "suite.sql", "suite_update");
  const std::vector<sqlite3_int64> integers = {
      m_suite_start_timestamps.at(test_suite.name()) + test_suite.elapsed_time(),
      test_suite.successful_test_count(), test_suite.failed_test_count(),
      test_suite.skipped_test_count(), incomplete_count,
      m_suite_ids.at(test_suite.name())};
  if (m_type == Type::SQLite) {
    ExecuteSqlite(m_sqlite_db, sql, {test_suite.Failed() ? "failed" : "passed"},
                  integers);
  } else {
    ExecutePostgreSQL(m_postgresql_db, sql,
                      {test_suite.Failed() ? "failed" : "passed"}, integers);
  }
}
#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
void SqlTestEventListener::OnTestCaseEnd(const TestCase&) {}
#endif
void SqlTestEventListener::OnEnvironmentsTearDownStart(const UnitTest&) {
  const std::string sql =
      ReadStatement(m_type, "environment.sql", "environment_setup_update");
  if (m_type == Type::SQLite) {
    ExecuteSqlite(m_sqlite_db, sql, {"tearing_down"}, {m_environment_id});
  } else {
    ExecutePostgreSQL(m_postgresql_db, sql, {"tearing_down"},
                      {m_environment_id});
  }
}
void SqlTestEventListener::OnEnvironmentsTearDownEnd(const UnitTest& unit_test) {
  const std::string sql =
      ReadStatement(m_type, "environment.sql", "environment_update");
  const std::vector<sqlite3_int64> integers = {
      NowMillis(), unit_test.successful_test_count(), unit_test.failed_test_count(),
      unit_test.skipped_test_count(),
      unit_test.total_test_count() - unit_test.successful_test_count() -
          unit_test.failed_test_count() - unit_test.skipped_test_count(),
      m_environment_id};
  if (m_type == Type::SQLite) {
    ExecuteSqlite(m_sqlite_db, sql, {unit_test.Failed() ? "failed" : "passed"},
                  integers);
  } else {
    ExecutePostgreSQL(m_postgresql_db, sql,
                      {unit_test.Failed() ? "failed" : "passed"}, integers);
  }
}
void SqlTestEventListener::OnTestIterationEnd(const UnitTest&, int) {}
void SqlTestEventListener::OnTestProgramEnd(const UnitTest& unit_test) {
  const std::string sql = ReadStatement(m_type, "program.sql", "program_update");
  const std::vector<sqlite3_int64> integers = {
      m_program_start_timestamp + unit_test.elapsed_time(),
      unit_test.successful_test_count(), unit_test.failed_test_count(),
      unit_test.skipped_test_count(),
      unit_test.total_test_count() - unit_test.successful_test_count() -
          unit_test.failed_test_count() - unit_test.skipped_test_count(),
      m_program_id};
  if (m_type == Type::SQLite) {
    ExecuteSqlite(m_sqlite_db, sql, {unit_test.Failed() ? "failed" : "passed"},
                  integers);
  } else {
    ExecutePostgreSQL(m_postgresql_db, sql,
                      {unit_test.Failed() ? "failed" : "passed"}, integers);
  }
}
}  // namespace testing
