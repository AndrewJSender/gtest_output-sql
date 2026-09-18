-- Copyright (c) 2026 Andrew J. Sender.
-- All rights reserved.

CREATE TABLE IF NOT EXISTS suite (
  id {{PK}},
  program_id {{INT}} NOT NULL,
  name TEXT NOT NULL,
  start_timestamp {{INT}},
  end_timestamp {{INT}},
  result TEXT,
  FOREIGN KEY(program_id) REFERENCES program(id)
);

-- suite_insert
INSERT INTO suite (name, result, start_timestamp, program_id)
VALUES (?, ?, ?, ?){{RETURNING}};

-- suite_update
UPDATE suite
SET result = ?, end_timestamp = ?
WHERE id = ?;
