// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#ifndef POSTGRESQL_BACKEND_HPP_
#define POSTGRESQL_BACKEND_HPP_

#include <cstdint>
#include <string>
#include <vector>

#include <libpq-fe.h>

#include "sql_backend.hpp"

namespace testing {

// PostgreSQL implementation of SqlBackend. Owns the libpq connection and the
// schema/statement files under the "postgresql" SQL schema directory.
class PostgreSqlBackend : public SqlBackend {
 public:
  explicit PostgreSqlBackend(const std::string& url);
  ~PostgreSqlBackend() override;

  void Execute(const char* file_name, const char* marker,
              const std::vector<std::string>& values,
              const std::vector<std::int64_t>& integers) override;
  std::int64_t InsertRow(const char* file_name, const char* marker,
                         const std::vector<std::string>& values,
                         const std::vector<std::int64_t>& integers) override;

 private:
  void ExecuteScript(const char* file_name);
  void MigrateTimestampColumns();
  PGresult* ExecuteStatement(const std::string& sql,
                             const std::vector<std::string>& values,
                             const std::vector<std::int64_t>& integers);

  PGconn* m_db = nullptr;
};

}  // namespace testing

#endif  // POSTGRESQL_BACKEND_HPP_
