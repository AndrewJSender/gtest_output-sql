


#include "sql_test_event_listner.hpp"

#include <iostream>
#include <stdexcept>
#include <chrono>
#include <fstream>
#include <string>

namespace testing {

namespace {

std::string ReadSql(const char* file_name) {
#ifdef SQL_TEST_EVENT_LISTENER_SQL_DIR
  const std::filesystem::path sql_path =
      std::filesystem::path(SQL_TEST_EVENT_LISTENER_SQL_DIR) / file_name;
#else
  const std::filesystem::path sql_path =
      std::filesystem::path("sql") / file_name;
#endif
  std::ifstream file(sql_path);
  if (!file) {
    throw std::runtime_error("Unable to open SQL file '" + sql_path.string() +
                             "'");
  }
  return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

void CheckSqlite(int result, sqlite3* db, const char* operation) {
  if (result != SQLITE_OK && result != SQLITE_DONE && result != SQLITE_ROW) {
    throw std::runtime_error(std::string(operation) + ": " + sqlite3_errmsg(db));
  }
}

void ExecuteScript(sqlite3* db, const char* file_name) {
  std::string sql = ReadSql(file_name);
  const std::size_t statement_marker = sql.find("-- ");
  if (statement_marker != std::string::npos) {
    sql.resize(statement_marker);
  }
  char* error_message = nullptr;
  const int result =
      sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error_message);
  if (result != SQLITE_OK) {
    const std::string error =
        error_message != nullptr ? error_message : "unknown SQLite error";
    sqlite3_free(error_message);
    throw std::runtime_error("Unable to execute SQL file '" +
                             std::string(file_name) + "': " + error);
  }
}

std::string ReadStatement(const char* file_name, const char* marker) {
  const std::string sql = ReadSql(file_name);
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

sqlite3_int64 InsertRow(sqlite3* db, const char* sql,
                        const std::vector<std::string>& values,
                        const std::vector<sqlite3_int64>& integers) {
  sqlite3_stmt* statement = nullptr;
  CheckSqlite(sqlite3_prepare_v2(db, sql, -1, &statement, nullptr), db,
              "Unable to prepare SQLite statement");
  int parameter = 1;
  for (const auto& value : values) {
    CheckSqlite(sqlite3_bind_text(statement, parameter++, value.c_str(), -1,
                                  SQLITE_TRANSIENT),
                db, "Unable to bind SQLite text");
  }
  for (const auto integer : integers) {
    CheckSqlite(sqlite3_bind_int64(statement, parameter++, integer), db,
                "Unable to bind SQLite integer");
  }
  CheckSqlite(sqlite3_step(statement), db, "Unable to insert SQLite row");
  sqlite3_finalize(statement);
  return sqlite3_last_insert_rowid(db);
}

void Execute(sqlite3* db, const char* sql,
             const std::vector<std::string>& values,
             const std::vector<sqlite3_int64>& integers) {
  sqlite3_stmt* statement = nullptr;
  CheckSqlite(sqlite3_prepare_v2(db, sql, -1, &statement, nullptr), db,
              "Unable to prepare SQLite statement");
  int parameter = 1;
  for (const auto& value : values) {
    CheckSqlite(sqlite3_bind_text(statement, parameter++, value.c_str(), -1,
                                  SQLITE_TRANSIENT),
                db, "Unable to bind SQLite text");
  }
  for (const auto integer : integers) {
    CheckSqlite(sqlite3_bind_int64(statement, parameter++, integer), db,
                "Unable to bind SQLite integer");
  }
  CheckSqlite(sqlite3_step(statement), db, "Unable to update SQLite row");
  sqlite3_finalize(statement);
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

SqlTestEventListener::SqlTestEventListener(std::filesystem::path db_path) {
  const int result = sqlite3_open_v2(
      db_path.string().c_str(), &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
      nullptr);
  if (result != SQLITE_OK) {
    const std::string error = m_db != nullptr ? sqlite3_errmsg(m_db)
                                              : "unable to allocate SQLite handle";
    if (m_db != nullptr) {
      sqlite3_close(m_db);
      m_db = nullptr;
    }
    throw std::runtime_error("Unable to open SQLite database '" +
                             db_path.string() + "': " + error);
  }

  Execute(m_db, "PRAGMA foreign_keys = ON;", {}, {});
  Execute(m_db, "PRAGMA user_version = 1;", {}, {});
  ExecuteScript(m_db, "program.sql");
  ExecuteScript(m_db, "environment.sql");
  ExecuteScript(m_db, "suite.sql");
  ExecuteScript(m_db, "test.sql");
  const int write_result = SQLITE_OK;
  if (write_result != SQLITE_OK) {
    sqlite3_close(m_db);
    m_db = nullptr;
    throw std::runtime_error("Unable to initialize SQLite database '" +
                             db_path.string() + "': unable to initialize database");
  }
}

SqlTestEventListener::~SqlTestEventListener() {
  if (m_db != nullptr) {
    sqlite3_close(m_db);
  }
}

void SqlTestEventListener::OnTestProgramStart(const UnitTest& unit_test) {
  m_program_id = InsertRow(
      m_db,
      ReadStatement("program.sql", "program_insert").c_str(),
      {"program", "running"}, {unit_test.start_timestamp()});
}

void SqlTestEventListener::OnTestIterationStart(const UnitTest&, int) {}
void SqlTestEventListener::OnEnvironmentsSetUpStart(const UnitTest&) {
  m_environment_id = InsertRow(
      m_db,
      ReadStatement("environment.sql", "environment_insert").c_str(),
      {"environment", "running"}, {NowMillis(), m_program_id});
}
void SqlTestEventListener::OnEnvironmentsSetUpEnd(const UnitTest&) {
  Execute(m_db, ReadStatement("environment.sql", "environment_setup_update").c_str(),
          {"setup_complete"}, {m_environment_id});
}
void SqlTestEventListener::OnTestSuiteStart(const TestSuite& test_suite) {
  m_suite_ids[test_suite.name()] = InsertRow(
      m_db,
      ReadStatement("suite.sql", "suite_insert").c_str(),
      {test_suite.name(), "running"},
      {test_suite.start_timestamp(), m_program_id});
}
#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
void SqlTestEventListener::OnTestCaseStart(const TestCase&) {}
#endif
void SqlTestEventListener::OnTestStart(const TestInfo& test_info) {
  const auto suite_id = m_suite_ids.at(test_info.test_suite_name());
  m_test_ids[TestKey(test_info)] = InsertRow(
      m_db, ReadStatement("test.sql", "test_insert").c_str(),
      {test_info.name(), "running"},
      {NowMillis(), suite_id});
}
void SqlTestEventListener::OnTestDisabled(const TestInfo& test_info) {
  const auto suite_id = m_suite_ids.at(test_info.test_suite_name());
  m_test_ids[TestKey(test_info)] = InsertRow(
      m_db,
      ReadStatement("test.sql", "test_disabled_insert").c_str(),
      {test_info.name(), "disabled"}, {suite_id, 1});
}
void SqlTestEventListener::OnTestPartResult(const TestPartResult&) {}
void SqlTestEventListener::OnTestEnd(const TestInfo& test_info) {
  const TestResult* result = test_info.result();
  const char* result_name = result->Skipped() ? "skipped"
                            : result->Failed() ? "failed"
                                               : "passed";
  const int pass_count = result->Passed() ? 1 : 0;
  const int failed_count = result->Failed() ? 1 : 0;
  const int skip_count = result->Skipped() ? 1 : 0;
  Execute(m_db,
      ReadStatement("test.sql", "test_update").c_str(),
          {result_name},
          {result->start_timestamp() + result->elapsed_time(), pass_count,
           failed_count, skip_count, 0, m_test_ids.at(TestKey(test_info))});
}
void SqlTestEventListener::OnTestSuiteEnd(const TestSuite& test_suite) {
  const int incomplete_count =
      test_suite.total_test_count() - test_suite.successful_test_count() -
      test_suite.failed_test_count() - test_suite.skipped_test_count();
  Execute(m_db,
      ReadStatement("suite.sql", "suite_update").c_str(),
          {test_suite.Failed() ? "failed" : "passed"},
          {test_suite.start_timestamp() + test_suite.elapsed_time(),
           test_suite.successful_test_count(), test_suite.failed_test_count(),
           test_suite.skipped_test_count(), incomplete_count,
           m_suite_ids.at(test_suite.name())});
}
#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
void SqlTestEventListener::OnTestCaseEnd(const TestCase&) {}
#endif
void SqlTestEventListener::OnEnvironmentsTearDownStart(const UnitTest&) {
  Execute(m_db,
          ReadStatement("environment.sql", "environment_setup_update").c_str(),
          {"tearing_down"}, {m_environment_id});
}
void SqlTestEventListener::OnEnvironmentsTearDownEnd(const UnitTest& unit_test) {
  Execute(m_db,
      ReadStatement("environment.sql", "environment_update").c_str(),
          {unit_test.Failed() ? "failed" : "passed"},
          {NowMillis(), unit_test.successful_test_count(),
           unit_test.failed_test_count(), unit_test.skipped_test_count(),
           unit_test.total_test_count() - unit_test.successful_test_count() -
               unit_test.failed_test_count() - unit_test.skipped_test_count(),
           m_environment_id});
}
void SqlTestEventListener::OnTestIterationEnd(const UnitTest&, int) {}
void SqlTestEventListener::OnTestProgramEnd(const UnitTest& unit_test) {
  Execute(m_db,
      ReadStatement("program.sql", "program_update").c_str(),
          {unit_test.Failed() ? "failed" : "passed"},
          {unit_test.start_timestamp() + unit_test.elapsed_time(),
           unit_test.successful_test_count(), unit_test.failed_test_count(),
           unit_test.skipped_test_count(),
           unit_test.total_test_count() - unit_test.successful_test_count() -
               unit_test.failed_test_count() - unit_test.skipped_test_count(),
           m_program_id});
}

}  // namespace testing
