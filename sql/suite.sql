-- Copyright (c) 2026 Andrew J. Sender.
-- All rights reserved.

CREATE TABLE IF NOT EXISTS suite (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  program_id INTEGER NOT NULL,
  name TEXT NOT NULL,
  start_timestamp INTEGER,
  end_timestamp INTEGER,
  result TEXT,
  pass_count INTEGER NOT NULL DEFAULT 0,
  failed_count INTEGER NOT NULL DEFAULT 0,
  skip_count INTEGER NOT NULL DEFAULT 0,
  incomplete_count INTEGER NOT NULL DEFAULT 0,
  FOREIGN KEY(program_id) REFERENCES program(id)
);

-- suite_insert
INSERT INTO suite (name, result, start_timestamp, program_id)
VALUES (?, ?, ?, ?);

-- suite_update
UPDATE suite
SET result = ?, end_timestamp = ?, pass_count = ?, failed_count = ?,
    skip_count = ?, incomplete_count = ?
WHERE id = ?;
