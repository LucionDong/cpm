/*
 *
 * Copyright (C) 2023-12-29 10:46 dongbin <dongbin0625@163.com>
 *
 */
// #include <esvcpm/utils/log.h>
// #include <esvcpm/utils/log.h>
// #include <esvcpm/utils/log.h>
#include <jansson.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <utils/log.h>

#include "config/easeview_config.h"
#include "json_parse.h"
#include "ut_include/utarray.h"
// #include "outside_service_manager.h"
// #include "rs232_recv.h"
int open_files_to_json(const char *filepath, json_t **root);
int get_config_json_array_and_size(json_t *root, int *array_size, json_t **uarts);
// int get_config_json_array_and_size(json_t *root, int *array_size, json_t *uarts);
int make_empty_config_frame(unsigned char **config_frame, int uart_enble_count);
int make_config_frame_element(json_t *uarts, int enable_port_count, serial_config_frame_t *serial_config_frame);

static UT_array *easeview_config_array = NULL;
int parse_easeview_config_json(json_t *esv_driver_232_configs_array) {
    char **easeview_config_array_value;
    utarray_new(easeview_config_array, &ut_str_icd);
    load_uart_config_from_db(easeview_config_array);

    if (easeview_config_array == NULL) {
        nlog_info("easeview_config_array is NULL");
        return -1;
    }

    while ((easeview_config_array_value = (char **) utarray_next(easeview_config_array, easeview_config_array_value))) {
        json_error_t error;
        json_t *tmp_json_array_value = json_loads(*easeview_config_array_value, 0, &error);
        json_array_append(esv_driver_232_configs_array, tmp_json_array_value);
        json_decref(tmp_json_array_value);
    }

    return 0;
}

int make_config_frame(serial_config_frame_t *serial_config_frame) {
    // json_t *root = json_object();
    json_t *uarts = json_array();
    parse_easeview_config_json(uarts);
    // open_files_to_json(FILE_PATH, &root);
    int uart_enble_count = json_array_size(uarts);

    // get_config_json_array_and_size(root, &uart_enble_count, &uarts);
    make_empty_config_frame(&serial_config_frame->config_frame, uart_enble_count);
    make_config_frame_element(uarts, uart_enble_count, serial_config_frame);

    nlog_info("serial_config_frame: ");
    hnlog_notice(serial_config_frame->config_frame, serial_config_frame->config_frame_len);
    json_decref(uarts);
    // json_decref(root);

    // serial_config_frame->config_frame = malloc(sizeof(unsigned char) * CONFIG_FRAME_LEN);
    // unsigned char tmp[] = {0xee, 0x45, 0x00, 20, 6, 0, 0,  2, 1, 1, 0,    1,    7,   8,
    //                        0,    1,    1,    1,  0, 1, 12, 8, 0, 1, 0x50, 0x0b, 0x1a};
    //
    // memcpy(serial_config_frame->config_frame, tmp, CONFIG_FRAME_LEN);
    // // (*serial_config_frame)->config_frame = (unsigned char[]);
    // serial_config_frame->config_frame_len = CONFIG_FRAME_LEN;
    return 0;
}

#if 0
int main() {
    serial_config_frame_t *serial_config_frame = malloc(sizeof(serial_config_frame_t));
    make_config_frame(serial_config_frame);

    if (serial_config_frame->config_frame) {
        free(serial_config_frame->config_frame);
    }
    free(serial_config_frame);

    return 0;
}
#endif

int open_files_to_json(const char *filepath, json_t **root) {
    *root = json_load_file(filepath, 0, NULL);
    if (*root == NULL) {
        printf("json_load_file error\n");
        // nlog_info("json_load_file error");
        return -1;
    }
    char *json_dump_string = json_dumps(*root, JSON_INDENT(4));
    printf("%s\n", json_dump_string);
    free(json_dump_string);

    return 0;
}

int get_config_json_array_and_size(json_t *root, int *array_size, json_t **uarts) {
    *uarts = json_object_get(root, "uarts");
    char *uarts_dumps = json_dumps(*uarts, 0);
    printf("uarts_dumps: %s\n", uarts_dumps);
    free(uarts_dumps);
    if (!*uarts) {
        printf("uarts is NULL\n");
        return -1;
    }

    *array_size = json_array_size(*uarts);
    if (*array_size == 0) {
        printf("uarts array is NULL or not a json array\n");
        return -1;
    }

    return 0;
}

int make_empty_config_frame(unsigned char **config_frame, int uart_enble_count) {
    int frame_len = uart_enble_count * FIX_CONFIG_FRAME_ELEMENT_LEN + FIX_CONFIG_FRAME_LEN;
    printf("frame_len: %d\n", frame_len);
    *config_frame = malloc(sizeof(unsigned char) * (frame_len));
    if (*config_frame == NULL) {
        // nlog_error("malloc error");
    }

    memset(*config_frame, 0x00, frame_len);
    // *config_frame[0] = FRAME_HEADER;
    (*config_frame)[0] = 0xee;
    // *config_frame[1] = DEF_CONFIG_FRAME;
    (*config_frame)[1] = 0x45;
    (*config_frame)[3] = uart_enble_count * FIX_CONFIG_FRAME_ELEMENT_LEN +
        ENABLE_COUNT_PART_LEN;  //每一个单独的配置帧数据部分的固定长度与使能数量乘积 + 配置帧使能字段长度
    (*config_frame)[4] = 6;
    (*config_frame)[7] = uart_enble_count;
    // *config_frame[frame_len - 1] = FRAME_TAIL;
    (*config_frame)[frame_len - 1] = 0x1a;

    return 0;
}

