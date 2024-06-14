#ifndef _MQTT_H_
#define _MQTT_H_
int topic_matches_wildcard(const char *topic, const char *wildcard);
int get_pk_dn_from_thingsub_topic(const char *topic, const int start_token_index, char **pk, char **dn);
#endif /* ifndef _MQTT_H_ */
