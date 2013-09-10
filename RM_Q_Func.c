#include "EDF_Algo_DVS.h"

void create_Queue()
{
	Q = (Queue *)malloc(sizeof(Queue));
	Q->front = NULL;
	Q->rear = NULL;
}

Bool IsEmpty()
{
	if(Q->front == NULL && Q->rear == NULL)
	return True;
	else
	return False;
}

node *create_node(node *z)
{
	z = (node *)malloc(sizeof(node));
	z->next = NULL;
	z->prev = NULL;
	
	return z;
}

void Enqueue(node *z)
{
	node *y;
	node *x;
	x = Q->front;
	y = NULL;
	if(Q->front == NULL && Q->rear == NULL)
	{
		Q->front = z;
		Q->rear = z;
	}
	
	else
	{
		while(x!=NULL && (x->job).deadline <= (z->job).deadline)
		{
			if((x->job).deadline == (z->job).deadline && x->job.release_time>z->job.release_time) {
			//	fprintf(fpout,"broken enqueue with same deadline  %d(%d-%d) after  %d(%d-%d)\n",z->job.id,z->job.release_time, z->job.deadline,y->job.id,y->job.release_time,y->job.deadline);
				break;
			}

			y = x;
			x = x->next;
		}
		
		if(y == NULL)
		{
			z->next = Q->front;
			Q->front->prev = z;
			Q->front = z;
		}
		else if(x == NULL)
		{
			Q->rear->next = z;
			z->prev = Q->rear;
			Q->rear = z;
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


void Dequeue()
{
	if(Q->front == Q->rear)
	{
		Q->front = Q->rear = NULL;
	}
	
	else 
	{
		Q->front = Q->front->next;
		Q->front->prev = NULL;
	}
}

int Print_Current_Status()
{
	node *temp;
	temp = Q->front;
	int count=0;
	if(temp == NULL)
	{
		fprintf(fpout,"Current Status of ready Queue : CPU IDLE size(%d)\n\n\n",count);
		if(time!=decision_pt)
			fprintf(fp_excel_sched,"%f\t%f\tIdle\tExecute\tMIN_FREQ\n",time,decision_pt);						
		return 0;
	}
	fprintf(fpout,"Current Status of ready Queue : ");
	while(temp!=NULL)
	{
		count++;
		fprintf(fpout,"T[%d]\t",(temp->job).id);
		temp = temp->next;
	}
	fprintf(fpout,"\nsize=%d\n",count);
	if(count>no_of_tasks) fprintf(fpout,"overflow ur buffer making all ur data suffer\n");
	return 1;
}

void Calc_Plausible_Time()
{
	node *temp;
	node *temp1;
	temp = Q->front;
	float min_plausible_time;
	//printf("IN FUNC %f\n",(temp->job).plausible_time);
	fflush(stdout);
	while(temp != Q->rear)
	{
		temp1 = temp->next;
		min_plausible_time = (temp1->job).plausible_time;
		//printf("MIN = %f\n",min_plausible_time);
		while(temp1 != NULL)
		{
			if((temp1->job).plausible_time < min_plausible_time)
				min_plausible_time = (temp1->job).plausible_time;
			temp1 = temp1->next;
		}
		(temp->job).plausible_time = MIN((min_plausible_time - (temp->job).max_computation_time),(temp->job).plausible_time);
		fprintf(fpout,"Plau T[%d] is %f\n",(temp->job).id,(temp->job).plausible_time);
		temp = temp->next;
	}
	fprintf(fpout,"Plau T[%d] is %f\n",(temp->job).id,(temp->job).plausible_time);
}
void Calc_Urgent_Time()
{
	node *temp;
	temp = Q->front;
	fprintf(fpout,"URGE time of T[%d] is %f and rls time %d\n",(temp->job).id,(temp->job).start_time,(temp->job).release_time);
	temp = Q->front->next;
	node *temp1;
	
	while(temp != NULL)
	{
		temp1 = temp->prev;
		(temp->job).start_time = (temp1->job).start_time + (temp1->job).max_computation_time;
		fprintf(fpout,"URGE time of T[%d] is %f and rls time %d\n",(temp->job).id,(temp->job).start_time,(temp->job).release_time);
		temp = temp->next;
	}
}
					
void Print_Queue()
{
	node *temp;
	temp = Q->front;
	while(temp != NULL)
	{
		printf("T[] : %d\t%d\t%d\t%f\n",(temp->job).release_time, (temp->job).deadline, (temp->job).period, (temp->job).max_computation_time);
		temp = temp->next;
	}
}
