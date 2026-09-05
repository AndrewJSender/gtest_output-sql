# gtest_output-sql

Support `--gtest_output` to SQL/SQLite.

## Example

The example builds `example/main.cpp`, the sources in `src/`, and SQLite as a
static library from the `submodules/sqlite` submodule.

```sh
cmake -S example -B build -G Xcode
cmake --build build --config Debug --target example
./build/Debug/example
```

Initialize the submodules before configuring:

```sh
git submodule update --init --recursive
```
