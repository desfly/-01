#include "service.h"
#include "mdns.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MB_SERVICES_DIR "/etc/minibox/services.d"
#define MB_MAX_SERVICES 8

static int has_suffix(const char *s, const char *suffix) {
    size_t a = strlen(s), b = strlen(suffix);
    return a >= b && strcmp(s + a - b, suffix) == 0;
}

int main(int argc, char **argv) {
    const char *dir = argc > 1 ? argv[1] : MB_SERVICES_DIR;
    mb_service_t services[MB_MAX_SERVICES];
    char hostname[64] = "minibox";
    DIR *d = opendir(dir);
    struct dirent *de;
    unsigned count = 0;
    int rc;

    if (!d) {
        fprintf(stderr, "minibox-discoveryd: cannot open %s: %s\n", dir, strerror(errno));
        return 1;
    }
    while ((de = readdir(d)) != NULL) {
        char path[512];
        if (de->d_name[0] == '.' || !has_suffix(de->d_name, ".service")) continue;
        if (count == MB_MAX_SERVICES) { closedir(d); return 2; }
        if (snprintf(path, sizeof(path), "%s/%s", dir, de->d_name) >= (int)sizeof(path)) { closedir(d); return 2; }
        rc = mb_service_load(path, &services[count]);
        if (rc) { fprintf(stderr, "minibox-discoveryd: invalid %s: %d\n", path, rc); closedir(d); return 2; }
        count++;
    }
    closedir(d);
    if (!count) return 3;
    if (gethostname(hostname, sizeof(hostname) - 1) != 0 || !hostname[0]) strcpy(hostname, "minibox");
    hostname[sizeof(hostname) - 1] = 0;
    rc = mb_mdns_publish_once(services, count, hostname);
    if (rc) {
        fprintf(stderr, "minibox-discoveryd: mDNS publish failed: %d\n", rc);
        return 4;
    }
    printf("minibox-discoveryd: published %u service(s) as %s.local\n", count, hostname);
    return 0;
}
