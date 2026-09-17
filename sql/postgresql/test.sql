-- Copyright (c) 2026 Andrew J. Sender.
-- All rights reserved.

CREATE TABLE IF NOT EXISTS test (
  id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
  suite_id BIGINT NOT NULL,
  name TEXT NOT NULL,
  start_timestamp BIGINT,
  end_timestamp BIGINT,
  result TEXT,
  FOREIGN KEY(suite_id) REFERENCES suite(id)
);

-- test_insert
INSERT INTO test (name, result, start_timestamp, suite_id)
VALUES ($1, $2, $3, $4)
RETURNING id;

-- test_disabled_insert
INSERT INTO test (name, result, suite_id)
VALUES ($1, $2, $3)
RETURNING id;

-- test_update
UPDATE test
SET result = $1, end_timestamp = $2
WHERE id = $3;
