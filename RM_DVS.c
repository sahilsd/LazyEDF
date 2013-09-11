#include "EDF_Algo_DVS.h"

float findMin(float a, float b)
{
	float min = -1;
	if(a<b)// && a>=0)
	min = a;
	else //if(b>=0)
	min = b;
	
	//fprintf(fpout,"MIN IS %f\n",min);
	return min;
}

int Check_Sleep(node *Q_Hyper_node)
{
	//fprintf(fpout,"plau %f, time %f minarr %d\n",(Q_Hyper_node->job).plausible_time,time,find_min_arrival());
	//fprintf(fpout,"SLEEPTIME IS %f xyz is %f\n",(Q_Hyper_node->job).plausible_time - time,((float)find_min_arrival()-time));
	
//EDIT2: We don't consider min_arrival while deciding to shut down. Since arrival is just an interrupt and can be served when CPU is wakes up
//	if(findMin(((Q_Hyper_node->job).plausible_time - time),((float)find_min_arrival()-time)) >= zzzTime)
	fprintf(fpout,"Sleep thr:%f [ ] next plau:%f\n",zzzTime,(Q_Hyper_node->job).plausible_time);
	if(((Q_Hyper_node->job).plausible_time - time) >= zzzTime && time!=(float)(find_min_arrival()))
		return 1;
	else
		return 0;
}

void find_hyperperiod()
{
	int i,j;
	int flag;
	int max;
	
	max = t[0].period;
	for(i=1; i<no_of_tasks; i++)
	{
		if(t[i].period > max)
		max = t[i].period;
	}
	
	flag = 1;
	
	for(i=max; flag==1; i=i+max)
	{
		for(j=0; j<no_of_tasks; j++)
		{
			if(i%t[j].period != 0)
			break;
		}
		if(j==no_of_tasks)
		{
			hyperperiod = i;
			flag = 0;
		}
	}
}

float calc_utilization()
{
	int i = 0;
	float utilization = 0;
	int instance;
	
	while(i<no_of_tasks)
	{
		instance = decision_pt/t[i].period;
		utilization += job[i][instance].ui;
		//fprintf(fpout,"Utilization[%d] : %f \n",i,job[i][instance].ui);
		i++;
	}
	
	return utilization;
}

void select_frequency()
{
	int i = 0;
	float utilization;
	utilization = calc_utilization();
	fprintf(fpout,"Utilization: %f\n\n",utilization);
	
	if (utilization == 0)
	operating_frequency = 0;
	
	else if(utilization > 0 && utilization <= freq[0])
	operating_frequency = freq[0];
	
	else
	{
		i = 1;
		for(i=1; i<no_of_freq; i++)
		{
			if(utilization <= freq[i] && utilization > freq[i-1])
			{
				operating_frequency = freq[i];
				break;
			}
		}
		if(i == no_of_tasks)
		operating_frequency = freq[i-1];
		
	}
}
		
int min_period()
{
	int min;
	int i;
	min = t[0].period;
	i = 1;
	while(i<no_of_tasks)
	{
		if(t[i].period < min)
		min = t[i].period;
		i++;
	}
	return min;
}

