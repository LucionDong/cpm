/**
 * NEURON IIoT System for Industry 4.0
 * Copyright (C) 2020-2022 EMQ Technologies Co., Ltd All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/
#include <nng/nng.h>
#include <nng/protocol/pair1/pair.h>
#include <nng/supplemental/util/platform.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "event/event.h"
/* #include "persist/persist.h" */
#include "adapter.h"
#include "adapter/adapter_internal.h"
#include "adapter/driver/driver_internal.h"
#include "errcodes.h"
#include "node_manager.h"
#include "plugin_manager.h"
#include "storage.h"
#include "utils/log.h"
#include "utils/time.h"
/* #include "subscribe.h" */
/* #include "outside_service_manager.h" */
#include "connection/mqtt/lan_mqtt5_service.h"
#include "manager.h"
#include "manager_internal.h"

// definition for adapter names
#define DEFAULT_DASHBOARD_ADAPTER_NAME DEFAULT_DASHBOARD_PLUGIN_NAME

static const char *const url = "inproc://neu_manager";
/* static const char *const url = "ipc:///tmp/nng/cpm/esv_core_plugin_manager"; */

static int manager_loop(enum neu_event_io_type type, int fd, void *usr_data);
static int manager_level_check(void *usr_data);
inline static void reply(neu_manager_t *manager, neu_reqresp_head_t *header, void *data);
inline static nng_msg *trans_data_dup(nng_msg *msg);
inline static void trans_data_free(nng_msg *msg);
typedef nng_msg *(*msg_dup)(nng_msg *msg);
inline static void forward_msg_dup(neu_manager_t *manager, nng_msg *msg, nng_pipe pipe, msg_dup dup);
inline static void forward_msg(neu_manager_t *manager, nng_msg *msg, const char *node);
inline static void notify_monitor(neu_manager_t *manager, neu_reqresp_type_e event, void *data);
static void start_static_adapter(neu_manager_t *manager, const char *name);
static int update_timestamp(void *usr_data);
static void start_single_adapter(neu_manager_t *manager, const char *name, const char *plugin_name, bool display);

neu_manager_t *neu_manager_create() {
    int rv = 0;
    neu_manager_t *manager = calloc(1, sizeof(neu_manager_t));
    neu_event_io_param_t param = {
        .usr_data = (void *) manager,
        .cb = manager_loop,
    };

    neu_event_timer_param_t timestamp_timer_param = {
        .second = 0,
        .millisecond = 10,
        .cb = update_timestamp,
        .type = NEU_EVENT_TIMER_NOBLOCK,
    };

    neu_event_timer_param_t timer_level = {
        .second = 60,
        .millisecond = 0,
        .cb = manager_level_check,
        .type = NEU_EVENT_TIMER_BLOCK,
    };

    manager->events = neu_event_new();
    manager->plugin_manager = neu_plugin_manager_create();
    manager->node_manager = neu_node_manager_create();
    manager->esv_outside_service_manager = esv_outside_service_manager_create();

    esv_outside_service_manager_set_neu_manager(manager->esv_outside_service_manager, manager);
    /* manager->subscribe_manager = neu_subscribe_manager_create(); */
    /* manager->template_manager  = neu_template_manager_create(); */

    /* rv = nng_pair1_open_poly(&manager->socket); */
    /* assert(rv == 0); */

    /* rv = nng_listen(manager->socket, url, NULL, 0); */
    /* assert(rv == 0); */
    /* nlog_info("manager listen url:%s", url); */

    /* nng_socket_set_int(manager->socket, NNG_OPT_RECVBUF, 8192); */
    /* nng_socket_set_int(manager->socket, NNG_OPT_SENDBUF, 8292); */
    /* nng_socket_set_ms(manager->socket, NNG_OPT_SENDTIMEO, 1000); */
    /* nng_socket_get_int(manager->socket, NNG_OPT_RECVFD, &param.fd); */
    /* manager->loop = neu_event_add_io(manager->events, param); */

    manager->timestamp_lev_manager = 0;

    /* neu_metrics_init(); */
    /* start_static_adapter(manager, DEFAULT_DASHBOARD_PLUGIN_NAME); */

    /* if (manager_load_plugin(manager) != 0) { */
    /*     nlog_warn("load plugin error"); */
    /* } */
    if (esv_manager_load_232_plugin_from_db(manager) != 0) {
        nlog_warn("load plugin error");
    }

    /* UT_array *single_plugins = */
    /*     neu_plugin_manager_get_single(manager->plugin_manager); */

    /* utarray_foreach(single_plugins, neu_resp_plugin_info_t *, plugin) */
    /* { */
    /*     start_single_adapter(manager, plugin->single_name, plugin->name, */
    /*                          plugin->display); */
    /* } */
    /* utarray_free(single_plugins); */

    /* if (manager_load_template(manager) != 0) { */
    /*     nlog_warn("load template error"); */
    /* } */

    // mqtt3
    /* esv_lan_mqtt_service_t *esv_lan_mqtt_service = esv_lan_mqtt_service_create(); */
    /* esv_lan_mqtt_srvice_set_manager(esv_lan_mqtt_service, manager); */
    /* manager->esv_lan_mqtt_service = esv_lan_mqtt_service; */

    // mqtt5
    esv_lan_mqtt5_service_t *esv_lan_mqtt5_service = esv_lan_mqtt5_service_create();
    esv_lan_mqtt5_srvice_set_manager(esv_lan_mqtt5_service, manager);
    manager->esv_lan_mqtt5_service = esv_lan_mqtt5_service;

    nlog_debug("to load 232 node");
    /* manager_load_node(manager); */
    esv_manager_load_232_node(manager);
    /* while (neu_node_manager_exist_uninit(manager->node_manager)) { */
    /*     usleep(1000 * 100); */
    /* } */

    // mqtt3
    /* nlog_debug("&manager=%p", manager); */
    /* lan_mqtt_service_start(manager->esv_lan_mqtt_service); */
    // mqtt5
    nlog_debug("to start cpm232 lan mqtt5 service");
    lan_mqtt5_service_start(manager->esv_lan_mqtt5_service);
    /* manager_load_subscribe(manager); */

    /* timer_level.usr_data = (void *) manager; */
    /* manager->timer_lev   = neu_event_add_timer(manager->events, timer_level); */

    /* timestamp_timer_param.usr_data = (void *) manager; */
    /* manager->timer_timestamp = */
    /*     neu_event_add_timer(manager->events, timestamp_timer_param); */

    nlog_notice("manager start");
    return manager;
}

