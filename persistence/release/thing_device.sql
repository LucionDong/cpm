--- Add thing_model table ---
CREATE TABLE IF NOT EXISTS
  thing_model (
    product_key TEXT NOT NULL,
    product_name TEXT NOT NULL,
    thing_model_function_block_id TEXT NOT NULL DEFAULT 'default',
    thing_model TEXT NOT NULL,
    UNIQUE (product_key, product_name, thing_model_function_block_id)
  );

--- Add device table ---
CREATE TABLE IF NOT EXISTS
  thing_device (
    iot_id TEXT NOT NULL,
    product_key TEXT NOT NULL,
    device_name TEXT NOT NULL,
    device_secret TEXT NOT NULL,
    device_nick_name TEXT NOT NULL,
    driver_name TEXT NOT NULL,
    thing_model_function_block_id TEXT NOT NULL DEFAULT 'default',
    device_config TEXT NOT NULL,
    device_config_index INTEGER NOT NULL,
    UNIQUE (iot_id),
    UNIQUE (product_key, device_name)
  );