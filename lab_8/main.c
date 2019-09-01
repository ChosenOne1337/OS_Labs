#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>

#define OPEN_ERROR (-1)
#define CLOSE_ERROR (-1)
#define FCNTL_ERROR (-1)
#define CALLOC_ERROR (NULL)
#define SYSTEM_ERROR (-1)

#define SUCCESS_CODE (0)
#define FAILURE_CODE (-1)
#define MAKE_CMD_ERROR (NULL)

#define START_POS (0)
#define WHOLE_FILE_LEN (0)

enum Argv {
    ProgPathIndex,
    FilePathIndex,
    RequiredArgc
};

void print_usage(char *progPath) {
    fprintf(stderr, "Usage: %s <filename>\n", basename(progPath));
}

int open_file(char *filepath) {
    int fd = open(filepath, O_RDWR);
    if (fd == OPEN_ERROR) {
        perror("open_file::open(..) error");
        return FAILURE_CODE;
    }
    return SUCCESS_CODE;
}

int close_file(int fd) {
    int returnCode = close(fd);
    if (returnCode == CLOSE_ERROR) {
        perror("close_file::close(..) error");
        return FAILURE_CODE;
    }
    return SUCCESS_CODE;
}

int lock_file(int fd, short lockType) {
    struct flock lock = {
        .l_type = lockType,
        .l_whence = SEEK_SET,
        .l_start = START_POS,
        .l_len = WHOLE_FILE_LEN
    };

    int returnCode = fcntl(fd, F_SETLK, &lock);
    if (returnCode != FCNTL_ERROR) {
        return SUCCESS_CODE;
    }

    if (errno == EAGAIN || errno == EACCES) {
        fprintf(stderr, "file is locked, try again later\n");
        return FAILURE_CODE;
    }

    perror("lock_file::fcntl(..) error");
    return FAILURE_CODE;
}

int unlock_file(int fd) {
    return lock_file(fd, F_UNLCK);
}

char *make_command(char *filepath) {
    static const char *format = "nano %s";
    static const size_t prefixLength = sizeof (format) - 2;

    size_t filepathLength = strlen(filepath);
    char *command = (char*) calloc(prefixLength + filepathLength + 1, sizeof (char));
    if (command == CALLOC_ERROR) {
        perror("make_command::calloc(..) error");
        return MAKE_CMD_ERROR;
    }

    sprintf(command, format, filepath);

    return command;
}

int test_lock_type(int fd, char *filepath, short lockType) {
    char *command = make_command(filepath);
    if (command == MAKE_CMD_ERROR) {
        return FAILURE_CODE;
    }

    int returnCode = lock_file(fd, lockType);
    if (returnCode == FAILURE_CODE) {
        free(command);
        return FAILURE_CODE;
    }

    returnCode = system(command);
    if (returnCode == SYSTEM_ERROR) {
        perror("test_lock_types::system(..) error");
        unlock_file(fd);
        free(command);
        return FAILURE_CODE;
    }

    returnCode = unlock_file(fd);
    if (returnCode == FAILURE_CODE) {
        free(command);
        return FAILURE_CODE;
    }

    free(command);
    return SUCCESS_CODE;
}

int main(int argc, char *argv[]) {
    if (argc != RequiredArgc) {
        print_usage(argv[ProgPathIndex]);
        return EXIT_FAILURE;
    }

    char *filepath = argv[FilePathIndex];
    int fd = open_file(argv[FilePathIndex]);
    if (fd == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    int returnCode = test_lock_type(fd, filepath, F_RDLCK);
    if (returnCode == FAILURE_CODE) {
        close_file(fd);
        return EXIT_FAILURE;
    }

    returnCode = test_lock_type(fd, filepath, F_WRLCK);
    if (returnCode == FAILURE_CODE) {
        close_file(fd);
        return EXIT_FAILURE;
    }

    returnCode = close_file(fd);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
