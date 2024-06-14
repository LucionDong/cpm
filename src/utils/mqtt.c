#include <stdlib.h>
#include <string.h>

int topic_matches_wildcard(const char *topic, const char *wildcard) {
    // Handle "#" wildcard at the end
    if (wildcard[0] == '#' && wildcard[1] == '\0') {
        return 1; // "#" matches everything
    }

    // Handle "+" and "#" wildcards
    while (*topic && *wildcard) {
        if (*wildcard == '+') {
            // Skip to the next level in the topic
            while (*topic && *topic != '/') {
                topic++;
            }
            wildcard++;
        } else if (*wildcard == '#') {
            return 1; // "#" matches anything remaining
        } else if (*topic != *wildcard) {
            return 0; // Characters don't match
        } else {
            topic++;
            wildcard++;
        }
    }

    // Make sure both topic and wildcard are at the end
    return (*topic == '\0' && (*wildcard == '\0' || *wildcard == '#'));
}

int get_pk_dn_from_thingsub_topic(const char *topic, const int start_token_index, char **pk, char **dn) {

	char *topic2 = strdup(topic);
	char *token;
	int token_count = 0;
	token = strtok(topic2, "/");
	while (token != NULL) {
		if (token_count == start_token_index) {
			*pk = strdup(token);
		} else if (token_count == (start_token_index + 1)) {
			*dn = strdup(token);
		}
		if (token_count == (start_token_index + 1)) {
			break;
		}
		token_count++;
		token = strtok(NULL, "/");		
	}

	free(topic2);
	return 0;
}
