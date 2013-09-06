#ifndef EDF_ALGO_H
#define EDF_ALGO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Volts 2.4
#define SleepEnergy 0.5
#define IdleEnergy 2.0
#define StaticEnergy 3.0
#define Freq 1
#define SLEEPTHR 1.0000000
#define MIN(a,b) a<b?a:b
#define MAX(a,b) a>b?a:b

float time;
int no_of_decn_pts;
int no_of_freq_switches;
int sleepFlag;
int finishTask;
float zzzTime;
float decision_pt;
int min_arrival;
float curr_exec;
FILE *fpin;
FILE *fp_job;
FILE *fpout;
FILE *fp_freq;
FILE *fp_param;
FILE *fp_lat;
FILE *fp_time_per_freq;
FILE *fp_output;
FILE *fp_execution_times;
FILE *fp_excel_sched;
FILE *fp_plau;
FILE *hyper;
int no_of_tasks;
int no_of_freq;
int hyperperiod;
float alpha;
float prev_alpha;
float operating_frequency;
float *freq;
float energy;
int preemptions;
double latency;
int cache_impacts;
float sleepenergy;
float idleenergy;
float sleepTimeTotal;
float *time_per_freq;
float *min_exec_tasks;
float *max_exec_tasks;
float *total_exec_tasks;
float *avg_exec_tasks;
float *min_lat;
float *max_lat;
float *avg_lat;
float *total_lat;

	
typedef enum {True = 1, False = 0}Bool;	/*True & False*/

typedef struct 				/*structure for an instance of the task*/
	{
		int id;
		int release_time;	
		int deadline;
		float max_computation_time;	
		int period;
		float remaining_time;
		float abs_rem;
		float ui;
		float invocation;
		float plausible_time;
		float start_time;
		float slack;
		float execution_time;
		int instance;
	}job_instance;				

job_instance **job;

typedef struct	
	{
		int id;
		int release_time;
		int deadline;
		float max_computation_time;
		int period;
	}task;

task *t;

typedef struct node			/*node in the queue of tasks*/
	{
		job_instance job;
		struct node *next;
		struct node *prev;
	}node;

typedef struct Queue			/*Queue for all tasks*/
	{
		node *front;
		node *rear;
	}Queue;

Queue *Q;
Queue *Q_Hyper;
node *min;
node *prev_min;
/*Queue functions*/
void create_Queue();
node *create_node(node *z);
void Print_Queue();
void Enqueue(node *z);
void Dequeue();
Bool IsEmpty();
void Calc_Plausible_Time();
void Calc_Urgent_Time();
void Calc_Slack();

/*HyperQueue Functions*/
void create_Hyper_Queue();
void Enqueue_Hyper(node *z);
void Calc_Plausible_Time_Hyper();
void Calc_Urgent_Time_Hyper();
void Print_Queue_Hyper();
void print_plausible_hyper();
/*Scheduling functions*/
float calc_utilization();		
void select_frequency();
void Schedule();
int Check_Sleep(node *);
#endif /*EDF_ALGO_H */
