#include "stdafx.h"
#include <omp.h>
#include<Windows.h>
#include<cmath>
#include<time.h>

#define NUM_THREADS 4
int isPrime(int n)
{
	int i;
	for (i = 2; i <= sqrt(1.0*n); i++)
		if (n%i == 0)  return 0;
	return 1;
}
int _tmain(int argc, _TCHAR* argv[])
{
	int N;
	omp_set_num_threads(NUM_THREADS);
	int i, num = 0;
	clock_t t1, t2, t3, t4, t5, t6;
	printf("N: ");
	scanf_s("%d", &N);
	t1 = clock();
#pragma omp parallel for reduction(+:num)
	for (i = 2; i <= N; i++)
	{
		num += isPrime(i);
	}
	t2 = clock();
	printf("素数共有%d个\n", num);
	t5 = t2 - t1;
	printf("并行时间是%.9f\n", double(t5)/CLOCKS_PER_SEC);
	num = 0;
	t3 = clock();
	for (i = 2; i <= N; i++)
	{
		num += isPrime(i);
	}
	t4 = clock();
	t6 = t4 - t3;
	printf("素数共有%d个\n", num);
	printf("串行时间是%.9f\n", double(t6)/ CLOCKS_PER_SEC);
	printf("加速比是 %.9f", double(t6) / double(t5));
	system("pause");
	return 0;
}
