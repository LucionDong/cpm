#include <errno.h>
#include <esvcpm/utils/log.h>

#include "mcurs232_handle.h"
#include "mcurs232_helper_functions.h"
#include "rs232-plugin.h"
#include "serial_port.h"

char *mcu_rs232_devie_path = "/dev/ttyS2";

static int mcu_rs232_reading_cb(enum neu_event_io_type type, int fd, void *usr_data);
int open_mcu_rs232_port(neu_plugin_t *plugin);
int close_mcu_rs232_port(neu_plugin_t *plugin);

int handle_start(neu_plugin_t *plugin) {
    open_mcu_rs232_port(plugin);
    plugin->epoll_events = neu_event_new();
    neu_event_io_param_t param = {
        .fd = plugin->mcu_rs232_port->handle,
        .usr_data = (void *) plugin,
        .cb = mcu_rs232_reading_cb,
    };
    plugin->mcu_rs232_reading_event = neu_event_add_io(plugin->epoll_events, param);
    return 0;
}

int handle_stop(neu_plugin_t *plugin) {
    close_mcu_rs232_port(plugin);
    neu_event_del_io(plugin->epoll_events, plugin->mcu_rs232_reading_event);
    neu_event_close(plugin->epoll_events);
    return 0;
}

int open_mcu_rs232_port(neu_plugin_t *plugin) {
    plugin->mcu_rs232_port = open_port(mcu_rs232_devie_path);
    if (!plugin->mcu_rs232_port) {
        plog_error(plugin, "open mcu rs232 port(%s) error!", mcu_rs232_devie_path);
        return -1;
    }

    plog_info(plugin, "open mcu rs232 port(%s) success!", mcu_rs232_devie_path);

    return 0;
}

int close_mcu_rs232_port(neu_plugin_t *plugin) {

    if (!plugin->mcu_rs232_port) {
        plog_warn(plugin, "close mcu rs232 port(null) error!");
        return -1;
    }

    int rv = close_port(plugin->mcu_rs232_port);
    if (rv) {
        plog_warn(plugin, "close mcu rs232 port(%s) error:%s", mcu_rs232_devie_path, strerror(errno));
    } else {
        plugin->mcu_rs232_port = NULL;
        plog_info(plugin, "close mcu rs232 port(%s) success!", mcu_rs232_devie_path);
    }

    return rv;
}

static int mcu_rs232_reading_cb(enum neu_event_io_type type, int fd, void *usr_data) {
    neu_plugin_t *plugin = (neu_plugin_t *) usr_data;
    plog_notice(plugin, "mcu rs232 reading event callback type: %d", type);

    if (type == NEU_EVENT_IO_READ) {
        char *readBuf = NULL;
        int readBufSize = 0;
        readBytes(plugin->mcu_rs232_port, &readBuf, &readBufSize);
        plog_info(plugin, "read buf size: %d", readBufSize);
        if (readBufSize <= 0) {
            return 0;
        }

        plog_info(plugin, "read buf:");
        hnlog_notice(readBuf, readBufSize);
        free(readBuf);
    }

    return 0;
}
