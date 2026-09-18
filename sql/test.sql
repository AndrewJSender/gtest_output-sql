-- Copyright (c) 2026 Andrew J. Sender.
-- All rights reserved.

CREATE TABLE IF NOT EXISTS test (
  id {{PK}},
  suite_id {{INT}} NOT NULL,
  name TEXT NOT NULL,
  start_timestamp {{INT}},
  end_timestamp {{INT}},
  result TEXT,
  FOREIGN KEY(suite_id) REFERENCES suite(id)
);

-- test_insert
INSERT INTO test (name, result, start_timestamp, suite_id)
VALUES (?, ?, ?, ?){{RETURNING}};

-- test_disabled_insert
INSERT INTO test (name, result, suite_id)
VALUES (?, ?, ?){{RETURNING}};

-- test_update
UPDATE test
SET result = ?, end_timestamp = ?
WHERE id = ?;
