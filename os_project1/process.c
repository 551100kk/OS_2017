#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>

#define LOG(str)	syscall(332, str)
#define TIME		(int)1e9
#define MAXLEN		100

void unit_time() {
	volatile unsigned long j;
    for(j = 0; j < 1000000UL; j++);
}

int main(int argc, char** argv) {
    
	int exc_time;
	sscanf(argv[1], "%d", &exc_time);

    int pid = getpid();

    int i;
    for (i = 0; i < exc_time; i++) unit_time();

    struct timespec end;
    
    char buffer[MAXLEN];
    clock_gettime(CLOCK_REALTIME, &end);
    sprintf(buffer, "[project1] %d %s %ld.%09ld\n", \
    	pid, argv[2], \
    	end.tv_sec, end.tv_nsec);
    
    //perror(buffer);
    LOG(buffer);

    return 0;
}
