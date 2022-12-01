#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

/****                    Structs                      ****/
typedef struct job{
	void* params;
	void (*function)(void* arg); 
	struct job* next;
}job;

typedef struct threadPool{
	pthread_t* pool;
	struct job* jobsHead; 
    struct job* jobsTail;
	int numTasks;
    int numTasksAlive;
	pthread_mutex_t *jobMutex;
    pthread_mutex_t *aliveMutex;
	pthread_cond_t *mutexCond;

}threadPool;

/****                  Functions                     ****/

int addJob(void (*function)(void* arg), void* params, threadPool* thPool)
{
    struct job* newJob = (struct job*) malloc(sizeof(job));
    if (newJob == NULL)
    {
        return -1;
    }

    newJob->function = function;
    newJob->params = params;

    if(thPool->jobsTail == NULL)
    {
        thPool->jobsHead = newJob;
        thPool->jobsTail = newJob;
    }
    else
    {
        thPool->jobsTail->next = newJob;
        thPool->jobsTail = thPool->jobsTail->next;
    }

    pthread_cond_signal(thPool->mutexCond);
    return 0;
}

struct job* getJob(threadPool* thPool)
{
    struct job* nextJob = thPool->jobsHead;
    if (thPool->jobsHead == thPool->jobsTail)
    {
        thPool->jobsHead = NULL;
        thPool->jobsTail = NULL;
    }
    else
    {
        thPool->jobsHead = thPool->jobsHead->next;
    }

    return nextJob;
}

void *runThreadWrapper(void* params)
{
    struct threadPool *thPool = (struct threadPool*) params;
    struct job* currJob;
    do{
        pthread_mutex_lock(thPool->jobMutex);
        while (thPool->jobsTail == NULL)
		{
			pthread_cond_wait(thPool->mutexCond, thPool->jobMutex);
		}

        currJob = getJob(thPool);
        pthread_mutex_lock(thPool->aliveMutex);
        thPool->numTasksAlive++;
        printf("%d ",thPool->numTasksAlive);
        pthread_mutex_unlock(thPool->aliveMutex);
        pthread_mutex_unlock(thPool->jobMutex);
        currJob->function(currJob->params);
        free(currJob);
        pthread_mutex_lock(thPool->aliveMutex);
        thPool->numTasksAlive--;
        pthread_mutex_unlock(thPool->aliveMutex);

    }while(1); 
}

int numTasksAlive(threadPool *thPool){
    
    return thPool->numTasksAlive;
}

struct threadPool* thPoolInit(int numOfTasks)
{
    struct threadPool* thPool = (struct threadPool*) malloc(sizeof(struct threadPool));
    
    thPool->jobMutex = (pthread_mutex_t* ) malloc(sizeof(pthread_mutex_t));
    thPool->mutexCond = (pthread_cond_t* ) malloc(sizeof(pthread_cond_t));
    thPool->aliveMutex = (pthread_mutex_t* ) malloc(sizeof(pthread_mutex_t));

    thPool->jobsHead = NULL;
    thPool->jobsTail = NULL;

    pthread_mutex_init(thPool->jobMutex,NULL);
    pthread_cond_init(thPool->mutexCond, NULL);
    pthread_mutex_init(thPool->aliveMutex,NULL);

    thPool->numTasks = numOfTasks;
    thPool->numTasksAlive = 0;
    thPool->pool = (pthread_t *) malloc((sizeof(pthread_t) * thPool->numTasks));
    for(int i = 0 ; i < thPool->numTasks ; i++){
        if (pthread_create(&thPool->pool[i], NULL, runThreadWrapper, thPool) != 0)
		{
			perror("Failed to create the thread");
		}
    }

    return thPool;
}

/*
DO NOT USE THIS FUNCTION ALREADY, PROBLEM WITH DESTROY COND!!!
*/
int poolRelease(threadPool* thPool)
{
    free(thPool->pool);
    
	pthread_mutex_destroy(thPool->jobMutex);
    pthread_mutex_destroy(thPool->aliveMutex);
    int i;
    for(i = 0 ; i < thPool->numTasks ; i++){
        pthread_cond_signal(thPool->mutexCond);
    }
    printf("freed \n");
    pthread_cond_destroy((thPool->mutexCond));

    while(thPool->jobsHead != NULL){
        struct job* jb= thPool->jobsHead;
        thPool->jobsHead = thPool->jobsHead->next;
        free(jb);
        printf("freed\n");
    }

    free(thPool->jobsTail);
    free(thPool->jobsHead);

    return 0;
}