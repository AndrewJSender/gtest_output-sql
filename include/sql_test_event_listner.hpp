// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#ifndef SQL_TEST_EVENT_LISTNER_HPP_
#define SQL_TEST_EVENT_LISTNER_HPP_

#include <gtest/gtest.h>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace testing {

class SqlBackend;

// GoogleTest event listener that streams test results to a SQL database as
// testing progresses. Supports SQLite and PostgreSQL via SqlBackend
// implementations selected by the constructor overload used.
class SqlTestEventListener : public TestEventListener {
 public:
  struct PostgreSqlTag {};

  explicit SqlTestEventListener(std::filesystem::path db_path);
  SqlTestEventListener(PostgreSqlTag, std::string url);
  ~SqlTestEventListener() override;
  void OnTestProgramStart(const UnitTest& unit_test) override;
  void OnTestIterationStart(const UnitTest& unit_test,
                            int iteration) override;
  void OnEnvironmentsSetUpStart(const UnitTest& unit_test) override;
  void OnEnvironmentsSetUpEnd(const UnitTest& unit_test) override;
  void OnTestSuiteStart(const TestSuite& test_suite) override;
//  Legacy API is deprecated but still available
#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  void OnTestCaseStart(const TestCase& test_case) override;
#endif  //  GTEST_REMOVE_LEGACY_TEST_CASEAPI_

  void OnTestStart(const TestInfo& test_info) override;
  void OnTestDisabled(const TestInfo& test_info) override;
  void OnTestPartResult(const TestPartResult& test_part_result) override;
  void OnTestEnd(const TestInfo& test_info) override;
  void OnTestSuiteEnd(const TestSuite& test_suite) override;
#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  void OnTestCaseEnd(const TestCase& test_case) override;
#endif  //  GTEST_REMOVE_LEGACY_TEST_CASEAPI_

  void OnEnvironmentsTearDownStart(const UnitTest& unit_test) override;
  void OnEnvironmentsTearDownEnd(const UnitTest& unit_test) override;
  void OnTestIterationEnd(const UnitTest& unit_test,
                          int iteration) override;
  void OnTestProgramEnd(const UnitTest& unit_test) override;

 private:
  std::unique_ptr<SqlBackend> m_backend;
  std::int64_t m_program_id = 0;
  std::int64_t m_program_start_timestamp = 0;
  std::unordered_map<int, std::int64_t> m_iteration_ids;
  std::unordered_map<int, std::int64_t> m_iteration_start_timestamps;
  std::int64_t m_environment_id = 0;
  std::int64_t m_environment_start_timestamp = 0;
  std::unordered_map<std::string, std::int64_t> m_suite_ids;
  std::unordered_map<std::string, std::int64_t> m_suite_start_timestamps;
  std::unordered_map<std::string, std::int64_t> m_test_ids;
  std::unordered_map<std::string, std::int64_t> m_test_start_timestamps;
  std::int64_t m_current_test_id = 0;
};

}  // namespace testing

#endif  // SQL_TEST_EVENT_LISTNER_HPP_
