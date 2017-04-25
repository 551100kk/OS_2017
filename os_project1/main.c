#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define MAXLEN	10
#define SCHCNT	4

char schedule[SCHCNT][MAXLEN] = {"FIFO", "RR", "SJF", "PSJF"};
char path[SCHCNT][MAXLEN] = {"./fifo", "./rr", "./sjf", "./psjf"};

int main() {
	
	char type[MAXLEN];

	int idx = 0;
	while (1) {
		read(0, &type[idx], 1);
		if (type[idx] > 'Z' || type[idx] < 'A') break;
	    idx ++;
	}
	type[idx] = 0;
	int i;
	for (i = 0; i < SCHCNT; i++) {
		if (strcmp(type, schedule[i]) != 0) continue;
		
		if (execl(path[i], &path[i][2], (char *) NULL) < 0){
			perror("Exec Error");
		}
	}

	return 0;
}
