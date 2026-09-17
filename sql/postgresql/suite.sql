-- Copyright (c) 2026 Andrew J. Sender.
-- All rights reserved.

CREATE TABLE IF NOT EXISTS suite (
  id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
  program_id BIGINT NOT NULL,
  name TEXT NOT NULL,
  start_timestamp BIGINT,
  end_timestamp BIGINT,
  result TEXT,
  pass_count INTEGER NOT NULL DEFAULT 0,
  failed_count INTEGER NOT NULL DEFAULT 0,
  skip_count INTEGER NOT NULL DEFAULT 0,
  incomplete_count INTEGER NOT NULL DEFAULT 0,
  FOREIGN KEY(program_id) REFERENCES program(id)
);

-- suite_insert
INSERT INTO suite (name, result, start_timestamp, program_id)
VALUES ($1, $2, $3, $4)
RETURNING id;

-- suite_update
UPDATE suite
SET result = $1, end_timestamp = $2, pass_count = $3, failed_count = $4,
    skip_count = $5, incomplete_count = $6
WHERE id = $7;
