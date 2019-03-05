#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TIMEFORMAT_STR_LEN 64

void set_timezone(char *timezone);
char* get_formatted_localtime(char *fmtStr, size_t maxSize, time_t timestamp);

int main(void) {
    // unix time (UTC)
    time_t currTime = time(NULL);
    if (currTime == -1) {
        fprintf(stderr, "Failed to obtain the current time\n");
        exit(EXIT_FAILURE);
    }
    printf("Local time: %s\n", ctime(&currTime));

    // set California time zone
    set_timezone("America/Los_Angeles");
    // get local time in California
    char timeFmt[TIMEFORMAT_STR_LEN] = {0};
    get_formatted_localtime(timeFmt, sizeof(timeFmt), currTime);
    printf("California time: %s\n", timeFmt);

    return EXIT_SUCCESS;
}


void set_timezone(char *timezone) {
#define STRING_BUF_LEN 64

    static char buf[STRING_BUF_LEN] = {0};
    static const char errorMsg[] = "Failed to set a timezone";
    // build TZ environment variable definition
    long charsWritten = snprintf(buf, sizeof(buf), "TZ=%s", timezone);
    if (charsWritten >= STRING_BUF_LEN) {
        fprintf(stderr, "%s: buffer overflow\n", errorMsg);
        return;
    }
    if (charsWritten < 0) {
        perror(errorMsg);
        return;
    }
    // set TZ environment variable
    int errCode = putenv(buf);
    if (errCode != 0) {
        perror(errorMsg);
        exit(EXIT_FAILURE);
    }

#undef STRING_BUF_LEN
}

char* get_formatted_localtime(char *fmtStr, size_t maxSize, time_t timestamp) {
    struct tm *timePtr = localtime(&timestamp);

    // get formatted time string
    size_t fmtStrLen = strftime(fmtStr, maxSize, "%c", timePtr);
    if (fmtStrLen == 0) {
        fprintf(stderr, "Error: time formatted string overflow\n");
        exit(EXIT_FAILURE);
    }

    return fmtStr;
}
