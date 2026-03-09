#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char *restrict image_urls = getenv("MEARIE__image_urls");
    if (image_urls != NULL)
        printf("{\"@mearie\":{\"action\":\"image.attach\",\"data\":%s}}\n", image_urls);
    return EXIT_SUCCESS;
}
