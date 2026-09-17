// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#ifndef SQLITE_BACKEND_HPP_
#define SQLITE_BACKEND_HPP_

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "sql_backend.hpp"
#include "sqlite3.h"

namespace testing {

// SQLite implementation of SqlBackend. Owns the SQLite connection and the
// schema/statement files under the "sqlite" SQL schema directory.
class SqliteBackend : public SqlBackend {
 public:
  explicit SqliteBackend(std::filesystem::path db_path);
  ~SqliteBackend() override;

  void Execute(const char* file_name, const char* marker,
              const std::vector<std::string>& values,
              const std::vector<std::int64_t>& integers) override;
  std::int64_t InsertRow(const char* file_name, const char* marker,
                         const std::vector<std::string>& values,
                         const std::vector<std::int64_t>& integers) override;

 private:
  void ExecuteScript(const char* file_name);
  void ExecuteStatement(const std::string& sql,
                        const std::vector<std::string>& values,
                        const std::vector<std::int64_t>& integers);

  sqlite3* m_db = nullptr;
};

}  // namespace testing

#endif  // SQLITE_BACKEND_HPP_