void neu_manager_destroy(neu_manager_t *manager) {
    neu_reqresp_head_t header = {.type = NEU_REQ_NODE_UNINIT};
    neu_req_node_init_t uninit = {0};
    nng_msg *uninit_msg = NULL;
    UT_array *pipes = neu_node_manager_get_pipes_all(manager->node_manager);

    neu_event_del_timer(manager->events, manager->timer_lev);
    neu_event_del_timer(manager->events, manager->timer_timestamp);
    strcpy(header.sender, "manager");
    utarray_foreach(pipes, nng_pipe *, pipe) {
        uninit_msg = neu_msg_gen(&header, &uninit);
        nng_msg_set_pipe(uninit_msg, *pipe);

        if (nng_sendmsg(manager->socket, uninit_msg, 0) != 0) {
            nng_msg_free(uninit_msg);
            nlog_warn("manager -> %d uninit msg send fail", (*pipe).id);
        }
    }
    utarray_free(pipes);

    while (1) {
        usleep(1000 * 100);
        if (neu_node_manager_size(manager->node_manager) == 0) {
            break;
        }
    }

    /* neu_subscribe_manager_destroy(manager->subscribe_manager); */
    neu_node_manager_destroy(manager->node_manager);
    neu_plugin_manager_destroy(manager->plugin_manager);
    /* neu_template_manager_destroy(manager->template_manager); */
    /* esv_outside_service_manager_destory(manager->esv_outside_service_manager); */

    nng_close(manager->socket);
    neu_event_del_io(manager->events, manager->loop);
    neu_event_close(manager->events);

    // mqtt3
    /* lan_mqtt_service_stop(manager->esv_lan_mqtt_service); */
    // mqtt5
    lan_mqtt5_service_stop(manager->esv_lan_mqtt5_service);

    free(manager);
    nlog_notice("manager exit");
}

const char *neu_manager_get_url() {
    return url;
}

static int manager_level_check(void *usr_data) {
    neu_manager_t *manager = (neu_manager_t *) usr_data;

    if (0 != manager->timestamp_lev_manager) {
        struct timeval tv = {0};
        int64_t diff = {0};
        int64_t delay_s = 600;
        zlog_category_t *neuron = zlog_get_category("neuron");

        gettimeofday(&tv, NULL);
        diff = tv.tv_sec - manager->timestamp_lev_manager;
        if (delay_s <= diff) {
            int ret = zlog_level_switch(neuron, default_log_level);
            if (0 != ret) {
                nlog_error("Modify default log level fail, ret:%d", ret);
            }
        }
    }

    return 0;
}

