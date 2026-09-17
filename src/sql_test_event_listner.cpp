// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#include "sql_test_event_listner.hpp"

#include <chrono>
#include <utility>

#include "postgresql_backend.hpp"
#include "sqlite_backend.hpp"

namespace testing {
namespace {

std::int64_t NowMillis() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

std::string TestKey(const TestInfo& test_info) {
  return std::string(test_info.test_suite_name()) + '\0' + test_info.name();
}

}  // namespace

SqlTestEventListener::SqlTestEventListener(std::filesystem::path db_path)
    : m_backend(std::make_unique<SqliteBackend>(std::move(db_path))) {}

SqlTestEventListener::SqlTestEventListener(std::string url)
    : m_backend(std::make_unique<PostgreSqlBackend>(url)) {}

SqlTestEventListener::~SqlTestEventListener() = default;

void SqlTestEventListener::OnTestProgramStart(const UnitTest& unit_test) {
  m_program_start_timestamp = unit_test.start_timestamp();
  m_program_id = m_backend->InsertRow("program.sql", "program_insert",
                                      {"running"}, {m_program_start_timestamp});
}

void SqlTestEventListener::OnTestIterationStart(const UnitTest&, int) {}

void SqlTestEventListener::OnEnvironmentsSetUpStart(const UnitTest&) {
  m_environment_start_timestamp = NowMillis();
  m_environment_id = m_backend->InsertRow(
      "environment.sql", "environment_insert", {"running"},
      {m_environment_start_timestamp, m_program_id});
}

void SqlTestEventListener::OnEnvironmentsSetUpEnd(const UnitTest&) {
  m_backend->Execute("environment.sql", "environment_setup_update",
                     {"setup_complete"}, {m_environment_id});
}

void SqlTestEventListener::OnTestSuiteStart(const TestSuite& test_suite) {
  const auto start_timestamp = NowMillis();
  m_suite_start_timestamps[test_suite.name()] = start_timestamp;
  m_suite_ids[test_suite.name()] = m_backend->InsertRow(
      "suite.sql", "suite_insert", {test_suite.name(), "running"},
      {start_timestamp, m_program_id});
}

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
void SqlTestEventListener::OnTestCaseStart(const TestCase&) {}
#endif

void SqlTestEventListener::OnTestStart(const TestInfo& test_info) {
  const auto suite_id = m_suite_ids.at(test_info.test_suite_name());
  const auto start_timestamp = NowMillis();
  m_test_start_timestamps[TestKey(test_info)] = start_timestamp;
  m_test_ids[TestKey(test_info)] = m_backend->InsertRow(
      "test.sql", "test_insert", {test_info.name(), "running"},
      {start_timestamp, suite_id});
}

void SqlTestEventListener::OnTestDisabled(const TestInfo& test_info) {
  const auto suite_id = m_suite_ids.at(test_info.test_suite_name());
  m_test_ids[TestKey(test_info)] = m_backend->InsertRow(
      "test.sql", "test_disabled_insert", {test_info.name(), "disabled"},
      {suite_id});
}

void SqlTestEventListener::OnTestPartResult(const TestPartResult&) {}

void SqlTestEventListener::OnTestEnd(const TestInfo& test_info) {
  const TestResult* result = test_info.result();
  const char* result_name = result->Skipped() ? "skipped"
                            : result->Failed() ? "failed"
                                               : "passed";
  m_backend->Execute(
      "test.sql", "test_update", {result_name},
      {m_test_start_timestamps.at(TestKey(test_info)) + result->elapsed_time(),
       m_test_ids.at(TestKey(test_info))});
}

void SqlTestEventListener::OnTestSuiteEnd(const TestSuite& test_suite) {
  const int incomplete_count = test_suite.total_test_count() -
                               test_suite.successful_test_count() -
                               test_suite.failed_test_count() -
                               test_suite.skipped_test_count();
  m_backend->Execute(
      "suite.sql", "suite_update", {test_suite.Failed() ? "failed" : "passed"},
      {m_suite_start_timestamps.at(test_suite.name()) +
           test_suite.elapsed_time(),
       test_suite.successful_test_count(), test_suite.failed_test_count(),
       test_suite.skipped_test_count(), incomplete_count,
       m_suite_ids.at(test_suite.name())});
}

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
void SqlTestEventListener::OnTestCaseEnd(const TestCase&) {}
#endif

void SqlTestEventListener::OnEnvironmentsTearDownStart(const UnitTest&) {
  m_backend->Execute("environment.sql", "environment_setup_update",
                     {"tearing_down"}, {m_environment_id});
}

void SqlTestEventListener::OnEnvironmentsTearDownEnd(const UnitTest& unit_test) {
  m_backend->Execute(
      "environment.sql", "environment_update",
      {unit_test.Failed() ? "failed" : "passed"},
      {NowMillis(), unit_test.successful_test_count(),
       unit_test.failed_test_count(), unit_test.skipped_test_count(),
       unit_test.total_test_count() - unit_test.successful_test_count() -
           unit_test.failed_test_count() - unit_test.skipped_test_count(),
       m_environment_id});
}

void SqlTestEventListener::OnTestIterationEnd(const UnitTest&, int) {}

void SqlTestEventListener::OnTestProgramEnd(const UnitTest& unit_test) {
  m_backend->Execute(
      "program.sql", "program_update",
      {unit_test.Failed() ? "failed" : "passed"},
      {m_program_start_timestamp + unit_test.elapsed_time(),
       unit_test.successful_test_count(), unit_test.failed_test_count(),
       unit_test.skipped_test_count(),
       unit_test.total_test_count() - unit_test.successful_test_count() -
           unit_test.failed_test_count() - unit_test.skipped_test_count(),
       m_program_id});
}

}  // namespace testing
