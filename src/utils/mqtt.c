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
