#include "utils/log.h"
#include "storage.h"

int manager_load_plugin(neu_manager_t *manager)
{
    UT_array *plugin_infos = NULL;

    int rv = neu_persister_load_plugins(&plugin_infos);
    if (rv != 0) {
        return rv;
    }

    utarray_foreach(plugin_infos, char **, name)
    {
        rv                    = neu_manager_add_plugin(manager, *name);
        const char *ok_or_err = (0 == rv) ? "success" : "fail";
        nlog_notice("load plugin %s, lib:%s", ok_or_err, *name);
    }

    utarray_foreach(plugin_infos, char **, name) { free(*name); }
    utarray_free(plugin_infos);

    return rv;
}
