#define _GNU_SOURCE
#define _USE_GNU
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#define MAXINT      2147483647
#define max_numP    100
#define BUFFER      100

typedef struct process {
    char name[12];
    int readyT;
    int execT;
    int ID;
} Process;

int p_read[max_numP];
Process pro[max_numP];

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

int main (){
    
    int process_num, j;

    scanf("%d",&process_num);
    for (j = 0; j < process_num; j++){
        scanf("%s %d %d", pro[j].name, &(pro[j].readyT), &(pro[j].execT));
        pro[j].ID = j;
    }

    struct sched_param param;

    qsort(pro, process_num, sizeof(Process), compare_Process);

    cpu_set_t mask, s_mask;
    CPU_ZERO( &mask);
    CPU_ZERO( &s_mask);
    CPU_SET( 1, &mask);
    CPU_SET( 2, &s_mask);

    sched_setaffinity( getpid(), sizeof(cpu_set_t), &s_mask);

    int time = 0, np = 0, status, mintime, pid, select;
    int sched_queue[process_num];
    while (np != process_num){
        if (pro[np].readyT > time){
            time++;
            unit_time();
            continue;
        }
        select = -1;
        mintime = MAXINT;
        for (j = 0; j < process_num; j++){
            if (pro[j].readyT > time) break;
            if (pro[j].execT < mintime && p_read[j] == 0) {
                select = j;
                mintime = pro[j].execT;
            }
        }
        if (select != -1){
            p_read[select] = 1;
            time+=pro[select].execT;
            sched_queue[select]=np;
            np++;
        }   
    }
    np=0;
    time=0;
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

        param.sched_priority=99-sched_queue[np];
        if ( sched_setscheduler( pid, SCHED_FIFO, &param) < 0)
            perror("SET_SCHED");
        
        printf("%s %d\n", pro[np].name, pid); 

        np++;
    }
    for(j=0;j<process_num;j++){
        wait(&status);
    }
    return 0;
}