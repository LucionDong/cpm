/*
 *
 * Copyright (C) 2023-12-22 15:01 dongbin <dongbin0625@163.com>
 *
 */

#include <errno.h>

#include "utils/log.h"
#include "utils/utarray.h"
#include "utils/utlist.h"
// #include <esvcpm/utils/log.h>
// #include <esvcpm/utils/utarray.h>
// #include <esvcpm/utils/utlist.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include "../manager_adapter_msg.h"
#include "../mcurs232/serial_port.h"
#include "../outside_service_manager.h"
#include "../outside_service_manager_internal.h"
#include "../parser_config_json/config_parser.h"
#include "device.h"
#include "event/event.h"
#include "parser_rs232_frame.h"

char *mcu_rs232_devie_path = "/dev/ttyS2";

static int mcu_rs232_reading_cb(enum neu_event_io_type type, int fd, void *usr_data);
int open_mcu_rs232_port(esv_outside_service_manager_t *outside_service_manager);
int handle_stop(esv_outside_service_manager_t *outside_service_manager);
int close_mcu_rs232_port(Mcurs232_class_t *mcurs232_class);
int make_send_to_plugin_msg(esv_between_adapter_driver_msg_t *send_to_plugin_msg, const unsigned char *msg,
                            int msg_len);
int pop_complete_list(complete_frame_list_t **head);
int move_all_complete_list_node(Mcurs232_class_t *mcurs232_class, complete_frame_list_t **tmp_head);
int check_serial_port_message(Mcurs232_class_t *mcurs232_class);
int write_to_mcu(esv_outside_service_manager_t *outside_service_manager, void *frame_buf, int buf_length);
// int send_complete_frame_to_plugin_or_adapter(esv_outside_service_manager_t *outside_service_manager);

// uint16_t calculate_crc16(const uint8_t *data, size_t length) {
//     uint16_t crc = 0xFFFF;
//
//     for (size_t i = 0; i < length; ++i) {
//         crc ^= data[i];
//         for (int j = 0; j < 8; ++j) {
//             if (crc & 0x0001) {
//                 crc = (crc >> 1) ^ 0xA001;  // 0xA001是CRC-16/MODBUS的生成多项式
//             } else {
//                 crc >>= 1;
//             }
//         }
//     }
//
//     return crc;
// }
//

// void make_ack_frame(Mcurs232_class_t *mcurs232_class) {
void composition_ack_frame(Mcurs232_class_t *mcurs232_class) {
    uint8_t *read_buf = utarray_front(mcurs232_class->serial_port_read_buf_head);
    if (read_buf[TYPE_DISTANCE_FROM_FRAME_HEADER] == 0x42 || read_buf[FRAME_CONTROL_DOMAIN] == 0x00 ||
        read_buf[TYPE_DISTANCE_FROM_FRAME_HEADER] == 0x44 || read_buf[TYPE_DISTANCE_FROM_FRAME_HEADER] == 0x00) {
        nlog_info("ack ignore");
        return;
    }

    uint8_t ack_frame[10] = {0xee, 0x42, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a};
    memcpy(ack_frame + 4, read_buf + 4, 3);

    uint16_t ack_frame_crc_ret = calculate_crc16(ack_frame, 7);
    memcpy(ack_frame + 7, &ack_frame_crc_ret, 2);

    int ret = write(mcurs232_class->mcu_rs232_port->handle, ack_frame, sizeof(ack_frame));

    hnlog_notice(ack_frame, sizeof(ack_frame));
    nlog_info("ack_frame write: %d", ret);
}

int destroy_mcurs232_share(Mcurs232_class_t *mcurs_class) {
    if (mcurs_class->serial_port_trans_share) {
        free(mcurs_class->serial_port_trans_share);
    }

    if (mcurs_class->complete_frame_list_share) {
        free(mcurs_class->complete_frame_list_share);
    }

    return 0;
}

