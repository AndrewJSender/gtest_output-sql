// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#ifndef SQL_BACKEND_HPP_
#define SQL_BACKEND_HPP_

#include <cstdint>
#include <string>
#include <vector>

namespace testing {

// Abstracts the SQL dialect-specific operations the event listener needs so
// that SqlTestEventListener never has to branch on database type.
class SqlBackend {
 public:
  virtual ~SqlBackend() = default;

  // Executes the marked statement in `file_name`, ignoring any result rows.
  virtual void Execute(const char* file_name, const char* marker,
                       const std::vector<std::string>& values,
                       const std::vector<std::int64_t>& integers) = 0;

  // Executes the marked insert statement in `file_name` and returns the
  // primary key of the inserted row.
  virtual std::int64_t InsertRow(
      const char* file_name, const char* marker,
      const std::vector<std::string>& values,
      const std::vector<std::int64_t>& integers) = 0;
};

}  // namespace testing

#endif  // SQL_BACKEND_HPP_
