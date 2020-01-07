#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <errno.h>

volatile int Num_of_threads;
volatile int BigN, NumOfBlocks;
volatile int *Array_of_Num;
volatile int GCounter = 0;
pthread_mutex_t UPDMutex;
volatile bool ENDTIME = false;



struct blck_elem
{
	int sorted;
	int locked;
	int size;
	int *arr;
	int maininpair;
};

struct blck_elem *blocks;
void readfile();
int optimaize();
void IsItDone();
void mergeSortRecursive(int left, int right);
void merge(int left, int right);
void merge_mul(int idx);
void Updating();

void *Thread_entry(void* param)
{
	volatile int i, j;
	while (!ENDTIME)
	{
		pthread_mutex_lock(&UPDMutex);
		j = 0;
		for(i = 0; i < NumOfBlocks; i += 2)
		{
			if (blocks[i].sorted != 1 && blocks[i].locked != 1)
			{
				if (NumOfBlocks == 1)
					break;
				blocks[i].locked = 1;
				if (i + 1 != NumOfBlocks)
					blocks[i + 1].locked = 1;
				merge_mul(i);
				break;
			}
			j++;
		}

		if (NumOfBlocks == 1)
		{
			ENDTIME == true;
			pthread_mutex_unlock(&UPDMutex);
			break;
		}

		j = 0;
		for(i = 0; i < NumOfBlocks; i++)
		{
			if (blocks[i].sorted == 1)
				j++;
		}
		if (j == NumOfBlocks)
		{
			Updating();
			pthread_mutex_unlock(&UPDMutex);
		}
		else
		{
			pthread_mutex_unlock(&UPDMutex);
		}
	}
	return 0;
}

int main()
{
	/*HANDLE*/pthread_t *handles;
	int NumThreadsToCreate, i;
	time_t t;
	double time;
	readfile();
	int temp;
	NumThreadsToCreate = optimaize();

	if (Num_of_threads == 1 || NumThreadsToCreate <= 1)
	{
		t = clock();
		mergeSortRecursive(0,BigN);
		time = 1000 * ((double)clock() - (double)t) / (double)CLOCKS_PER_SEC;
		FILE *OutFile, *timeFile;
		if (OutFile = fopen("output.txt", "w+"))
		{
			fprintf(OutFile, "%d\n", Num_of_threads);
			fprintf(OutFile, "%d\n", BigN);
			for (i = 0; i < BigN; i++)
			{
				if (i == BigN - 1)
				{
					fprintf(OutFile, "%d\n", Array_of_Num[i]);
					break;
				}
				fprintf(OutFile, "%d ", Array_of_Num[i]);
			}
			fclose(OutFile);
		}
		else
			printf("Error: can`t open output.txt\n");


		if (timeFile = fopen("time.txt", "w+"))
		{
			fprintf(timeFile, "%d\n", (unsigned int)time);
			fclose(timeFile);
		}
		else
			printf("Error: can`t open time.txt\n");

		return 0;
	}

	handles = (pthread_t*)malloc(NumThreadsToCreate * sizeof(pthread_t));
	pthread_mutex_init(&UPDMutex,0);
	for (i = 0; i < NumOfBlocks; i++)
	{
		if (blocks[i].arr[0] > blocks[i].arr[1])
		{
			temp = blocks[i].arr[0];
			blocks[i].arr[0] = blocks[i].arr[1];
			blocks[i].arr[1] = temp;
			
		}
	}
	temp = Num_of_threads;
	Num_of_threads = NumThreadsToCreate;

	
	t = clock();
	for (i = 0; i < NumThreadsToCreate; i++)
	{
		if (0 !=  pthread_create(&handles[i], NULL, Thread_entry, new int(i)))
		{
			printf("CreateThread failed. GetLastError(): %u\n", errno);
			return -1;
		}
	}
	// Îæèäàíèå çàâåðøåíèÿ ïîòîêîâ
	for (i = 0; i < NumThreadsToCreate; i++)
		pthread_join(handles[i], 0); 
	time = 1000 * ((double)clock() - (double)t) / (double)CLOCKS_PER_SEC;

	printf("All threads finished analyzing\n");
	// Çàêðûòèå îïèñàòåëåé ñîçäàííûõ ïîòîêîâ
	pthread_mutex_destroy(&UPDMutex);
	FILE *OutFile, *timeFile;
	if (OutFile = fopen("output.txt", "w+"))
	{
		fprintf(OutFile, "%d\n", temp);
		fprintf(OutFile, "%d\n", BigN);
		for (i = 0; i < BigN; i++)
		{
			if (i == BigN - 1)
			{
				fprintf(OutFile, "%d\n", blocks[0].arr[i]);
				break;
			}
			fprintf(OutFile, "%d ", blocks[0].arr[i]);
		}
		fclose(OutFile);
	}
	else
		printf("Error: can`t open output.txt\n");
	if (timeFile = fopen("time.txt", "w+"))
	{
		fprintf(timeFile, "%d\n", (unsigned int)time);
		fclose(timeFile);
	}
	else
		printf("Error: can`t open time.txt\n");
	return 0;

}

