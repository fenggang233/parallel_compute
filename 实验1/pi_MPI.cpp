
#include "mpi.h"
#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[]) {
	int my_rank, num_procs;
	int i, n = 0;
	double sum, width, local, mypi, pi;
	double start = 0.0, stop = 0.0;
	int proc_len;
	//MPI_MAX_PROCESSOR_NAME是MPI预定义的宏，，即MPI所允许的机器名字的最大长度。
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Get_processor_name(processor_name, &proc_len);
	//printf("Process %d of %d\n", my_rank, num_procs);
	if (my_rank == 0) {
		scanf_s("%d", &n);
		printf("\n");
		start = MPI_Wtime();
	}
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	sum = 0.0;
	width = 1.0 / n;
	//每个进程my_rank，计算4.0/(1.0+local*local)放入sum
	for (i = my_rank; i<n; i += num_procs) {
		local = width * ((double)i + 0.5);
		sum += 4.0 / (1.0 + local * local);
	}
	mypi = width * sum;
	MPI_Reduce(&mypi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	//打印结果
	if (my_rank == 0) {
		printf("PI is %.20f\n", pi);
		stop = MPI_Wtime();
		printf("Time:%f on %s\n", stop - start, processor_name);
		fflush(stdout);
	}
	MPI_Finalize();
	getchar();
	getchar();
	getchar();
	return 0;
}
