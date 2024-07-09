/*
 * config_data_cache.h
 * Copyright (C) 2024 dongbin <dongbin0625@163.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __CONFIG_DATA_CACHE_H__
#define __CONFIG_DATA_CACHE_H__
#include "ut_include/utarray.h"

typedef struct config_data_cache config_data_cache_t;

struct config_data_cache {
    UT_array *config_data_cache_array;
    int config_data_cache_array_length;
};

#endif /* !__CONFIG_DATA_CACHE_H__ */
