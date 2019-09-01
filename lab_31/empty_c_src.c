#include <stdio.h>
#include <stdlib.h>
#include "readline.h"

int main() {
	char *str = calloc(10, 1);
	ssize_t bytesRead;
	size_t bufsiz;
	while ((bytesRead = read_line(&str, &bufsiz, stdin)) != -1) {
		printf("read %zd\n", bytesRead);
		printf("%s\n", str);
	}

	printf("%d %d\n", feof(stdin), ferror(stdin));
}