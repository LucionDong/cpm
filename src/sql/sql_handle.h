#ifndef SQL_HANDLE_H
#define SQL_HANDLE_H

#include <sqlite3.h>

#define PLUGIN_NODE_DATABASE_LOCATE "/usr/local/iot/persistence/easeview_thing.db"

typedef struct sql_handle {
    sqlite3 *plugin_node_db;
} sql_handle_t;

int select_plugin_node(sql_handle_t *sql_handle, const char *node_name, char *value);

int sql_handle_init(sql_handle_t *sql_handle);
sql_handle_t *sql_handle_create();
void sql_handle_fini(sql_handle_t **sql_handle);

#endif