static int manager_loop(enum neu_event_io_type type, int fd, void *usr_data) {
    neu_manager_t *manager = (neu_manager_t *) usr_data;
    int rv = 0;
    nng_msg *msg = NULL;
    neu_reqresp_head_t *header = NULL;

    if (type == NEU_EVENT_IO_CLOSED || type == NEU_EVENT_IO_HUP) {
        nlog_warn("manager socket(%d) recv closed or hup %d.", fd, type);
        return 0;
    }

    rv = nng_recvmsg(manager->socket, &msg, NNG_FLAG_NONBLOCK);
    if (rv != 0) {
        nlog_warn("manager recv msg error: %d", rv);
        return 0;
    }
    /* hnlog_notice(nng_msg_body(msg), nng_msg_capacity(msg)); */
    header = (neu_reqresp_head_t *) nng_msg_body(msg);

    nlog_info("manager recv msg from: %s to %s, type: %s", header->sender, header->receiver,
              neu_reqresp_type_string(header->type));
    switch (header->type) {
        /* case NEU_REQRESP_TRANS_DATA: { */
        /*     neu_reqresp_trans_data_t *cmd = (neu_reqresp_trans_data_t *) &header[1]; */
        /*     UT_array *apps = neu_subscribe_manager_find(manager->subscribe_manager, */
        /*                                                 cmd->driver, cmd->group); */
        /*     if (apps != NULL) { */
        /*         utarray_foreach(apps, neu_app_subscribe_t *, app) */
        /*         { */
        /*             forward_msg_dup(manager, msg, app->pipe, trans_data_dup); */
        /*             nlog_debug("forward trans data to pipe: %d", app->pipe.id); */
        /*         } */
        /*         utarray_free(apps); */
        /*     } */

        /*     trans_data_free(msg); */
        /*     break; */
        /* } */
        /* case NEU_REQ_UPDATE_LICENSE: { */
        /*     UT_array *pipes = neu_node_manager_get_pipes_all(manager->node_manager); */

        /*     utarray_foreach(pipes, nng_pipe *, pipe) */
        /*     { */
        /*         forward_msg_dup(manager, msg, *pipe, NULL); */
        /*         nlog_notice("forward license update to pipe: %d", pipe->id); */
        /*     } */
        /*     utarray_free(pipes); */

        /*     break; */
        /* } */
        case NEU_REQ_NODE_INIT: {
            neu_req_node_init_t *init = (neu_req_node_init_t *) &header[1];
            nng_pipe pipe = nng_msg_get_pipe(msg);

            if (0 != neu_node_manager_update(manager->node_manager, init->node, pipe)) {
                nlog_warn("bind node %s to pipe(%d) fail", init->node, pipe.id);
                break;
            }

            if (init->state == NEU_NODE_RUNNING_STATE_RUNNING || init->state == NEU_NODE_RUNNING_STATE_STOPPED) {
                neu_adapter_t *adapter = neu_node_manager_find(manager->node_manager, init->node);
                if (init->state == NEU_NODE_RUNNING_STATE_RUNNING) {
                    neu_adapter_start(adapter);
                } else if (init->state == NEU_NODE_RUNNING_STATE_STOPPED) {
                    neu_adapter_stop(adapter);
                }
            }

            nlog_notice("bind node %s to pipe(%d)", init->node, pipe.id);
            break;
        }
        case NEU_REQ_GET_PLUGIN: {
            UT_array *plugins = neu_manager_get_plugins(manager);
            neu_resp_get_plugin_t resp = {.plugins = plugins};

            header->type = NEU_RESP_GET_PLUGIN;
            strcpy(header->receiver, header->sender);
            reply(manager, header, &resp);
            break;
        }
        case NEU_REQ_ADD_NODE: {
            neu_req_add_node_t *cmd = (neu_req_add_node_t *) &header[1];
            int error = neu_manager_add_node(manager, cmd->node, cmd->plugin, false, false);
            neu_resp_error_t e = {.error = error};

            if (error == NEU_ERR_SUCCESS) {
                manager_storage_add_node(manager, cmd->node);
                /* notify_monitor(manager, NEU_REQ_ADD_NODE_EVENT, cmd); */
            }

            header->type = NEU_RESP_ERROR;
            strcpy(header->receiver, header->sender);
            reply(manager, header, &e);
            break;
        }
        case NEU_REQ_GET_NODE: {
            neu_req_get_node_t *cmd = (neu_req_get_node_t *) &header[1];
            UT_array *nodes = neu_manager_get_nodes(manager, cmd->type, cmd->plugin, cmd->node);
            neu_resp_get_node_t resp = {.nodes = nodes};

            header->type = NEU_RESP_GET_NODE;
            strcpy(header->receiver, header->sender);
            reply(manager, header, &resp);
            break;
        }
        case NEU_REQ_GET_NODES_STATE: {
            neu_resp_get_nodes_state_t resp = {0};
            UT_array *states = neu_node_manager_get_state(manager->node_manager);

            resp.states = utarray_clone(states);

            strcpy(header->receiver, header->sender);
            strcpy(header->sender, "manager");
            header->type = NEU_RESP_GET_NODES_STATE;
            reply(manager, header, &resp);

            utarray_free(states);
            break;
        }
        case NEU_REQ_GET_NODE_SETTING:
        case NEU_REQ_NODE_CTL: {
            if (neu_node_manager_find(manager->node_manager, header->receiver) == NULL) {
                neu_resp_error_t e = {.error = NEU_ERR_NODE_NOT_EXIST};
                header->type = NEU_RESP_ERROR;
                neu_msg_exchange(header);
                /* reply(manager, header, &e); */
            } else {
                forward_msg(manager, msg, header->receiver);
            }

            break;
        }

        case NEU_REQ_NODE_CTL_EVENT: {
            if (neu_node_manager_find(manager->node_manager, header->receiver)) {
                forward_msg(manager, msg, header->receiver);
            }

            break;
        }

        case NEU_REQ_NODE_SETTING:
            /* case NEU_REQ_NODE_SETTING_EVENT: */
            {
                neu_req_node_setting_t *cmd = (neu_req_node_setting_t *) &header[1];

                if (neu_node_manager_find(manager->node_manager, header->receiver) == NULL) {
                    neu_resp_error_t e = {.error = NEU_ERR_NODE_NOT_EXIST};
                    header->type = NEU_RESP_ERROR;
                    neu_msg_exchange(header);
                    reply(manager, header, &e);
                    free(cmd->setting);
                } else {
                    forward_msg(manager, msg, header->receiver);
                }
                break;
            }

        case NEU_RESP_GET_NODE_SETTING:
        case NEU_RESP_ERROR: {
            forward_msg(manager, msg, header->receiver);
            break;
        }

            /* case ESV_THING_MODEL_TRANS_DATA_INPROC: { */
            /* 	 esv_thing_model_trans_data_inproc_t *cmd = (esv_thing_model_trans_data_inproc_t *) &header[1]; */
            /* 	 nlog_info("ESV_THING_MODEL_TRANS_DATA_INPROC method: %d driver: %s, product_key: %s, device_name: %s",
             * cmd->method, cmd->driver, cmd->product_key, cmd->device_name); */
            /* 	 if (NULL != cmd->data_root) { */
            /* 		 esv_outside_service_manager_thing_model_msg_send(manager->esv_outside_service_manager, cmd); */
            /* 		 char *data_root_str = json_dumps(cmd->data_root, 0); */
            /* 		 if (NULL != data_root_str) { */
            /* 			 nlog_info("thing model data root: %s", data_root_str); */
            /* 			 free(data_root_str); */
            /* 		 } */
            /* 		 json_decref(cmd->data_root); */
            /* 	 } */
            /* 	 free(cmd->driver); */
            /* 	 free(cmd->product_key); */
            /* 	 free(cmd->device_name); */
            /* 	 break; */
            /*  } */

        default:
            assert(false);
            break;
    }

    nng_msg_free(msg);
    return 0;
}

