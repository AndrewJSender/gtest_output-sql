-- Copyright (c) 2026 Andrew J. Sender.
-- All rights reserved.

CREATE TABLE IF NOT EXISTS test_result_part (
  id {{PK}},
  test_id {{INT}} NOT NULL,
  type TEXT,
  file_name TEXT,
  line_number INTEGER,
  message TEXT,
  timestamp {{INT}},
  FOREIGN KEY(test_id) REFERENCES test(id)
);

-- test_result_part_insert
INSERT INTO test_result_part (type, file_name, message, test_id, line_number,
                              timestamp)
VALUES (?, ?, ?, ?, ?, ?){{RETURNING}};
