-- Copyright (c) 2026 Andrew J. Sender.
-- All rights reserved.

CREATE TABLE IF NOT EXISTS test (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  suite_id INTEGER NOT NULL,
  name TEXT NOT NULL,
  start_timestamp INTEGER,
  end_timestamp INTEGER,
  result TEXT,
  FOREIGN KEY(suite_id) REFERENCES suite(id)
);

-- test_insert
INSERT INTO test (name, result, start_timestamp, suite_id)
VALUES (?, ?, ?, ?);

-- test_disabled_insert
INSERT INTO test (name, result, suite_id)
VALUES (?, ?, ?);

-- test_update
UPDATE test
SET result = ?, end_timestamp = ?
WHERE id = ?;
