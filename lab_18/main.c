#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#define STAT_ERROR (-1)

typedef enum ArgvIndex {
    ProgPathIndex,
    FirstArgIndex,
    MinArgc = 2
} ArgvIndex;

void print_usage(char *progPath) {
    fprintf(stderr, "Usage: %s <file1> <file2> ...\n", basename(progPath));
}

void print_info(char *filePath, struct stat *statbuf);

int main(int argc, char *argv[]) {
    if (argc < MinArgc) {
        char *progPath = argv[ProgPathIndex];
        print_usage(progPath);
        return EXIT_FAILURE;
    }

    struct stat statbuf;
    for (int argvIndex = FirstArgIndex; argvIndex < argc; ++argvIndex) {
        char *filePath = argv[argvIndex];
        int returnCode = stat(filePath, &statbuf);
        if (returnCode == STAT_ERROR) {
            fprintf(stderr, "stat(\"%s\") error: %s\n", filePath, strerror(errno));
            continue;
        }
        print_info(filePath, &statbuf);
    }

    return EXIT_SUCCESS;
}

void print_file_type(struct stat *statbuf) {
    mode_t mode = statbuf->st_mode;
    if (S_ISREG(mode)) {
        putchar('-');
    } else if (S_ISDIR(mode)) {
        putchar('d');
    } else {
        putchar('?');
    }
}

void print_file_permissions(struct stat *statbuf) {
    static const char flags[] = {'r', 'w', 'x'};
    static const int flagsNumber = sizeof (flags) / sizeof (char);

    static const mode_t permissionBits[] = {
        S_IRUSR, S_IWUSR, S_IXUSR,
        S_IRGRP, S_IWGRP, S_IXGRP,
        S_IROTH, S_IWOTH, S_IXOTH
    };
    static const int bitsNumber = sizeof (permissionBits) / sizeof (mode_t);

    mode_t mode = statbuf->st_mode;
    char permissions[] = "---------";
    for (int bitIndex = 0; bitIndex < bitsNumber; ++bitIndex) {
        if (mode & permissionBits[bitIndex]) {
            permissions[bitIndex] = flags[bitIndex % flagsNumber];
        }
    }

    printf("%s ", permissions);
}

void print_hard_links_number(struct stat *statbuf) {
    static const int fieldWidth = 3;

    nlink_t hardLinksNum = statbuf->st_nlink;
    printf("%*lu ", fieldWidth, hardLinksNum);
}

void print_owners(struct stat *statbuf) {
    static const int fieldWidth = 10;

    uid_t ownerUID = statbuf->st_uid;
    struct passwd *pw = getpwuid(ownerUID);
    if (pw == NULL) {
        printf("%*d ", fieldWidth, ownerUID);
    } else {
        printf("%*s ", fieldWidth, pw->pw_name);
    }

    gid_t ownerGID = statbuf->st_gid;
    struct group *grp = getgrgid(ownerGID);
    if (grp == NULL) {
        printf("%*d ", fieldWidth, ownerGID);
    } else {
        printf("%*s ", fieldWidth, grp->gr_name);
    }

}

void print_file_size(struct stat *statbuf) {
    static const int fieldWidth = 10;

    mode_t mode = statbuf->st_mode;
    if (S_ISREG(mode)) {
        off_t fileSize = statbuf->st_size;
        printf("%*ld ", fieldWidth, fileSize);
    } else {
        printf("%*c ", fieldWidth, ' ');
    }
}

void print_last_modified_date(struct stat *statbuf) {
    static const char stringEndChar = '\0';

    char *lastModifiedDate = ctime(&statbuf->st_mtime);
    size_t dateLength = strlen(lastModifiedDate);
    lastModifiedDate[dateLength - 1] = stringEndChar;

    printf("%s ", lastModifiedDate);
}

void print_info(char *filePath, struct stat *statbuf) {
    print_file_type(statbuf);

    print_file_permissions(statbuf);

    print_hard_links_number(statbuf);

    print_owners(statbuf);

    print_file_size(statbuf);

    print_last_modified_date(statbuf);

    char *fileName = basename(filePath);

    printf("%s\n", fileName);
}
