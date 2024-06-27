#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>

#include "utils/log.h"
#include "easeview_config.h"

static const char *easeview_db_file     = "/usr/local/iot/persistence/easeview.db";
static sqlite3 *   easeivew_db         = NULL;

int esv_easeview_config_persister_create()
{
    int rv = sqlite3_open(easeview_db_file, &easeivew_db);

    if (SQLITE_OK != rv) {
        nlog_fatal("db `%s` fail: %s", easeview_db_file, sqlite3_errstr(rv));
        return -1;
    }
    sqlite3_busy_timeout(easeivew_db, 100 * 1000);
    return 0;
}

void esv_easeview_config_persister_destroy()
{
    sqlite3_close(easeivew_db);
}

static int query_uart_config_from_db(esv_uart_configs_t **configs_result) {
	sqlite3_stmt *stmt = NULL;
	const char *query ="SELECT value FROM easeview_config WHERE group_name = 'uart' and name IN ('uart1', 'uart2', 'uart3', 'uart4')";
    esv_uart_configs_t *configs      = calloc(1, sizeof(esv_uart_configs_t));
    if (configs == NULL) {
        return -1;
    }

	if (SQLITE_OK != sqlite3_prepare_v2(easeivew_db, query, -1, &stmt, NULL)) {
        nlog_error("prepare `%s` fail: %s", query, sqlite3_errmsg(easeivew_db));
        goto error;
    }

	// Execute the statement and count rows
	int step = sqlite3_step(stmt);
	while (SQLITE_ROW == step) {
		configs->n_config++;
        step = sqlite3_step(stmt);
    }
    if (SQLITE_DONE != step) {
        nlog_warn("query count `%s` fail: %s", query, sqlite3_errmsg(easeivew_db));
		goto sql_done_error;
    }

    configs->configs = calloc(configs->n_config, sizeof(esv_uart_config_t));
    esv_uart_config_t *p_config = configs->configs;

	// Reset the statement to use the results again
	sqlite3_reset(stmt);
	// Use the results again
	step = sqlite3_step(stmt);
	while (SQLITE_ROW == step) {
		char *config_str = strdup((char *) sqlite3_column_text(stmt, 0));
		if (config_str == NULL) {
			goto sql_error;	
		}

		*p_config = config_str;
		p_config++;

sql_error:
        step = sqlite3_step(stmt);
    }

    if (SQLITE_DONE != step) {
        nlog_warn("query `%s` fail: %s", query, sqlite3_errmsg(easeivew_db));
		goto sql_done_error;
    }

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
