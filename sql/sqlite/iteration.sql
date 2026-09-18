-- Copyright (c) 2026 Andrew J. Sender.
-- All rights reserved.

CREATE TABLE IF NOT EXISTS iteration (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  program_id INTEGER NOT NULL,
  iteration_index INTEGER NOT NULL,
  start_timestamp INTEGER,
  end_timestamp INTEGER,
  result TEXT,
  FOREIGN KEY(program_id) REFERENCES program(id)
);

-- iteration_insert
INSERT INTO iteration (result, iteration_index, start_timestamp, program_id)
VALUES (?, ?, ?, ?);

-- iteration_update
UPDATE iteration
SET result = ?, end_timestamp = ?
WHERE id = ?;
