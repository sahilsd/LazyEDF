objects = RM_Algo_DVS_Main.o RM_Q_Func.o RM_DVS.o RM_Q_Func_Hyper.o
header = EDF_Algo_DVS.h

edit : $(objects)
	cc -o runcc $(objects) $(header)
RM_Algo_Main.o : RM_Algo_Main.c
	cc -c -g RM_Algo_Main.c
RM_Q_Func.o : RM_Q_Func.c
	cc -c -g RM_Q_Func.c
RM_Q_Func_Hyper.o : RM_Q_Func_Hyper.c
	cc -c -g RM_Q_Func_Hyper.c
RM_DVS.o : RM_DVS.c
	cc -c -g RM_DVS.c
clean :
	rm runcc $(objects) schedule.txt parameters.txt latencies.txt time_per_freq.txt output_param.txt execution_times.txt table_schedule.txt plausibles.txt Hyper_debug.txt 

