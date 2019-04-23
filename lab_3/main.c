#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

void print_uids() {
    uid_t uid = getuid();
    uid_t euid = geteuid();
    printf("Effective user ID: %lu\n"
           "Real user ID:      %lu\n\n",
           (unsigned long) uid, (unsigned long) euid);
}

void open_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open %s: %s\n", filename, strerror(errno));
        return;
    }
    printf("Successfully opened %s\n\n", filename);
    fclose(file);
}

void equate_euid_to_uid() {
    uid_t uid = getuid();
    int errCode = setuid(uid);
    if (errCode) {
        perror("equate_euid_to_uid() error while calling setuid(...)");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input file>\n", basename(argv[0]));
        return EXIT_FAILURE;
    }
    char *filename = argv[1];

    print_uids();
    open_file(filename);

    equate_euid_to_uid();

    print_uids();
    open_file(filename);

    return EXIT_SUCCESS;
}
