#define _GNU_SOURCE
#define _USE_GNU
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>


typedef struct task{
    char name[20];
    int start_time,
        exec_time,
        pid;
} task;

int cmp(const void* a, const void *b){
    task c = *(task *)a;
    task d = *(task *)b;
    if( c.start_time != d.start_time ) return ( c.start_time > d.start_time );
    return ( c.exec_time > d.exec_time );
}

void unit_time(){
    volatile unsigned long x;
    for( x = 0 ; x < 1000000UL ; x++ );
    return;
}

int main(){

    struct sched_param high, s_high, t_high, low;
    high.sched_priority =  sched_get_priority_max(SCHED_RR);
    s_high.sched_priority =  sched_get_priority_max(SCHED_RR) - 1;
    t_high.sched_priority =  sched_get_priority_max(SCHED_RR) - 2;
    low.sched_priority =  sched_get_priority_min(SCHED_RR);

    int number, i;
    cpu_set_t mask, s_mask;
    CPU_ZERO( &mask);
    CPU_ZERO( &s_mask);
    CPU_SET( 1, &mask);
    CPU_SET( 2, &s_mask);
    sched_setaffinity( getpid(), sizeof(cpu_set_t), &s_mask);
    scanf("%d", &number);
    task* task_list = malloc( sizeof(task) * number );

    for( i = 0 ; i < number ; i++ ){
        scanf("%s %d %d", task_list[i].name, &(task_list[i].start_time), &(task_list[i].exec_time));
        task_list[i].pid = 0;
    }

    qsort( task_list, number, sizeof(task), cmp);
    int now_time = 0, finish_process = 0, index = 0, already_index = 0, running_id = -1, second_id = -1;

    while(finish_process != number){

        while( already_index != number && now_time == task_list[already_index].start_time ){
            if( task_list[already_index].pid == 0 ){

                char fexec_time[20], tmp[100];
                struct timespec child_start;
                sprintf( fexec_time, "%d", task_list[already_index].exec_time);
                clock_gettime(CLOCK_REALTIME, &child_start);
                sprintf( tmp, "%ld.%ld", child_start.tv_sec, child_start.tv_nsec);                
                task_list[already_index].pid = fork();
                
                if( task_list[already_index].pid == 0 ){                   
                    execlp("./process", "./process", fexec_time, tmp, (char *)NULL );
                    perror("exec error");
                }
                sched_setaffinity( task_list[already_index].pid, sizeof(cpu_set_t), &mask);

                if( running_id == -1 ){
                    sched_setscheduler( task_list[already_index].pid, SCHED_RR, &s_high );
                    running_id = already_index;
                }
                else if( task_list[running_id].exec_time > task_list[already_index].exec_time ){
                    sched_setscheduler( task_list[already_index].pid, SCHED_RR, &high );
                    sched_setscheduler( task_list[running_id].pid, SCHED_RR, &t_high );
                    sched_setscheduler( task_list[already_index].pid, SCHED_RR, &s_high );
                    if( second_id != -1 ) sched_setscheduler( task_list[second_id].pid, SCHED_RR, &low );                    
                    second_id = running_id;
                    running_id = already_index;
                }
                else if ( second_id == -1 ){
                    second_id = already_index;
                    sched_setscheduler( task_list[second_id].pid, SCHED_RR, &t_high );
                }
                else if( task_list[second_id].exec_time > task_list[already_index].exec_time ){
                    sched_setscheduler( task_list[second_id].pid, SCHED_RR, &low );
                    sched_setscheduler( task_list[already_index].pid, SCHED_RR, &t_high );
                    second_id = already_index;
                }
                else sched_setscheduler( task_list[already_index].pid, SCHED_RR, &low );                
            }
            already_index++;
        }

        if( running_id != -1 ){
            unit_time();
            now_time++;
            task_list[running_id].exec_time--;
            if( task_list[running_id].exec_time == 0){

                sched_setscheduler( task_list[running_id].pid, SCHED_RR, &low );
                wait(0);
                task_list[running_id].start_time = -1;
                running_id = second_id;
                finish_process++;

                if( running_id != -1 ){
                    sched_setscheduler( task_list[running_id].pid, SCHED_RR, &s_high );
                    int min_id = -1;
                    for( index = 0 ; index < already_index ; index++){
                        if( task_list[index].start_time != -1 && index != running_id ){
                            if( min_id == -1 ) min_id = index;
                            else if ( task_list[min_id].exec_time > task_list[index].exec_time ) min_id = index;
                        }
                    }
                    second_id = min_id;
                    if( second_id != -1 ) sched_setscheduler( task_list[second_id].pid, SCHED_RR, &t_high );
                }
            }
        }
        else{
            unit_time();
            now_time++;
        }
    }

    for( i = 0 ; i < number ; i ++ ) printf("%s %d\n", task_list[i].name, task_list[i].pid);
    free( task_list);

    return 0;
}