int destroy_mcurs232_pthread(Mcurs232_class_t *mcurs_class) {
    pthread_mutex_destroy(&mcurs_class->complete_frame_list_share->mcurs_share_mutex);
    pthread_mutex_destroy(&mcurs_class->serial_port_trans_share->mcurs_share_mutex);

    pthread_cond_destroy(&mcurs_class->complete_frame_list_share->mcurs_share_cond);
    pthread_cond_destroy(&mcurs_class->serial_port_trans_share->mcurs_share_cond);

    return 0;
}

int destroy_mcurs232_class(esv_outside_service_manager_t *outside_service_manager) {
    if (outside_service_manager->mcurs232_class) {
        free(outside_service_manager->mcurs232_class);
    }

    return 0;
}

int init_mcurs_share(mcurs_share_t *share) {
    memset(&share->mcurs_share_mutex_attr, 0, sizeof(pthread_mutexattr_t));
    // pthread_mutexattr_init(&share->mcurs_share_mutex_attr);
    // pthread_mutexattr_settype(&share->mcurs_share_mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);

    // pthread_mutex_init(&share->mcurs_share_mutex, &share->mcurs_share_mutex_attr);
    pthread_mutex_init(&share->mcurs_share_mutex, NULL);
    pthread_cond_init(&share->mcurs_share_cond, NULL);

    return 0;
}

int init_mcurs_class(Mcurs232_class_t *mcurs_class) {
    mcurs_class->complete_frame_list_share = malloc(sizeof(mcurs_share_t));
    mcurs_class->serial_port_trans_share = malloc(sizeof(mcurs_share_t));

    init_mcurs_share(mcurs_class->serial_port_trans_share);
    init_mcurs_share(mcurs_class->complete_frame_list_share);

    mcurs_class->complete_frame_list_head = NULL;
    UT_icd char_icd = {sizeof(char), NULL, NULL, NULL};
    utarray_new(mcurs_class->serial_port_read_buf_head, &char_icd);

    mcurs_class->frame_event = FIND_FRAME_HEADER;

    return 0;
}

int handle_start(esv_outside_service_manager_t *outside_service_manager) {
    open_mcu_rs232_port(outside_service_manager);
    outside_service_manager->events = neu_event_new();
    neu_event_io_param_t param = {
        .fd = outside_service_manager->mcurs232_class->mcu_rs232_port->handle,
        .usr_data = (void *) outside_service_manager,
        .cb = mcu_rs232_reading_cb,
    };
    outside_service_manager->event_io_ctx = neu_event_add_io(outside_service_manager->events, param);

    return 0;
}

int handle_stop(esv_outside_service_manager_t *outside_service_manager) {
    close_mcu_rs232_port(outside_service_manager->mcurs232_class);
    neu_event_del_io(outside_service_manager->events, outside_service_manager->event_io_ctx);
    neu_event_close(outside_service_manager->events);
    return 0;
}

int open_mcu_rs232_port(esv_outside_service_manager_t *outside_service_manager) {
    outside_service_manager->mcurs232_class->mcu_rs232_port = open_port(mcu_rs232_devie_path);
    if (!outside_service_manager->mcurs232_class->mcu_rs232_port) {
        nlog_error("open mcu rs232 port(%s) error!", mcu_rs232_devie_path);
        return -1;
    }

    nlog_info("open mcu rs232 port(%s) success!", mcu_rs232_devie_path);

    return 0;
}

int close_mcu_rs232_port(Mcurs232_class_t *mcurs232_class) {
    // int close_mcu_rs232_port(esv_outside_service_manager_t *outside_service_manager) {
    if (!mcurs232_class->mcu_rs232_port) {
        nlog_warn("close mcu rs232 port(null) error!");
        return -1;
    }

    int rv = close_port(mcurs232_class->mcu_rs232_port);
    if (rv) {
        nlog_warn("close mcu rs232 port(%s) error:%s", mcu_rs232_devie_path, strerror(errno));
    } else {
        mcurs232_class->mcu_rs232_port = NULL;
        nlog_info("close mcu rs232 port(%s) success!", mcu_rs232_devie_path);
    }

    return rv;
}

