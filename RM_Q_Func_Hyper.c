#include "EDF_Algo_DVS.h"

void create_Hyper_Queue()
{
	Q_Hyper = (Queue *)malloc(sizeof(Queue));
	Q_Hyper->front = NULL;
	Q_Hyper->rear = NULL;
}
float next_highest_plausible(node *x)
{
	float plau = x->job.plausible_time;

	while(x->next->job.plausible_time < plau) x=x->next;
	
	return x->next->job.plausible_time;
}

int find_min_period(node *x)
{
	int i=0,min=t[0].period;
	for(i=0;i<no_of_tasks;i++)
		if(t[i].period < min)
			min=t[i].period;
	fprintf(hyper,"Min period %d\n",min);
	if(x!=NULL)
		if(x->job.period == min) min=(int)hyperperiod;
	
	return min;	
}
int need_break(node *tsk)
{
	int min_period = find_min_period(tsk);
	if(tsk->job.max_computation_time >= (float)min_period)
		return 1;
	
	return 0;
}

float find_granular_wcet()
{
	int i=0;
	for(i=0;i<no_of_tasks;i++)
		if(t[i].period == find_min_period(NULL)) break;
	fprintf(hyper,"Granular wcet = %f\n",(float)t[i].period - t[i].max_computation_time);
	return (float)t[i].period - t[i].max_computation_time;
}
void Calc_Plausible_Time_Hyper()
{
	node *temp;
	node *temp1;
	node *z;
	node *br;
	prev_min = NULL;
	temp = Q_Hyper->rear;
	float min_plausible_time;
	float granular_wcet;
	float rem_wcet=0;
	float wcet;
	fflush(stdout);
	while(temp != Q_Hyper->front)
	{
		temp1 = temp->prev;
		printf("T[%d][%d] deadline %d\n",temp->job.id,temp->job.instance,temp->job.deadline);
		if(need_break(temp))
		{							//breaks i into granular wcets and enqueue(replace) consecutively
			granular_wcet = find_granular_wcet();
			temp->job.plausible_time = temp->job.deadline-granular_wcet;
			wcet = temp->job.max_computation_time;
			rem_wcet = temp->job.max_computation_time - granular_wcet;
			temp->job.max_computation_time = granular_wcet;
			while(rem_wcet > granular_wcet)
			{
				z = create_node(z);
				(z->job).id = (temp->job).id;
				(z->job).release_time = (temp->job).release_time;
				(z->job).max_computation_time = granular_wcet;				//**
				(z->job).deadline = temp->job.deadline - wcet + rem_wcet;	//**
				(z->job).period = (temp1->job).period;
				(z->job).plausible_time = z->job.deadline - granular_wcet;	//**
				(z->job).start_time = (float)temp1->job.release_time;
				(z->job).slack = 0;
				(z->job).execution_time = 0;
				z->job.instance = temp->job.instance;
				Enqueue_Hyper(z);

				rem_wcet -= granular_wcet;
			}
			if(rem_wcet > 0)
			{
				z = create_node(z);
				(z->job).id = (temp->job).id;
				(z->job).release_time = (temp->job).release_time;
				(z->job).max_computation_time = rem_wcet;				//*****
				(z->job).deadline = temp->job.deadline - wcet + rem_wcet;	//**
				(z->job).period = (temp1->job).period;
				(z->job).plausible_time = z->job.deadline - rem_wcet;	//**
				(z->job).start_time = (float)temp1->job.release_time;
				(z->job).slack = 0;
				(z->job).execution_time = 0;
				z->job.instance = temp->job.instance;
				Enqueue_Hyper(z);
			}
		}
		else
		{
			temp1->job.plausible_time = MIN(temp1->job.plausible_time , temp->job.plausible_time - temp1->job.max_computation_time);		
			//update the deadline also... deadline(i) = plausible(i+1)!!
			temp1->job.deadline = temp->job.plausible_time;
		}
		temp = temp->prev;
	}
}
void Enqueue_Hyper(node *z)
{
	node *y;
	node *x;
	x = Q_Hyper->front;
	y = NULL;

	if(Q_Hyper->front == NULL && Q_Hyper->rear == NULL)
	{
		Q_Hyper->front = z;
		Q_Hyper->rear = z;
	}
	
	else
	{
		while(x!=NULL && (x->job).deadline < (z->job).deadline)
		{
			y = x;
			x = x->next;
		}
		
		if(y == NULL)
		{
			z->next = Q_Hyper->front;
			Q_Hyper->front->prev = z;
			Q_Hyper->front = z;
		}
		else if(x == NULL)
		{
			Q_Hyper->rear->next = z;
			z->prev = Q_Hyper->rear;
			Q_Hyper->rear = z;
		}
		else
		{
			z->next = y->next;
			z->next->prev = z;
			y->next = z;
			z->prev = y;
		}
	}
}

void Calc_Urgent_Time_Hyper()
{
	node *temp;
	temp = Q_Hyper->front;
	fprintf(fp_plau,"T[%d][%d]\t%d >\t%f\t%d\t(%f)\n",(temp->job).id,temp->job.instance,(temp->job).release_time,(temp->job).plausible_time,(temp->job).deadline,temp->job.max_computation_time);
//	fprintf(fp_plau,"URGE time of T[%d] is %f Rls time is %d ",(temp->job).id,(temp->job).start_time,(temp->job).release_time);
//	fprintf(fp_plau,"Plau T[%d] is %f\n",(temp->job).id,(temp->job).plausible_time);
	temp = Q_Hyper->front->next;
	node *temp1;
	
	while(temp != NULL)
	{
		temp1 = temp->prev;
		(temp->job).start_time = MAX(((temp1->job).start_time + (temp1->job).max_computation_time),(temp->job).start_time);
		fprintf(fp_plau,"T[%d][%d]\t%d >\t%f\t%d\t(%f)\n",(temp->job).id,temp->job.instance,(temp->job).release_time,(temp->job).plausible_time,(temp->job).deadline,temp->job.max_computation_time);

//	fprintf(fp_plau,"URGE time of T[%d] is %f Rls time is %d ",(temp->job).id,(temp->job).start_time,(temp->job).release_time);
//		fprintf(fp_plau,"Plau T[%d] is %f\n",(temp->job).id,(temp->job).plausible_time);
		temp = temp->next;
	}
}

void Calc_Slack()
{
	node *temp;
	temp = Q_Hyper->front;
	fprintf(fpout,"Slack T[%d] is %f\n",(temp->job).id,(temp->job).slack);
	node *temp1;
	temp = temp->next;
	while(temp != NULL)
	{
		temp1 = temp->prev;
		(temp->job).slack = (temp->job).start_time - ((temp1->job).start_time + (temp1->job).max_computation_time);
		fprintf(fpout,"Slack T[%d] is %f\n",(temp->job).id,(temp->job).slack);
		temp = temp->next;
	}
}

void Print_Queue_Hyper()
{
	node *temp;
	FILE *f1 = fopen("hyper_queue.txt","a+");
	temp = Q_Hyper->front;
	while(temp != NULL)
	{
		fprintf(f1,"T[] : %d\t%d\t%d\t%f\n",(temp->job).release_time, (temp->job).deadline, (temp->job).period, (temp->job).max_computation_time);
		fflush(stdout);
		temp = temp->next;
	}
	fclose(f1);
}
