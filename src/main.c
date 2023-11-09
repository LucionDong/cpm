#include <signal.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include "core/manager.h"
#include "utils/log.h"
#include "utils/time.h"

#include "argparse.h"
#include "daemon.h"

static bool           exit_flag         = false;
static neu_manager_t *g_manager         = NULL;
zlog_category_t *     neuron            = NULL;
bool                  disable_jwt       = false;
/* int                   default_log_level = ZLOG_LEVEL_NOTICE; */
int                   default_log_level = ZLOG_LEVEL_DEBUG;
char                  host_port[24]     = { 0 };

int64_t global_timestamp = 0;

static void sig_handler(int sig)
{
    nlog_warn("recv sig: %d", sig);

    if (sig == SIGINT || sig == SIGTERM) {
        /* neu_manager_destroy(g_manager); */
        neu_persister_destroy();
		esv_persister_destroy();
        zlog_fini();
    }
    exit_flag = true;
    exit(-1);
}

static int neuron_run(const neu_cli_args_t *args)
{
    struct rlimit rl = { 0 };
    int           rv = 0;

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGABRT, sig_handler);
    signal(SIGSEGV, sig_handler);

    // try to enable core dump
    rl.rlim_cur = rl.rlim_max = RLIM_INFINITY;
    if (setrlimit(RLIMIT_CORE, &rl) < 0) {
        nlog_warn("neuron process failed enable core dump, ignore");
    }

    rv = neu_persister_create(args->config_dir);
    assert(rv == 0);

    rv = esv_persister_create(args->config_dir);
    assert(rv == 0);

    zlog_notice(neuron, "neuron start, daemon: %d, version: %s (%s %s)",
                args->daemonized, "1.0",
                "1.0", "1.0");
    g_manager = neu_manager_create();
    if (g_manager == NULL) {
        nlog_fatal("neuron process failed to create neuron manager, exit!");
        return -1;
    }

    while (!exit_flag) {
        sleep(1);
    }

    return 0;
}

int main(int argc, char *argv[])
{
	int            rv     = 0;
    int            status = 0;
    int            signum = 0;
    pid_t          pid    = 0;
    neu_cli_args_t args   = { 0 };

    global_timestamp = neu_time_ms();
    neu_cli_args_init(&args, argc, argv);

    disable_jwt = args.disable_auth;
    snprintf(host_port, sizeof(host_port), "http://%s:%d", args.ip, args.port);

	if (args.daemonized) {
        // become a daemon, this should be before calling `init`
        daemonize();
    }

	/* printf("to init zlog: %s\n", args.log_init_file); */
    zlog_init(args.log_init_file);
    neuron = zlog_get_category("neuron");

	/* while (1) { */
	/* 	usleep(1000); */
	/* 	zlog_info(neuron, "zlog_info") */
	/* } */

	zlog_level_switch(neuron, default_log_level);

	rv = neuron_run(&args);

main_end:
    neu_cli_args_fini(&args);
    zlog_fini();
    return rv;
}
