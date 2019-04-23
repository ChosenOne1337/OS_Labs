#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <errno.h>

void print_uid_and_gid() {
    uid_t uid = getuid();
    uid_t euid = geteuid();
    gid_t gid = getgid();
    gid_t egid = getegid();
    printf("Effective UID: %lu\n"
           "Real UID:      %lu\n"
           "Effective GID: %lu\n"
           "Real GID:      %lu\n\n",
           (unsigned long) euid, (unsigned long) uid,
           (unsigned long) egid, (unsigned long) gid);
}

void become_group_leader() {
    pid_t pid = getpid();
    int errCode = setpgid(pid, pid);
    if (errCode) {
        perror("become_group_leader() error while calling setpgid(..)");
    }
}

void print_process_info() {
    pid_t pid = getpid();
    pid_t pgrp = getpgrp();
    pid_t ppid = getppid();
    printf("Process ID:        %lu\n"
           "Parent process ID: %lu\n"
           "Process group ID:  %lu\n\n",
           (unsigned long) pid, (unsigned long) ppid,
           (unsigned long) pgrp);
}

int parse_value(char *valueStr, unsigned long *val) {
    char *endValPtr = NULL;
    unsigned long parsedValue = strtoul(valueStr, &endValPtr, 10);
    if (valueStr == endValPtr || *endValPtr != '\0') {
        fprintf(stderr, "parse_value(..) error: "
                        "failed to convert the string \"%s\" to value\n", valueStr);
        return 1;
    }
    if (errno == ERANGE) {
        fprintf(stderr, "parse_value(..) error: "
                        "the converted value (\"%s\") falls out of range\n", valueStr);
        return 1;
    }
    *val = parsedValue;

    return 0;
}

int set_resource_soft_limit(int resource, unsigned long softLim) {
    struct rlimit rlim;
    int errCode = getrlimit(resource, &rlim);
    if (errCode) {
        perror("set_resourse_soft_limit() error");
        return 1;
    }
    rlim.rlim_cur = softLim;
    errCode = setrlimit(resource, &rlim);
    if (errCode) {
        perror("set_resourse_soft_limit() error");
        return 1;
    }
    return 0;
}

void print_ulimit_value() {
    struct rlimit rlim;
    int errCode = getrlimit(RLIMIT_FSIZE, &rlim);
    if (errCode) {
        perror("print_ulimit_value() error while calling getrlimit(..)");
        return;
    }

    printf("File size limit is ");
    if (rlim.rlim_cur == RLIM_INFINITY) {
        printf("unlimited");
    } else {
        printf("%lu bytes", rlim.rlim_cur);
    }
    printf("\n\n");
}

void set_ulimit_value(char *valueStr) {
    unsigned long softLim = 0;
    int errCode = parse_value(valueStr, &softLim);
    if (errCode) {
        return;
    }

    set_resource_soft_limit(RLIMIT_FSIZE, softLim);
}

void print_core_file_size() {
    struct rlimit rlim;
    int errCode = getrlimit(RLIMIT_CORE, &rlim);
    if (errCode) {
        perror("print_core_file_size() error while calling getrlimit(..)");
        return;
    }

    printf("Core file size limit is ");
    if (rlim.rlim_cur == RLIM_INFINITY) {
        printf("unlimited");
    } else {
        printf("%lu bytes", rlim.rlim_cur);
    }
    printf("\n\n");
}

void set_core_file_size(char *valueStr) {
    unsigned long softLim = 0;
    int errCode = parse_value(valueStr, &softLim);
    if (errCode) {
        return;
    }

    set_resource_soft_limit(RLIMIT_CORE, softLim);
}

void print_working_directory() {
    long size = pathconf(".", _PC_PATH_MAX);
    char *cwd = getcwd(NULL, size);
    if (cwd == NULL) {
        perror("print_working_directory() error while calling getcwd(..)");
        return;
    }
    printf("Current working directory is %s\n\n", cwd);
    free(cwd);
}

void print_environ_vars() {
    extern char **environ;
    for (char **env = environ; *env != NULL; ++env) {
        printf("%s\n", *env);
    }
    printf("\n");
}

void set_environ_var(char *envVar) {
    char *equalCharPtr = strchr(envVar, '=');
    if (equalCharPtr == NULL) {
        fprintf(stderr, "Environment variable definition ought to have the following format:\n"
                        "<varname>=<varvalue>\n");
        return;
    }

    int errorCode = putenv(envVar);
    if (errorCode) {
        perror("set_environ_var(..) error while calling putenv(..)");
        return;
    }
}

int main(int argc, char *argv[]) {
    char options[] = ":ispuU:cC:dvV:";
    // disable getopt's error messages
    opterr = 0;

    int c = 0;
    while ((c = getopt(argc, argv, options)) != -1) {
        switch (c) {
        case 'i': print_uid_and_gid();          break;
        case 's': become_group_leader();        break;
        case 'p': print_process_info();         break;
        case 'u': print_ulimit_value();         break;
        case 'U': set_ulimit_value(optarg);     break;
        case 'c': print_core_file_size();       break;
        case 'C': set_core_file_size(optarg);   break;
        case 'd': print_working_directory();    break;
        case 'v': print_environ_vars();         break;
        case 'V': set_environ_var(optarg);      break;

        case ':':
            fprintf(stderr, "Missing argument for option -%c\n", optopt);
            break;
        case '?':
            fprintf(stderr, "Unknown option -%c\n", optopt);
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}
