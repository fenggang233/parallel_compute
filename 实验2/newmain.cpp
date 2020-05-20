#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int num_car = 100000;
int num_cycle[] = { 2000,500,300 };

const int v0 = 4, vmax = 8, p = 5;

typedef struct car
{
	int speed;
	int distance;
	int poisition;
}car;
car cars[100000];

int count[10] = { 0,0,0,0,0, 0,0,0,0,0 };
int pos_count[20 * 8 + 10000 * 8] = { 0 };

int main(int argc, char *argv[])
{

	FILE*fp = fopen("result.txt", "w");
	FILE*fp2 = fopen("statistic.txt", "w");

	freopen("out.txt", "w", stdout);
	int i = 0;
	for (i = 0; i<num_car; i++){
		cars[i].speed = v0;
		cars[i].poisition = vmax * i;
		cars[i].distance = vmax;
	}

	int     my_rank, numprocs;
	clock_t starttime, endtime;


	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	int* mpi_buffer = (int *)malloc(sizeof(int) * 1000000);
	MPI_Buffer_attach(mpi_buffer, sizeof(int) * 1000000);

	starttime = clock();
	int j = 0;
	for (j = 0; j<2000; j++){
		i = (num_car / numprocs * my_rank);
		if (my_rank != 0)MPI_Bsend(&(cars[i].poisition), 1, MPI_INT, my_rank - 1, j * 10 + my_rank, MPI_COMM_WORLD);

		for (; i<num_car / numprocs * (my_rank + 1) - 1; i++){
			cars[i].distance = cars[i + 1].poisition - cars[i].poisition;
			if (cars[i].distance>cars[i].speed && cars[i].speed<vmax)cars[i].speed++;
			if (cars[i].distance <= cars[i].speed)cars[i].speed = cars[i].distance - 1;
			srand(i*num_car + j);
			if (cars[i].speed>1){
				int r = rand() % 10;
				if (r<p)cars[i].speed--;
			}
			//更新位置
			cars[i].poisition += cars[i].speed;

		}
		if (my_rank != numprocs - 1){
			int temp;
			MPI_Status status;
			MPI_Recv(&(temp), 1, MPI_INT, my_rank + 1, j * 10 + my_rank + 1, MPI_COMM_WORLD, &status);
			//更新距离
			cars[i].distance = temp - cars[i].poisition;
		}
		//更新速度
		if (cars[i].speed<vmax)cars[i].speed++;
		if (cars[i].distance <= cars[i].speed)cars[i].speed = cars[i].distance - 1;
		srand((unsigned)time(NULL));
		if (cars[i].speed>1 && rand() % 10< p)cars[i].speed--;

		//更新位置
		cars[i].poisition += cars[i].speed;
	}

	endtime = clock();
	printf("rank=%d time:%lf\n", my_rank, (double)(endtime - starttime) / CLOCKS_PER_SEC);
	
	MPI_Barrier(MPI_COMM_WORLD);

	if (my_rank != numprocs-1)MPI_Send((cars + my_rank * num_car / numprocs), sizeof(car)*num_car / numprocs, MPI_BYTE, numprocs-1, my_rank, MPI_COMM_WORLD);
	else {
		MPI_Status status;
		for(int j=0;j<numprocs-1;j++)MPI_Recv((cars + my_rank * num_car / numprocs), sizeof(car)*num_car / numprocs, MPI_BYTE, j, j, MPI_COMM_WORLD, &status);

		int a;
		for (a = 0; a<num_car; a++){
			fprintf(fp, "%d %d:%d %d %d\n", my_rank, a, cars[a].speed, cars[a].poisition, cars[a].distance);
		}

		for (i = 0; i<num_car; i++){
			count[cars[i].speed]++;
			pos_count[cars[i].poisition / 1000]++;
		}

		int k;
		for (k = 0; k<10; k++){
			fprintf(fp2, "%d\t:%d\n", k, count[k]);
		}
		for (k = 0; k<2 * 8 + num_car * 8 / 1000; k++){
			fprintf(fp2, "%d\t%d\n", k, pos_count[k]);
		}
	}
	
	

	MPI_Barrier(MPI_COMM_WORLD);
	fclose(fp);
	fclose(fp2);
	fclose(stdout);
	MPI_Finalize();
	


	return 0;
}