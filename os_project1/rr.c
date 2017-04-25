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
	int start_time,
		exec_time,
		pid;
   char name[20];
} task;

typedef struct t_node{
	task work;
	struct t_node *next;
} t_node;

t_node* T_Queue_Head, * T_Queue_Tail;
struct sched_param high, s_high, low;

int queue_empty(){
	return ( T_Queue_Head == ( t_node *)NULL );
}

void queue_push( t_node* tmp ){
	
	if( T_Queue_Tail != ( t_node *)NULL ) T_Queue_Tail->next = tmp;
	T_Queue_Tail = tmp;

	if( queue_empty() ){
		T_Queue_Head = tmp;
		sched_setscheduler( (T_Queue_Head->work).pid, SCHED_RR, &high);
		return;
	}
	else if( T_Queue_Head->next != ( t_node *)NULL ) 
			sched_setscheduler( (T_Queue_Head->next->work).pid, SCHED_RR, &s_high);
	
	sched_setscheduler( (T_Queue_Tail->work).pid, SCHED_RR, &low);
	return;
}

void queue_pop_push(){

	if( (T_Queue_Head->work).exec_time == 0 ){
		t_node *tmp = T_Queue_Head;
		T_Queue_Head = T_Queue_Head->next;
		if( queue_empty() ) T_Queue_Tail = ( t_node *) NULL;
		free(tmp);
	}
	else{
		T_Queue_Tail->next = T_Queue_Head;
		T_Queue_Tail = T_Queue_Head;
		if( T_Queue_Head->next != ( t_node *)NULL ) T_Queue_Head = T_Queue_Head->next;
		T_Queue_Tail->next = ( t_node *)NULL;
	}

	if( ! queue_empty() ){
		sched_setscheduler( (T_Queue_Tail->work).pid, SCHED_RR, &low);
		sched_setscheduler( (T_Queue_Head->work).pid, SCHED_RR, &high);
		if( T_Queue_Head->next != ( t_node *)NULL )
			sched_setscheduler( (T_Queue_Head->next->work).pid, SCHED_RR, &s_high);
	}
	return;
}

int cmp(const void* a, const void *b){
    task c = *(task *)a;
    task d = *(task *)b;
    return ( c.start_time > d.start_time );
}

void unit_time(){
    volatile unsigned long x;
    for( x = 0 ; x < 1000000UL ; x++ );
    return;
}

int main(){

	high.sched_priority =  sched_get_priority_max(SCHED_RR);
	s_high.sched_priority =  sched_get_priority_max(SCHED_RR) - 1;
	low.sched_priority =  sched_get_priority_min(SCHED_RR);

	int number, i;
	cpu_set_t mask, s_mask;
	CPU_ZERO( &mask);
	CPU_ZERO( &s_mask);
	CPU_SET( 0, &mask);
	CPU_SET( 1, &s_mask);
	sched_setaffinity( getpid(), sizeof(cpu_set_t), &s_mask);
	scanf("%d", &number);
	task* task_list = malloc( sizeof(task) * number );
	T_Queue_Head = (t_node *)( t_node *)NULL;
	T_Queue_Tail = (t_node *)( t_node *)NULL;

	for( i = 0 ; i < number ; i++ ){
		scanf("%s %d %d", task_list[i].name, &(task_list[i].start_time), &(task_list[i].exec_time));
		task_list[i].pid = 0;
	}

	qsort( task_list, number, sizeof(task), cmp);
	int now_time = 0, finish_process = 0, index = 0, check = 0, already_index = 0, running_id = -1, time_slice = 0;

	while( finish_process != number ){

		while( already_index != number && task_list[already_index].start_time == now_time ){
			char fexec_time[20], tmp[100];
            struct timespec child_start;
            sprintf( fexec_time, "%d", task_list[already_index].exec_time);
            clock_gettime(CLOCK_REALTIME, &child_start);
            sprintf( tmp, "%ld.%09ld", child_start.tv_sec, child_start.tv_nsec);                
            task_list[already_index].pid = fork();
                
            if( task_list[already_index].pid == 0 ){                   
                execlp("./process", "./process", fexec_time, tmp, (char *)NULL );
                perror("exec error");
            }

            sched_setaffinity( task_list[already_index].pid, sizeof(cpu_set_t), &mask);

            t_node *node = malloc( sizeof( t_node));
            (node->work).exec_time = task_list[already_index].exec_time;
            (node->work).pid = task_list[already_index].pid;
            node->next = ( t_node *)NULL;
            sched_setscheduler( task_list[already_index].pid, SCHED_RR, &low);
            queue_push(node);
            already_index++;
		}

		now_time++;
		unit_time();
		
		if( !queue_empty() ){
			time_slice++;
			T_Queue_Head->work.exec_time--;
			if( T_Queue_Head -> work.exec_time == 0 ) finish_process++;
			if( time_slice==500 || T_Queue_Head->work.exec_time == 0 ){
				time_slice = 0;
				queue_pop_push();
			}
		}
	}

	for( i = 0 ; i < number ; i ++ ){
		wait(0);
		printf("%s %d\n", task_list[i].name, task_list[i].pid);
	}

	free( task_list);
	return 0;
}


		/*if( running_id == -1 ){
			while( already_number != number && task_list[already_number].start_time == now_time ){
				char fexec_time[20], tmp[100];
                struct timespec child_start;
                sprintf( fexec_time, "%d", task_list[already_index].exec_time);
                clock_gettime(CLOCK_REALTIME, &child_start);
                sprintf( tmp, "%ld.%ld", child_start.tv_sec, child_start.tv_nsec);                
                task_list[already_index].pid = fork();
                
                if( task_list[already_index].pid == 0 ){                   
                    execlp("./process", "./process", fexec_time, tmp, (char *)( t_node *)NULL );
                    perror("exec error");
                }
                sched_setaffinity( task_list[already_index].pid, sizeof(cpu_set_t), &mask);
			}
		}*/
		/*check = 0;
		for( index = 0 ; index < number ; index++ ){
			
			if( task_list[index].start_time <= now_time && task_list[index].pid == 0 ){
				task_list[index].pid = fork();
				if( task_list[index].pid == 0 ){
					char fexec_time[20];
					sprintf( fexec_time, "%d", task_list[index].exec_time);
					execlp("./process", "./process", fexec_time, (char *)( t_node *)NULL );
					perror("exec error");
				}
				else sched_setaffinity( task_list[index].pid, sizeof(cpu_set_t), &mask);
			}

			if( task_list[index].pid != 0  && task_list[index].exec_time != 0){
				int time_slice = 0;
				check = 1;
				sched_setscheduler( task_list[index].pid, SCHED_RR, &high);
				while( task_list[index].exec_time != 0 && time_slice < 500){
					unit_time();
					now_time++;
					task_list[index].exec_time--;
					time_slice++;
				}
				sched_setscheduler( task_list[index].pid, SCHED_RR, &low);

				if( task_list[index].exec_time == 0 && task_list[index].start_time >= 0 ){
					finish_process++;
					task_list[index].start_time = -1;
				}
			}

		}
		if(check==0){
			unit_time();

			now_time++;
		}*/
