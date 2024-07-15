#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>
#include <jansson.h>

#include "utils/log.h"
#include "easeview_user_config.h"

static const char *easeview_user_db_file     = "/usr/local/iot/persistence/easeview_user.db";
static sqlite3 *   easeivew_user_db         = NULL;

int esv_easeview_user_config_persister_create()
{
    int rv = sqlite3_open(easeview_user_db_file, &easeivew_user_db);

    if (SQLITE_OK != rv) {
        nlog_fatal("db `%s` fail: %s", easeview_user_db_file, sqlite3_errstr(rv));
        return -1;
    }
    sqlite3_busy_timeout(easeivew_user_db, 100 * 1000);
    return 0;
}

void esv_easeview_user_config_persister_destroy()
{
    sqlite3_close(easeivew_user_db);
}

static int query_uart_config_from_db(esv_uart_configs_t **configs_result) {
	sqlite3_stmt *stmt = NULL;
	/* const char *query ="SELECT value FROM easeview_config WHERE group_name = 'uart' and name IN ('uart1', 'uart2', 'uart3', 'uart4')"; */
	const char *query_sql ="SELECT device_config FROM easeview_user_config WHERE product_key = 'ESMGTWEV2RYET1KX1R44' LIMIT 1 ";
    esv_uart_configs_t *configs      = calloc(1, sizeof(esv_uart_configs_t));
    if (configs == NULL) {
        return -1;
    }

	if (SQLITE_OK != sqlite3_prepare_v2(easeivew_user_db, query_sql, -1, &stmt, NULL)) {
        nlog_error("prepare `%s` fail: %s", query_sql, sqlite3_errmsg(easeivew_user_db));
        goto error;
    }

	int step = sqlite3_step(stmt);
	if (SQLITE_ROW == step) {
		char *config_str = strdup((char *) sqlite3_column_text(stmt, 0));
		if (config_str == NULL) {
			goto sql_error;	
		}

		json_error_t error;
		json_t *root = json_loads(config_str, JSON_DECODE_ANY, &error);
		if (root == NULL) {
			nlog_error("easeview user config json load error, line: %d, column: %d, position: %d, info: "
					"%s",
					error.line, error.column, error.position, error.text);
			goto sql_error;
		}

		json_t *property_configs_ob = json_object_get(root, "propertyConfigs");
		if (property_configs_ob == NULL) {
			nlog_warn("do not get propertyConfigs");
			goto sql_error;
		}

		json_t *uart_ob = json_object_get(property_configs_ob, "uart");
		if (uart_ob == NULL) {
			nlog_warn("do not get uart");
			goto sql_error;
		}


		size_t uart_count = json_object_size(uart_ob);
		nlog_debug("uart has %lld uart configs", (long long)uart_count);

		configs->n_config = uart_count;
		configs->configs = calloc(configs->n_config, sizeof(esv_uart_config_t));
		esv_uart_config_t *p_config = configs->configs;

		const char *key;
		json_t *value;
		json_object_foreach(uart_ob, key, value) {
			char *uart_config_str = json_dumps(value, 0);
			*p_config = uart_config_str;
			p_config++;
		}

sql_error:
        step = sqlite3_step(stmt);
		if (config_str != NULL) {
			free(config_str);
		}
		if (root != NULL) {
			json_decref(root);
		}
    } else {
		nlog_warn("query '%s' return 0", query_sql);
    }

    if (SQLITE_DONE != step) {
        nlog_warn("query `%s` fail: %s", query_sql, sqlite3_errmsg(easeivew_user_db));
		goto sql_done_error;
    }

	// Execute the statement and count rows
	/* int step = sqlite3_step(stmt); */
	/* while (SQLITE_ROW == step) { */
	/* 	configs->n_config++; */
        /* step = sqlite3_step(stmt); */
    /* } */
    /* if (SQLITE_DONE != step) { */
        /* nlog_warn("query count `%s` fail: %s", query, sqlite3_errmsg(easeivew_user_db)); */
	/* 	goto sql_done_error; */
    /* } */

    /* configs->configs = calloc(configs->n_config, sizeof(esv_uart_config_t)); */
    /* esv_uart_config_t *p_config = configs->configs; */

	/* // Reset the statement to use the results again */
	/* sqlite3_reset(stmt); */
	/* // Use the results again */
	/* step = sqlite3_step(stmt); */
	/* while (SQLITE_ROW == step) { */
		/* char *config_str = strdup((char *) sqlite3_column_text(stmt, 0)); */
		/* if (config_str == NULL) { */
			/* goto sql_error; */	
		/* } */

		/* *p_config = config_str; */
		/* p_config++; */

/* sql_error: */
    /*     step = sqlite3_step(stmt); */
    /* } */

    /* if (SQLITE_DONE != step) { */
    /*     nlog_warn("query `%s` fail: %s", query, sqlite3_errmsg(easeivew_user_db)); */
		/* goto sql_done_error; */
    /* } */

    *configs_result = configs;
	goto sql_done;


error:
sql_done_error:
    if (configs != NULL) {
		esv_uart_configs_free(configs);
    }
	if (stmt != NULL) {
		sqlite3_finalize(stmt);
	}
    return EXIT_FAILURE;

sql_done:
    sqlite3_finalize(stmt);
    return EXIT_SUCCESS;
}

int load_uart_config_from_db() {

	/* TODO:  <27-06-24, winston> 
	 * 修改函数入参，传递uart config
	 * */
    esv_uart_configs_t *configs = NULL;
    int rv = query_uart_config_from_db(&configs);
    if (rv != 0) {
        return rv;
    }

    for (int i = 0; i < configs->n_config; i++) {
        char *config = configs->configs[i];
		nlog_debug("uart config: %s", config);
    }

    /* free(configs->configs); */
    /* free(configs); */
	esv_uart_configs_free(configs);
    return 0;
}


void esv_uart_configs_free(esv_uart_configs_t *configs)
{

    esv_uart_config_t *p_config = configs->configs;
    for (int i = 0; i < configs->n_config; i++) {
        free(*p_config);
        p_config++;
    }

    free(configs->configs);

    free(configs);
}
