# gtest_output-sql

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

## Example

The example builds `example/main.cpp`, the sources in `src/`, and SQLite as a
static library from the `submodules/sqlite` submodule.

```sh
cmake -S example -B build -G Xcode
cmake --build build --config Debug --target example
./build/Debug/example --gtest_output=sqlite:test_results.db
```

Initialize the submodules before configuring:

```sh
git submodule update --init --recursive
```
