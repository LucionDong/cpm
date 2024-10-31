#include "sql/sql_handle.h"

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esvcpm/utils/log.h"

static int select_plugin_node_callback(void *data, int argc, char **argv, char **azColName) {
    nlog_info("select plugin node callback");
    char *value = (char *) data;
    for (int i = 0; i < argc; i++) {
        nlog_info("azColName[%d] = %s", i, azColName[i]);
        sprintf(value, "%s", argv[i]);
    }
    return 0;
}

int select_plugin_node(sql_handle_t *sql_handle, const char *node_name, char *value) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "SELECT * FROM `plugin_node` WHERE `node_name` = '%s'", node_name);
    nlog_info("select plugin node sql: %s", sql);

    int rc = sqlite3_exec(sql_handle->plugin_node_db, sql, select_plugin_node_callback, value, NULL);
    if (rc != SQLITE_OK) {
        nlog_error("select plugin node failed");
        return -1;
    }
    nlog_info("node_name: %s, node_id: %s", node_name, value);

    return 0;
}

int sql_handle_init(sql_handle_t *sql_handle) {
    int rv = sqlite3_open(PLUGIN_NODE_DATABASE_LOCATE, &sql_handle->plugin_node_db);
    if (rv != SQLITE_OK) {
        nlog_error("open plugin node database failed");
        return -1;
    }
    return 0;
}

sql_handle_t *sql_handle_create() {
    sql_handle_t *sql_handle = calloc(1, sizeof(sql_handle_t));
    sql_handle_init(sql_handle);
    return sql_handle;
}

void sql_handle_fini(sql_handle_t **sql_handle) {
    sqlite3_close((*sql_handle)->plugin_node_db);
    free(*sql_handle);
}