int print_read_buf(Mcurs232_class_t *mcurs232_class) {
    // int print_read_buf(esv_outside_service_manager_t *outside_service_manager) {
    int count = 0;
    pthread_mutex_lock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
    printf("print_read_buf lock\n");
    // pthread_mutex_lock(&outside_service_manager->mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
    unsigned char result[1024];
    unsigned char *p;
    for (p = (unsigned char *) utarray_front(mcurs232_class->serial_port_read_buf_head); p != NULL;
         p = (unsigned char *) utarray_next(mcurs232_class->serial_port_read_buf_head, p)) {
        result[count++] = *p;
    }
    hnlog_notice(result, count);
    // hzlog_info(c, (unsigned char *) result, count);
    printf("\n");

    // pthread_mutex_unlock(&outside_service_manager->mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
    pthread_mutex_unlock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
    printf("print_read_buf unlock\n");
    // pthread_mutex_unlock(&plugin->serial_port_trans_mutex);

    return 0;
}

int push_back_serial_port_read_buf(Mcurs232_class_t *mcurs232_class, const unsigned char *buf, int buf_length) {
    printf("set_read_buf starti: %ld\n", syscall(SYS_gettid));
    pthread_mutex_lock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
    printf("set_read_buf lock: %ld\n", syscall(SYS_gettid));
    //  if (ret == 0) {
    for (int i = 0; i < buf_length; ++i) {
        utarray_push_back(mcurs232_class->serial_port_read_buf_head, &buf[i]);
    }
    // printf("---------------\n");
    // print_read_buf(plugin);
    // pthread_mutex_unlock(&plugin->serial_port_trans_mutex);
    printf("set_read_buf unlock: %ld\n", syscall(SYS_gettid));
    pthread_mutex_unlock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
    printf("set_read_buf unlock over: %ld\n", syscall(SYS_gettid));

    // nlog_info("check_serial_port_message start");
    // check_serial_port_message(mcurs232_class);
    // nlog_info("check_serial_port_message over");

    return 0;
}

int insert_new_element_to_complete_frame_list(Mcurs232_class_t *mcurs232_class, char *complete_frame_ch,
                                              int complete_len, frame_e frame_type) {
    complete_frame_list_t *new_frame_element = calloc(1, sizeof(complete_frame_list_t));
    new_frame_element->frame_type = frame_type;
    new_frame_element->frame_buf = (unsigned char *) malloc(complete_len * sizeof(unsigned char));
    new_frame_element->frame_buf_size = complete_len;

    memcpy(new_frame_element->frame_buf, complete_frame_ch, complete_len);

    pthread_mutex_lock(&mcurs232_class->complete_frame_list_share->mcurs_share_mutex);
    printf("new_frame_element buf\n");
    hnlog_notice(new_frame_element->frame_buf, complete_len);
    // hzlog_info(c, new_frame_element->frame_buf, complete_len);

    DL_APPEND(mcurs232_class->complete_frame_list_head, new_frame_element);
    pthread_cond_signal(&mcurs232_class->complete_frame_list_share->mcurs_share_cond);
    pthread_mutex_unlock(&mcurs232_class->complete_frame_list_share->mcurs_share_mutex);

    printf("insert over\n");

    return 0;
}