inline static void forward_msg_dup(neu_manager_t *manager, nng_msg *msg, nng_pipe pipe, msg_dup dup) {
    nng_msg *out_msg;

    nng_msg_dup(&out_msg, msg);
    if (dup != NULL) {
        out_msg = dup(out_msg);
    }
    nng_msg_set_pipe(out_msg, pipe);
    if (nng_sendmsg(manager->socket, out_msg, 0) == 0) {
        nlog_info("forward msg to pipe %d", pipe.id);
    } else {
        nlog_warn("forward msg to pipe %d fail", pipe.id);
        nng_msg_free(out_msg);
    }
}

inline static nng_msg *trans_data_dup(nng_msg *msg) {
    neu_reqresp_head_t *header = (neu_reqresp_head_t *) nng_msg_body(msg);
    assert(header->type == NEU_REQRESP_TRANS_DATA);
    neu_reqresp_trans_data_t *cmd = (neu_reqresp_trans_data_t *) &header[1];

    for (int i = 0; i < cmd->n_tag; i++) {
        if (cmd->tags[i].value.type == NEU_TYPE_PTR) {
            uint8_t *ptr = calloc(cmd->tags[i].value.value.ptr.length, 1);
            memcpy(ptr, cmd->tags[i].value.value.ptr.ptr, cmd->tags[i].value.value.ptr.length);
            cmd->tags[i].value.value.ptr.ptr = ptr;
        }
    }

    return msg;
}

