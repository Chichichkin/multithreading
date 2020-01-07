#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <Windows.h>

int Num_of_threads;
int BigN, MainSum;
int *Array_of_Num;
unsigned long long step;
volatile int THREADSRESULT = 0;
CRITICAL_SECTION cs;

void readfile();
int singlethread(int a, int idx);


DWORD WINAPI Thread_entry(void *param)
{
	int idx = (int)param;
	int temp = pow(2.0, (double)BigN - 1.0), j;
	unsigned long long sign, max,i;
	// мб тут
	sign = step * idx;
	max = step;
	if (idx + 1 == Num_of_threads)
		max = temp - sign;
	
	for (i = 0; i < max; i++)
	{
		temp = Array_of_Num[0];
		for (j = 1; j < BigN; j++)
		{
			if (sign & (1 << BigN - 1 - j))
			{
				temp += Array_of_Num[j];
			}
			else
			{
				temp -= Array_of_Num[j];
			}
		}
		if (temp == MainSum)
		{
			EnterCriticalSection(&cs);
			THREADSRESULT++;
			LeaveCriticalSection(&cs);
		}
		sign++;
	}
	return 0;
}

int main()
{
	HANDLE *handles;
	time_t t;
	double time;
	int NumThreadsToCreate,i;
	readfile();
	NumThreadsToCreate = Num_of_threads;
	int POW = pow(2.0, (double)BigN - 1.0);
	if (BigN - 1 > 63)
	{
		printf("too many symbols\n");
		return 0;
	}
	while (POW/ NumThreadsToCreate < 4 && NumThreadsToCreate > 1)
		NumThreadsToCreate--;
	
	if (Num_of_threads == 1 || NumThreadsToCreate == 1)
	{
		t = clock();
		int result = singlethread(Array_of_Num[0],0);
		time = 1000 * ((double)clock() - (double)t) / (double)CLOCKS_PER_SEC;
		FILE *OutFile, *timeFile;
		if (OutFile = fopen("output.txt", "w+"))
		{
			fprintf(OutFile, "%d\n", Num_of_threads);
			fprintf(OutFile, "%d\n", BigN);
			fprintf(OutFile, "%d\n", result);
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

	step = floor(pow(2.0, (double)BigN - 1.0) / (double)NumThreadsToCreate);
	handles = (HANDLE*)malloc(NumThreadsToCreate * sizeof(HANDLE));
	int temp = Num_of_threads;
	Num_of_threads = NumThreadsToCreate;
	InitializeCriticalSection(&cs);

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
	DeleteCriticalSection(&cs);
	printf("All threads finished analyzing\n");
	// Закрытие описателей созданных потоков
	for (i = 0; i < NumThreadsToCreate; i++)
		CloseHandle(handles[i]);
	printf("%d\n", THREADSRESULT);

	FILE *OutFile, *timeFile;
	if (OutFile = fopen("output.txt", "w+"))
	{
		fprintf(OutFile, "%d\n", temp);
		fprintf(OutFile, "%d\n", BigN);
		fprintf(OutFile, "%d\n", THREADSRESULT);
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

		Array_of_Num = (int*)malloc(BigN * sizeof(int));
		i = BigN;

		for (i = 0; i < BigN; i++)
			fscanf(f, "%d", &Array_of_Num[i]);
		fscanf(f, "%d", &MainSum);
	}
	else
		printf("Error: can't open input.txt\n");
	fclose(f);
}
int singlethread(int a,int idx)
{
	int count = 0;
	if (idx != BigN - 1)
	{
		count += singlethread(a + Array_of_Num[idx + 1], idx + 1);
		count += singlethread(a - Array_of_Num[idx + 1], idx + 1);
	}
	else
		if (a == MainSum)
			count++;
	return count;

}