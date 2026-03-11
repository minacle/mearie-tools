#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void) {
    char *path = getenv("MEARIE__path");
    if (!path || *path != '/')
        return EXIT_FAILURE;
    if (open(path, O_CREAT | O_EXCL, S_IRWXU | S_IRWXG | S_IRWXO) < 0)
        return EXIT_FAILURE;
    puts(path);
    return EXIT_SUCCESS;
}