frame_e check_serial_port_type(Mcurs232_class_t *mcurs232_class) {
    nlog_info("check_serial_port_type");
    unsigned char *tmp_frame_ch = (unsigned char *) utarray_front(mcurs232_class->serial_port_read_buf_head);
    hnlog_notice(tmp_frame_ch, 2);
    hnlog_notice(tmp_frame_ch, 8);
    if (tmp_frame_ch[TYPE_DISTANCE_FROM_FRAME_HEADER] == DEF_ASK_CONFIG_FRAME) {
        nlog_info("serial_port_type: CONFIG");
        return ASK_CONFIG;
    } else if (tmp_frame_ch[TYPE_DISTANCE_FROM_FRAME_HEADER] == DEF_CONFIG_FRAME) {
        return CONFIG;
    } else if (tmp_frame_ch[TYPE_DISTANCE_FROM_FRAME_HEADER] == DEF_COMMAND_FRAME &&
               (tmp_frame_ch[FRAME_COMMAND_TYPE] == DEF_READ_COMMAND_TYPE ||
                tmp_frame_ch[FRAME_COMMAND_TYPE] == DEF_WRITE_COMMAND_TYPE)) {
        return COMMAND;
    } else if (tmp_frame_ch[TYPE_DISTANCE_FROM_FRAME_HEADER] == DEF_NORMAL_ACK_FRAME) {
        return NORMAL_ACK;
    } else if (tmp_frame_ch[TYPE_DISTANCE_FROM_FRAME_HEADER] == DEF_COMMAND_FRAME &&
               tmp_frame_ch[FRAME_COMMAND_TYPE] == DEF_STATUS_COMMAND_TYPE) {
        nlog_info("status command");
        return ACK_TO_MQTT;
    }
    nlog_info("check_serial_port_type_end");

    return ERROR;
}

void *send_complete_func(void *arg) {
    esv_outside_service_manager_t *outside_service_manager = (esv_outside_service_manager_t *) arg;

    nlog_info("send_complete_frame_to_plugin start");
    while (1) {
        int count = 0;
        complete_frame_list_t *elt;
        pthread_mutex_lock(&outside_service_manager->mcurs232_class->complete_frame_list_share->mcurs_share_mutex);
        DL_COUNT(outside_service_manager->mcurs232_class->complete_frame_list_head, elt, count);
        while (count < 1) {
            pthread_cond_wait(&outside_service_manager->mcurs232_class->complete_frame_list_share->mcurs_share_cond,
                              &outside_service_manager->mcurs232_class->complete_frame_list_share->mcurs_share_mutex);
            DL_COUNT(outside_service_manager->mcurs232_class->complete_frame_list_head, elt, count);
        }
        pthread_mutex_unlock(&outside_service_manager->mcurs232_class->complete_frame_list_share->mcurs_share_mutex);
        complete_frame_list_t *tmp_head = NULL;
        move_all_complete_list_node(outside_service_manager->mcurs232_class, &tmp_head);

        while (tmp_head) {
            if (tmp_head->frame_type == ASK_CONFIG) {
                //请求配置帧校验版本号
                if (tmp_head->frame_buf[DEF_ASK_CONFIG_FRAME_VERSION_LOCATION] < DEF_ASK_CONFIG_FRAME_VERSION_NUM ||
                    tmp_head->frame_buf_size < 11) {
                    pop_complete_list(&tmp_head);
                    break;
                }
                serial_config_frame_t *serial_config_frame = malloc(sizeof(serial_config_frame_t));
                make_config_frame(serial_config_frame);
                hnlog_notice(serial_config_frame->config_frame, serial_config_frame->config_frame_len);
                int ret = write(outside_service_manager->mcurs232_class->mcu_rs232_port->handle,
                                serial_config_frame->config_frame, serial_config_frame->config_frame_len);
                nlog_info("write ret: %d", ret);
                if (ret == 0 || ret == -1) {
                    nlog_error("write to mcurs232 error");
                }
                nlog_info("complete_frmae trans over");
                if (serial_config_frame->config_frame)
                    free(serial_config_frame->config_frame);
                free(serial_config_frame);
            } else if (tmp_head->frame_type == COMMAND) {
                hnlog_notice(tmp_head->frame_buf, tmp_head->frame_buf_size);
                int ret = write_to_mcu(outside_service_manager, tmp_head->frame_buf, tmp_head->frame_buf_size);
                //     int ret = write(outside_service_manager->mcurs232_class->mcu_rs232_port->handle,
                //     tmp_head->frame_buf,
                //                     tmp_head->frame_buf_size);
                nlog_info("write ret: %d", ret);
                if (ret == 0 || ret == -1) {
                    nlog_error("write to mcurs232 error");
                }
                nlog_info("complete_frame_trans over");
            } else if (tmp_head->frame_type == ACK_TO_MQTT) {
                hnlog_notice(tmp_head->frame_buf, tmp_head->frame_buf_size);
                esv_between_adapter_driver_msg_t send_to_plugin_msg;
                if (tmp_head->frame_buf[1] == 0x00) {
                    return 0;
                }
                make_send_to_plugin_msg(&send_to_plugin_msg, tmp_head->frame_buf, tmp_head->frame_buf_size);
                forward_msg_to_esvdriver(outside_service_manager->neu_manager, &send_to_plugin_msg);
            }
            pop_complete_list(&tmp_head);
        }
    }
    return 0;
}

