--- Add adapters table ---
CREATE TABLE IF NOT EXISTS
  nodes (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT PRIMARY KEY check(length(name) <= 32),
    type integer(1) NOT NULL check(type IN (1, 2)),
    state integer(1) NOT NULL check(state BETWEEN 1 AND 4),
    plugin_name TEXT NOT NULL check(length(plugin_name) <= 32),
    lib_name TEXT NOT NULL,
    preset_config TEXT NOT NULL，
    preset_config_index integer NOT NULL,
    node_id TEXT NOT NULL
  );

--- Add settings table ---
CREATE TABLE IF NOT EXISTS
  settings (
    node_name TEXT NOT NULL,
    setting TEXT NOT NULL,
    UNIQUE (node_name),
    FOREIGN KEY (node_name) REFERENCES nodes (name) ON UPDATE CASCADE ON DELETE CASCADE
  );