-- Copyright (c) 2026 Andrew J. Sender.
-- All rights reserved.

CREATE TABLE IF NOT EXISTS iteration (
  id {{PK}},
  program_id {{INT}} NOT NULL,
  iteration_index INTEGER NOT NULL,
  start_timestamp {{INT}},
  end_timestamp {{INT}},
  result TEXT,
  FOREIGN KEY(program_id) REFERENCES program(id)
);

-- iteration_insert
INSERT INTO iteration (result, iteration_index, start_timestamp, program_id)
VALUES (?, ?, ?, ?){{RETURNING}};

-- iteration_update
UPDATE iteration
SET result = ?, end_timestamp = ?
WHERE id = ?;
