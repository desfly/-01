#include "service.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define MB_SERVICES_DIR "/etc/minibox/services.d"
#define MB_MAX_SERVICES 8

static int has_suffix(const char *s, const char *suffix) {
    size_t a = strlen(s), b = strlen(suffix);
    return a >= b && strcmp(s + a - b, suffix) == 0;
}

int main(int argc, char **argv) {
    const char *dir = argc > 1 ? argv[1] : MB_SERVICES_DIR;
    mb_service_t services[MB_MAX_SERVICES];
    DIR *d = opendir(dir);
    struct dirent *de;
    unsigned count = 0;

    if (!d) {
        fprintf(stderr, "minibox-discoveryd: cannot open %s: %s\n", dir, strerror(errno));
        return 1;
    }

    while ((de = readdir(d)) != NULL) {
        char path[512];
        int rc;
        if (de->d_name[0] == '.' || !has_suffix(de->d_name, ".service")) continue;
        if (count == MB_MAX_SERVICES) {
            fprintf(stderr, "minibox-discoveryd: too many services (max %u)\n", MB_MAX_SERVICES);
            closedir(d);
            return 2;
        }
        if (snprintf(path, sizeof(path), "%s/%s", dir, de->d_name) >= (int)sizeof(path)) {
            fprintf(stderr, "minibox-discoveryd: service path too long\n");
            closedir(d);
            return 2;
        }
        rc = mb_service_load(path, &services[count]);
        if (rc) {
            fprintf(stderr, "minibox-discoveryd: invalid %s: %d\n", path, rc);
            closedir(d);
            return 2;
        }
        printf("service name=%s type=%s port=%u path=%s txt=%s\n",
               services[count].name, services[count].type,
               services[count].port, services[count].path,
               services[count].txt);
        count++;
    }
    closedir(d);
    if (!count) {
        fprintf(stderr, "minibox-discoveryd: no service contracts in %s\n", dir);
        return 3;
    }
    printf("minibox-discoveryd: loaded %u service(s)\n", count);
    return 0;
}
