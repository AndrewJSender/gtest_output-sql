-- Copyright (c) 2026 Andrew J. Sender.
-- All rights reserved.

CREATE TABLE IF NOT EXISTS suite (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  program_id INTEGER NOT NULL,
  name TEXT NOT NULL,
  start_timestamp INTEGER,
  end_timestamp INTEGER,
  result TEXT,
  FOREIGN KEY(program_id) REFERENCES program(id)
);

-- suite_insert
INSERT INTO suite (name, result, start_timestamp, program_id)
VALUES (?, ?, ?, ?);

-- suite_update
UPDATE suite
SET result = ?, end_timestamp = ?
WHERE id = ?;
