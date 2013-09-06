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
	fprintf(fpout,"The utilization is : %f\n\n\n",utilization);
	
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
	fprintf(fpout,"***Changing plau[%d=%d] from %f to %f+%f*%f***\n",task,(temp->job).id,(temp->job).plausible_time,(temp->job).plausible_time,exec,alpha);	
	(temp->job).plausible_time += exec*alpha;
}	
float calc_decision_pt()
{
	float min;
	int i = 1;

	min_arrival = find_min_arrival();
	fprintf(fpout,"MIN ARRIVAL : %d\n",min_arrival);
	if((Q->front) != NULL)
	{
/* EDIT1:To avoid the unaccounted time from finishing time to the arrival time,
 * modify curr_exec in order to execute task from time to min_arrival
 * even if it cant be executed from time to time+remaining time
 */	
		if(min_arrival < time + (Q->front->job).remaining_time && time!=min_arrival && min_arrival-time<SLEEPTHR)	//provisional exec
			curr_exec = min_arrival;
		else
			curr_exec = (Q->front->job).remaining_time + time;
		fprintf(fpout,"TIME : %f, REM TIME : %f Alpha : %f Prev_Alpha : %f\n",time,(Q->front->job).remaining_time,alpha,prev_alpha);
		fprintf(fpout,"CURR EXEC : %f\n",curr_exec);
	}
	else
	{
		return min_arrival;
	}
	
	if(min_arrival < curr_exec)
	min = min_arrival;
	else
	min = curr_exec;
	
	return min;		
}
int update_deadline(node *hyper_head,node *x)
{
//mara toda phoda 
	node * temp = hyper_head;
	while(temp->next != NULL)
	{
		if(x->job.id == temp->job.id && x->job.instance == temp->job.instance)//BUG:release time of subjobs causing problem while comparing 2 fragments of subjobs!
		{
			printf("%d=%d || %d==%d\n",temp->job.release_time/temp->job.period, x->job.release_time/ x->job.period,temp->job.instance, x->job.instance);
			//if(temp->job.instance != x->job.instance) exit(1);
			fprintf(fpout,"compare to update %d<>%d\n",x->job.deadline,temp->job.deadline);
			if(x->job.deadline == temp->job.deadline)
			{
					temp=temp->next;
					continue;
			}
			if(x->job.deadline != temp->job.deadline)
			{
				x->job.deadline = temp->job.deadline;
//				x->job.plausible_time = temp->job.plausible_time;
			//	fprintf(plau_debug,"T[%d][%d] = %f\n",temp->job.id,temp->job.release_time/temp->job.period,temp->job.plausible_time);
				fprintf(fpout,"QQQ PLAUU T[%d][%d] = %f\n",temp->job.id,temp->job.release_time/temp->job.period,temp->job.plausible_time);
				//x->job.abs_rem = temp->job.abs_rem;
				//x->job.remaining_time = ??
				fprintf(fpout,"Deadline updated\n");
				return 1;
			}
			else
			{
				fprintf(fpout,"no deadline update\n");
				return 0;
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
//printf("remove %d|%d\n",i,rls/period);
	//if(hyper!=NULL)
	while(hyper->job.deadline <=  rls+period)
	{
		if(hyper->job.id == i && hyper->job.release_time == rls)
		{
			hyper->prev->next = hyper->next;
			hyper->next->prev = hyper->prev;
			//fprintf(fpout,"removed %d|%d(%f)\n",i,rls/period,hyper->job.plausible_time);
			printf("removed %d|%d(%f)\n",i,rls/period,hyper->job.plausible_time);
		}
		hyper = hyper->next;
	}
}
void Schedule()
{
	finishTask=1;
	int flag = 0;
	int i = 0;
	char provisional = ' ';
	int check;
	node *z;
	float time_bef_dec = 0;
	int instance;
	float utilization;
	node *x;
	float exec;
	int freq_find;
	//node *preempt;	
	//node *Q_head;
	float prev_alpha = 1;
	node *Q_Hyper_head = Q_Hyper->front;
	while(time < hyperperiod)
	{	
		//preempt = Q->front;
	
		time_bef_dec = time;
		decision_pt = calc_decision_pt();
		//BUG: while calculating decision point in case of a provisional execution
/*		if(decision_pt > Q_Hyper_head->job.plausible_time &&  decision_pt!=curr_exec)
		{
		printf("chamya\n\n");
			decision_pt = Q_Hyper_head->job.plausible_time;
			if(time >= Q_Hyper_head->job.plausible_time)
			{
				alpha = 1;
				if(min_arrival < Q_Hyper_head->next->job.plausible_time) decision_pt = min_arrival; 
				else decision_pt = Q_Hyper_head->next->job.plausible_time;
//				decision_pt = min_arrival;
				curr_exec = decision_pt;
				fprintf(fpout,"Ting\n%f=min(%d,%f)-%f",decision_pt,min_arrival,Q_Hyper_head->next->job.plausible_time,alpha);
			printf("pingu\n");
			}
			curr_exec = decision_pt;
			fprintf(fpout,"are dadadaada(%f,%f-%d)\n\n\n",curr_exec,decision_pt,Q_Hyper_head->job.deadline);
		}*/
		//BUG:because we broke the jobs, even at t = q->head deadline, q->remainig_time>=0
		no_of_decn_pts++;
		
			if(!Print_Current_Status())
			{
				idleenergy += IdleEnergy*(decision_pt - time_bef_dec);
			}
			/*if(Q->front!=NULL)
			{
				printf("%f<>%f\n",Q_Hyper_head->job.plausible_time,Q->front->job.plausible_time);		
			}*/
			
		fprintf(fpout,"Decision : %f (T[%d]-%f)",decision_pt,(Q_Hyper_head->job).id,(Q_Hyper_head->job).plausible_time);
		if(Q->front != NULL) fprintf(fpout,"||%f",Q->front->job.plausible_time);
		fprintf(fpout,"\n");
		if(decision_pt == curr_exec)
		{
			for(freq_find=0; freq_find<no_of_freq; freq_find++)
			{
				if(alpha == freq[freq_find])
				break;
			}
/*			if(decision_pt-time > Q_Hyper_head->job.abs_rem / alpha+0.01)
				printf("Parat gandanaarrr...%d-%f ",Q_Hyper_head->job.id, Q_Hyper_head->job.plausible_time);
			if(Q->front->job.id != Q_Hyper_head->job.id && decision_pt-time > Q_Hyper_head->next->job.abs_rem / alpha+0.01)
					printf("Parat parat gandanaarrr...%d-%f ",Q_Hyper_head->next->job.id,Q_Hyper_head->next->job.plausible_time);
*/
			//if((Q->front->job).id != (Q_Hyper_head->job).id)
			if((Q_Hyper_head->job).plausible_time > time)//+remaining time)
			{
				if((Q_Hyper_head->job).id != Q->front->job.id)	fprintf(fpout,"wont be equal... lol no problem\n");
				
				provisional = 'p';
			/*Every provisional execution will go from Time till Next plausible time (not till decision_pt)*/	
				//MIN_FREQ
//				operating_frequency = freq[0];
//				alpha = operating_frequency;
				fprintf(fpout,"dvfs exec alpha (%d,%d)\n",(Q->front->job).id, (Q_Hyper_head->job).id);
				decision_pt = (Q_Hyper_head->job).plausible_time;
				exec = decision_pt-time;
				//change plausible time!!!!
				//update_plausible((Q->front->job).id,Q_Hyper_head,decision_pt-time,alpha);
				fprintf(fpout,"hyper head still at T[%d] (%f)\n",(Q_Hyper_head->job).id,(Q_Hyper_head->job).plausible_time);
			}
			else
				provisional = '*';

			if(Q_Hyper_head->job.id != Q->front->job.id)	fprintf(fpout,"\n\n***LOL***\n\n");
			
			fprintf(fpout,"Current task T[%d][%d] executed from %f to %f with frequency %f\n",(Q->front->job).id,(Q->front->job).release_time / (Q->front->job).period ,time,decision_pt,alpha);
			fprintf(fp_excel_sched,"%f\t%f\tT[%d][%d]\tExecute(%c)\t%f\n",time,decision_pt,(Q->front->job).id+1,(Q->front->job).release_time / (Q->front->job).period +1,provisional,alpha);
			exec = decision_pt - time;
			total_exec_tasks[(Q->front->job).id] += exec;
			(Q->front->job).execution_time += exec;
			fprintf(fp_execution_times,"Total Execution time of Task[%d] is %f\n",(Q->front->job).id,(Q->front->job).execution_time);
			
			if((Q->front->job).execution_time < min_exec_tasks[(Q->front->job).id])
			{
				fprintf(fpout,"min exec T[%d]=%f\n",(Q->front->job).id,(Q->front->job).execution_time);
				min_exec_tasks[(Q->front->job).id] = (Q->front->job).execution_time;
			}
			if((Q->front->job).execution_time > max_exec_tasks[(Q->front->job).id])
				max_exec_tasks[(Q->front->job).id] = (Q->front->job).execution_time; 
			
			
			time_per_freq[freq_find] += exec;
			
			(Q->front->job).abs_rem = (Q->front->job).abs_rem - (exec * alpha);
			printf("exe %d %d-%c [@%f]\n",Q->front->job.id, Q_Hyper_head->job.id,provisional,time);
//			if(provisional == '*' && Q->front->job.id != Q_Hyper_head->job.id){printf("lololol\n"); exit(0);}
			instance = (Q->front->job).release_time / (Q->front->job).period;
			job[(Q->front->job).id][instance].ui = (float)(job[(Q->front->job).id][instance].invocation)/t[(Q->front->job).id].period;
			//fprintf(fpout,"New Utilization : %f\n",job[(Q->front->job).id][instance].ui);
			//fprintf(fpout,"REMTIME***** IS %f\n",(Q->front->job).remaining_time);
			
			energy += (float)alpha*Volts*Volts*alpha*alpha*Freq*(Q->front->job).remaining_time;
			//fprintf(fpout,"ENERGY ****** IS ***** %f\n",energy);
			
			latency = decision_pt - (Q->front->job).release_time;
			if(latency < min_lat[(Q->front->job).id])
				min_lat[(Q->front->job).id] = latency;
			if(latency > max_lat[(Q->front->job).id])
				max_lat[(Q->front->job).id] = latency;
			
			total_lat[(Q->front->job).id] += latency;
			fprintf(fp_lat,"Latency of (%d %d) released at %d and completed at %f is %f\n",(Q->front->job).id,instance,(Q->front->job).release_time,decision_pt,latency);
			fflush(fp_lat);
		
			time = decision_pt;			
				//fprintf(fpout,"Moving plau %f>%f+%f\n",time , (Q_Hyper_head->job).plausible_time,(Q->front->job).invocation);

//			if(Q_Hyper_head->job.abs_rem < 0.01)
			if((Q->front->job).abs_rem<0.01) 		//T[i][j] act exec exhausted completely
			{
				//fprintf(fpout,"Moving plau %f>%f+%f\n",time , (Q_Hyper_head->job).plausible_time,(Q->front->job).invocation);
				if(Q_Hyper_head->next != NULL)// && time >= (Q_Hyper_head->job).plausible_time+(Q->front->job).invocation )
				{
					Q_Hyper_head = Q_Hyper_head->next;
					//remove all the subjobs of T[i][j] from hyper queue****
				//	remove_hyper(Q_Hyper_head);
				}
				fprintf(fpout,"(%f) DEQUEUEUEUE (plau moved T[%d] %f ***%f***)\n",(Q->front->job).abs_rem,Q_Hyper_head->job.id,(Q_Hyper_head->job).plausible_time,Q_Hyper_head->job.abs_rem);				
				Dequeue();

				finishTask = 1;
			}
			else
			{
				finishTask = 0;	//since the task didnt finish, it means some task executed for some time => ideally we should update its plausible time
				fprintf(fpout,"PREEMPTIONS *****T[%d]-%f-%d\n",(Q->front->job).id,(Q->front->job).abs_rem,preemptions);
				//fprintf(fp_excel_sched,"preemptions++ (%d)\n",preemptions);
				preemptions ++;
				
				//abs_rem>0 but time>=deadline means the current subjob in hyper queue finished. So update deadline
//				if(decision_pt>=Q->front->job.deadline) 
//BUG:since deadline and plausible time do not carry any mutual constraints, decision_pt should not be compared with deadline	

				if(Q_Hyper_head->next != NULL)
					if(decision_pt>=Q_Hyper_head->next->job.plausible_time) 				
					{
						update_deadline(Q_Hyper_head,Q->front);
						Q->front = Q->front->next;
						fprintf(fpout,"moving Q->front to T[%d](%d)\n",Q->front->job.id, Q->front->job.deadline);
					//printf("\nmoved %d<>%d\n",Q->front->job.deadline,Q_Hyper_head->job.deadline);
	//					Q->front = NULL;
						Enqueue(Q->front->prev);
						fprintf(fpout,"new Q->front - T[%d](%d)\n",Q->front->job.id,Q->front->job.deadline);
					
						if(Q->front->job.id == Q_Hyper_head->next->job.id) Q_Hyper_head = Q_Hyper_head->next;			
					}		
			}
			if(Check_Sleep(Q_Hyper_head) && finishTask)
			{
				operating_frequency = 1;
				alpha = operating_frequency;
				if(alpha != prev_alpha)
				{
					no_of_freq_switches++;
					prev_alpha = alpha;
				}
				//decision_pt = findMin(((Q_Hyper_head->job).plausible_time),((float)find_min_arrival()));
				decision_pt = (Q_Hyper_head->job).plausible_time;
				fprintf(fpout,"sahil:plau %f\n",(Q_Hyper_head->job).plausible_time);
				//if(time > (Q_Hyper_head->job).plausible_time && (Q_Hyper_head->job).plausible_time-time <(float)find_min_arrival()-time && Q_Hyper_head->next != NULL && finishTask)
					//Q_Hyper_head = Q_Hyper_head->next;					
				//fprintf(fpout,"sahil2:plau %f\n",(Q_Hyper_head->job).plausible_time);				
				fprintf(fpout,"zzzzz... from %f to %f \n", time, decision_pt);

				fprintf(fp_excel_sched,"%f\t%f\t____\tSleep\t____\n",time,decision_pt);				
				sleepenergy += 1*SleepEnergy;
				sleepTimeTotal += (decision_pt - time);
				//sleepFlag = 1;
				int task=0;
				while(task<no_of_tasks)
				{
					if((int)(time)/t[task].period != (int)(decision_pt)/t[task].period && (int)decision_pt%t[task].period)
					{
						z = create_node(z);
						instance = (int)decision_pt/t[task].period;
						(z->job).id = t[task].id;
						(z->job).release_time = instance*t[task].period;
						(z->job).max_computation_time = job[task][instance].max_computation_time;
						(z->job).deadline = job[task][instance].deadline;
						//(z->job).period = t[task].period;
						(z->job).period = t[task].period;
						(z->job).remaining_time = (float)job[task][instance].remaining_time;
						(z->job).abs_rem = (float)job[task][instance].abs_rem;
						//(z->job).ui = (float)(z->job).max_computation_time / t[task].period;
						(z->job).ui = (float)(z->job).max_computation_time / (z->job).period;
						job[task][instance].ui = (z->job).ui;
						(z->job).plausible_time = (float)(job[task][instance].deadline - job[task][instance].max_computation_time);
						(z->job).start_time = (float)job[task][instance].release_time;
						(z->job).slack = 0;
						(z->job).execution_time = 0;
						(z->job).invocation = (float)job[task][instance].invocation;
						z->job.instance = z->job.release_time / z->job.period;
					//	z->job.plausible_time = get_plau(z,Q_Hyper_head);
			update_deadline(Q_Hyper_head,z);
						Enqueue(z);
						t[task].release_time += t[task].period;
						finishTask=1;
					
						fprintf(fpout,"Delayed arrival of T[%d] at %f with deadline %d (rls %d)\n",task,decision_pt,job[task][instance].deadline,t[task].release_time);
						fprintf(fpout,"DELAYED ARR:T[%d] %d %d\n",t[task].release_time/t[task].period,(int)(time)/t[task].period,(int)(decision_pt)/t[task].period);
						fprintf(fp_excel_sched,"%f\t_______\tT[%d][%d]\tDelayedArrival\t____\n",decision_pt,task+1,t[task].release_time/t[task].period);
					}
					task++;
				}
				time = decision_pt;
				
			}
			else
			{
				if(decision_pt == (Q_Hyper_head->job).plausible_time) {
				operating_frequency = 1;
				alpha = operating_frequency;}
				else
				{
					select_frequency();
					alpha = operating_frequency; }
					if(alpha != prev_alpha)
					{
						no_of_freq_switches++;
						prev_alpha = alpha;
					}
				
				//sleepFlag = 0;
			}
			fprintf(fpout,"Operating Frequency : %f\n",operating_frequency);
			
			if(Q->front!=NULL)
			{
				(Q->front->job).remaining_time = (float)((Q->front->job).abs_rem)/alpha;
			}
			
			if((Q->front != NULL) && (Q->front->next != NULL) && (Q->front->job).id != (Q->front->next->job).id)
			{
				cache_impacts ++;
				fprintf(fpout,"CACHE IMPACT NO. ***** %d\n",cache_impacts);
			}
		
			flag = 1;
		}
		
		if(decision_pt == min_arrival)
		{
			if(!(IsEmpty()) && flag != 1)
			{
				(Q->front->job).remaining_time = (Q->front->job).remaining_time - (decision_pt-time_bef_dec);
				total_exec_tasks[(Q->front->job).id] += decision_pt-time_bef_dec;
				for(freq_find=0; freq_find<no_of_freq; freq_find++)
				{
					if(alpha == freq[freq_find])
					break;
				}
			
				time_per_freq[freq_find] += (decision_pt-time_bef_dec);
				(Q->front->job).execution_time += decision_pt-time_bef_dec;
			
				(Q->front->job).abs_rem = (Q->front->job).abs_rem - ((decision_pt-time_bef_dec)*alpha);
				fprintf(fpout,"Remaining time : %f\n",(Q->front->job).remaining_time);
				
				energy += (float)Volts*Volts*Freq*alpha*alpha*alpha*(decision_pt - time_bef_dec);
				//fprintf(fpout,"ENERGY ******ARR BEF DEP IS ***** %f\n",energy);
			}
			time = decision_pt;
			i = 0;
			while(i<no_of_tasks)
			{
				check = min_arrival%(t[i].period);
				if(check == 0)
				{
					z = create_node(z);
					instance = decision_pt/t[i].period;
					(z->job).id = t[i].id;
					(z->job).release_time = decision_pt;
					(z->job).max_computation_time = job[i][instance].max_computation_time;
					(z->job).deadline = job[i][instance].deadline;
					(z->job).period = t[i].period;
					(z->job).remaining_time = (float)job[i][instance].remaining_time;
					(z->job).abs_rem = (float)job[i][instance].abs_rem;
					(z->job).ui = (float)(z->job).max_computation_time / t[i].period;
					job[i][instance].ui = (z->job).ui;
					(z->job).plausible_time = (float)(job[i][instance].deadline - job[i][instance].max_computation_time);
				//		z->job.plausible_time = get_plau(z,Q_Hyper_head);
					(z->job).start_time = (float)job[i][instance].release_time;
					(z->job).slack = 0;
					(z->job).execution_time = 0;
					(z->job).invocation = (float)job[i][instance].invocation;
					z->job.instance = z->job.release_time / z->job.period;
			update_deadline(Q_Hyper_head,z);
					Enqueue(z);
					t[i].release_time += t[i].period;
					fprintf(fpout,"Job of task%d arrived with deadline %d at time %f (fT %d)\n",i+1,(z->job).deadline,time,finishTask);
					fprintf(fp_excel_sched,"%f\t_______\tT[%d][%d]\tArrival\t____\n",time,i+1,t[i].release_time/t[i].period);
					//printf("Job of task%d arrived with deadline %d\n",i+1,(z->job).deadline);
					fprintf(fpout,"\n\n");
			//to avoid unnecessary provisional execution, reset finishTask flag
					finishTask = 1;
				}
				i++;
				
				
			}
			
			//Print_Queue();
			//Calc_Plausible_Time();
			//Calc_Urgent_Time();
			
						
//ORIG			if(Q_Hyper_head->next != NULL && time > (Q_Hyper_head->job).plausible_time)
			//fprintf(fpout,"NEW CONDITION %f = %f + %f\n",time , (Q_Hyper_head->job).plausible_time,(Q->front->job).invocation);
			if(Q_Hyper_head->next != NULL && time >= (Q_Hyper_head->job).plausible_time+(Q->front->job).invocation && finishTask)
			{
					//fprintf(fpout,"NEW CONDITION SATISFIED\n");
					//Q_Hyper_head = Q_Hyper_head->next;
			}
			if(Check_Sleep(Q_Hyper_head) && finishTask) //&& prev task finished
			{
				operating_frequency = 1;
				alpha = operating_frequency;
				if(alpha != prev_alpha)
				{
					no_of_freq_switches++;
					prev_alpha = alpha;
				}
				//decision_pt = findMin(((Q_Hyper_head->job).plausible_time - time),((float)find_min_arrival()-time)) + time;
				decision_pt = (Q_Hyper_head->job).plausible_time;
				//if((Q_Hyper_head->job).plausible_time-time < (float)find_min_arrival()-time && Q_Hyper_head->next != NULL && finishTask)
					//Q_Hyper_head = Q_Hyper_head->next;				
				fprintf(fpout,"zzzzz... from %f to %f\n", time, decision_pt);
				fprintf(fp_excel_sched,"%f\t%f\t____\tSleep\t____\n",time,decision_pt);
				sleepenergy += 1*SleepEnergy;
				sleepTimeTotal += (decision_pt - time);
				//sleepFlag = 1;
				int task=0;
				while(task<no_of_tasks)
				{
					if((int)(time)/t[task].period != (int)(decision_pt)/t[task].period && (int)decision_pt % t[task].period)
					{
						z = create_node(z);
						instance = (int)decision_pt/t[task].period;
						(z->job).id = t[task].id;
						(z->job).release_time = instance*t[task].period;
						(z->job).max_computation_time = job[task][instance].max_computation_time;
						(z->job).deadline = job[task][instance].deadline;
						//(z->job).period = t[task].period;
						(z->job).period = t[task].period;
						(z->job).remaining_time = (float)job[task][instance].remaining_time;
						(z->job).abs_rem = (float)job[task][instance].abs_rem;
						//(z->job).ui = (float)(z->job).max_computation_time / t[task].period;
						(z->job).ui = (float)(z->job).max_computation_time / (z->job).period;
						job[task][instance].ui = (z->job).ui;
						(z->job).plausible_time = (float)(job[task][instance].deadline - job[task][instance].max_computation_time);
						//z->job.plausible_time = get_plau(z,Q_Hyper_head);
						(z->job).start_time = (float)job[task][instance].release_time;
						(z->job).slack = 0;
						(z->job).execution_time = 0;
						(z->job).invocation = (float)job[task][instance].invocation;
						z->job.instance = z->job.release_time / z->job.period;
				update_deadline(Q_Hyper_head,z);
						Enqueue(z);
						t[task].release_time += t[task].period;
						finishTask=1;
					
						fprintf(fpout,"Delayed arrival of T[%d] at %f with deadline %d (rls %d)\n",task,decision_pt,job[task][instance].deadline,t[task].release_time);
						fprintf(fpout,"DELAYED ARR:T[%d] %d %d\n",t[task].release_time/t[task].period,(int)(time)/t[task].period,(int)(decision_pt)/t[task].period);
						fprintf(fp_excel_sched,"%f\t_______\tT[%d][%d]\tDelayedArrival\t____\n",decision_pt,task+1,t[task].release_time/t[task].period);
					}
					task++;
				}
				time = decision_pt;
			}
			else if(time < (Q_Hyper_head->job).plausible_time) //if min_arrival = next plausible then ~@~
			{
				if(decision_pt == (Q_Hyper_head->job).plausible_time){
				operating_frequency = 1;
				alpha = operating_frequency; }
				else
				{
					select_frequency();
					alpha = operating_frequency; }
					if(alpha != prev_alpha)
					{
						no_of_freq_switches++;
						prev_alpha = alpha;
					}
				
				//sleepFlag = 0;
			}

			fprintf(fpout,"ALPHA : %f\n",alpha);
			if(Q->front!=NULL)
			{
				(Q->front->job).remaining_time = (float)((Q->front->job).abs_rem)/alpha;
			}
			
			if((Q->front != NULL) && (Q->front->next != NULL) && (Q->front->job).id != (Q->front->next->job).id)
			{
				cache_impacts ++;
				fprintf(fpout,"CACHE IMPACT NO. ***** %d\n",cache_impacts);
			}
		
		}
		
		flag = 0;
		time = decision_pt;
		fprintf(fpout,"Time : %f (plau[%d] %f)\n",time,(Q_Hyper_head->job).id,(Q_Hyper_head->job).plausible_time);	
		fprintf(fpout,"\n\n");
		//Print_Current_Status();
	
//		if(time==2920)	usleep(1000000);
		fflush(fpout);

	}
			
}
