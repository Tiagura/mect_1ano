/* ************************************************************
* Xenomai - creates a periodic task
*	
* Paulo Pedreiras
* 	Out/2020: Upgraded from Xenomai V2.5 to V3.1    
* 
************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

#include <sys/mman.h> // For mlockall

// Xenomai API (former Native API)
#include <alchemy/task.h>
#include <alchemy/timer.h>
#include <alchemy/queue.h>

#include "circularBuffer.h"

#define MS_2_NS(ms)(ms*1000*1000) /* Convert ms to ns */

/* *****************************************************
 * Define task structure for setting input arguments
 * *****************************************************/
 struct taskArgsStruct {
	 RTIME taskPeriod_ns;
	 int some_other_arg;
 }; 

/* *******************
 * Task attributes 
 * *******************/ 
#define TASK_MODE 0  	// No flags
#define TASK_STKSZ 0 	// Default stack size

#define TASK_SENSOR_PRIO 90	// RT priority [0..99]
#define ACK_PERIOD_MS MS_2_NS(1000)

#define TASK_PROCESSING_PRIO 70 	// RT priority [0..99]

#define TASK_STORAGE_PRIO 50 	// RT priority [0..99]

RT_TASK task_sensor_desc; // Task decriptor
RT_TASK task_processing_desc; // Task decriptor
RT_TASK task_storage_desc; // Task decriptor

RT_QUEUE queue_s_to_p; // Queue SENSOR TO PROCESSING descriptor
RT_QUEUE queue_p_to_s; // Queue PROCESSING TO STORAGE descriptor

int read_at_line=0;	//number of line supossed to read

/* *********************
* Function prototypes
* **********************/
void catch_signal(int sig); 	/* Catches CTRL + C to allow a controlled termination of the application */
void wait_for_ctrl_c(void);
void task_sensor_code(void *args); 		/* Task sensor body */
void task_processing_code(void *args); 	/* Task processing body */
void task_storage_code(void *args); 	/* Task storage body */


/* ******************
* Main function
* *******************/ 
int main(int argc, char *argv[]) {
	int err; 
	struct taskArgsStruct taskAArgs, taskBArgs, taskCArgs;
	
	cpu_set_t cpu_set;
	/* Forces the process to execute only on CPU0 */
	CPU_ZERO(&cpu_set);
	CPU_SET(0,&cpu_set);
	if(sched_setaffinity(0, sizeof(cpu_set), &cpu_set)) {
		printf("\n Lock of process to CPU0 failed!!!");
		return(-1);
	}
	

	/* Lock memory to prevent paging */
	mlockall(MCL_CURRENT|MCL_FUTURE); 

	/* Create RT task */
	/* Args: descriptor, name, stack size, priority [0..99] and mode (flags for CPU, FPU, joinable ...) */
	//TASK A
	err=rt_task_create(&task_sensor_desc, "Task sensor", TASK_STKSZ, TASK_SENSOR_PRIO, TASK_MODE);
	if(err) {
		printf("Error creating task sensor (error code = %d).",err);
		return err;
	} else 
		printf("Task a created successfully.");

	err=rt_task_set_affinity(&task_sensor_desc, &cpu_set);
	if(err){
		printf(" Error forcing task sensor to CPU0 (error code = %d)\n",err);
		return err;
	}else
		printf(" Task sensor asssigned to CPU0\n");

	//TASK B
	err=rt_task_create(&task_processing_desc, "Task processing", TASK_STKSZ, TASK_PROCESSING_PRIO, TASK_MODE);
	if(err) {
		printf("Error creating task processing (error code = %d).",err);
		return err;
	} else 
		printf("Task processing created successfully.");
	
	err=rt_task_set_affinity(&task_processing_desc, &cpu_set);
	if(err){
		printf(" Error forcing task processing to CPU0 (error code = %d)\n",err);
		return err;
	}else
		printf(" Task processing asssigned to CPU0\n");

	//TASK C
	err=rt_task_create(&task_storage_desc, "Task storage", TASK_STKSZ, TASK_STORAGE_PRIO, TASK_MODE);
	if(err) {
		printf("Error creating task storage (error code = %d).",err);
		return err;
	} else 
		printf("Task storage created successfully.");

	err=rt_task_set_affinity(&task_storage_desc, &cpu_set);
	if(err){
		printf(" Error forcing task storage to CPU0 (error code = %d)\n",err);
		return err;
	}else
		printf(" Task storage asssigned to CPU0\n");

	//QUEUE_S_TO_P
	err=rt_queue_create(&queue_s_to_p, "sensor_to_processing", 2, 1, Q_PRIO);
	if(err) {
		printf("Error creating queue sensor to processing (error code = %d)\n",err);
		return err;
	} else 
		printf("Queue sensor to processing created successfully\n");

	//QUEUE_P_TO_S
	err=rt_queue_create(&queue_p_to_s, "processing_to_storage", 2, 1, Q_PRIO);
	if(err) {
		printf("Error creating queue processing to storage (error code = %d)\n",err);
		return err;
	} else 
		printf("Queue processing to storage created successfully\n");



	/* Start RT task */
	/* Args: task decriptor, address of function/implementation and argument*/
	//TASK A
	taskAArgs.taskPeriod_ns = ACK_PERIOD_MS; 	
    rt_task_start(&task_sensor_desc, &task_sensor_code, (void *)&taskAArgs);

	//TASK B
    rt_task_start(&task_processing_desc, &task_processing_code, (void *)&taskBArgs);

	//TASK C	
    rt_task_start(&task_storage_desc, &task_storage_code, (void *)&taskCArgs);

	/* wait for termination signal */	
	wait_for_ctrl_c();

	return 0;
		
}

