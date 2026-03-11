#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main(void) {
    char *path = getenv("MEARIE__path");
    char *path_p = NULL;
    char *mkpath = NULL;
    char *mkpath_p = NULL;
    if (!path|| *path != '/')
        return EXIT_FAILURE;
    if (path[1] == '\0') {
        puts(path);
        return EXIT_SUCCESS;
    }
    if (!(path = strdup(path)))
        return EXIT_FAILURE;
    if (!(mkpath = malloc(strlen(path) + 1)))
        return EXIT_FAILURE;
    *mkpath = '\0';
    mkpath_p = mkpath;
    path_p = strtok(path, "/");
    if (!path_p)
        return EXIT_FAILURE;
    do {
        size_t len = strlen(path_p);
        *mkpath_p++ = '/';
        memcpy(mkpath_p, path_p, len);
        mkpath_p += len;
        *mkpath_p = '\0';
        if (mkdir(mkpath, S_IRWXU | S_IRWXG | S_IRWXO)) {
            struct stat st;
            if (errno != EEXIST || stat(mkpath, &st) || !S_ISDIR(st.st_mode))
                return EXIT_FAILURE;
        }
    }
    while ((path_p = strtok(NULL, "/")));
    puts(mkpath);
    return EXIT_SUCCESS;
}
