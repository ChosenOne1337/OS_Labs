#include <stdio.h>
#include "readline.h"

int main() {
	// char *line = NULL;
	// size_t bufsiz = 0;
	// ssize_t bytesRead;
	// while ((bytesRead = read_line(&line, &bufsiz, stdin)) != -1) {
	// 	printf("%s", line);
	// }
	char line[256] = {0};
	while (fgets(line, 256, stdin) != NULL) {
		printf("%s", line);
	}
}