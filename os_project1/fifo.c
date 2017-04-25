#define _GNU_SOURCE
#define _USE_GNU
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>

#define max_numP    10000
#define PRONAME     100
#define BUFFER      100

typedef struct process {
    char name[PRONAME];
    int readyT;
    int execT;
    int ID;
} Process;

void unit_time() {
    volatile unsigned long j;
    for (j = 0; j < 1000000UL; j++);
}

int compare_Process(const void *a, const void *b) {
    Process *P1 = (Process *)a;
    Process *P2 = (Process *)b;
    if (P1->readyT < P2->readyT) return -1;
    if (P1->readyT > P2->readyT) return 1;
    if (P1->ID < P2->ID) return -1;
    if (P1->ID > P2->ID) return 1;
    return 0;
}

Process pro[max_numP];

int main () {
    
    int process_num, j;
    scanf("%d", &process_num);
    for (j = 0; j < process_num; j++){
        scanf("%s %d %d", pro[j].name, &(pro[j].readyT), &(pro[j].execT));
        pro[j].ID = j;
    }

    struct sched_param param;
    
    cpu_set_t mask, s_mask;
    CPU_ZERO( &mask);
    CPU_ZERO( &s_mask);
    CPU_SET( 1, &mask);
    CPU_SET( 0, &s_mask);
    
    qsort( pro, process_num, sizeof(Process), compare_Process);

    if ( sched_setaffinity( getpid(), sizeof(cpu_set_t), &s_mask) < 0)
        perror("CPU error");
    
    int np = 0, time = 0, status;

    while (np != process_num){

        if (pro[np].readyT > time){
            time++;
            unit_time(); 
            continue;
        }
        
        struct timespec start;
        clock_gettime(CLOCK_REALTIME, &start);
        char buffer[BUFFER];
        sprintf(buffer, "%ld.%09ld", start.tv_sec, start.tv_nsec);

        int pid = fork();

        if (pid < 0)  perror("error");
        
        if (pid == 0){
            if ( sched_setaffinity( getpid(), sizeof(cpu_set_t), &mask) < 0)
                perror("CPU error");
            
            char exec_time[BUFFER];
            sprintf( exec_time, "%d", pro[np].execT);
            execlp("./process", "process", exec_time, buffer, (char *) NULL);
        }

        param.sched_priority=99-np;
        if ( sched_setscheduler( pid, SCHED_FIFO, &param) < 0)
            perror("SET_SCHED");
        
        printf("%s %d\n", pro[np].name, pid); 

        np++;
    }

    for(j = 0; j < process_num; j++)
        wait(&status);
    return 0;
}
