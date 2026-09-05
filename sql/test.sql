CREATE TABLE IF NOT EXISTS test (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  suite_id INTEGER NOT NULL,
  name TEXT NOT NULL,
  start_timestamp INTEGER,
  end_timestamp INTEGER,
  result TEXT,
  pass_count INTEGER NOT NULL DEFAULT 0,
  failed_count INTEGER NOT NULL DEFAULT 0,
  skip_count INTEGER NOT NULL DEFAULT 0,
  incomplete_count INTEGER NOT NULL DEFAULT 0,
  FOREIGN KEY(suite_id) REFERENCES suite(id)
);

-- test_insert
INSERT INTO test (name, result, start_timestamp, suite_id)
VALUES (?, ?, ?, ?);

-- test_disabled_insert
INSERT INTO test (name, result, suite_id, incomplete_count)
VALUES (?, ?, ?, ?);

-- test_update
UPDATE test
SET result = ?, end_timestamp = ?, pass_count = ?, failed_count = ?,
    skip_count = ?, incomplete_count = ?
WHERE id = ?;
