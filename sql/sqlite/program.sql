-- Copyright (c) 2026 Andrew J. Sender.
-- All rights reserved.

CREATE TABLE IF NOT EXISTS program (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  start_timestamp INTEGER,
  end_timestamp INTEGER,
  result TEXT
);

-- program_insert
INSERT INTO program (result, start_timestamp) VALUES (?, ?);

-- program_update
UPDATE program
SET result = ?, end_timestamp = ?
WHERE id = ?;
