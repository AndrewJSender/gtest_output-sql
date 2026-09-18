-- Copyright (c) 2026 Andrew J. Sender.
-- All rights reserved.

CREATE TABLE IF NOT EXISTS environment (
  id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
  program_id BIGINT NOT NULL,
  start_timestamp BIGINT,
  end_timestamp BIGINT,
  result TEXT,
  FOREIGN KEY(program_id) REFERENCES program(id)
);

-- environment_insert
INSERT INTO environment (result, start_timestamp, program_id)
VALUES ($1, $2, $3)
RETURNING id;

-- environment_setup_update
UPDATE environment SET result = $1 WHERE id = $2;

-- environment_update
UPDATE environment
SET result = $1, end_timestamp = $2
WHERE id = $3;
