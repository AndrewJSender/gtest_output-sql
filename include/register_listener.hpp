// Copyright (c) 2026 Andrew J. Sender.
// All rights reserved.

#ifndef REGISTER_LISTENER_HPP_
#define REGISTER_LISTENER_HPP_

#include "parse_argument.hpp"
#include "sql_test_event_listner.hpp"

void RegisterSqlTestEventListenerIfRequested(std::optional<SqlOutput> sql_output);

#endif  // REGISTER_LISTENER_HPP_
