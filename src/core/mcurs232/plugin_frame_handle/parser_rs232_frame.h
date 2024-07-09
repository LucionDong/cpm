/*
 * rs232_recv.h
 * Copyright (C) 2023 dongbin <dongbin0625@163.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __RS232_RECV_H__
#define __RS232_RECV_H__

typedef struct mcurs232_class mcurs232_class_t;

#include "../../outside_service_manager.h"
#include "../mcurs232_helper_functions.h"

#define FRAME_CONTROL_DOMAIN 2
#define DATA_LENGTH_DISTANCE_FROM_FRAME_HEADER 3
#define UART_PORT_DISTANCE_FROM_FRAME_HEADER 7
#define DATA_LENGTH_TO_FRAME_END_OFFSET 3
#define TYPE_DISTANCE_FROM_FRAME_HEADER 1
#define FRAME_COMMAND_TYPE 7

#define FRAME_HEADER 0xee
#define FRAME_TAIL 0x1a
#define DEF_ASK_CONFIG_FRAME 0x44
#define DEF_CONFIG_FRAME 0x45
#define DEF_COMMAND_FRAME 0x46
#define DEF_NORMAL_ACK_FRAME 0x42

#define DEF_ASK_CONFIG_FRAME_VERSION_LOCATION 7
#define DEF_ASK_CONFIG_FRAME_VERSION_NUM 0x15

#define DEF_STATUS_COMMAND_TYPE 0x03
#define DEF_WRITE_COMMAND_TYPE 0x01
#define DEF_READ_COMMAND_TYPE 0x02

#define MAX_SEND_TO_MCU_BUF_NUM 65

typedef enum {
    REQUEST_CONFIG_FRAME = 1,
    RESPONSE_CONFIG_FRAME,
    FRAME_TO_MCU,
    ERROR,
    NORMAL_ACK,
    FRAME_FROM_MCU,
} frame_e;

typedef enum {
    FIND_FRAME_HEADER = 1,
    FIND_FRAME_DATA_LENGTH,
    FIND_FRAME_TAIL,
} event_e;

typedef struct complete_frame_list {
    unsigned char *frame_buf;
    frame_e frame_type;
    int frame_buf_size;
    struct complete_frame_list *next, *prev;
} complete_frame_list_t;

typedef struct mcurs_share {
    pthread_mutex_t mcurs_share_mutex;
    pthread_cond_t mcurs_share_cond;
    pthread_mutexattr_t mcurs_share_mutex_attr;
} mcurs_share_t;

typedef struct mcurs232_class {
    pthread_t mcurs232_thread;
    pthread_t mcurs232_send_thread;

    mcurs_share_t *serial_port_trans_share;
    mcurs_share_t *complete_frame_list_share;

    UT_array *serial_port_read_buf_head;
    complete_frame_list_t *complete_frame_list_head;

    event_e frame_event;

    serialPort *mcu_rs232_port;
} mcurs232_class_t;

int init_mcurs_share(mcurs_share_t *share);
int init_mcurs_class(mcurs232_class_t *mcurs_class);
void *mcurs232_thread_func(void *arg);
int destroy_mcurs232_share(mcurs232_class_t *mcurs_class);
int destroy_mcurs232_pthread(mcurs232_class_t *mcurs_class);
int destroy_mcurs232_class(esv_outside_service_manager_t *outside_service_manager);
int push_back_serial_port_read_buf_and_check(mcurs232_class_t *mcurs232_class, const unsigned char *buf,
                                             int buf_length);
// int set_read_buf(mcurs232_class_t *mcurs232_class, const unsigned char *buf, int buf_length);
#endif /* !__RS232_RECV_H__ */
