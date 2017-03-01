#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>
#include<time.h>

#define compare(s1, s2) (strcmp(s1, s2) == 0)
#define NONE "none"
#define MUTEX "mutex"
#define SPINLOCK "spinlock"
#define READWRITELOCK "readwritelock"
#define SIGNALWAIT "signalwait"
#define SEMAPHORE "semaphore"

/*
Author: Emily Black
Date: 11/26/16

This file drives the main program.
*/

/* "volatile" prevents compiler optimizations of arithmetic operations on 'glob' */
static volatile int glob = 0;

/* Concurrency Variables */
pthread_mutex_t mutex1;
pthread_spinlock_t spinlock1;
pthread_rwlock_t rwlock1;
sem_t sem1;
pthread_cond_t cond1;

/* Loop 'arg' times incrementing 'glob' */
static void * threadFunc(void *arg)
{
    int loops = *((int *) arg);
    int loc, j;

    for (j = 0; j < loops; j++) {
        loc = glob;
        loc++;
        glob = loc;
    }

    return NULL;
}

static void * threadFuncMutex(void *arg)
{
	int loops = *((int *) arg);
	int loc, j;
	
	pthread_mutex_lock(&mutex1);
	/* Critical section */

    for (j = 0; j < loops; j++) {
        loc = glob;
        loc++;
        glob = loc;
    }
	
	/* End critical section */
    pthread_mutex_unlock(&mutex1);

    return NULL;
}

static void * threadFuncSpinlock(void *arg)
{
	int loops = *((int *) arg);
	int loc, j;
	
	pthread_spin_lock(&spinlock1);
	/* Critical section */

    for (j = 0; j < loops; j++) {
        loc = glob;
        loc++;
        glob = loc;
    }
	
	/* End critical section */
    pthread_spin_unlock(&spinlock1);

    return NULL;
}

static void * threadFuncRWLock(void *arg)
{
	int loops = *((int *) arg);
	int loc, j;
	
	pthread_rwlock_wrlock(&rwlock1);
	/* Critical section */

    for (j = 0; j < loops; j++) {
        loc = glob;
        loc++;
        glob = loc;
    }
	
	/* End critical section */
    pthread_rwlock_unlock(&rwlock1); 

    return NULL;
}

static void * threadFuncSigWait(void *arg)
{
	int loops = *((int *) arg);
	int loc, j;
	
	pthread_mutex_lock(&mutex1);
	/* Critical section */

	pthread_cond_wait(&cond1, &mutex1); //Tell other threads to wait
	
    for (j = 0; j < loops; j++) {
        loc = glob;
        loc++;
        glob = loc;
    }
	
	pthread_cond_signal(&cond1); //Tell other threads the resources are free
	
	/* End critical section */
    pthread_mutex_unlock(&mutex1);

    return NULL;
}

static void * threadFuncSemaphore(void *arg)
{
	int loops = *((int *) arg);
	int loc, j;
	
	sem_wait(&sem1);  //like an lock
	/* Critical section */

    for (j = 0; j < loops; j++) {
        loc = glob;
        loc++;
        glob = loc;
    }
	
	/* End critical section */
    sem_post(&sem1); //like an unlock

    return NULL;
}

