
#include "mpi.h"
#include <stdio.h>
#include <math.h>

int isPrime(int lyy_num)   //�ж��Ƿ�Ϊ����  
{
	int lyy_flag = 1;
	int lyy_s = sqrt(lyy_num*1.0);
	for (int j = 2; j <= lyy_s; j++)
	{
		if (lyy_num%j == 0)
		{
			lyy_flag = 0;
			break;
		}
	}
	return lyy_flag;
}

void main(int argc, char * argv[])
{
	int n = 0, myid, numprocs, i, pi, sum, mypi;
	double startwtime, endwtime;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	if (myid == 0)
	{
		printf("����һ�����֣�");
		fflush(stdout);
		scanf_s("%d", &n);
		startwtime = MPI_Wtime();
	}
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);   //��nֵ�㲥��ȥ
	sum = 0;
	for (i = myid * 2 + 1; i <= n; i += numprocs * 2)
		sum += isPrime(i);
	mypi = sum;
	MPI_Reduce(&mypi, &pi, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);  //��Լ
	if (myid == 0)
	{
		printf("���=%d\n", pi);
		endwtime = MPI_Wtime();
		printf("����ʱ��=%f\n", endwtime - startwtime);
	}
	//���г���
	sum = 0;
	double startwtime2 = MPI_Wtime();
	if (myid == 0)
	{
		for (i = 1; i <= n; i += 2)
			sum += isPrime(i);
		double endwtime2 = MPI_Wtime();
		printf("���=%d\n", sum);
		printf("����ʱ��=%f\n", endwtime2 - startwtime2);
	}
	
	MPI_Finalize();
	getchar();
}