int write_to_mcu(esv_outside_service_manager_t *outside_service_manager, void *frame_buf, int buf_length) {
    int write_ret = 0;
    uint8_t *send_frame_buf = malloc(sizeof(uint8_t) * buf_length);
    memcpy(send_frame_buf, frame_buf, buf_length);
    uint8_t *tmp_send_frame_buf_pointer = send_frame_buf;
    while (buf_length > MAX_SEND_TO_MCU_BUF_NUM) {
        write_ret += write(outside_service_manager->mcurs232_class->mcu_rs232_port->handle, tmp_send_frame_buf_pointer,
                           MAX_SEND_TO_MCU_BUF_NUM);
        hnlog_notice(tmp_send_frame_buf_pointer, MAX_SEND_TO_MCU_BUF_NUM);
        nlog_info("write_ret: %d", write_ret);
        buf_length -= MAX_SEND_TO_MCU_BUF_NUM;
        tmp_send_frame_buf_pointer += MAX_SEND_TO_MCU_BUF_NUM;
    }

    hnlog_notice(tmp_send_frame_buf_pointer, buf_length);
    write_ret +=
        write(outside_service_manager->mcurs232_class->mcu_rs232_port->handle, tmp_send_frame_buf_pointer, buf_length);
    if (send_frame_buf) {
        free(send_frame_buf);
    }

    return write_ret;
    // }
}

int pop_complete_list(complete_frame_list_t **head) {
    int count;
    complete_frame_list_t *elt;
    DL_COUNT(*head, elt, count);
    if (count < 1) {
        return -1;
    }

    complete_frame_list_t *del = *head;
    DL_DELETE(*head, del);
    if (del->frame_buf) {
        free(del->frame_buf);
    }
    free(del);
    return 0;
}

int move_all_complete_list_node(Mcurs232_class_t *mcurs232_class, complete_frame_list_t **tmp_head) {
    int count;
    complete_frame_list_t *elt;
    pthread_mutex_lock(&mcurs232_class->complete_frame_list_share->mcurs_share_mutex);
    DL_COUNT(mcurs232_class->complete_frame_list_head, elt, count);
    if (count < 1) {
        pthread_mutex_unlock(&mcurs232_class->complete_frame_list_share->mcurs_share_mutex);
        return -1;
    }

    *tmp_head = mcurs232_class->complete_frame_list_head;
    mcurs232_class->complete_frame_list_head = NULL;
    pthread_mutex_unlock(&mcurs232_class->complete_frame_list_share->mcurs_share_mutex);

    return 0;
}

int erase_serial_port_read_buf(Mcurs232_class_t *mcurs232_class, int length) {
    // pthread_mutex_lock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
    utarray_erase(mcurs232_class->serial_port_read_buf_head, 0, length);
    // utarray_erase(plugin->serial_port_read_buf, 0, length);
    // pthread_mutex_unlock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
    // pthread_mutex_unlock(&plugin->serial_port_trans_mutex);

    return 0;
}

