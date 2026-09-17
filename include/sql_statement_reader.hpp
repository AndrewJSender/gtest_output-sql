// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#ifndef SQL_STATEMENT_READER_HPP_
#define SQL_STATEMENT_READER_HPP_

#include <string>

namespace testing {

// Returns the CREATE TABLE portion (everything before the first "-- marker"
// comment) of the SQL file `file_name` in the `dialect` schema directory
// (e.g. "sqlite" or "postgresql").
std::string ReadSqlSchema(const char* dialect, const char* file_name);

// Returns the SQL statement immediately following the "-- marker" comment in
// the SQL file `file_name` in the `dialect` schema directory.
std::string ReadSqlStatement(const char* dialect, const char* file_name,
                             const char* marker);

}  // namespace testing

#endif  // SQL_STATEMENT_READER_HPP_
