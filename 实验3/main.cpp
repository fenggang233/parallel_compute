#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <math.h>

typedef struct ball
{
	double px, py;
	double vx, vy;
	double ax, ay;
}ball;
ball ball_list[256];
const int N = 256;
const double GM = 6.67E-7;//G*M
int timestep = 100;
double delta_t = 0.0;
double r = 0.01;//球的半径
int cycle_times = 20000;//周期数
int size = 0;//方阵宽
FILE* fp;
char filename[4][20] = { "result1.txt","result2.txt","result3.txt","result4.txt" };

void compute_force(int index)
{
	ball_list[index].ax = 0;
	ball_list[index].ay = 0;
	for (int i = 0; i<N; i++)
	{
		if (i != index)
		{
			double dx = ball_list[i].px - ball_list[index].px;
			double dy = ball_list[i].py - ball_list[index].py;
			double d = (dx*dx + dy * dy);
			if (d<r*r)d = r * r;
			d *= sqrt(d);

			ball_list[index].ax += GM * (dx) / d;
			ball_list[index].ay += GM * (dy) / d;
			
		}

	}
	
}
void compute_velocities(int index)
{

	ball_list[index].vx += ball_list[index].ax*delta_t;
	ball_list[index].vy += ball_list[index].ay*delta_t;
	
}
void compute_positions(int index)
{

	ball_list[index].px += ball_list[index].vx*delta_t;
	if (ball_list[index].px>((size - 1) / 100.0))ball_list[index].px = (size - 1) / 100.0;
	if (ball_list[index].px<0)ball_list[index].px = 0;
	ball_list[index].py += ball_list[index].vy*delta_t;
	if (ball_list[index].py>((size - 1) / 100.0))ball_list[index].py = (size - 1) / 100.0;
	if (ball_list[index].py<0)ball_list[index].py = 0;
	
}


void main(int argc, char *argv[])
{
	
	delta_t = 1.0 / timestep;
	
	size = (int)sqrt(N);
	
	for (int i = 0; i<N; i++)
	{
		ball_list[i].px = 0.01*(i%size);
		ball_list[i].py = 0.01*(i / size);

		ball_list[i].vx = 0;
		ball_list[i].vy = 0;

		ball_list[i].ax = 0;
		ball_list[i].ay = 0;
	}
	int     myid, numprocs;
	clock_t starttime, endtime;
	int    namelen;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	double* mpi_buffer = (double*)malloc(sizeof(double) * 1000000);
	MPI_Buffer_attach(mpi_buffer, sizeof(double) * 1000000);

	
	starttime = clock();
	for (int i = 0; i<cycle_times; i++)
	{
		for (int j = 0; j<numprocs; j++)
		{
			if (j != myid)
				MPI_Bsend((ball_list + (N / numprocs)*myid), sizeof(ball)*N / numprocs, MPI_BYTE, j, i * 10 + myid, MPI_COMM_WORLD);
		}
		for (int j = 0; j<numprocs; j++)
		{
			if (j != myid)
			{
				MPI_Status status;
				MPI_Recv((ball_list + (N / numprocs)*j), sizeof(ball)*N / numprocs, MPI_BYTE, j, i * 10 + j, MPI_COMM_WORLD, &status);
			}
		}
		for (int j = (N / numprocs)*myid; j<(N / numprocs)*(myid + 1); j++)
		{
			compute_force(j);
		}

		MPI_Barrier(MPI_COMM_WORLD);
		for (int j = (N / numprocs)*myid; j<(N / numprocs)*(myid + 1); j++)
		{
			compute_velocities(j);
			compute_positions(j);
		}
		
		MPI_Barrier(MPI_COMM_WORLD);
	}
	endtime = clock();
	printf("rank=%d time:%lf\n", myid, (double)(endtime - starttime) / CLOCKS_PER_SEC);
	

	if (myid != 0)
	{
		
		MPI_Send((ball_list + (N / numprocs)*myid), sizeof(ball)*N / numprocs, MPI_BYTE, 0, myid, MPI_COMM_WORLD);
	}
	if (myid == 0)
	{
		fp = fopen(filename[numprocs - 1], "w");
		for (int i = 1; i<numprocs; i++)
		{
			MPI_Status status;
			
			MPI_Recv((ball_list + (N / numprocs)*i), sizeof(ball)*N / numprocs, MPI_BYTE, i, i, MPI_COMM_WORLD, &status);
		}
		for (int i = 0; i<N; i++)
		{
		
			fprintf(fp, "%lf,%lf\n", ball_list[i].px, ball_list[i].py);
		}
		fclose(fp);
	}

	MPI_Finalize();

}