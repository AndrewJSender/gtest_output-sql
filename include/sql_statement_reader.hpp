// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#ifndef SQL_STATEMENT_READER_HPP_
#define SQL_STATEMENT_READER_HPP_

#include <string>

namespace testing {

// Returns the CREATE TABLE portion (everything before the first "-- marker"
// comment) of the SQL file `file_name` in the shared `sql/` directory,
// rewritten for `dialect` ("sqlite" or "postgresql"). The files use
// dialect-neutral placeholders ("{{PK}}", "{{INT}}", "{{RETURNING}}") that
// are substituted per dialect.
std::string ReadSqlSchema(const char* dialect, const char* file_name);

// Returns the SQL statement immediately following the "-- marker" comment in
// the SQL file `file_name`, rewritten for `dialect`. For "postgresql", "?"
// parameter placeholders are rewritten to "$1", "$2", ... in order.
std::string ReadSqlStatement(const char* dialect, const char* file_name,
                             const char* marker);

}  // namespace testing

#endif  // SQL_STATEMENT_READER_HPP_