void append_array(unsigned char *src, size_t src_start, unsigned char *dest, size_t dest_size) {
    memcpy(src + src_start, dest, dest_size);
}

int check_device_type(const char *device_type) {
    printf("device_type: %s\n", device_type);
    if (strcmp(device_type, "general") == 0) {
        return GENERAL;
    }

    return -1;
}

int get_baudrate(int *baurate) {
    printf("baurate: %d\n", *baurate);
    switch (*baurate) {
        case 110:
            *baurate = 1;
            break;
        case 300:
            *baurate = 2;
            break;
        case 600:
            *baurate = 3;
            break;
        case 1200:
            *baurate = 4;
            break;
        case 2400:
            *baurate = 5;
            break;
        case 4800:
            *baurate = 6;
            break;
        case 9600:
            *baurate = 7;
            break;
        case 14400:
            *baurate = 8;
            break;
        case 19200:
            *baurate = 9;
            break;
        case 38400:
            *baurate = 10;
            break;
        case 57600:
            *baurate = 11;
            break;
        case 115200:
            *baurate = 12;
            break;
        case 230400:
            *baurate = 13;
            break;
        case 460800:
            *baurate = 14;
            break;
        case 921600:
            *baurate = 15;
            break;
        case 3072000:
            *baurate = 16;
            break;
    }

    return 0;
}

int get_parity(const char *parity) {
    if (strcmp(parity, "none") == 0) {
        return 0;
    } else if (!strcmp(parity, "odd")) {
        return 1;
    } else if (!strcmp(parity, "even")) {
        return 2;
    }

    return -1;
}

uint16_t calculate_crc16(const uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF;

    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;  // 0xA001是CRC-16/MODBUS的生成多项式
                                            //
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

int make_config_frame_element(json_t *uarts, int enable_port_count, serial_config_frame_t *serial_config_frame) {
    json_t *uarts_value;
    size_t uarts_index;
    json_array_foreach(uarts, uarts_index, uarts_value) {
        unsigned char tmp[CONFIG_PART_OFFSET];
        memset(tmp, 0x00, CONFIG_PART_OFFSET);

        json_t *uart_config = json_object_get(uarts_value, "uartConfig");
        tmp[0] = MASTER;
        //         if (json_boolean_value(json_object_get(uart_config, "masterMode"))) {
        //     tmp[0] = MASTER;
        // } else {
        //     tmp[0] = SLAVE;
        // }

        tmp[1] = 0x01;
        //    switch (check_device_type(json_string_value(json_object_get(uart_config, "deviceType")))) {
        // case GENERAL:
        //     // append_array(tmp, 1, 0x01, 1);
        //     printf("GENERAL\n");
        //     tmp[1] = 0x01;
        //     break;
        //    }

        tmp[3] = atoi(json_string_value(json_object_get(uarts_value, "port")));
        // tmp[3] = json_integer_value(json_object_get(uarts_value, "uartNum"));
        // tmp[3] = 0x01;
        // int baudrate = json_integer_value(json_object_get(uart_config, "baudrate"));
        int baudrate = atoi(json_string_value(json_object_get(uarts_value, "baudrate")));
        get_baudrate(&baudrate);
        tmp[4] = baudrate;
        tmp[5] = atoi(json_string_value(json_object_get(uarts_value, "dataBYtes")));
        tmp[6] = get_parity(json_string_value(json_object_get(uarts_value, "parity")));
        tmp[7] = atoi(json_string_value(json_object_get(uart_config, "stopbits")));

        //         for (int i = 0; i < sizeof(tmp); ++i) {
        // printf("%x\n", tmp[i]);
        //         }

        append_array(serial_config_frame->config_frame, (uarts_index + 1) * CONFIG_PART_OFFSET, tmp, sizeof(tmp));
    }
    uint16_t config_frame_crc_ret = calculate_crc16(
        serial_config_frame->config_frame, FIX_CONFIG_FRAME_LEN + enable_port_count * FIX_CONFIG_FRAME_ELEMENT_LEN - 3);
    append_array(serial_config_frame->config_frame,
                 FIX_CONFIG_FRAME_LEN + enable_port_count * FIX_CONFIG_FRAME_ELEMENT_LEN - 3, &config_frame_crc_ret,
                 FIX_CRC_LEN);
    serial_config_frame->config_frame_len = FIX_CONFIG_FRAME_LEN + enable_port_count * FIX_CONFIG_FRAME_ELEMENT_LEN;
    return 0;
}

// int make_config_json(json_t *uarts, serial_config_frame_t *serial_config_frame) {
// size_t index;
// json_t *array_value;
//
// json_array_foreach(uarts, index, array_value) {
// }
// }

#if 0
int main() {
    json_t *root = json_object();
    json_t *uarts;
    int array_size = 0;
    open_files_to_json(FILE_PATH, &root);

    get_config_json_array_and_size(root, &array_size, uarts);
    if (uarts == NULL) {
        printf("uarts is NULL\n");
        return -1;
    }
    printf("array_size: %d\n", array_size);

    json_decref(root);

    return 0;
}

int main() {
    serial_config_frame_t *serial_config_frame;

    serial_config_frame = malloc(sizeof(serial_config_frame_t));
    // serial_config_frame->config_frame_len = CONFIG_FRAME_LEN;
    make_config_frame(serial_config_frame);

    for (int i = 0; i < CONFIG_FRAME_LEN; ++i) {
        printf("%d\n", serial_config_frame->config_frame[i]);
    }

    return 0;
}
#endif