int find_min_arrival()
{
	int i = 1;
	int min;
	int max_div;
	
	min = t[0].release_time;
	while(i<no_of_tasks)
	{
		if(min > t[i].release_time)
		min = t[i].release_time;
		i++;
	}
	
	return min;	
					
}
int update_plausible(int task, node * hyper_node, float exec, float alpha)
{
	node *temp = hyper_node;
	while((temp->job).id != task) temp=temp->next;
	//fprintf(fpout,"***Changing plau[%d=%d] from %f to %f+%f*%f***\n",task,(temp->job).id,(temp->job).plausible_time,(temp->job).plausible_time,exec,alpha);	
	(temp->job).plausible_time += exec*alpha;
}	
float calc_decision_pt(int next_deadline)
{
	float min;
	int i = 1;
	float finish_time = (Q->front->job).remaining_time/alpha + time;
	no_of_decn_pts++;
	min_arrival = find_min_arrival();
	if(Q->front != NULL)						//CPU <> IDLE
	{
		fprintf(fpout,"Decision point min(%f,%f,%f)\n",(float)min_arrival,(float)next_deadline,finish_time);
		if((float)min_arrival < (float)next_deadline)
		{
			if((float)min_arrival < finish_time) return min_arrival;
			else return finish_time;
		}
		else if((float)next_deadline < finish_time)
			return next_deadline;
		else
			return finish_time;
	}
	return min_arrival;
}
int update_deadline(node *hyper_head,node *x)
{
//mara toda phoda 
	node * temp = hyper_head;
	while(temp->next != NULL)
	{
		if(x->job.id == temp->job.id && x->job.instance == temp->job.instance)
		{
			if(x->job.deadline == temp->job.deadline)
			{
					temp=temp->next;
					continue;
			}
			else 
			{
				x->job.deadline = temp->job.deadline;
				return 1;
			}
		}
		temp = temp->next;
	}
	return 0;	
}
void remove_hyper(node * hyper)
{
	int i = hyper->prev->job.id;
	int rls = hyper->prev->job.release_time;
	int period = hyper->prev->job.period;

	while(hyper->next != NULL && hyper->job.deadline <=  rls+period)
	{
		if(hyper->job.id == i && hyper->job.release_time == rls)
		{
			hyper->prev->next = hyper->next;
			hyper->next->prev = hyper->prev;
			fprintf(fpout,"removed %d|%d(%f)\n",i,rls/period,hyper->job.plausible_time);
		}
		hyper = hyper->next;
	}
}
void move_subjob(Queue **rq, node *hq)
{
	node *tempRQ;
	while(hq!=NULL)
	{
		if((*rq)->front->job.id == hq->job.id && (*rq)->front->job.instance == hq->job.instance)
		{
			(*rq)->front->job.deadline = hq->job.deadline;				//update deadline
			
			tempRQ = (*rq)->front;						//Q->front temp
			if((*rq)->front->next != NULL)
			{
				(*rq)->front = (*rq)->front->next;		//Q->front++
				(*rq)->front->prev->next=NULL;			//Q->front prev = NULL
				(*rq)->front->prev=NULL;			

				Enqueue(tempRQ);						//Q->front->prev enqueue
			}
			fprintf(fpout,"moved subjob to dd=%d\n",tempRQ->job.deadline);
			Print_Current_Status();
		}
		hq = hq->next;
	}
}
void wrapper_enqueue(float from, float to)
{
	node *z;
											//Enqueue code//
	int task=0;
	int instance = (int)to/t[task].period;
	while(task<no_of_tasks)
	{
		if((int)(from)/t[task].period != (int)(to)/t[task].period || !(int)to%t[task].period)
		{
			z = create_node(z);
			instance = (int)to/t[task].period;
			(z->job).id = t[task].id;
			(z->job).release_time = instance*t[task].period;
			(z->job).max_computation_time = job[task][instance].max_computation_time;
			(z->job).deadline = job[task][instance].deadline;
			(z->job).period = t[task].period;
			(z->job).remaining_time = (float)job[task][instance].remaining_time;
			(z->job).abs_rem = (float)job[task][instance].abs_rem;
			(z->job).ui = (float)(z->job).max_computation_time / (z->job).period;
			job[task][instance].ui = (z->job).ui;
			(z->job).plausible_time = (float)(job[task][instance].deadline - job[task][instance].max_computation_time);
			(z->job).start_time = (float)job[task][instance].release_time;
			(z->job).slack = 0;
			(z->job).execution_time = 0;
			(z->job).invocation = (float)job[task][instance].invocation;
			z->job.instance = z->job.release_time / z->job.period;
//			update_deadline(Q_Hyper_head,z);
			Enqueue(z);
			t[task].release_time += t[task].period;
					
			fprintf(fpout,"Delayed arrival of T[%d][%d] at %f with deadline %d (rls %d)\n",task,z->job.instance,decision_pt,job[task][instance].deadline,t[task].release_time);
		fprintf(fp_excel_sched,"%f\t_______\tT[%d][%d]\tDelayedArrival\t____\n",decision_pt,task,t[task].release_time/t[task].period);
		}
		task++;
	}

}