inline static void trans_data_free(nng_msg *msg) {
    neu_reqresp_head_t *header = (neu_reqresp_head_t *) nng_msg_body(msg);
    assert(header->type == NEU_REQRESP_TRANS_DATA);
    neu_reqresp_trans_data_t *cmd = (neu_reqresp_trans_data_t *) &header[1];

    for (int i = 0; i < cmd->n_tag; i++) {
        if (cmd->tags[i].value.type == NEU_TYPE_PTR) {
            free(cmd->tags[i].value.value.ptr.ptr);
        }
    }
}

inline static void forward_msg(neu_manager_t *manager, nng_msg *msg, const char *node) {
    nng_msg *out_msg;
    nng_pipe pipe = neu_node_manager_get_pipe(manager->node_manager, node);

    nng_msg_dup(&out_msg, msg);
    nng_msg_set_pipe(out_msg, pipe);
    if (nng_sendmsg(manager->socket, out_msg, 0) == 0) {
        nlog_info("forward msg to %s", node);
    } else {
        nlog_warn("forward msg to pipe (%s)%d fail", node, pipe.id);
        nng_msg_free(out_msg);
    }
}

inline static void notify_monitor(neu_manager_t *manager, neu_reqresp_type_e event, void *data) {
    nng_pipe pipe = neu_node_manager_get_pipe(manager->node_manager, "monitor");
    if (0 == pipe.id) {
        nlog_error("no monitor node");
        return;
    }

    neu_reqresp_head_t header = {
        .sender = "manager",
        .receiver = "monitor",
        .type = event,
    };

    nng_msg *msg = neu_msg_gen(&header, data);
    nng_msg_set_pipe(msg, pipe);

    int ret = nng_sendmsg(manager->socket, msg, 0);
    if (ret != 0) {
        nng_msg_free(msg);
        nlog_warn("notify %s of %s, error: %s", header.receiver, neu_reqresp_type_string(header.type),
                  nng_strerror(ret));
    }
}

static void start_static_adapter(neu_manager_t *manager, const char *name) {
    neu_adapter_t *adapter = NULL;
    neu_plugin_instance_t instance = {0};
    neu_adapter_info_t adapter_info = {
        .name = name,
    };

    neu_plugin_manager_load_static(manager->plugin_manager, name, &instance);
    adapter_info.handle = instance.handle;
    adapter_info.module = instance.module;

    /* adapter = neu_adapter_create(manager->esv_outside_service_manager, &adapter_info, true); */
    adapter = neu_adapter_create(manager->esv_outside_service_manager, &adapter_info, true);
    neu_node_manager_add_static(manager->node_manager, adapter);
    neu_adapter_init(adapter, false);
    neu_adapter_start(adapter);
}

/* static void start_single_adapter(neu_manager_t *manager, const char *name, */
/*                                  const char *plugin_name, bool display) */
/* { */
/*     neu_adapter_t *       adapter      = NULL; */
/*     neu_plugin_instance_t instance     = { 0 }; */
/*     neu_adapter_info_t    adapter_info = { */
/*         .name = name, */
/*     }; */

/*     if (0 != */
/*         neu_plugin_manager_create_instance(manager->plugin_manager, plugin_name, */
/*                                            &instance)) { */
/*         return; */
/*     } */

/*     adapter_info.handle = instance.handle; */
/*     adapter_info.module = instance.module; */
/*     adapter             = neu_adapter_create(&adapter_info, true); */

/*     neu_node_manager_add_single(manager->node_manager, adapter, display); */
/*     if (display) { */
/*         manager_storage_add_node(manager, name); */
/*     } */

/*     neu_adapter_init(adapter, false); */
/*     neu_adapter_start_single(adapter); */
/* } */

inline static void reply(neu_manager_t *manager, neu_reqresp_head_t *header, void *data) {
    nng_msg *msg = neu_msg_gen(header, data);
    nng_pipe pipe = neu_node_manager_get_pipe(manager->node_manager, header->receiver);

    nng_msg_set_pipe(msg, pipe);
    int ret = nng_sendmsg(manager->socket, msg, 0);

    if (ret != 0) {
        nng_msg_free(msg);
        nlog_warn("reply %s to %s, error: %d", neu_reqresp_type_string(header->type), header->receiver, ret);
    }
}

static int update_timestamp(void *usr_data) {
    (void) usr_data;
    global_timestamp = neu_time_ms();
    return 0;
}
