#ifndef _EASEVIEW_CONFIG_H_
#define _EASEVIEW_CONFIG_H_ 

typedef char *esv_uart_config_t;

typedef struct {
    int                           n_config;
    esv_uart_config_t *configs;
} esv_uart_configs_t;

int esv_easeview_config_persister_create();
void esv_easeview_config_persister_destroy();
void esv_uart_configs_free(esv_uart_configs_t *configs);
int load_uart_config_from_db();
#endif /* ifndef _EASEVIEW_CONFIG_H_ */
