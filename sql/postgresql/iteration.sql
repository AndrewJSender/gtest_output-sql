-- Copyright (c) 2026 Andrew J. Sender.
-- All rights reserved.

CREATE TABLE IF NOT EXISTS iteration (
  id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
  program_id BIGINT NOT NULL,
  iteration_index INTEGER NOT NULL,
  start_timestamp BIGINT,
  end_timestamp BIGINT,
  result TEXT,
  FOREIGN KEY(program_id) REFERENCES program(id)
);

-- iteration_insert
INSERT INTO iteration (result, iteration_index, start_timestamp, program_id)
VALUES ($1, $2, $3, $4)
RETURNING id;

-- iteration_update
UPDATE iteration
SET result = $1, end_timestamp = $2
WHERE id = $3;
