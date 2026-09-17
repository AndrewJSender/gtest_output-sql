// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#include "postgresql_backend.hpp"

#include <stdexcept>

#include "sql_statement_reader.hpp"

namespace testing {
namespace {

constexpr char kDialect[] = "postgresql";

std::string DecodeUrlComponent(const std::string& value) {
  std::string decoded;
  decoded.reserve(value.size());
  for (std::size_t i = 0; i < value.size(); ++i) {
    if (value[i] != '%') {
      decoded += value[i] == '+' ? ' ' : value[i];
      continue;
    }
    if (i + 2 >= value.size()) {
      throw std::runtime_error("Invalid percent-encoded PostgreSQL URL");
    }
    const auto hex_value = [](char character) -> int {
      if (character >= '0' && character <= '9') {
        return character - '0';
      }
      if (character >= 'a' && character <= 'f') {
        return character - 'a' + 10;
      }
      if (character >= 'A' && character <= 'F') {
        return character - 'A' + 10;
      }
      return -1;
    };
    const int high = hex_value(value[i + 1]);
    const int low = hex_value(value[i + 2]);
    if (high < 0 || low < 0) {
      throw std::runtime_error("Invalid percent-encoded PostgreSQL URL");
    }
    decoded += static_cast<char>((high << 4) | low);
    i += 2;
  }
  return decoded;
}

std::string QueryValue(const std::string& query, const std::string& name) {
  std::size_t start = 0;
  while (start < query.size()) {
    const std::size_t end = query.find('&', start);
    const std::string parameter =
        query.substr(start, end == std::string::npos ? end : end - start);
    const std::size_t equals = parameter.find('=');
    if (parameter.substr(0, equals) == name) {
      return equals == std::string::npos ? "" : parameter.substr(equals + 1);
    }
    if (end == std::string::npos) {
      break;
    }
    start = end + 1;
  }
  return "";
}

// Connects using a standard libpq connection URI, or, for Amazon RDS IAM
// authentication URLs (`Action=connect` with a `DBUser` query parameter),
// translates the endpoint and signed token into libpq connection parameters
// with the token supplied as the password over a required TLS connection.
PGconn* Connect(const std::string& url) {
  constexpr char postgresql_prefix[] = "postgresql://";
  const std::string endpoint = url.substr(sizeof(postgresql_prefix) - 1);
  const std::size_t query_start = endpoint.find('?');
  if (query_start == std::string::npos) {
    return PQconnectdb(url.c_str());
  }
  const std::string query = endpoint.substr(query_start + 1);
  const std::string user = QueryValue(query, "DBUser");
  if (QueryValue(query, "Action") != "connect" || user.empty()) {
    return PQconnectdb(url.c_str());
  }

  std::string authority = endpoint.substr(0, query_start);
  if (!authority.empty() && authority.back() == '/') {
    authority.pop_back();
  }
  const std::size_t port_separator = authority.rfind(':');
  const std::string host = authority.substr(0, port_separator);
  const std::string port = port_separator == std::string::npos
                               ? "5432"
                               : authority.substr(port_separator + 1);
  if (host.empty() || port.empty()) {
    throw std::runtime_error("Invalid PostgreSQL IAM authentication URL");
  }
  const std::string decoded_user = DecodeUrlComponent(user);
  const char* keywords[] = {"host", "port", "user", "password", "sslmode",
                            nullptr};
  const char* values[] = {host.c_str(), port.c_str(), decoded_user.c_str(),
                          endpoint.c_str(), "require", nullptr};
  return PQconnectdbParams(keywords, values, 0);
}

std::vector<std::string> Parameters(
    const std::vector<std::string>& values,
    const std::vector<std::int64_t>& integers) {
  std::vector<std::string> parameters = values;
  parameters.reserve(values.size() + integers.size());
  for (std::int64_t integer : integers) {
    parameters.push_back(std::to_string(integer));
  }
  return parameters;
}

}  // namespace

PostgreSqlBackend::PostgreSqlBackend(const std::string& url)
    : m_db(Connect(url)) {
  if (m_db == nullptr) {
    throw std::runtime_error("Unable to connect to PostgreSQL");
  }
  if (PQstatus(m_db) != CONNECTION_OK) {
    const std::string error = PQerrorMessage(m_db);
    PQfinish(m_db);
    m_db = nullptr;
    throw std::runtime_error("Unable to connect to PostgreSQL: " + error);
  }

  for (const char* file_name :
       {"program.sql", "environment.sql", "suite.sql", "test.sql"}) {
    ExecuteScript(file_name);
  }
  MigrateTimestampColumns();
}

PostgreSqlBackend::~PostgreSqlBackend() {
  if (m_db != nullptr) {
    PQfinish(m_db);
  }
}

PGresult* PostgreSqlBackend::ExecuteStatement(
    const std::string& sql, const std::vector<std::string>& values,
    const std::vector<std::int64_t>& integers) {
  const std::vector<std::string> parameters = Parameters(values, integers);
  std::vector<const char*> parameter_values;
  parameter_values.reserve(parameters.size());
  for (const auto& parameter : parameters) {
    parameter_values.push_back(parameter.c_str());
  }
  return PQexecParams(m_db, sql.c_str(),
                      static_cast<int>(parameter_values.size()), nullptr,
                      parameter_values.data(), nullptr, nullptr, 0);
}

void PostgreSqlBackend::ExecuteScript(const char* file_name) {
  const std::string sql = ReadSqlSchema(kDialect, file_name);
  PGresult* result = PQexec(m_db, sql.c_str());
  if (result == nullptr || PQresultStatus(result) != PGRES_COMMAND_OK) {
    const std::string error = PQerrorMessage(m_db);
    PQclear(result);
    throw std::runtime_error("Unable to execute SQL file '" +
                             std::string(file_name) + "': " + error);
  }
  PQclear(result);
}

void PostgreSqlBackend::MigrateTimestampColumns() {
  // Older databases created before timestamps used BIGINT will still have
  // INTEGER columns, which overflow for millisecond epoch values. Widen any
  // such columns in place, preserving existing data.
  constexpr char migration[] = R"SQL(
DO $$
DECLARE
  target_table TEXT;
  target_column TEXT;
BEGIN
  FOREACH target_table IN ARRAY ARRAY['program', 'environment', 'suite', 'test']
  LOOP
    FOREACH target_column IN ARRAY ARRAY['start_timestamp', 'end_timestamp']
    LOOP
      IF EXISTS (
          SELECT 1
          FROM information_schema.columns
          WHERE table_schema = current_schema()
            AND table_name = target_table
            AND column_name = target_column
            AND data_type <> 'bigint') THEN
        EXECUTE format(
            'ALTER TABLE %I ALTER COLUMN %I TYPE BIGINT',
            target_table, target_column);
      END IF;
    END LOOP;
  END LOOP;
END $$;
)SQL";
  PGresult* result = PQexec(m_db, migration);
  if (result == nullptr || PQresultStatus(result) != PGRES_COMMAND_OK) {
    const std::string error = PQerrorMessage(m_db);
    PQclear(result);
    throw std::runtime_error("Unable to migrate PostgreSQL timestamp columns: " +
                             error);
  }
  PQclear(result);
}

