#include "stdafx.h"
#include <windows.h>
#include <time.h>
#include <omp.h>
#include <iostream>
#include <iomanip>
using namespace std;

//static long num_steps = 100000;//定义所分的块数
#define NUM_THREADS 4 //定义所开启的线程数
int _tmain(int argc, _TCHAR* argv[])
{
	int i;
	omp_set_num_threads(NUM_THREADS);//开启线程
	double x, sum = 0.0, pi;
	clock_t start_time, end_time, tol;
	long num_steps = 0;
	printf("N: ");
	scanf_s("%d", &num_steps);
	double step = 1.0 / (double)num_steps;

	//并行--------------------------------------
	start_time = clock();
#pragma omp parallel sections reduction(+:sum) private(x,i)
	{
#pragma omp section
		{
			for (i = omp_get_thread_num(); i<num_steps; i = i + NUM_THREADS)
			{
				x = (i + 0.5)*step;
				sum = sum + 4.0 / (1.0 + x * x);

			}
		}
#pragma omp section
		{
			for (i = omp_get_thread_num(); i<num_steps; i = i + NUM_THREADS)
			{
				x = (i + 0.5)*step;
				sum = sum + 4.0 / (1.0 + x * x);

			}
		}
#pragma omp section
		{
			for (i = omp_get_thread_num(); i<num_steps; i = i + NUM_THREADS)
			{
				x = (i + 0.5)*step;
				sum = sum + 4.0 / (1.0 + x * x);

			}
		}
#pragma omp section
		{
			for (i = omp_get_thread_num(); i<num_steps; i = i + NUM_THREADS)
			{
				x = (i + 0.5)*step;
				sum = sum + 4.0 / (1.0 + x * x);

			}
		}

	}
	pi = step * sum;
	end_time = clock();
	tol = end_time - start_time;
	cout << "Pi=" << pi << endl;
	//cout << "并行time=" << setprecision(9) <<double(tol) / CLOCKS_PER_SEC << endl;
	printf("并行time=%.9f\n", double(tol) / CLOCKS_PER_SEC);

	//串行-----------------------------------
	sum = 0.0;
	start_time = clock();
	for (i = 0; i<num_steps; i++)
	{
		x = (i + 0.5)*step;
		sum = sum + 4.0 / (1.0 + x * x);
	}
	pi = step * sum;
	end_time = clock();

	cout << "Pi=" << pi << endl;
	printf("串行time=%.9f\n", double(end_time-start_time) / CLOCKS_PER_SEC);

	system("pause");
	return 0;
}