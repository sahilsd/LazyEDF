#include "EDF_Algo_DVS.h"

int main(int argc, char *argv[])
{
	int i = 0, j = 0;
	node *z;
	int instances;
	energy = 0;
	preemptions = 0;
	latency = 0;
	cache_impacts = 0;
	zzzTime = 1.0000;
	create_Queue();
	prev_alpha = 1;
	alpha = 1;
	operating_frequency = 1;
	sleepFlag = 0;
	finishTask=0;
	
	fpin = fopen(argv[1],"r+");
	fpout = fopen("schedule.txt","a+");
	fp_job = fopen(argv[2],"r+");
	fp_freq = fopen("frequencies.txt","r+");
	fp_param = fopen("parameters.txt","a+");
	fp_lat = fopen("latencies.txt","a+");
	fp_time_per_freq = fopen("time_per_freq.txt","a+");
	fp_output = fopen("output_param.txt","a+");
	fp_execution_times = fopen("execution_times.txt","a+");
	fp_excel_sched = fopen("table_schedule.txt","a+");
	fp_plau = fopen("plausibles.txt","a+");
//	plau_debug = fopen("plau_debug.txt","a+");
	fprintf(fp_excel_sched,"Start\tFinish\tTask\tCPU Decision\tFrequency\n\n");
	
	fscanf(fpin,"%d",&no_of_tasks);
	t = (task *)malloc(no_of_tasks*sizeof(task));
	job = (job_instance **)malloc(no_of_tasks*sizeof(job_instance *));
	min_exec_tasks = (float *)malloc(no_of_tasks * sizeof(float));
	max_exec_tasks = (float *)malloc(no_of_tasks * sizeof(float));
	total_exec_tasks = (float *)malloc(no_of_tasks * sizeof(float));
	avg_exec_tasks = (float *)malloc(no_of_tasks * sizeof(float));
	min_lat = (float *)malloc(no_of_tasks * sizeof(float));
	max_lat = (float *)malloc(no_of_tasks * sizeof(float));
	avg_lat = (float *)malloc(no_of_tasks * sizeof(float));
	total_lat = (float *)malloc(no_of_tasks * sizeof(float));
	fprintf(fpout,"The no. of tasks are %d\n\n\n",no_of_tasks);
	
	fscanf(fp_freq,"%d",&no_of_freq);
	time_per_freq = (float *)malloc(no_of_freq * sizeof(float));
	freq = (float *)malloc(no_of_freq * sizeof(float));
	
	for(i=0; i<no_of_freq; i++)
	{
		fscanf(fp_freq,"%f",&freq[i]);
		time_per_freq[i] = 0;
	}
	
	fprintf(fpout, "The available frequencies are %d\n",no_of_freq);
	for(i=0; i<no_of_freq; i++)
	fprintf(fpout,"%f\n",freq[i]);
	fprintf(fpout,"\n\n");
	fprintf(fp_plau,"T[i][j]\tRls\tPlau\tDeadline\truntime\n\n");
	
	time = 0;
	min_arrival = 9999999;
	curr_exec = 9999999;
	/*creating tasks*/
	for(i=0; i<no_of_tasks; i++)
	{
		fscanf(fpin,"%d %d %d %f %d", &t[i].id, &t[i].release_time, &t[i].period, &t[i].max_computation_time,  &t[i].deadline);
		min_exec_tasks[i] = 99999;
		max_exec_tasks[i] = -99999;
		min_lat[i] = 99999;
		max_lat[i] = -99999;
	}
	
	find_hyperperiod();
	for(i=0; i<no_of_tasks; i++)
	{
		instances = hyperperiod/t[i].period;
		job[i] = (job_instance *)malloc(instances*sizeof(job_instance));
		for(j=0; j<instances; j++)
		{
			job[i][j].id = t[i].id;
			job[i][j].release_time = t[i].period * j;
			job[i][j].period = t[i].period;
			fscanf(fp_job,"%f",&job[i][j].max_computation_time);
			job[i][j].deadline = t[i].deadline * (j+1);
			fscanf(fp_job,"%f",&job[i][j].invocation);
			job[i][j].remaining_time = (float)job[i][j].invocation;
			job[i][j].abs_rem = (float)job[i][j].invocation;
			job[i][j].plausible_time = (float)(job[i][j].deadline - job[i][j].max_computation_time);
			job[i][j].start_time = (float)job[i][j].release_time;
			job[i][j].slack = 0;
			job[i][j].execution_time = 0;
		}
	}
	
	//select_frequency();
	//alpha = operating_frequency;
	//fprintf(fpout,"Operating Frequency : %f\n\n",operating_frequency);

	fprintf(fp_lat,"The latencies of the tasks are : \n\n\n");
	
	
	/*preprocessing*/
	create_Hyper_Queue();
	i = 0;
	hyper = fopen("Hyper_debug.txt","a+");
	while(i<no_of_tasks)
	{
		instances = hyperperiod/t[i].period;
		for(j=0; j<instances; j++)
		{
			z = create_node(z);
			(z->job).id = t[i].id;
			(z->job).release_time = job[i][j].release_time;
			(z->job).max_computation_time = job[i][j].max_computation_time;
			(z->job).deadline = job[i][j].deadline;
			(z->job).period = t[i].period;
			(z->job).remaining_time = (float)job[i][j].remaining_time;
			//(z->job).ui = (float)(z->job).max_computation_time / t[i].period;
			//job[i][instance].ui = (z->job).ui;
			(z->job).abs_rem = (float)job[i][j].invocation;
			(z->job).plausible_time = (float)(job[i][j].deadline - job[i][j].max_computation_time);
			(z->job).start_time = (float)job[i][j].release_time;
			(z->job).slack = 0;
			(z->job).execution_time = 0;
			z->job.instance = z->job.release_time / z->job.period;
			fprintf(hyper,"Enqueued T[%d][%d-%d] (r-%d, dd-%d, plau-%f)\n",t[i].id,z->job.id,job[i][j].release_time/t[i].period,job[i][j].release_time,job[i][j].deadline,(z->job).plausible_time);
			Enqueue_Hyper(z);
		}
		i++;
	}

	Calc_Plausible_Time_Hyper();
	printf("xyz\n");
	Calc_Urgent_Time_Hyper();
	Calc_Slack();
	Schedule();	
	
	fprintf(fp_param,"\n\nThe parameters of the algorithm are :\n\n\n");
	fprintf(fp_param,"Total Energy consumption per unit capacitance is : %f\n",energy);
	fprintf(fp_param,"Total Sleep Energy is : %f\n",sleepenergy);
	//fprintf(fp_param,"Total Idle Energy is : %f\n",idleenergy);
	fprintf(fp_param, "Total sleep time is : %f\n", sleepTimeTotal);
	fprintf(fp_param, "Total Static Energy is : %f\n",(StaticEnergy * (hyperperiod - sleepTimeTotal)));
	fprintf(fp_param,"Total Energy is : %f\n",energy+sleepenergy+(StaticEnergy * (hyperperiod - sleepTimeTotal)));
	fprintf(fp_time_per_freq,"The Time per frequency is :\n\n");
	for(i=0; i<no_of_freq; i++)
	{
		fprintf(fp_time_per_freq,"Freq[%f] : %f \n\n",freq[i],time_per_freq[i]);
	}
	fprintf(fp_time_per_freq,"Sleeptime : %f\n",sleepTimeTotal);
	
	fprintf(fp_time_per_freq,"The Time per Voltage is :\n\n");
	for(i=0; i<no_of_freq; i++)
	{	
		fprintf(fp_time_per_freq,"Voltage[%f] : %f \n\n",freq[i]*Volts,time_per_freq[i]);
	}	
	
	for(i=0; i<no_of_tasks; i++)
	{
		avg_lat[i] = (total_lat[i] / (hyperperiod/t[i].period));
		avg_exec_tasks[i] = (total_exec_tasks[i] / (hyperperiod/t[i].period));
	}
//	fprintf(fp_output,"The total number of decision points are : %d\n\n",no_of_decn_pts);
	fprintf(fp_output,"The following are total exec times of all tasks\n\n");
	fprintf(fp_output,"# of Tasks\tHyperperiod\tTotal Energy\tDecision pts\tFrequency Switches\tPreemptions\tCacheImpacts\t<Period\tWCET\tMINACET\tMAXACET\tTOTALACET\tAVGACET\tMINLATENCY\tMAXLATENCY\tAVGLATENCY>...<time per frequency>\n");
	fprintf(fp_output,"%d\t%d\t%f\t%d\t%d\t%d\t%d\t",no_of_tasks,hyperperiod,energy+sleepenergy+(StaticEnergy * (hyperperiod - sleepTimeTotal)),no_of_decn_pts, no_of_freq_switches,preemptions,cache_impacts);
	for(i=0; i<no_of_tasks; i++)
	{
		fprintf(fp_output,"%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t",t[i].period,t[i].max_computation_time,min_exec_tasks[i],max_exec_tasks[i],total_exec_tasks[i],avg_exec_tasks[i],min_lat[i],max_lat[i],avg_lat[i]);
	}
	for(i=0; i<no_of_freq; i++)
	{
		fprintf(fp_output,"Freq[%f] : %f \t",freq[i],time_per_freq[i]);
	}
	fprintf(fp_output,"\nNEW LINE\n");
	
	return 0;
}