void readfile()
{
	FILE* f;
	int i,j=0,k;
	if (f = fopen("input.txt", "r+"))
	{
		fscanf(f, "%d", &Num_of_threads);
		fscanf(f, "%d", &BigN);
		if (BigN < 1000)
		{
			Array_of_Num = (volatile int*)malloc(BigN * sizeof(volatile int));
			for (i = 0; i < BigN; i++)
				fscanf(f, "%d", &Array_of_Num[i]);
		}
		else
		{
			NumOfBlocks = BigN / 2;
			if (NumOfBlocks * 2 < BigN)
				NumOfBlocks++;
			blocks = (struct blck_elem*)malloc(NumOfBlocks * sizeof(struct blck_elem));
			for ( i = 0; i < NumOfBlocks; i++)
			{
				blocks[i].sorted = 0;
				blocks[i].locked = 0;
				blocks[i].maininpair = 0;
				blocks[i].arr = (int*)malloc(2 * sizeof(int));
			}
			for (i = 0; i < BigN; i++)
			{
				k = i % 2;
				if (i > 0 && i % 2 == 0)
					j++;
				fscanf(f, "%d", &blocks[j].arr[k]);
				blocks[j].size = k + 1;
			}
		}
		
	}
	else
		printf("Error: can't open input.txt\n");
	fclose(f);
}

void merge(int left, int right)
{
	int i = 0, j = 0,k, mid = (right + left) / 2;
	int *buf = (int*)malloc((right - left) * sizeof(int));

	while ((left + i < mid) && (mid + j < right))
	{
		if (Array_of_Num[left + i] < Array_of_Num[mid + j])
		{
			buf[i + j] = Array_of_Num[left + i];
			i++;
		}
		else
		{
			buf[i + j] = Array_of_Num[mid + j];
			j++;
		}
	}
	while (left + i < mid)
	{
		buf[i + j] = Array_of_Num[left + i];
		i++;
	}
	while (mid + j < right)
	{
		buf[i + j] = Array_of_Num[mid + j];
		j++;
	}
	for (k = 0; k < i + j; k++)
		Array_of_Num[left + k] = buf[k];
}
void mergeSortRecursive(int left, int right)
{
	if (left + 1 >= right)
		return;
	int mid = (left + right) / 2;
	mergeSortRecursive(left, mid);
	mergeSortRecursive(mid, right);
	merge(left, right);
}
void merge_mul(int idx)
{
	if (idx == NumOfBlocks-1)
	{
		blocks[idx].maininpair = 1;
		blocks[idx].sorted = 1;
		blocks[idx].locked = 0;
		return;
	}
	int *buf, *mas1, *mas2;
	int j = 0, k = 0, i = 0;
	int size1 = blocks[idx].size, size2 = blocks[idx + 1].size;
	mas1 = (int*)malloc(size1 * sizeof(int));
	mas2 = (int*)malloc(size2 * sizeof(int));
	for (i = 0; i < size1; i++)
		mas1[i] = blocks[idx].arr[i];
	for (j = 0; j < size2; j++)
		mas2[j] = blocks[idx + 1].arr[j];
	buf = (int*)malloc((size1 + size2)*sizeof(int));
	j = i = k = 0;
	while(i < size1 && j < size2)
	{
		if(mas1[i] < mas2[j])
		{
			buf[k] = mas1[i];
			k++;
			i++;
		}
		else
		{
		buf[k] = mas2[j];
		k++;
		j++;
		}
	}
	while(i < size1)
	{
		buf[k] = mas1[i];
		k++;
		i++;
	}
	while(j < size2)
	{
		buf[k] = mas2[j];
		k++;
		j++;
	}
	blocks[idx].arr = (int*)malloc((size1+size2)*sizeof(int));
	for (i = 0; i < size1 + size2; i++)
		blocks[idx].arr[i] = buf[i];
	blocks[idx].size = size1 + size2;
	blocks[idx + 1].sorted = 1;
	blocks[idx + 1].locked = 0;
	blocks[idx + 1].maininpair = -1;
	blocks[idx].maininpair = 1;
	blocks[idx].sorted = 1;
	blocks[idx].locked = 0;

}

void Updating()
{
	printf("U P D A T I N G \n");
	int i,n = 1,j,k = 1,l; // Ìá ïðîáëåìû òóò ïðè ïåðåñòðîéêå 
	for (i = 0; i < NumOfBlocks; i++)
	{
		
		if (blocks[i].maininpair == -1 )
		{		
			j = i + 1;
			while (j < NumOfBlocks)
			{

				if (blocks[j].maininpair == 1)
				{
					blocks[i].sorted = 0;
					blocks[i].locked = 0;
					blocks[i].maininpair = 0;
					blocks[i].size = blocks[j].size;
					blocks[i].arr = (int*)malloc(blocks[i].size * sizeof(int));
					for (k = 0; k < blocks[i].size; k++)
						blocks[i].arr[k] = blocks[j].arr[k];
					blocks[j].maininpair = -1;
					n++;
					break;
				}
				j++;
			}
			if (j == NumOfBlocks)
				break;
		}
	}
	if (n * 2 < NumOfBlocks)
		n++;
	NumOfBlocks = n;
	blocks = (struct blck_elem*)realloc(blocks,NumOfBlocks * sizeof(struct blck_elem));
	blocks[0].sorted = 0;

}
void IsItDone()
{
	int i = 0;
	while (blocks[i].sorted == 1 && i < NumOfBlocks)
	{
		i++;
	}
	if (i == NumOfBlocks)
		ENDTIME = true;
}
int optimaize()
{
	int temp = BigN / 16, thrds = Num_of_threads;
	if(BigN < 1000)
		return 1;
	while (thrds > temp)
		thrds--;
	return thrds;
}