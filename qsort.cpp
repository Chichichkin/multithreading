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
HANDLE *mutexs, BlocksMutex,ChooseMtx,ArrayMutex,UPDBLOCKMTX;
volatile bool ENDTIME = false;



struct blck_elem
{
	int left;
	int right;
	int sorted;
	int locked;
};

struct blck_elem *blocks;
void readfile();
void qs_mul(int idx);
void qs(int first, int last);
int optimaize();
void IsItDone();

DWORD WINAPI Thread_entry(void* param)
{
	// Èíäåêñ ïîòîêà ïåðåäàåòñÿ â param
	volatile int i,j=0;
	while (!ENDTIME)
	{
		WaitForSingleObject(ChooseMtx, INFINITE);
		//printf("Thread %d got firs global lock\n",(int)param);
		WaitForSingleObject(BlocksMutex, INFINITE);
		//printf("Thread %d got both global lock\n",(int)param);
		for (i = 0; i < NumOfBlocks; i++)
		{
			if (blocks[i].locked == 0  && blocks[i].sorted == 0)
			{
				blocks[i].locked = 1;
				j = 0;
				ReleaseMutex(BlocksMutex);
				ReleaseMutex(ChooseMtx);
				qs_mul(i);
				break;
			}
			j++;
		}
		if (j == NumOfBlocks)
		{
			j = 0;
			ReleaseMutex(ChooseMtx);
			ReleaseMutex(BlocksMutex);
		}

		WaitForSingleObject(BlocksMutex, INFINITE);
		GCounter += 1;

		if (!ENDTIME)
			IsItDone();
		ReleaseMutex(BlocksMutex);
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

	NumThreadsToCreate = optimaize();



	if (Num_of_threads == 1 || NumThreadsToCreate == 1)
	{
		t = clock();
		qs(0, BigN - 1);
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
	ChooseMtx = CreateMutex(NULL, FALSE, NULL);
	BlocksMutex = CreateMutex(NULL, FALSE, NULL);
	ArrayMutex = CreateMutex(NULL, FALSE, NULL);
	UPDBLOCKMTX = CreateMutex(NULL, FALSE, NULL);

	int temp = Num_of_threads;
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
	// Îæèäàíèå çàâåðøåíèÿ ïîòîêîâ
	WaitForMultipleObjects(NumThreadsToCreate, handles, TRUE, INFINITE);
	time = 1000 * ((double)clock() - (double)t) / (double)CLOCKS_PER_SEC;

	printf("All threads finished analyzing\n");
	// Çàêðûòèå îïèñàòåëåé ñîçäàííûõ ïîòîêîâ
	for (i = 0; i < NumThreadsToCreate; i++)
		CloseHandle(handles[i]);
	CloseHandle(ArrayMutex);
	CloseHandle(BlocksMutex);
	CloseHandle(ChooseMtx);
	FILE *OutFile, *timeFile;
	if (OutFile = fopen("output.txt", "w+"))
	{
		fprintf(OutFile, "%d\n", temp);
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

void readfile()
{
	FILE* f;
	int i;
	if (f = fopen("input.txt", "r+"))
	{
		fscanf(f, "%d", &Num_of_threads);
		fscanf(f, "%d", &BigN);

		Array_of_Num = (volatile int*)malloc(BigN * sizeof(volatile int));
		i = BigN;

		for (i = 0; i < BigN; i++)
			fscanf(f, "%d", &Array_of_Num[i]);
		NumOfBlocks = 1;
		blocks = (struct blck_elem*)malloc(NumOfBlocks * sizeof(struct blck_elem));
		blocks[0].left = 0;
		blocks[0].right = BigN - 1;
		blocks[0].sorted = 0;
		blocks[0].locked = 0;

	}
	else
		printf("Error: can't open input.txt\n");
	fclose(f);
}

void qs_mul(int idx)
{
	if (blocks[idx].right - blocks[idx].left < 2)
	{
		WaitForSingleObject(BlocksMutex, INFINITE);
		WaitForSingleObject(ArrayMutex, INFINITE);
		if (Array_of_Num[blocks[idx].right] < Array_of_Num[blocks[idx].left])
		{
			int temp = Array_of_Num[blocks[idx].right];
			Array_of_Num[blocks[idx].right] = Array_of_Num[blocks[idx].left];
			Array_of_Num[blocks[idx].left] = temp;
		}
		ReleaseMutex(ArrayMutex);
		blocks[idx].sorted = 1;
		ReleaseMutex(BlocksMutex);
		return;
	}
	int mid, count, i;
	int l = blocks[idx].left, r = blocks[idx].right; 
	WaitForSingleObject(ArrayMutex, INFINITE);
	mid = Array_of_Num[(l + r) / 2]; //âû÷èñëåíèå îïîðíîãî ýëåìåíòà
	ReleaseMutex(ArrayMutex);
	do
	{
		while (Array_of_Num[l] < mid) l++;
		while (Array_of_Num[r] > mid) r--;
		if (l > blocks[idx].right || r < blocks[idx].left)
			break;
		if (l <= r) //ïåðåñòàíîâêà ýëåìåíòîâ
		{
			WaitForSingleObject(ArrayMutex, INFINITE);
			count = Array_of_Num[l];
			Array_of_Num[l] = Array_of_Num[r];
			Array_of_Num[r] = count;
			ReleaseMutex(ArrayMutex);
			l++;
			r--;
		}
	} while (l <= r);

	WaitForSingleObject(BlocksMutex, INFINITE);
	WaitForSingleObject(UPDBLOCKMTX, INFINITE);
	if (r < blocks[idx].left || blocks[idx].right < l && blocks[idx].right - blocks[idx].left < 2)
	{
		int i;
		for ( i = blocks[idx].left; i < blocks[idx].right; i++)
		{
			printf("%d ", Array_of_Num[i]);
		}
		printf("\n");
		WaitForSingleObject(ArrayMutex, INFINITE);
		if (Array_of_Num[blocks[idx].right] < Array_of_Num[blocks[idx].left])
		{
			int temp = Array_of_Num[blocks[idx].right];
			Array_of_Num[blocks[idx].right] = Array_of_Num[blocks[idx].left];
			Array_of_Num[blocks[idx].left] = temp;
		}
		ReleaseMutex(ArrayMutex);
		blocks[idx].sorted = 1;
		blocks[idx].locked = 0;
		ReleaseMutex(UPDBLOCKMTX);
		ReleaseMutex(BlocksMutex);
		return;
	}
	if (r < blocks[idx].left)
	{
		blocks[idx].left = l;
		ReleaseMutex(UPDBLOCKMTX);
		ReleaseMutex(BlocksMutex);
		return;
	}
	if (blocks[idx].right < l)
	{
		blocks[idx].right = r;
		ReleaseMutex(UPDBLOCKMTX);
		ReleaseMutex(BlocksMutex);
		return;

	}

	NumOfBlocks++;
	int numofnewblock = NumOfBlocks - 1;
	blocks = (struct blck_elem*)realloc(blocks, NumOfBlocks * sizeof(struct blck_elem));
	blocks[numofnewblock].right = blocks[idx].right;
	blocks[idx].right = r;
	blocks[numofnewblock].left = l;
	blocks[NumOfBlocks - 1].sorted = 0;
	blocks[NumOfBlocks - 1].locked = 0;
	blocks[idx].locked = 0;
	ReleaseMutex(UPDBLOCKMTX);
	ReleaseMutex(BlocksMutex);

}
void qs(int first, int last)
{
	int mid, count, i;
	int f = first, l = last;
	mid = Array_of_Num[(f + l) / 2]; //âû÷èñëåíèå îïîðíîãî ýëåìåíòà
	do
	{
		while (Array_of_Num[f] < mid) f++;
		while (Array_of_Num[l] > mid) l--;
		if (f <= l) //ïåðåñòàíîâêà ýëåìåíòîâ
		{
			count = Array_of_Num[f];
			Array_of_Num[f] = Array_of_Num[l];
			Array_of_Num[l] = count;
			f++;
			l--;
		}
	} while (f < l);
	if (first < l) qs(first, l);
	if (f < last) qs(f, last);
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
	int temp = BigN / 4, thrds = Num_of_threads;
	while (thrds > temp)
		thrds--;
	return thrds;
}