int find_frame_head_and_change_event(Mcurs232_class_t *mcurs232_class) {

    // 检查长度是否大于等于1
    // 1: 更改状态并返回
    // 0：错误，删除数组第一个字节并返回-1

    // printf("find_frame_head_and_change_event lock before\n");
    // pthread_mutex_lock(&plugin->serial_port_trans_mutex);
    // pthread_mutex_lock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
    // printf("find_frame_head_and_change_event lock after\n");
    unsigned char *buf = (unsigned char *) utarray_front(mcurs232_class->serial_port_read_buf_head);
    if (utarray_len(mcurs232_class->serial_port_read_buf_head) > 1 && buf[0] == FRAME_HEADER) {
        mcurs232_class->frame_event = FIND_FRAME_DATA_LENGTH;
        // printf("check head true\n");
        // pthread_mutex_unlock(&plugin->serial_port_trans_mutex);
        // pthread_mutex_unlock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
        return 0;
    }
    // pthread_mutex_unlock(&plugin->serial_port_trans_mutex);
    // pthread_mutex_unlock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);

    printf("find_frame_head erase\n");
    erase_serial_port_read_buf(mcurs232_class, 1);
    return -1;
}

int find_frame_data_length_and_change_event(Mcurs232_class_t *mcurs232_class, int *frame_data_length) {
    //检查utarray长度是否大于帧头到数据长度的距离
    // 1：更改状态并返回
    // 0：返回-1

    // pthread_mutex_lock(&plugin->serial_port_trans_mutex);
    // pthread_mutex_lock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
    unsigned char *buf = (unsigned char *) utarray_front(mcurs232_class->serial_port_read_buf_head);
    printf("find_frame_data_length utarray_len: %d\n", utarray_len(mcurs232_class->serial_port_read_buf_head));
    if (utarray_len(mcurs232_class->serial_port_read_buf_head) >= DATA_LENGTH_DISTANCE_FROM_FRAME_HEADER + 1 &&
        utarray_len(mcurs232_class->serial_port_read_buf_head) >= buf[DATA_LENGTH_DISTANCE_FROM_FRAME_HEADER]) {
        mcurs232_class->frame_event = FIND_FRAME_TAIL;
        // pthread_mutex_unlock(&plugin->serial_port_trans_mutex);
        // pthread_mutex_unlock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
        *frame_data_length = buf[DATA_LENGTH_DISTANCE_FROM_FRAME_HEADER];
        return 0;
    }
    // pthread_mutex_unlock(&plugin->serial_port_trans_mutex);
    // pthread_mutex_unlock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
    return -1;
}

int find_frame_tail_and_change_event(Mcurs232_class_t *mcurs232_class, int tail_distance_from_head) {
    //计算出帧头到帧尾的距离
    //判断utarray当前的长度是否大于该距离
    // 1：检查帧尾所在位置是匹配1a
    ////1：匹配成功，拷贝该内容到链表中
    ////0：该帧错误，删除帧头到该处的内容并更改状态
    // 0：该帧错误，删除帧头到该处的内容并更改状态

    // pthread_mutex_lock(&plugin->serial_port_trans_mutex);
    // pthread_mutex_lock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
    unsigned char *buf = (unsigned char *) utarray_front(mcurs232_class->serial_port_read_buf_head);
    // int tmp_distance_from_head =
    // DATA_LENGTH_DISTANCE_FROM_FRAME_HEADER + frame_data_length + DATA_LENGTH_TO_FRAME_END_OFFSET;

    if (utarray_len(mcurs232_class->serial_port_read_buf_head) > tail_distance_from_head) {
        if (buf[tail_distance_from_head] == FRAME_TAIL) {
            // printf("utarray_len is %d\n", utarray_len(plugin->serial_port_read_buf));
            // pthread_mutex_unlock(&plugin->serial_port_trans_mutex);
            // pthread_mutex_unlock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
            nlog_info("buf[tail_distance_from_head] == FRAME_TAIL");
            mcurs232_class->frame_event = FIND_FRAME_HEADER;
            return 0;
        } else {
            nlog_info("buf[tail_distance_from_head] != FRAME_TAIL");
            erase_serial_port_read_buf(mcurs232_class, 1);
            // printf("utarray_len is %d\n", utarray_len(plugin->serial_port_read_buf));
            // print_read_buf(plugin);
            // pthread_mutex_unlock(&plugin->serial_port_trans_mutex);
            // pthread_mutex_unlock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
            mcurs232_class->frame_event = FIND_FRAME_HEADER;
            return -1;
        }
    } else {
        // pthread_mutex_unlock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
        // pthread_mutex_unlock(&plugin->serial_port_trans_mutex);
        nlog_info("utarray_len < tail_distance_from_head");
        mcurs232_class->frame_event = FIND_FRAME_HEADER;
        // erase_serial_port_read_buf(mcurs232_class, 1);
        // printf("utarray_len is %d\n", utarray_len(plugin->serial_port_read_buf));
        return -1;
    }
}

