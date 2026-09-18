-- Copyright (c) 2026 Andrew J. Sender.
-- All rights reserved.

CREATE TABLE IF NOT EXISTS program (
  id {{PK}},
  start_timestamp {{INT}},
  end_timestamp {{INT}},
  result TEXT
);

-- program_insert
INSERT INTO program (result, start_timestamp) VALUES (?, ?){{RETURNING}};

-- program_update
UPDATE program
SET result = ?, end_timestamp = ?
WHERE id = ?;
