#include "utils/log.h"

#include "adapter/storage.h"
#include "storage.h"

// easeview
/* int esv_manager_load_node(neu_manager_t *manager) */
/* { */
/*     UT_array *node_infos = NULL; */
/*     int       rv         = 0; */

/*     rv = esv_persister_load_nodes(&node_infos); */
/*     if (0 != rv) { */
/*         nlog_error("failed to load adapter infos"); */
/*         return -1; */
/*     } */

/*     utarray_foreach(node_infos, esv_persist_node_info_t *, node_info) */
/*     { */
/*         rv                    = neu_manager_add_node(manager, node_info->node_name, */
/*                                   node_info->plugin_name, node_info->state, */
/*                                   true); */
/*         const char *ok_or_err = (0 == rv) ? "success" : "fail"; */
/*         nlog_notice("load adapter %s type:%d, name:%s plugin:%s state:%d", */
/*                     ok_or_err, node_info->node_type, node_info->node_name, */
/*                     node_info->plugin_name, node_info->state); */
/*     } */

/*     utarray_free(node_infos); */
/*     return rv; */
/* } */

int esv_manager_load_232_node(neu_manager_t *manager)
{
    UT_array *node_infos = NULL;
    int       rv         = 0;

    rv = esv_persister_load_232_nodes(&node_infos);
    if (0 != rv) {
        nlog_error("failed to load adapter infos");
        return -1;
    }

    utarray_foreach(node_infos, esv_persist_node_info_t *, node_info)
    {
        rv                    = neu_manager_add_node(manager, node_info->node_name,
                                  node_info->plugin_name, node_info->state,
                                  true);
        const char *ok_or_err = (0 == rv) ? "success" : "fail";
        nlog_notice("load adapter %s type:%d, name:%s plugin:%s state:%d",
                    ok_or_err, node_info->node_type, node_info->node_name,
                    node_info->plugin_name, node_info->state);
    }

    utarray_free(node_infos);
    return rv;
}

int esv_manager_load_plugin_from_db(neu_manager_t *manager)
{
    UT_array *plugin_infos = NULL;

    /* int rv = neu_persister_load_plugins(&plugin_infos); */
    int rv = esv_persister_load_plugins_from_db(&plugin_infos);
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
// easeview end
void manager_storage_add_node(neu_manager_t *manager, const char *node)
{
    int                     rv        = 0;
    neu_persist_node_info_t node_info = {};

    rv = neu_manager_get_node_info(manager, node, &node_info);
    if (0 != rv) {
        nlog_error("unable to get adapter:%s info", node);
        return;
    }

    rv = neu_persister_store_node(&node_info);
    if (0 != rv) {
        nlog_error("failed to store adapter info");
    }

    neu_persist_node_info_fini(&node_info);
}

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

int manager_load_node(neu_manager_t *manager)
{
    UT_array *node_infos = NULL;
    int       rv         = 0;

    rv = neu_persister_load_nodes(&node_infos);
    if (0 != rv) {
        nlog_error("failed to load adapter infos");
        return -1;
    }

    utarray_foreach(node_infos, neu_persist_node_info_t *, node_info)
    {
        rv                    = neu_manager_add_node(manager, node_info->name,
                                  node_info->plugin_name, node_info->state,
                                  true);
        const char *ok_or_err = (0 == rv) ? "success" : "fail";
        nlog_notice("load adapter %s type:%d, name:%s plugin:%s state:%d",
                    ok_or_err, node_info->type, node_info->name,
                    node_info->plugin_name, node_info->state);
    }

    utarray_free(node_infos);
    return rv;
}
