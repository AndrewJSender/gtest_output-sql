# gtest_output-sql

[![C++ CI](https://github.com/AndrewJSender/gtest_output-sql/actions/workflows/ci.yml/badge.svg)](https://github.com/AndrewJSender/gtest_output-sql/actions/workflows/ci.yml)

Support `--gtest_output` to SQL/SQLite.

## Why SQL output?

SQL output makes test results structured, queryable, and easier to integrate
with other tools than a text or XML report. Results can be filtered,
aggregated, compared across runs, and joined with metadata using standard SQL.
SQLite also keeps the output in a single portable database file without
requiring a separate database server.

Results are written as testing progresses through the GoogleTest event
listener. If the test process crashes or is interrupted before the suite
finishes, results already committed to the database remain available for
inspection instead of being lost with an incomplete report. This also makes it
possible to monitor a long-running test run while it is still executing by
querying the database. A shared database can centralize results from multiple
test processes or machines, while storing distinct test runs prevents newer
results from overwriting historical data.

## Quick Start

Initialize the submodules before configuring:

```sh
git submodule update --init --recursive
```

The example builds `example/main.cpp`, the sources in `src/`, and SQLite as a
static library from the `submodules/sqlite` submodule.

```sh
cmake -S example -B build -G Xcode
cmake --build build --config Debug --target example
./build/Debug/example --gtest_output=sqlite:test_results.db
```

## Database Schema

The output database contains four main tables that capture test execution results:

### `program` table
Top-level information about a test program execution.

| Column | Type | Description |
|--------|------|-------------|
| `id` | INTEGER | Primary key (auto-increment) |
| `name` | TEXT | Name of the test program |
| `start_timestamp` | INTEGER | Unix timestamp when execution started |
| `end_timestamp` | INTEGER | Unix timestamp when execution ended |
| `result` | TEXT | Overall result: `"PASSED"`, `"FAILED"`, or `"SKIPPED"` |
| `pass_count` | INTEGER | Total passed tests |
| `failed_count` | INTEGER | Total failed tests |
| `skip_count` | INTEGER | Total skipped tests |
| `incomplete_count` | INTEGER | Total incomplete tests |

### `environment` table
Test environment/configuration information associated with a program.

| Column | Type | Description |
|--------|------|-------------|
| `id` | INTEGER | Primary key (auto-increment) |
| `program_id` | INTEGER | Foreign key to `program` table |
| `name` | TEXT | Environment name |
| `start_timestamp` | INTEGER | Unix timestamp when environment setup started |
| `end_timestamp` | INTEGER | Unix timestamp when environment teardown ended |
| `result` | TEXT | Environment setup result |
| `pass_count` | INTEGER | Passed tests in this environment |
| `failed_count` | INTEGER | Failed tests in this environment |
| `skip_count` | INTEGER | Skipped tests in this environment |
| `incomplete_count` | INTEGER | Incomplete tests in this environment |

### `suite` table
Test suite information (a collection of tests grouped logically).

| Column | Type | Description |
|--------|------|-------------|
| `id` | INTEGER | Primary key (auto-increment) |
| `program_id` | INTEGER | Foreign key to `program` table |
| `name` | TEXT | Suite name (e.g., `"CalculatorTests"`) |
| `start_timestamp` | INTEGER | Unix timestamp when suite started |
| `end_timestamp` | INTEGER | Unix timestamp when suite ended |
| `result` | TEXT | Suite result: `"PASSED"`, `"FAILED"`, or `"SKIPPED"` |
| `pass_count` | INTEGER | Passed tests in this suite |
| `failed_count` | INTEGER | Failed tests in this suite |
| `skip_count` | INTEGER | Skipped tests in this suite |
| `incomplete_count` | INTEGER | Incomplete tests in this suite |

### `test` table
Individual test case results.

| Column | Type | Description |
|--------|------|-------------|
| `id` | INTEGER | Primary key (auto-increment) |
| `suite_id` | INTEGER | Foreign key to `suite` table |
| `name` | TEXT | Test name (e.g., `"AdditionTest"`) |
| `start_timestamp` | INTEGER | Unix timestamp when test started |
| `end_timestamp` | INTEGER | Unix timestamp when test ended |
| `result` | TEXT | Test result: `"PASSED"`, `"FAILED"`, `"SKIPPED"`, or `"NOTRUN"` |

## Usage Examples

After running tests, query the database with standard SQL:

### Find all failed tests

```sql
SELECT suite.name, test.name, test.result
FROM test
JOIN suite ON test.suite_id = suite.id
WHERE test.result = 'FAILED'
ORDER BY suite.name, test.name;
```

### Get test statistics

```sql
SELECT 
    suite.name,
    COUNT(*) as total_tests,
    SUM(CASE WHEN test.result = 'PASSED' THEN 1 ELSE 0 END) as passed,
    SUM(CASE WHEN test.result = 'FAILED' THEN 1 ELSE 0 END) as failed,
    SUM(CASE WHEN test.result = 'SKIPPED' THEN 1 ELSE 0 END) as skipped
FROM test
JOIN suite ON test.suite_id = suite.id
GROUP BY suite.name
ORDER BY failed DESC, suite.name;
```

### Compare results across multiple runs

```sql
-- This query requires unique identifying information per run
-- Consider adding a "run_id" column to the program table if you need this
SELECT 
    p.name as program,
    p.result,
    p.pass_count,
    p.failed_count,
    datetime(p.start_timestamp, 'unixepoch') as start_time
FROM program p
ORDER BY p.start_timestamp DESC
LIMIT 10;
```

### Find slow tests

```sql
SELECT 
    suite.name,
    test.name,
    (test.end_timestamp - test.start_timestamp) as duration_seconds
FROM test
JOIN suite ON test.suite_id = suite.id
WHERE test.end_timestamp IS NOT NULL
ORDER BY duration_seconds DESC
LIMIT 20;
```

### Export results to CSV

You can export query results directly:

```sh
# Using sqlite3 CLI
sqlite3 test_results.db "SELECT suite.name, test.name, test.result FROM test JOIN suite ON test.suite_id = suite.id;" -csv > results.csv
```

## Integration with Python

Example Python script to analyze test results:

```python
import sqlite3

conn = sqlite3.connect('test_results.db')
cursor = conn.cursor()

# Get failed tests
cursor.execute('''
    SELECT suite.name, test.name
    FROM test
    JOIN suite ON test.suite_id = suite.id
    WHERE test.result = "FAILED"
''')

failed_tests = cursor.fetchall()
for suite_name, test_name in failed_tests:
    print(f"❌ {suite_name}::{test_name}")

conn.close()
```

## Installation

### Requirements

- CMake 3.15+
- C++17 compatible compiler
- GoogleTest
- SQLite 3

### Building

```sh
# Clone the repository
git clone --recursive https://github.com/AndrewJSender/gtest_output-sql.git
cd gtest_output-sql

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build .

# Run tests
ctest
```

## Troubleshooting

### Database file not created

- Ensure your test executable links against this library
- Check that you're passing `--gtest_output=sqlite:filename.db` correctly
- Verify write permissions in the output directory

### Database locked error

- Multiple test processes writing to the same database can cause locking
- Consider using unique filenames per test run: `sqlite:test_results_${TIMESTAMP}.db`
- Or use WAL (Write-Ahead Logging) mode by executing: `PRAGMA journal_mode=WAL;`

### Incomplete results after crash

This is expected behavior—already-committed test results remain in the database. You can identify incomplete runs by checking for NULL `end_timestamp` values in the `program` table.

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines on how to:

- Report issues
- Propose features
- Submit pull requests
- Follow code style conventions

## License

This project is licensed under the MIT License—see [LICENSE](LICENSE) for details.

## Related Projects

- [GoogleTest](https://github.com/google/googletest) - Google's C++ testing framework
- [SQLite](https://www.sqlite.org/) - Self-contained SQL database engine
