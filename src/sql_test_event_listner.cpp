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

const char* TestPartResultTypeName(TestPartResult::Type type) {
  switch (type) {
    case TestPartResult::kSuccess:
      return "success";
    case TestPartResult::kNonFatalFailure:
      return "nonfatal_failure";
    case TestPartResult::kFatalFailure:
      return "fatal_failure";
    case TestPartResult::kSkip:
      return "skip";
  }
  return "unknown";
}

}  // namespace

SqlTestEventListener::SqlTestEventListener(std::filesystem::path db_path)
    : m_backend(std::make_unique<SqliteBackend>(std::move(db_path))) {}

SqlTestEventListener::SqlTestEventListener(PostgreSqlTag, std::string url)
    : m_backend(std::make_unique<PostgreSqlBackend>(url)) {}

SqlTestEventListener::~SqlTestEventListener() = default;

void SqlTestEventListener::OnTestProgramStart(const UnitTest& unit_test) {
  m_program_start_timestamp = unit_test.start_timestamp();
  m_program_id = m_backend->InsertRow("program.sql", "program_insert",
                                      {"running"}, {m_program_start_timestamp});
}

void SqlTestEventListener::OnTestIterationStart(const UnitTest&,
                                                int iteration) {
  const auto start_timestamp = NowMillis();
  m_iteration_start_timestamps[iteration] = start_timestamp;
  m_iteration_ids[iteration] = m_backend->InsertRow(
      "iteration.sql", "iteration_insert", {"running"},
      {iteration, start_timestamp, m_program_id});
}

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
  m_current_test_id = m_backend->InsertRow(
      "test.sql", "test_insert", {test_info.name(), "running"},
      {start_timestamp, suite_id});
  m_test_ids[TestKey(test_info)] = m_current_test_id;
}

void SqlTestEventListener::OnTestDisabled(const TestInfo& test_info) {
  const auto suite_id = m_suite_ids.at(test_info.test_suite_name());
  m_test_ids[TestKey(test_info)] = m_backend->InsertRow(
      "test.sql", "test_disabled_insert", {test_info.name(), "disabled"},
      {suite_id});
}

void SqlTestEventListener::OnTestPartResult(
    const TestPartResult& test_part_result) {
  if (m_current_test_id == 0) {
    // Test part results reported outside of a running test (e.g. during
    // environment setup) have no test row to attach to.
    return;
  }
  const char* file_name = test_part_result.file_name();
  m_backend->InsertRow(
      "test_result_part.sql", "test_result_part_insert",
      {TestPartResultTypeName(test_part_result.type()),
       file_name != nullptr ? file_name : "", test_part_result.message()},
      {m_current_test_id, test_part_result.line_number(), NowMillis()});
}

void SqlTestEventListener::OnTestEnd(const TestInfo& test_info) {
  const TestResult* result = test_info.result();
  const char* result_name = result->Skipped() ? "skipped"
                            : result->Failed() ? "failed"
                                               : "passed";
  m_backend->Execute(
      "test.sql", "test_update", {result_name},
      {m_test_start_timestamps.at(TestKey(test_info)) + result->elapsed_time(),
       m_test_ids.at(TestKey(test_info))});
  m_current_test_id = 0;
}

void SqlTestEventListener::OnTestSuiteEnd(const TestSuite& test_suite) {
  m_backend->Execute(
      "suite.sql", "suite_update", {test_suite.Failed() ? "failed" : "passed"},
      {m_suite_start_timestamps.at(test_suite.name()) +
           test_suite.elapsed_time(),
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
      {NowMillis(), m_environment_id});
}

void SqlTestEventListener::OnTestIterationEnd(const UnitTest& unit_test,
                                              int iteration) {
  m_backend->Execute(
      "iteration.sql", "iteration_update",
      {unit_test.Failed() ? "failed" : "passed"},
      {NowMillis(), m_iteration_ids.at(iteration)});
}

void SqlTestEventListener::OnTestProgramEnd(const UnitTest& unit_test) {
  m_backend->Execute(
      "program.sql", "program_update",
      {unit_test.Failed() ? "failed" : "passed"},
      {m_program_start_timestamp + unit_test.elapsed_time(), m_program_id});
}

}  // namespace testing
