#define TMP_ALLOC_IMPL
#include "../tmp_alloc.h"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    void *ptr = tmp_alloc(1024);
    if (ptr == NULL) {
        return 1;
    }

    tmp_free(ptr);
    if (ptr != NULL) {
        return 1;
    }

    return 0;
}
