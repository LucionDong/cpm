/*
 *
 * Copyright (C) 2024-07-03 17:55 dongbin <dongbin0625@163.com>
 *
 */
#include "../config/easeview_config.h"
#include "config_data_cache.h"
#include "esvcpm/utils/log.h"
#include "ut_include/utarray.h"

// static UT_array *config_data_cache_array = NULL;
int init_config_data_cache(config_data_cache_t *config_data_cache) {
    config_data_cache->config_data_cache_array = NULL;
    utarray_new(config_data_cache->config_data_cache_array, &ut_int_icd);
    load_uart_config_from_db(config_data_cache->config_data_cache_array);

    if (config_data_cache->config_data_cache_array == NULL) {
        nlog_warn("config data cache array is NULL");
        return -1;
    }

    config_data_cache->config_data_cache_array_length = utarray_len(config_data_cache->config_data_cache_array);
    return 0;
}

int check_for_config_cache_changes(config_data_cache_t *config_data_cache) {
    UT_array *tmp_config_array = NULL;
    utarray_new(tmp_config_array, &ut_int_icd);
    load_uart_config_from_db(tmp_config_array);

    if (tmp_config_array == NULL) {
        nlog_warn("tmp config array is NULL");
        return -1;
    }

    if (config_data_cache->config_data_cache_array_length != utarray_len(tmp_config_array)) {
        nlog_info("need change config data cache");
        return 1;
    }

    nlog_info("do not need change config data cache");
    return 0;
}

void change_config_cache_array(config_data_cache_t *config_data_cache) {
}
