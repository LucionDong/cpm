/*
 * json_parse.h
 * Copyright (C) 2023 dongbin <dongbin0625@163.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __JSON_PARSE_H__
#define __JSON_PARSE_H__

// #define FILE_PATH "./serial_config.json"
#include <stdint.h>
#include <stdlib.h>
#define FILE_PATH "/usr/local/etc/deviceconfig/custom_uart.json"
#define CONFIG_FRAME_LEN 27
#define FIX_FRAME_ELEMENT_LEN 7
#define FIX_CONFIG_FRAME_ELEMENT_LEN 8
#define FIX_CONFIG_FRAME_LEN 11
#define ENABLE_COUNT_PART_LEN 4
#define FIX_CONFIG_FRAME_CRC_OFFSET 3
#define FIX_CRC_LEN 2

#define CONFIG_PART_OFFSET 8
#define MASTER 0x01
#define SLAVE 0x00

typedef enum { GENERAL = 1 } device_type_e;

struct serial_config_frame {
    unsigned char *config_frame;
    int config_frame_len;
};

typedef struct serial_config_frame serial_config_frame_t;

int make_config_frame(serial_config_frame_t *serial_config_frame);
uint16_t calculate_crc16(const uint8_t *data, size_t length);

#endif /* !__JSON_PARSE_H__ */