//argc = number of command line arguments
//argv = contains arguments, starting with index 1 (index 0 = program name)
int main(int argc, char *argv[])
{
	int loops, threads, success, counter = 0;
	char * concurrencyType;

	if(argc == 4)
	{
		loops = atoi(argv[1]); //string to integer
		threads = atoi(argv[2]);
		concurrencyType = argv[3];
	}
    else
	{
		fprintf(stderr, "\nCommand to run program: ./hw5 <loops> <threads> <concurrency type>\n");
		fprintf(stderr, "\nCommand to seek help: ./hw5 -h\n");
		fprintf(stderr, "\nValid concurrency control mechanisms include: NONE, MUTEX, SPINLOCK, READWRITELOCK, SIGNALWAIT, SEMAPHORE.\n");
		exit(EXIT_FAILURE);
	}
	
	pthread_t* myThreads = malloc(sizeof(pthread_t) * threads);

	//START CLOCK
	clock_t begin = clock();

	//NONE
	if(compare(concurrencyType, NONE) == 0)
	{
		while(threads != 0)
		{
			success = pthread_create(&myThreads[counter], NULL, threadFunc, &loops);
			if(success != 0)
			{
				perror("Failed to create thread. Exiting now.");
				exit(EXIT_FAILURE);
			}
			
			success = pthread_join(myThreads[counter], NULL);
			if(success != 0)
			{
				perror("Failed to join thread. Exiting now.");
				exit(EXIT_FAILURE);
			}
			counter++;
			threads--;
		}		
	}
	
	//MUTEX
	else if(compare(concurrencyType, MUTEX) == 0)
	{
		if (pthread_mutex_init(&mutex1, NULL) != 0)
		{
			fprintf(stderr, "\n MUTEX INITIATION FAILED. ABORTING.\n");
			return 1;
		}
		
		while(threads != 0)
		{
			success = pthread_create(&myThreads[counter], NULL, threadFuncMutex, &loops);
			if(success != 0)
			{
				perror("Failed to create thread. Exiting now.");
				exit(EXIT_FAILURE);
			}
			
			success = pthread_join(myThreads[counter], NULL);
			if(success != 0)
			{
				perror("Failed to join thread. Exiting now.");
				exit(EXIT_FAILURE);
			}
			counter++;
			threads--;
		}
		pthread_mutex_destroy(&mutex1);
	}
	
	//SPINLOCK
	else if(compare(concurrencyType, SPINLOCK) == 0)
	{
		if (pthread_spin_init(&spinlock1, 0) != 0)
		{
			fprintf(stderr, "\n SPINLOCK INITIATION FAILED. ABORTING.\n");
			return 1;
		}
		
		while(threads != 0)
		{
			
			success = pthread_create(&myThreads[counter], NULL, threadFuncSpinlock, &loops);
			if(success != 0)
			{
				perror("Failed to create thread. Exiting now.");
				exit(EXIT_FAILURE);
			}
			
			success = pthread_join(myThreads[counter], NULL);
			if(success != 0)
			{
				perror("Failed to join thread. Exiting now.");
				exit(EXIT_FAILURE);
			}
			
			counter++;
			threads--;
		}
		pthread_spin_destroy(&spinlock1);
	}
	
	//READWRITELOCK
	else if(compare(concurrencyType, READWRITELOCK) == 0)
	{
		if (pthread_rwlock_init(&rwlock1, NULL) != 0)
		{
			fprintf(stderr, "\n RWLOCK INITIATION FAILED. ABORTING.\n");
			return 1;
		}
		
		while(threads != 0)
		{
			success = pthread_create(&myThreads[counter], NULL, threadFuncRWLock, &loops);
			if(success != 0)
			{
				perror("Failed to create thread. Exiting now.");
				exit(EXIT_FAILURE);
			}
			
			success = pthread_join(myThreads[counter], NULL);
			if(success != 0)
			{
				perror("Failed to join thread. Exiting now.");
				exit(EXIT_FAILURE);
			}
			counter++;
			threads--;
		}
		pthread_rwlock_destroy(&rwlock1); 
	}
	
	//SIGNALWAIT
	else if(compare(concurrencyType, SIGNALWAIT) == 0)
	{
		if (pthread_cond_init(&cond1, NULL) != 0 && pthread_mutex_init(&mutex1, NULL) != 0)
		{
			fprintf(stderr, "\n COND AND/OR MUTEX INITIATION FAILED. ABORTING.\n");
			return 1;
		}
		
		while(threads != 0)
		{
			success = pthread_create(&myThreads[counter], NULL, threadFuncSigWait, &loops);
			if(success != 0)
			{
				perror("Failed to create thread. Exiting now.");
				exit(EXIT_FAILURE);
			}
			
			success = pthread_join(myThreads[counter], NULL);
			if(success != 0)
			{
				perror("Failed to join thread. Exiting now.");
				exit(EXIT_FAILURE);
			}
			counter++;
			threads--;
		}
		pthread_cond_destroy(&cond1);
		pthread_mutex_destroy(&mutex1);
	}
	
	//SEMAPHORE
	else if(compare(concurrencyType, SEMAPHORE) == 0)
	{
		if (sem_init(&sem1, 0, 1) != 0)
		{
			fprintf(stderr, "\n SEMAPHORE INITIATION FAILED. ABORTING.\n");
			return 1;
		}
		
		while(threads != 0)
		{
			success = pthread_create(&myThreads[counter], NULL, threadFuncSemaphore, &loops);
			if(success != 0)
			{
				perror("Failed to create thread. Exiting now.");
				exit(EXIT_FAILURE);
			}
			
			success = pthread_join(myThreads[counter], NULL);
			if(success != 0)
			{
				perror("Failed to join thread. Exiting now.");
				exit(EXIT_FAILURE);
			}
			counter++;
			threads--;
		}
		sem_destroy(&sem1); 
	}
	
	//FAILURE
	else
	{
		fprintf(stderr, "\n Concurrency Type NOT valid. \n"); 
		exit(EXIT_FAILURE);
	}

	//END CLOCK
	clock_t end = clock();
	double total_time = ((double)(end - begin) / CLOCKS_PER_SEC) * 1000;

	printf("GLOB = %d\n", glob);
	printf("TOTAL TIME = %f MS\n", total_time);
    exit(EXIT_SUCCESS);
}