void Schedule()
{
	int i = 0;
	int check;
	node *z;
	float time_bef_dec = 0;
	int instance;
	float utilization;
	float exec;
	float next_plausible;
	int freq_find;
	float prev_alpha = 1;
	node *Q_Hyper_head = Q_Hyper->front;
	while(time < hyperperiod)
	{	
		fprintf(fpout,"Time : %f (a=%f)\t\t",time,alpha);
		wrapper_enqueue(time_bef_dec,time);
		time_bef_dec = time;
		decision_pt = calc_decision_pt(Q_Hyper_head->job.deadline);
		if(decision_pt < time) fprintf(fpout,"time travel----need towel\n");
		fprintf(fpout,"Decision point : %f\n\n",decision_pt);
		if(Q->front !=NULL)
		{
			fprintf(fpout,"(RQ:T[%d][%d] <> HQ[%d][%d])\n\n",Q->front->job.id,Q->front->job.instance,Q_Hyper_head->job.id,Q_Hyper_head->job.instance);
			printf("Time:%f | DP:%f\n",time,decision_pt);
		}				
		
		next_plausible = Q_Hyper_head->job.plausible_time;
		if(!Print_Current_Status())
		{
				if(next_plausible - time >= zzzTime)
				{
														//Shut down code//
					fprintf(fpout,"Sleep from %f to %f \n", time, next_plausible);
					fprintf(fp_excel_sched,"%f\t%f\t____\tSleep\t____\n",time,next_plausible);				

					sleepTimeTotal += (next_plausible - time);
					time = next_plausible;
					if(decision_pt != next_plausible) no_of_decn_pts++;
					
															//awake//					
					operating_frequency = 1;
					alpha = operating_frequency;
					if(alpha != prev_alpha)
					{
						no_of_freq_switches++;
						prev_alpha = alpha;
					}
					sleepenergy += 1*SleepEnergy;
					continue;
				}
				else									//idle code//				
				{
					operating_frequency = freq[0];
					alpha = operating_frequency;
					fprintf(fpout,"Idle from %f to %f \n", time, decision_pt);
					fprintf(fp_excel_sched,"%f\t%f\t____\tIdle\t%f\n",time,next_plausible,alpha);
					if(alpha != prev_alpha)
					{
						no_of_freq_switches++;
						prev_alpha = alpha;
					}
					idleenergy += IdleEnergy*(decision_pt - time_bef_dec);
					time = decision_pt;
					continue;
				}
		}
		else
		{
			if(next_plausible - time >= zzzTime)
			{
														//Shut down code//
					fprintf(fpout,"Sleep from %f to %f \n", time, next_plausible);
					fprintf(fp_excel_sched,"%f\t%f\t____\tSleep\t____\n",time,next_plausible);				

					sleepTimeTotal += (next_plausible - time);
					time = next_plausible;
					if(decision_pt != next_plausible) no_of_decn_pts++;
					
															//awake//					
					operating_frequency = 1;
					alpha = operating_frequency;
					if(alpha != prev_alpha)
					{
						no_of_freq_switches++;
						prev_alpha = alpha;
					}
					sleepenergy += 1*SleepEnergy;
					continue;
			}
			else										//execute code//
			{
				for(freq_find=0; freq_find<no_of_freq; freq_find++)
					if(alpha == freq[freq_find])
						break;
			
				if(time == next_plausible) fprintf(fpout,"After shut down exec with alpha=%f\n",alpha);
				fprintf(fpout,"task T[%d][%d] from %f to %f\n\n",Q->front->job.id, Q->front->job.instance,time,decision_pt);
				fprintf(fp_excel_sched,"%f\t%f\tT[%d][%d]\tExecute()\t%f\n",\
				time,decision_pt,(Q->front->job).id,(Q->front->job).release_time / (Q->front->job).period ,alpha);
				exec = decision_pt - time;
				total_exec_tasks[(Q->front->job).id] += exec;
				(Q->front->job).execution_time += exec;
				fprintf(fp_execution_times,"Total Execution time of Task[%d] is %f\n",\
				(Q->front->job).id,(Q->front->job).execution_time);
			
				if((Q->front->job).execution_time < min_exec_tasks[(Q->front->job).id])
				{
					fprintf(fpout,"min exec T[%d]=%f\n",(Q->front->job).id,(Q->front->job).execution_time);
					min_exec_tasks[(Q->front->job).id] = (Q->front->job).execution_time;
				}
				if((Q->front->job).execution_time > max_exec_tasks[(Q->front->job).id])
					max_exec_tasks[(Q->front->job).id] = (Q->front->job).execution_time; 
			
			
				time_per_freq[freq_find] += exec;
			
				(Q->front->job).abs_rem = (Q->front->job).abs_rem - (exec * alpha);
				fprintf(fpout,"Absolute remaining time %f, alpha %f\n",Q->front->job.abs_rem, alpha);
				instance = (Q->front->job).release_time / (Q->front->job).period;
				job[(Q->front->job).id][instance].ui = (float)(job[(Q->front->job).id][instance].invocation)/t[(Q->front->job).id].period;
				////fprintf(fpout,"New Utilization : %f\n",job[(Q->front->job).id][instance].ui);
				energy += (float)alpha*Volts*Volts*alpha*alpha*Freq*(Q->front->job).remaining_time;
			
				latency = decision_pt - (Q->front->job).release_time;
				if(latency < min_lat[(Q->front->job).id])
					min_lat[(Q->front->job).id] = latency;
				if(latency > max_lat[(Q->front->job).id])
					max_lat[(Q->front->job).id] = latency;
			
				total_lat[(Q->front->job).id] += latency;
				fprintf(fp_lat,"Latency of (%d %d) released at %d and completed at %f is %f\n",(Q->front->job).id,instance,(Q->front->job).release_time,decision_pt,latency);
				fflush(fp_lat);
		
				time = decision_pt;			
				fprintf(fpout,"Time : %f and decision pt : %f\n",time,decision_pt);
												//Queue moving code//
				if((Q->front->job).abs_rem<0.01)
				{
					Dequeue();
					if(Q_Hyper_head->next != NULL)
					{
						Q_Hyper_head = Q_Hyper_head->next;
						fprintf(fpout,"New plausible = T[%d][%d]-%f\n",\
						Q_Hyper_head->job.id,Q_Hyper_head->job.instance,Q_Hyper_head->job.plausible_time);
					}
					remove_hyper(Q_Hyper_head);						//remove all the subjobs of T[i][j] from hyper queue
					if(Q->front != NULL)		
						fprintf(fpout,"Dequeued to T[%d][%d] == T[%d][%d]\n\n",\
						Q->front->job.id,Q->front->job.instance,Q_Hyper_head->job.id,Q_Hyper_head->job.instance);
				}
				else if(decision_pt == Q_Hyper_head->job.deadline)
				{
					(Q->front->job).remaining_time = (float)((Q->front->job).abs_rem)/alpha;	//seg fault?				
					move_subjob(&Q,Q_Hyper_head);					
					preemptions++;
					if(Q_Hyper_head->next != NULL)
					{
						Q_Hyper_head = Q_Hyper_head->next;
						fprintf(fpout,"New plausible = T[%d][%d]-%f\n",\
						Q_Hyper_head->job.id,Q_Hyper_head->job.instance,Q_Hyper_head->job.plausible_time);
					}
				}
				else		//rem time--; deadline++; preemption++;
				{
					fprintf(fpout,"DEBUG:NEVER?\n");
					(Q->front->job).remaining_time = (float)((Q->front->job).abs_rem)/alpha;			//seg fault?
					preemptions++;
				}

				time = decision_pt;
				continue;
			}										
		}
	}
}