// int make_up_ch(Mcurs232_class_t *mcurs232_class, int complete_len, char **complete_frame_buf) {
int init_complete_frame_buf(Mcurs232_class_t *mcurs232_class, int complete_len, char **complete_frame_buf) {
    // pthread_mutex_lock(&plugin->serial_port_trans_mutex);
    // pthread_mutex_lock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
    unsigned char *buf = (unsigned char *) utarray_front(mcurs232_class->serial_port_read_buf_head);
    nlog_info("unsigned char buf utarray_front after");
    if (buf == NULL || utarray_len(mcurs232_class->serial_port_read_buf_head) < complete_len) {
        // pthread_mutex_unlock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
        // pthread_mutex_unlock(&plugin->serial_port_trans_mutex);
        return -1;
    }
    // pthread_mutex_unlock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
    // pthread_mutex_unlock(&plugin->serial_port_trans_mutex);

    *complete_frame_buf = (char *) malloc((complete_len) * sizeof(char));
    memcpy(*complete_frame_buf, buf, complete_len);
    nlog_info("======================add complete frame===================");
    hnlog_notice(*complete_frame_buf, complete_len);

    return 0;
}

int check_serial_port_message(Mcurs232_class_t *mcurs232_class) {
    // nlog_info("check_serial_port_message");
    printf("++++++++++++++++++check_serial_port_message to lock: %ld\n", syscall(SYS_gettid));
    pthread_mutex_lock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
    printf("-----------------check_serial_port_message lock: %ld\n", syscall(SYS_gettid));
    int frame_data_length = 0;
    while (utarray_len(mcurs232_class->serial_port_read_buf_head) > 0) {
        switch (mcurs232_class->frame_event) {
            case FIND_FRAME_HEADER: {
                nlog_info("find_frame_header");
                //找帧头
                int ret = find_frame_head_and_change_event(mcurs232_class);
                printf("utarray_len: %d\n", utarray_len(mcurs232_class->serial_port_read_buf_head));
                printf("find frame_head ret: %d\n", ret);
                // if (ret == -1) {
                // goto switch_end;
                // }
                break;
            }
            case FIND_FRAME_DATA_LENGTH: {
                //找数据长度
                int find_frame_data_length_ret =
                    find_frame_data_length_and_change_event(mcurs232_class, &frame_data_length);
                if (find_frame_data_length_ret == -1) {
                    // erase_serial_port_read_buf(mcurs232_class, 1);
                    mcurs232_class->frame_event = FIND_FRAME_DATA_LENGTH;
                    pthread_mutex_unlock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
                    return -1;
                }
                printf("end find_frame_data_length\n");
                break;
            }
            case FIND_FRAME_TAIL: {
                //找帧尾

                printf("begin find frame tail\n");
                int tmp_distance_from_head =
                    DATA_LENGTH_DISTANCE_FROM_FRAME_HEADER + 1 + frame_data_length + DATA_LENGTH_TO_FRAME_END_OFFSET;
                int tail_distance_from_head = tmp_distance_from_head - 1;
                nlog_info("tail distance from head: %d", tail_distance_from_head);
                int find_frame_tail_ret = find_frame_tail_and_change_event(mcurs232_class, tail_distance_from_head);
                if (find_frame_tail_ret == -1) {
                    pthread_mutex_unlock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
                    return -1;
                }

                frame_e frame_type = check_serial_port_type(mcurs232_class);
                char *complete_frame_buf = NULL;
                init_complete_frame_buf(mcurs232_class, tmp_distance_from_head, &complete_frame_buf);
                insert_new_element_to_complete_frame_list(mcurs232_class, complete_frame_buf, tmp_distance_from_head,
                                                          frame_type);

                composition_ack_frame(mcurs232_class);
                free(complete_frame_buf);
                erase_serial_port_read_buf(mcurs232_class, tmp_distance_from_head);

                break;
            }
        }
    switch_end : { printf("while_end\n"); }
    }
    printf("+++++++++++++++++++check_serial_port_message to unlock: %ld\n", syscall(SYS_gettid));
    pthread_mutex_unlock(&mcurs232_class->serial_port_trans_share->mcurs_share_mutex);
    printf("------------------check_serial_port_message unlock: %ld\n", syscall(SYS_gettid));
    return 0;
}

