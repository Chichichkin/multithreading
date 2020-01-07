#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <Windows.h>

volatile int Num_of_threads;
volatile int BigN, NumOfBlocks;
volatile int *Array_of_Num;
volatile int GCounter = 0;
HANDLE *BMutexs, BlocksMutex, ChooseMtx, ArrayMutex, UPDBLOCKMTX, UPDMutex;
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
int optimaize();
void mergeSortRecursive(int left, int right);
void merge(int left, int right);
void merge_mul(int idx);
void Updating();

DWORD WINAPI Thread_entry(void* param)
{
	// Индекс потока передается в param
	/*
	printf("\nidx:%d size:%d sorted:%d\n",i,blocks[i].size,blocks[i].sorted);
	printf("Array:\n");
	for(j = 0; J < blocks[i].size,j++)
		printf("%d, ",blocks[i].arr[j]);
	printf("\n");
	*/
	volatile int i, j;
	while (!ENDTIME)
	{
		WaitForSingleObject(ChooseMtx, INFINITE);
		//WaitForSingleObject(BlocksMutex, INFINITE);
		j = 0;
		for(i = 0; i < NumOfBlocks; i += 2)
		{
			if (blocks[i].sorted != 1 && blocks[i].locked != 1)
			{
				if (NumOfBlocks == 1)
					break;
				WaitForSingleObject(BMutexs[i], INFINITE);
				if(i+1 != NumOfBlocks)
					WaitForSingleObject(BMutexs[i + 1], INFINITE);
				if (blocks[i].sorted == 1 && blocks[i].locked == 1)
				{
					ReleaseMutex(ChooseMtx);
					//ReleaseMutex(BlocksMutex);
					ReleaseMutex(BMutexs[i]);
					if (i + 1 != NumOfBlocks)
						ReleaseMutex(BMutexs[i + 1]);
					break;
				}
				blocks[i].locked = 1;
				if (i + 1 != NumOfBlocks)
					blocks[i + 1].locked = 1;
				ReleaseMutex(ChooseMtx);
				//ReleaseMutex(BlocksMutex);
				merge_mul(i);
				ReleaseMutex(BMutexs[i]);
				if (i + 1 != NumOfBlocks)
					ReleaseMutex(BMutexs[i + 1]);
				break;
			}
			j++;
		}
		if((j * 2) + 1 >= NumOfBlocks)
		{
			j = 0;
			ReleaseMutex(ChooseMtx);
			//ReleaseMutex(BlocksMutex);
		}


		WaitForSingleObject(UPDMutex, INFINITE);

		if (NumOfBlocks == 1)
		{
			ENDTIME == true;
			break;
		}
		j = 0;
		WaitForSingleObject(BlocksMutex, INFINITE);
		j = 0;
		for(i = 0; i < NumOfBlocks; i++)
		{
			if (blocks[i].sorted == 1)
				j++;
		}
		if (j == NumOfBlocks)
		{
			Updating();
			ReleaseMutex(UPDMutex);
			ReleaseMutex(BlocksMutex);
		}
		else
		{
			ReleaseMutex(UPDMutex);
			ReleaseMutex(BlocksMutex);
		}
	}
	return 0;
}

int main()
{
	HANDLE *handles;
	int NumThreadsToCreate, i;
	time_t t;
	double time;
	readfile();
	int temp;
	NumThreadsToCreate = optimaize();

	if (Num_of_threads == 1 || NumThreadsToCreate == 1)
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

	handles = (HANDLE*)malloc(NumThreadsToCreate * sizeof(HANDLE));
	BMutexs = (HANDLE*)malloc(NumOfBlocks * sizeof(HANDLE));
	ChooseMtx = CreateMutex(NULL, FALSE, NULL);
	BlocksMutex = CreateMutex(NULL, FALSE, NULL);
	UPDMutex = CreateMutex(NULL, FALSE, NULL);

	for (i = 0; i < NumOfBlocks; i++)
	{
		BMutexs[i] = CreateMutex(NULL, FALSE, NULL);
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
		handles[i] = CreateThread(0, 0, Thread_entry, (void*)i, 0, NULL);
		if (0 == handles[i])
		{
			printf("CreateThread failed. GetLastError(): %u\n", GetLastError());
			return -1;
		}
	}
	// Ожидание завершения потоков
	WaitForMultipleObjects(NumThreadsToCreate, handles, TRUE, INFINITE);
	time = 1000 * ((double)clock() - (double)t) / (double)CLOCKS_PER_SEC;

	printf("All threads finished analyzing\n");
	// Закрытие описателей созданных потоков
	for (i = 0; i < NumThreadsToCreate; i++)
		CloseHandle(handles[i]);
	CloseHandle(BMutexs[0]);
	CloseHandle(BlocksMutex);
	CloseHandle(ChooseMtx);
	CloseHandle(UPDMutex);
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
		if (Num_of_threads == 1)
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
	WaitForSingleObject(BlocksMutex, INFINITE);
	if (idx == NumOfBlocks-1)
	{
		blocks[idx].maininpair = 1;
		blocks[idx].sorted = 1;
		blocks[idx].locked = 0;
		ReleaseMutex(BlocksMutex);
		return;
	}
	ReleaseMutex(BlocksMutex);
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
	WaitForSingleObject(BlocksMutex,INFINITE);
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
	ReleaseMutex(BlocksMutex);



}

void Updating()
{
	//printf("U P D A T I N G \n");
	int i,n = 1,j,k = 1,l; // Мб проблемы тут при перестройке 
	for (i = 0; i < NumOfBlocks; i++)
		WaitForSingleObject(BMutexs[i], INFINITE);
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
	for (i = NumOfBlocks-1; i >= n; i--)
	{
		ReleaseMutex(BMutexs[i]);
		CloseHandle(BMutexs[i]);
	}
	NumOfBlocks = n;
	blocks = (struct blck_elem*)realloc(blocks,NumOfBlocks * sizeof(struct blck_elem));
	blocks[0].sorted = 0;
	/*for (i = 0; i < NumOfBlocks; i++)
	{
		printf("\nidx:%d size:%d sorted:%d\n", i, blocks[i].size, blocks[i].sorted);
		printf("Array:\n");
		for (j = 0; j < blocks[i].size; j++)
			printf("%d, ", blocks[i].arr[j]);
		printf("\n ------------------------------------------------------\n");

	}
	printf("\n -----------------E N D   U P D A T I N G----------------------\n");*/
	for (i = 0; i < NumOfBlocks; i++)
		ReleaseMutex(BMutexs[i]);


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
	while (thrds > temp)
		thrds--;
	//if (BigN >= 100000)
		//thrds = 64;
	return thrds;
}