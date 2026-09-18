-- Copyright (c) 2026 Andrew J. Sender.
-- All rights reserved.

CREATE TABLE IF NOT EXISTS environment (
  id {{PK}},
  program_id {{INT}} NOT NULL,
  start_timestamp {{INT}},
  end_timestamp {{INT}},
  result TEXT,
  FOREIGN KEY(program_id) REFERENCES program(id)
);

-- environment_insert
INSERT INTO environment (result, start_timestamp, program_id)
VALUES (?, ?, ?){{RETURNING}};

-- environment_setup_update
UPDATE environment SET result = ? WHERE id = ?;

-- environment_update
UPDATE environment
SET result = ?, end_timestamp = ?
WHERE id = ?;
