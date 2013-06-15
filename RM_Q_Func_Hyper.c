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
void Calc_Plausible_Time_Hyper()
{
	node *temp;
	node *temp1;
	temp = Q_Hyper->rear;
	float min_plausible_time;
	//printf("IN FUNC %f\n",(temp->job).plausible_time);
	fflush(stdout);
	while(temp != Q_Hyper->front)
	{
		temp1 = temp->prev;
		min_plausible_time = (temp->job).plausible_time;
		
		(temp1->job).plausible_time = MIN((min_plausible_time - (temp1->job).max_computation_time),(temp1->job).plausible_time);
		temp = temp->prev;
	}
}

void Calc_Plausible_Time_Hyper_change()
{
	node *temp;
	node *temp1;
	node *z;

	temp = Q_Hyper->rear;
	float min_plausible_time;
	float rem_wcet=0;
	//printf("IN FUNC %f\n",(temp->job).plausible_time);
	fflush(stdout);
	while(temp != Q_Hyper->front->next)
	{
		printf("plau T[%d][%d](%d)",(temp1->job).id,(temp1->job).release_time/(temp1->job).period,temp1->job.deadline);
		temp1 = temp->prev;
		min_plausible_time = (temp->job).plausible_time;
		fprintf(hyper,"Updated @#$ plau T[%d][%d] from %f to %f (dd-%d)\n",(temp1->job).id,(temp1->job).release_time/(temp1->job).period, (temp1->job).plausible_time,MIN((min_plausible_time - (temp1->job).max_computation_time),(temp1->job).plausible_time),(temp1->job).deadline);
		(temp1->job).plausible_time = MIN((min_plausible_time - (temp1->job).max_computation_time),(temp1->job).plausible_time);
		//fprintf(hyper,"check update : %d+%d || %f\n",(temp1->prev->job).release_time , temp1->prev->job.max_computation_time,(temp1->job).plausible_time);
		
		//BUG: we need to find task with min release time > plausible time (not just the previous task)
		min = temp1;
//		while(temp->prev->job.release_time + temp->prev->job.max_computation_time >= temp1->job.release_time + temp1->job.max_computation_time)
		while(1)
		{
			if(temp->prev->job.plausible_time < temp1->job.release_time+temp1->job.max_computation_time)
			{
				min = temp1;
				break;
			}
			temp1 = temp1->prev;
			if(temp1 == NULL) { temp1 = temp->prev; break; }
		}

		fprintf(hyper,"***Found prev min(%d) rls T[%d][%d] = %f(%d)\n",min!=temp1,min->job.id, min->job.instance, min->job.plausible_time, min->job.deadline);
		temp1 = temp->prev;
		
//BUG same min found twice... to avoid this, move min towards left when same
/*		if(temp1->next->job.plausible_time == min->job.deadline && temp1 != min) 
		{
			fprintf(hyper,"lolol\n\n");
			temp1 = min->prev;
			while(1)
			{
				if(temp->prev->job.plausible_time < temp1->job.release_time+temp1->job.max_computation_time)
				{
					min = temp1;
					break;
				}
				temp1 = temp1->prev;
				if(temp1 == NULL) {min=temp->prev; temp1 = temp->prev; break; }
			}
			fprintf(hyper,"2***Found 2nd prev min(%d) rls T[%d][%d] = %f(%d)\n",min!=temp1,min->job.id, min->job.instance, min->job.plausible_time, min->job.deadline);
			temp1 = temp->prev;
		}
*/		
//		while((temp1->job).plausible_time < (temp1->prev->job).release_time + temp1->prev->job.max_computation_time)
		if((temp1->job).plausible_time < min->job.release_time + min->job.max_computation_time && temp1 != min)
		{
			fprintf(hyper,"Changing plau T[%d][%d] from %f to %d\n\n",(temp1->job).id,(temp1->job).release_time/(temp1->job).period, (temp1->job).plausible_time, (min->job).deadline);
		
			(temp1->job).plausible_time = (min->job).deadline;
		/*	if(temp1->job.deadline > min->job.plausible_time)
			{
				fprintf(hyper,"Gandla ka\n");
				//fprintf(fp_plau,"Gandla ki\n");
			}
		*/
			
			rem_wcet = ((temp1->job).max_computation_time) - (temp1->job.deadline - (temp1->job).plausible_time);
			temp1->job.max_computation_time -= rem_wcet;
//we get rem_wcet<0 whenever the task is too small to break (we have to move the entire task i.e. dequeue-> change deadline -> enqueu)

			if(rem_wcet < 0 )
			{
				fprintf(hyper,"can't be broken... so moving\n");
				rem_wcet += temp1->job.max_computation_time;
				temp1->prev->next = temp1->next;
				temp1->next->prev = temp1->prev;
				
				fprintf(hyper,"rem_wcet@@ T[%d][%d] %f = %f - (%f-%f)\n",(temp1->job).id,(temp1->job).release_time/(temp1->job).period,rem_wcet,((temp1->job).max_computation_time) , next_highest_plausible(temp1), (temp1->job).plausible_time);

				z = create_node(z);
				(z->job).id = (temp1->job).id;
				(z->job).release_time = (temp1->job).release_time;
				(z->job).max_computation_time = rem_wcet;
				(z->job).deadline = (min->job).plausible_time;
				(z->job).period = (temp1->job).period;
				//(z->job).remaining_time = (float)rem_wcet;
				//(z->job).ui = (float)(z->job).max_computation_time / t[i].period;
				//job[i][instance].ui = (z->job).ui;
				//(z->job).abs_rem = (temp1->job).abs_rem;
				(z->job).plausible_time = (float)((min->job).plausible_time - rem_wcet);
				(z->job).start_time = (float)temp1->job.release_time;
				(z->job).slack = 0;
				(z->job).execution_time = 0;
				z->job.instance = temp1->job.instance;
				fprintf(hyper,"moving T[%d][%d]** (r-%d, dd-%d, plau-%f)\n",z->job.id,z->job.release_time/z->job.period,z->job.release_time,z->job.deadline,z->job.plausible_time);
				Enqueue_Hyper(z);

			}
			else
			{
				fprintf(hyper,"rem_wcet T[%d][%d] %f = %f - (%f-%f)\n",(temp1->job).id,(temp1->job).release_time/(temp1->job).period,rem_wcet,((temp1->job).max_computation_time) , next_highest_plausible(temp1), (temp1->job).plausible_time);

				z = create_node(z);
				//here change release time to avoid mispalced enqueue!! to stop any further mishap, job->instance will be used
				(z->job).id = (temp1->job).id;
				(z->job).release_time = (temp1->job).release_time;
				(z->job).max_computation_time = rem_wcet;
				(z->job).deadline = (min->job).plausible_time;
				(z->job).period = (temp1->job).period;
				//(z->job).remaining_time = (float)rem_wcet;
				//(z->job).ui = (float)(z->job).max_computation_time / t[i].period;
				//job[i][instance].ui = (z->job).ui;
				//(z->job).abs_rem = rem_wcet;
				(z->job).plausible_time = (float)((min->job).plausible_time - rem_wcet);
				(z->job).start_time = (float)temp1->job.release_time;
				(z->job).slack = 0;
				(z->job).execution_time = 0;
				z->job.instance = temp1->job.instance;
				fprintf(hyper,"Enqueued T[%d][%d]** (r-%d, dd-%d, plau-%f, abs rem-%f)\n",z->job.id,z->job.release_time/z->job.period,z->job.release_time,z->job.deadline,z->job.plausible_time,z->job.abs_rem);
				Enqueue_Hyper(z);
			}
		//	temp1 = temp1->prev;
		}			

		temp = temp->prev;
		printf("(%d) ",temp->job.deadline);
printf("done\n");
	}
	
//	printf("XXX%d\n",temp->job.id);
	temp->job.plausible_time > (temp->next->job.plausible_time-temp->job.max_computation_time) ? temp->job.plausible_time = temp->next->job.plausible_time-temp->job.max_computation_time : 1;
	fprintf(hyper,"2nd last job %f<> %f\n",temp->job.plausible_time , (temp->next->job.plausible_time-temp->job.max_computation_time));
	while(temp->job.plausible_time < temp->prev->job.release_time+temp->prev->job.max_computation_time)
	{
		printf("STILL GG\n\n");
	}
	temp = temp->prev;
	temp->job.plausible_time > (temp->next->job.plausible_time-temp->job.max_computation_time) ? temp->job.plausible_time = temp->next->job.plausible_time-temp->job.max_computation_time : 1;
	fprintf(hyper,"last job %f <> %f\n",temp->job.plausible_time , (temp->next->job.plausible_time-temp->job.max_computation_time));

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
	fprintf(fp_plau,"T[%d][%d]\t%f\t%d\t%f\t%d\t(%f)\n",(temp->job).id,temp->job.release_time/temp->job.period,(temp->job).start_time,(temp->job).release_time,(temp->job).plausible_time,(temp->job).deadline,temp->job.max_computation_time);
//	fprintf(fp_plau,"URGE time of T[%d] is %f Rls time is %d ",(temp->job).id,(temp->job).start_time,(temp->job).release_time);
//	fprintf(fp_plau,"Plau T[%d] is %f\n",(temp->job).id,(temp->job).plausible_time);
	temp = Q_Hyper->front->next;
	node *temp1;
	
	while(temp != NULL)
	{
		temp1 = temp->prev;
		(temp->job).start_time = MAX(((temp1->job).start_time + (temp1->job).max_computation_time),(temp->job).start_time);
		fprintf(fp_plau,"T[%d][%d]\t%f\t%d\t%f\t%d\t(%f)\n",(temp->job).id,temp->job.release_time/temp->job.period,(temp->job).start_time,(temp->job).release_time,(temp->job).plausible_time,(temp->job).deadline,temp->job.max_computation_time);

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
	temp = Q_Hyper->front;
	while(temp != NULL)
	{
		printf("T[] : %d\t%d\t%d\t%f\n",(temp->job).release_time, (temp->job).deadline, (temp->job).period, (temp->job).max_computation_time);
		fflush(stdout);
		temp = temp->next;
	}
}