void PostgreSqlBackend::Execute(const char* file_name, const char* marker,
                                const std::vector<std::string>& values,
                                const std::vector<std::int64_t>& integers) {
  PGresult* result = ExecuteStatement(
      ReadSqlStatement(kDialect, file_name, marker), values, integers);
  if (result == nullptr || PQresultStatus(result) != PGRES_COMMAND_OK) {
    const std::string error = PQerrorMessage(m_db);
    PQclear(result);
    throw std::runtime_error("Unable to execute PostgreSQL statement: " +
                             error);
  }
  PQclear(result);
}

std::int64_t PostgreSqlBackend::InsertRow(
    const char* file_name, const char* marker,
    const std::vector<std::string>& values,
    const std::vector<std::int64_t>& integers) {
  PGresult* result = ExecuteStatement(
      ReadSqlStatement(kDialect, file_name, marker), values, integers);
  if (result == nullptr || PQresultStatus(result) != PGRES_TUPLES_OK) {
    const std::string error = PQerrorMessage(m_db);
    PQclear(result);
    throw std::runtime_error("Unable to insert PostgreSQL row: " + error);
  }
  if (PQntuples(result) != 1 || PQnfields(result) != 1) {
    PQclear(result);
    throw std::runtime_error(
        "Unable to insert PostgreSQL row: expected one returned identifier");
  }
  const std::string id = PQgetvalue(result, 0, 0);
  PQclear(result);
  try {
    return std::stoll(id);
  } catch (const std::invalid_argument&) {
    throw std::runtime_error(
        "Unable to insert PostgreSQL row: invalid returned identifier");
  } catch (const std::out_of_range&) {
    throw std::runtime_error(
        "Unable to insert PostgreSQL row: returned identifier out of range");
  }
}

}  // namespace testing