static int mcu_rs232_reading_cb(enum neu_event_io_type type, int fd, void *usr_data) {
    // neu_plugin_t *plugin = (neu_plugin_t *) usr_data;

    esv_outside_service_manager_t *outside_service_manager = (esv_outside_service_manager_t *) usr_data;
    nlog_notice("mcu rs232 reading event callback type: %d", type);

    if (type == NEU_EVENT_IO_READ) {
        unsigned char *readBuf = NULL;
        int readBufSize = 0;
        readBytes(outside_service_manager->mcurs232_class->mcu_rs232_port, &readBuf, &readBufSize);
        nlog_info("read buf size: %d", readBufSize);
        if (readBufSize <= 0) {
            return 0;
        }

        hnlog_notice(readBuf, readBufSize);
        push_back_serial_port_read_buf(outside_service_manager->mcurs232_class, readBuf, readBufSize);
        // sleep(10);
        // nlog_info("readBufSize :%d", readBufSize);
        // nlog_info("read buf:");
        // hnlog_notice(readBuf, readBufSize);
        free(readBuf);
    }

    return 0;
}

int make_send_to_plugin_msg(esv_between_adapter_driver_msg_t *send_to_plugin_msg, const unsigned char *msg,
                            int msg_len) {
    send_to_plugin_msg->method = ESV_TO_ADAPTER_MCURS_POST;
    send_to_plugin_msg->msg_type = ESV_TAM_BYTES_PTR;
    send_to_plugin_msg->msg = malloc(sizeof(unsigned char) * msg_len);
    if (send_to_plugin_msg->msg == NULL) {
        nlog_warn("Mcurs send_to_plugin_msg malloc failed");
        return -1;
    }

    memcpy(send_to_plugin_msg->msg, msg, msg_len);
    send_to_plugin_msg->msg_len = msg_len;

    return 0;
}

int make_send_to_adapter_msg(esv_between_adapter_driver_msg_t *send_to_adapter_msg, const unsigned char *msg,
                             int msg_len) {

    return 0;
}

void *mcurs232_thread_func(void *arg) {
    esv_outside_service_manager_t *outside_service_manager = (esv_outside_service_manager_t *) arg;
    handle_start(outside_service_manager);
    nlog_info("mcurs232_thread_func");
    pthread_create(&outside_service_manager->mcurs232_class->mcurs232_send_thread, NULL, send_complete_func,
                   outside_service_manager);

    pthread_exit(NULL);
}
