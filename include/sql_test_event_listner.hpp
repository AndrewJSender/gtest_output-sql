
// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#include <gtest/gtest.h>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "sqlite3.h"

namespace testing {

class SqlTestEventListener : public TestEventListener {
 public:
  explicit SqlTestEventListener(std::filesystem::path db_path);
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
 sqlite3* m_db = nullptr;
 sqlite3_int64 m_program_id = 0;
 sqlite3_int64 m_program_start_timestamp = 0;
 sqlite3_int64 m_environment_id = 0;
 sqlite3_int64 m_environment_start_timestamp = 0;
 std::unordered_map<std::string, sqlite3_int64> m_suite_ids;
 std::unordered_map<std::string, sqlite3_int64> m_suite_start_timestamps;
 std::unordered_map<std::string, sqlite3_int64> m_test_ids;
 std::unordered_map<std::string, sqlite3_int64> m_test_start_timestamps;
};

}  // namespace testing
