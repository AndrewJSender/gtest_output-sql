cmake_minimum_required(VERSION 3.16)

project(gtest_output_sql_example LANGUAGES C CXX)

include(ExternalProject)

set(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")
set(GTEST_SOURCE_DIR "${PROJECT_ROOT}/submodules/googletest")
set(SQLITE_SOURCE_DIR "${PROJECT_ROOT}/submodules/sqlite")
set(SQLITE_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/sqlite")
file(MAKE_DIRECTORY "${SQLITE_INSTALL_DIR}/include")

add_subdirectory("${GTEST_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/googletest")

ExternalProject_Add(sqlite_build
  SOURCE_DIR "${SQLITE_SOURCE_DIR}"
  CONFIGURE_COMMAND
    <SOURCE_DIR>/configure
    --disable-shared
    --enable-static
    --prefix=<INSTALL_DIR>
  BUILD_COMMAND $(MAKE)
  INSTALL_COMMAND $(MAKE) install
  BUILD_BYPRODUCTS "${SQLITE_INSTALL_DIR}/lib/libsqlite3.a"
  INSTALL_DIR "${SQLITE_INSTALL_DIR}"
)

add_library(sqlite STATIC IMPORTED GLOBAL)
set_target_properties(sqlite PROPERTIES
  IMPORTED_LOCATION "${SQLITE_INSTALL_DIR}/lib/libsqlite3.a"
  INTERFACE_INCLUDE_DIRECTORIES "${SQLITE_INSTALL_DIR}/include"
)
add_dependencies(sqlite sqlite_build)

add_executable(sample1
  "${GTEST_SOURCE_DIR}/googletest/samples/sample1.cc"
  "${GTEST_SOURCE_DIR}/googletest/samples/sample1_unittest.cc"
  "${PROJECT_ROOT}/src/sql_test_event_listner.cpp"
)
target_include_directories(sample1 PRIVATE
  "${GTEST_SOURCE_DIR}/googletest/samples"
  "${PROJECT_ROOT}/include"
)
target_link_libraries(sample1 PRIVATE sqlite gtest_main)
