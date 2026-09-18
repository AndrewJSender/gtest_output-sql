-- Copyright (c) 2026 Andrew J. Sender.
-- All rights reserved.

CREATE TABLE IF NOT EXISTS program (
  id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
  start_timestamp BIGINT,
  end_timestamp BIGINT,
  result TEXT
);

-- program_insert
INSERT INTO program (result, start_timestamp) VALUES ($1, $2)
RETURNING id;

-- program_update
UPDATE program
SET result = $1, end_timestamp = $2
WHERE id = $3;
