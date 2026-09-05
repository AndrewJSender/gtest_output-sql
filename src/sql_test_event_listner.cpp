


#include <iostream>

// identity
#include "sql_test_event_listner.hpp"

#include "sqlite3.h"

namespace testing {

SqlTestEventListener::SqlTestEventListener(std::filesystem::path db_path) {
  (void)db_path;
}

void SqlTestEventListener::OnTestProgramStart(const UnitTest& unit_test) {
  std::cout << "Test program started." << std::endl;
}

void SqlTestEventListener::OnTestIterationStart(const UnitTest&, int) {}
void SqlTestEventListener::OnEnvironmentsSetUpStart(const UnitTest&) {}
void SqlTestEventListener::OnEnvironmentsSetUpEnd(const UnitTest&) {}
void SqlTestEventListener::OnTestSuiteStart(const TestSuite&) {}
#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
void SqlTestEventListener::OnTestCaseStart(const TestCase&) {}
#endif
void SqlTestEventListener::OnTestStart(const TestInfo&) {}
void SqlTestEventListener::OnTestDisabled(const TestInfo&) {}
void SqlTestEventListener::OnTestPartResult(const TestPartResult&) {}
void SqlTestEventListener::OnTestEnd(const TestInfo&) {}
void SqlTestEventListener::OnTestSuiteEnd(const TestSuite&) {}
#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
void SqlTestEventListener::OnTestCaseEnd(const TestCase&) {}
#endif
void SqlTestEventListener::OnEnvironmentsTearDownStart(const UnitTest&) {}
void SqlTestEventListener::OnEnvironmentsTearDownEnd(const UnitTest&) {}
void SqlTestEventListener::OnTestIterationEnd(const UnitTest&, int) {}
void SqlTestEventListener::OnTestProgramEnd(const UnitTest&) {}

}  // namespace testing
