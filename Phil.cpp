#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>


int action_time, whole_time, Food[5]={0,0,0,0,0};
int Eating1 = 1, Eating2 = 3, switchphilosopher = 2;
sem_t sem,slave;
struct timespec start, finish;

void run_all_threads(pthread_t *threads);
unsigned long to_ms(struct timespec* tm);


void *philosopher(void *params)
{
    int i;
    int idx = *(int *)params;
    idx++;
    struct timespec current_time;

    while(1)
    {
        sem_wait(&sem);
        clock_gettime(CLOCK_REALTIME, &current_time);
        if (to_ms(&current_time) - to_ms(&start) >= whole_time)
        {
            sem_post(&sem);
            break;
        }
        //printf("Phil:%d trying to get fork\n", idx);
        sem_wait(&slave);
        if ((idx == Eating1 || idx == Eating2) && switchphilosopher == 2)
        {
            //printf("Phil:%d got fork\n", idx);
            clock_gettime(CLOCK_REALTIME, &current_time);
            printf("%lu:%d:T->E\n", to_ms(&current_time) - to_ms(&start), idx);
            Food[idx-1]++;
            sem_post(&slave);
            usleep(action_time);

            sem_wait(&slave);
            clock_gettime(CLOCK_REALTIME, &current_time);
            printf("%lu:%d:E->T\n", to_ms(&current_time) - to_ms(&start), idx);
            switchphilosopher--;
            if(switchphilosopher == 0)
            {
                Eating1++;
                Eating2++;
                Eating1 = Eating1 % 6;
                Eating2 = Eating2 % 6;
                if(!Eating1)
                    Eating1++;
                if(!Eating2)
                    Eating2++;
                switchphilosopher = 2;
                sem_post(&sem);
                sem_post(&sem);
                //printf("new idx1:%d idx2:%d\n",Eating1,Eating2);
             }
            sem_post(&slave);
            //printf("Phil:%d done loop\n", idx);
            
        }
        else
        {
            sem_post(&slave);
            sem_post(&sem);
            usleep(action_time);
        }
    }
    free(params);
    pthread_exit(NULL);
}

int main(int argc, char *args[])
{
    int i;
    if (argc != 3)
    {
        printf("Wrong input\n");
        return -1;
    }
    whole_time = atoi(args[1]);
    action_time = atoi(args[2]);
    action_time *= 1000;
    pthread_t philosophers[5];
    sem_init(&sem, 0, 2);
    sem_init(&slave, 0, 1);
    
    
    clock_gettime(CLOCK_REALTIME, &start);

    for(i = 4; i > -1; i--) 
        pthread_create(&philosophers[i], NULL, philosopher, new int(i));
    for(i = 0; i < 5; i++) 
        pthread_join(philosophers[i], 0);

    clock_gettime(CLOCK_REALTIME, &finish);

    printf("Exit time: %lu ms\n", to_ms(&finish) - to_ms(&start) );   
    for (i = 0; i < 5; i++)
     {
        printf("Philosopher %d eat  %d times\n",i+1,Food[i]);
     } 
    sem_destroy(&sem);
    sem_destroy(&slave);
}

unsigned long to_ms(struct timespec* tm)
{
    return ((unsigned long) tm->tv_sec * 1000 +
            (unsigned long) tm->tv_nsec / 1000000);
}