/* ***********************************
* Task body implementation
* *************************************/
void task_sensor_code(void *args) {
	RT_TASK *curtask;
	RT_TASK_INFO curtaskinfo;
	struct taskArgsStruct *taskArgs;

	unsigned long overruns;
	int err;
	
	/* Get task information */
	curtask=rt_task_self();
	rt_task_inquire(curtask,&curtaskinfo);
	taskArgs=(struct taskArgsStruct *)args;
	printf("Task %s init, period:%llu\n", curtaskinfo.name, taskArgs->taskPeriod_ns);
		
	/* Set task as periodic */
	err=rt_task_set_periodic(NULL, TM_NOW, taskArgs->taskPeriod_ns);

	//open file
	FILE *fp = fopen("sensordata.txt", "r");
	for(;;) {
		err=rt_task_wait_period(&overruns);
		if(err) {
			printf("task %s overrun!!!\n", curtaskinfo.name);
			break;
		}
		
		char *line = NULL;
		size_t len = 0; //size_t is an unsigned integer type of at least 16 bits

		if(getline(&line, &len, fp) == -1){	//if we reach EOF we start reading file again
			printf("---> FILE ENDED. STARTING READING FILE AGAIN <---\n");
			fclose(fp);
			FILE *fp = fopen("sensordata.txt", "r");
			getline(&line, &len, fp);
		}

		if((atoi(line))>65535){	//16 bit unsigned int -> max value = 65535 
			printf("	ERROR: SE -> NUMBER %d TOO BIG\n", atoi(line));
		}else if ((atoi(line))<0){//16 bit unsigned -> impossible to have negative sensor readings
			printf("	ERROR: SE -> NUMBER %d IS NEGATIVE\n", atoi(line));
		}else{
			printf("SE -> READ VALUE: %d\n", atoi(line));
			void * msg = rt_queue_alloc(&queue_s_to_p, 2);	//allocate memory
			printf("SE -> POS: %p\n", msg);
			if(!msg){
				printf("	ERROR: No memory available\n");
				return;
			}
			*((int *)msg) = atoi(line);
			printf("SE -> MEM VALUE: %d\n", *((int *)msg));
			printf("SE -> AWAKENED %d\n", rt_queue_send(&queue_s_to_p, msg, 2, Q_NORMAL));
		}

	}
	return;
}

void task_processing_code(void *args) {
	RT_TASK *curtask;
	RT_TASK_INFO curtaskinfo;
	struct taskArgsStruct *taskArgs;

	//create circular buffer
	circular_buffer *cb = cb_init();

	/* Get task information */
	curtask=rt_task_self();
	rt_task_inquire(curtask,&curtaskinfo);
	printf("Task %s init\n", curtaskinfo.name);

	for(;;) {
		
		//read queue
		void *line;
		ssize_t len;
		len = rt_queue_receive(&queue_s_to_p, &line, TM_INFINITE);	//read message on the queue
		printf("P -> RECEIVED %ld BYTES\n", len);
		printf("P -> RECEIVED VALUE: %d\n", (*(int *) line));
		
		cb_push(cb, (*(int *) line));	//put the message value on the buffer
		if(cb_is_full(cb)){ //if buffer is full we start calculating the filter
			void * msg = rt_queue_alloc(&queue_p_to_s, 2);	//allocate mem
			printf("P -> POS: %p\n", msg);
			if(!msg){
				printf("	ERROR: No memory available\n");
				return;
			}
			//average of the values inside the buffer
			uint16_t avg=0;
			cb_average(cb, &avg);
			(*(uint16_t *)msg) = avg;
			cb_print(cb);
			printf("MED: %d\n", avg);
			printf("P -> MEM VALUE: %d\n", (*(uint16_t*)msg));
			printf("P -> AWAKENED %d\n", rt_queue_send(&queue_p_to_s, msg, 2, Q_NORMAL));
		}

		if(!rt_queue_free(&queue_s_to_p, line))	//frees buffer to be used again
			printf("P -> MEM RELEASED WITH SUCCESS\n"); 
	
	}

	return;
}

void task_storage_code(void *args) {
	RT_TASK *curtask;
	RT_TASK_INFO curtaskinfo;
	struct taskArgsStruct *taskArgs;
	
	/* Get task information */
	curtask=rt_task_self();
	rt_task_inquire(curtask,&curtaskinfo);
	printf("Task %s init\n", curtaskinfo.name);

	//write to file
	FILE *fp = fopen("sensordataFiltered.txt", "a");	//append mode

	for(;;) {
		//read queue
		void *line;
		ssize_t len;
		len = rt_queue_receive(&queue_p_to_s, &line, TM_INFINITE);	//receive message from queue
		printf("ST -> RECEIVED %ld BYTES\n", len);
		printf("ST -> RECEIVED VALUE: %d\n", *(uint16_t *) line);
		fprintf(fp, "%d\n", *(uint16_t *) line);	//write to file
		printf("ST -> WRITTEN VALUE: %d\n", *(uint16_t *) line);
		if(!rt_queue_free(&queue_p_to_s, line))	//frees buffer to be used again
			printf("ST -> MEM RELEASED WITH SUCCESS\n"); 

	}
	fclose(fp);	//close file

	return;
}

/* **************************************************************************
 *  Catch control+c to allow a controlled termination
 * **************************************************************************/
 void catch_signal(int sig)
 {
	 return;
 }

void wait_for_ctrl_c(void) {
	signal(SIGTERM, catch_signal); //catch_signal is called if SIGTERM received
	signal(SIGINT, catch_signal);  //catch_signal is called if SIGINT received

	// Wait for CTRL+C or sigterm
	pause();
	
	// Will terminate
	printf("Terminating ...\n");
}