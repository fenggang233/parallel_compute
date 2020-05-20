#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include <time.h>
#include "mpi.h"

int i, j, k;
int N = 1000;



int cmp(const void * a, const void * b) {
	if (*(int*)a < *(int*)b) return -1;
	if (*(int*)a > *(int*)b) return 1;
	else return 0;
}

int main(int argc, char *argv[]) {
	freopen("res.txt", "w", stdout);
	int *array;
	array = (int *)malloc(N * sizeof(int));

	srand(100);
	for (k = 0; k < N; k++) {
		array[k] = rand() % 100000000;
	}
	MPI_Init(&argc, &argv);      //MPI��ʼ��
	int p, myId, *partitionSizes, *newPartitionSizes, nameLength;
	int subArraySize, startIndex, endIndex, *pivots, *newPartitions;
	char processorName[MPI_MAX_PROCESSOR_NAME];


	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Comm_rank(MPI_COMM_WORLD, &myId);
	MPI_Get_processor_name(processorName, &nameLength);

	printf("Process %d is on %s\n", myId, processorName);

	pivots = (int *)malloc(p * sizeof(int));
	partitionSizes = (int *)malloc(p * sizeof(int));
	newPartitionSizes = (int *)malloc(p * sizeof(int));
	for (k = 0; k < p; k++) {
		partitionSizes[k] = 0;
	}

	// ��ȡ��ʼλ�ú��������С
	startIndex = myId * N / p;
	if (p == (myId + 1)) {
		endIndex = N;
	}
	else {
		endIndex = (myId + 1) * N / p;
	}
	subArraySize = endIndex - startIndex;

	MPI_Barrier(MPI_COMM_WORLD);
	//���ø��׶κ���void phase1(int *array, int N, int startIndex, int subArraySize, int *pivots, int p)
	qsort(array + startIndex, subArraySize, sizeof(array[0]), cmp);

	for (i = 0; i < p; i++) {
		pivots[i] = array[startIndex + (i * (N / (p * p)))];
	}
	if (p > 1) {
		int *collectedPivots = (int *)malloc(p * p * sizeof(pivots[0]));
		int *phase2Pivots = (int *)malloc((p - 1) * sizeof(pivots[0]));          //��Ԫ
		int index = 0;

		//�ռ���Ϣ�������������Ľ��ܻ������а������н��̵ķ��ͻ����������ӡ�
		MPI_Gather(pivots, p, MPI_INT, collectedPivots, p, MPI_INT, 0, MPI_COMM_WORLD);
		if (myId == 0) {

			qsort(collectedPivots, p * p, sizeof(pivots[0]), cmp);          //�����������������������

																			// ��������������Ԫ��ѡ��
			for (i = 0; i < (p - 1); i++) {
				phase2Pivots[i] = collectedPivots[(((i + 1) * p) + (p / 2)) - 1];
			}
		}
		//���͹㲥
		MPI_Bcast(phase2Pivots, p - 1, MPI_INT, 0, MPI_COMM_WORLD);
		// ������Ԫ���֣������㻮�ֲ��ֵĴ�С
		for (i = 0; i < subArraySize; i++) {
			if (array[startIndex + i] > phase2Pivots[index]) {
				//�����ǰλ�õ����ִ�С������Ԫλ�ã��������һ������
				index += 1;

			}
			if (index == p) {
				//���һ�λ��֣��������ܳ�������ǰλ�ü��ɵõ����һ�������黮�ֵĴ�С
				partitionSizes[p - 1] = subArraySize - i + 1;
				break;
			}
			partitionSizes[index]++;   //���ִ�С����
		}
		free(collectedPivots);
		free(phase2Pivots);

		int totalSize = 0;
		int *sendDisp = (int *)malloc(p * sizeof(int));
		int *recvDisp = (int *)malloc(p * sizeof(int));

		// ȫ�ֵ�ȫ�ֵķ��ͣ�ÿ�����̿�����ÿ�������߷�����Ŀ��ͬ������.
		MPI_Alltoall(partitionSizes, 1, MPI_INT, newPartitionSizes, 1, MPI_INT, MPI_COMM_WORLD);

		// ���㻮�ֵ��ܴ�С�������»��ַ���ռ�
		for (i = 0; i < p; i++) {
			totalSize += newPartitionSizes[i];
		}
		newPartitions = (int *)malloc(totalSize * sizeof(int));

		// �ڷ��ͻ���֮ǰ���������sendbuf��λ�ƣ���λ�ƴ��������������̵�����
		sendDisp[0] = 0;
		recvDisp[0] = 0;      //���������recvbuf��λ�ƣ���λ�ƴ�����Ŵӽ��̽��ܵ�������
		for (i = 1; i < p; i++) {
			sendDisp[i] = partitionSizes[i - 1] + sendDisp[i - 1];
			recvDisp[i] = newPartitionSizes[i - 1] + recvDisp[i - 1];
		}

		//�������ݣ�ʵ��n�ε�Ե�ͨ��
		MPI_Alltoallv(&(array[startIndex]), partitionSizes, sendDisp, MPI_INT, newPartitions, newPartitionSizes, recvDisp, MPI_INT, MPI_COMM_WORLD);

		free(sendDisp);
		free(recvDisp);
		int *sortedSubList;
		int *indexes, *partitionEnds, *subListSizes, totalListSize;

		indexes = (int *)malloc(p * sizeof(int));
		partitionEnds = (int *)malloc(p * sizeof(int));
		indexes[0] = 0;
		totalListSize = newPartitionSizes[0];
		for (i = 1; i < p; i++) {
			totalListSize += newPartitionSizes[i];
			indexes[i] = indexes[i - 1] + newPartitionSizes[i - 1];
			partitionEnds[i - 1] = indexes[i];
		}
		partitionEnds[p - 1] = totalListSize;

		sortedSubList = (int *)malloc(totalListSize * sizeof(int));
		subListSizes = (int *)malloc(p * sizeof(int));
		recvDisp = (int *)malloc(p * sizeof(int));

		// �鲢����
		for (i = 0; i < totalListSize; i++) {
			int lowest = INT_MAX;
			int ind = -1;
			for (j = 0; j < p; j++) {
				if ((indexes[j] < partitionEnds[j]) && (newPartitions[indexes[j]] < lowest)) {
					lowest = newPartitions[indexes[j]];
					ind = j;
				}
			}
			sortedSubList[i] = lowest;
			indexes[ind] += 1;
		}

		// ���͸����б�Ĵ�С�ظ�������
		MPI_Gather(&totalListSize, 1, MPI_INT, subListSizes, 1, MPI_INT, 0, MPI_COMM_WORLD);

		// ����������ϵ������recvbuf��ƫ����
		if (myId == 0) {
			recvDisp[0] = 0;
			for (i = 1; i < p; i++) {
				recvDisp[i] = subListSizes[i - 1] + recvDisp[i - 1];
			}
		}

		//���͸��ź�������б�ظ�������
		MPI_Gatherv(sortedSubList, totalListSize, MPI_INT, array, subListSizes, recvDisp, MPI_INT, 0, MPI_COMM_WORLD);

		free(partitionEnds);
		free(sortedSubList);
		free(indexes);
		free(subListSizes);
		free(recvDisp);
	}

	if (myId == 0)
		for (k = 0; k < N; k++) {
			printf("%d ", array[k]);
		}
	printf("\n");
	if (p > 1) {
		free(newPartitions);
	}
	free(partitionSizes);
	free(newPartitionSizes);
	free(pivots);


	free(array);
	MPI_Finalize();
	fclose(stdout);

	return 0;
}
