#ifndef PARSE_ARGUMENT_HPP_
#define PARSE_ARGUMENT_HPP_

#include <filesystem>
#include <optional>

std::optional<std::filesystem::path> ParseGtestSqlOutputArgument(int* argc,
                                                                  char* argv[]);

#endif  // PARSE_ARGUMENT_HPP_
