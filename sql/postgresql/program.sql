-- Copyright (c) 2026 Andrew J. Sender.
-- All rights reserved.

CREATE TABLE IF NOT EXISTS program (
  id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
  name TEXT NOT NULL,
  start_timestamp BIGINT,
  end_timestamp BIGINT,
  result TEXT,
  pass_count INTEGER NOT NULL DEFAULT 0,
  failed_count INTEGER NOT NULL DEFAULT 0,
  skip_count INTEGER NOT NULL DEFAULT 0,
  incomplete_count INTEGER NOT NULL DEFAULT 0
);

-- program_insert
INSERT INTO program (name, result, start_timestamp) VALUES ($1, $2, $3)
RETURNING id;

-- program_update
UPDATE program
SET result = $1, end_timestamp = $2, pass_count = $3, failed_count = $4,
    skip_count = $5, incomplete_count = $6
WHERE id = $7;
