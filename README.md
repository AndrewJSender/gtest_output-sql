# gtest_output-sql

Support `--gtest_output` to SQL/SQLite.

## Example

The example builds a GoogleTest sample, the sources in `src/`, and SQLite as a
static library from the `submodules/sqlite` submodule.

```sh
cmake -S example -B build-example
cmake --build build-example --target sample1
./build-example/sample1
```

Initialize the submodules before configuring:

```sh
git submodule update --init --recursive
```
