


#include "sql_test_event_listner.hpp"

#include <iostream>
#include <stdexcept>

namespace testing {

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

  char* error_message = nullptr;
  const int write_result =
      sqlite3_exec(m_db, "PRAGMA user_version = 1;", nullptr, nullptr,
                   &error_message);
  if (write_result != SQLITE_OK) {
    const std::string error =
        error_message != nullptr ? error_message : "unable to initialize database";
    sqlite3_free(error_message);
    sqlite3_close(m_db);
    m_db = nullptr;
    throw std::runtime_error("Unable to initialize SQLite database '" +
                             db_path.string() + "': " + error);
  }
}

SqlTestEventListener::~SqlTestEventListener() {
  if (m_db != nullptr) {
    sqlite3_close(m_db);
  }
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
