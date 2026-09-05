CREATE TABLE IF NOT EXISTS program (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL,
  start_timestamp INTEGER,
  end_timestamp INTEGER,
  result TEXT,
  pass_count INTEGER NOT NULL DEFAULT 0,
  failed_count INTEGER NOT NULL DEFAULT 0,
  skip_count INTEGER NOT NULL DEFAULT 0,
  incomplete_count INTEGER NOT NULL DEFAULT 0
);

-- program_insert
INSERT INTO program (name, result, start_timestamp) VALUES (?, ?, ?);

-- program_update
UPDATE program
SET result = ?, end_timestamp = ?, pass_count = ?, failed_count = ?,
    skip_count = ?, incomplete_count = ?
WHERE id = ?;
