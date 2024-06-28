/*
 * rs232_recv.h
 * Copyright (C) 2023 dongbin <dongbin0625@163.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __RS232_RECV_H__
#define __RS232_RECV_H__

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

int init_mcurs_share(mcurs_share_t *share);
int init_mcurs_class(Mcurs232_class_t *mcurs_class);
void *mcurs232_thread_func(void *arg);
int destroy_mcurs232_share(Mcurs232_class_t *mcurs_class);
int destroy_mcurs232_pthread(Mcurs232_class_t *mcurs_class);
int destroy_mcurs232_class(esv_outside_service_manager_t *outside_service_manager);
int set_read_buf(Mcurs232_class_t *mcurs232_class, const unsigned char *buf, int buf_length);
#endif /* !__RS232_RECV_H__ */
