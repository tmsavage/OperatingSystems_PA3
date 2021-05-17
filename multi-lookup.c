#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <dirent.h>
#include <sys/time.h>
#include "util.h"
#include "multi-lookup.h"
//#include "util.c"

	/* Numbers to produce */
//static char *buffer[ARRAY_SIZE]; /* array of char* pointers -> an array of strings!!!. Now we have a staticly allocated array*/
//pthread_mutex_t my_mutex;
//pthread_cond_t condc, condp;
//int MAX_INPUT_FILES = 10;
//int MAX_RESOLVER_THREADS = 10;
//int MAX_REQUESTER_THREADS = 5;
//int MAX_NAME_LENGTH = 1025;

//All global variables removed. NOT THREAD-SAFE OR RE-ENTRANT!!


void* requester(void *args) { //originally args
	//struct Requester *reqStruct = (struct Requester*) Requester;
	
	struct args *buffer = (struct args*) args;
	pthread_mutex_t *mutex = buffer->my_mutex;
	sem_t *spaceAvailable = buffer->spaceAvailable;
	sem_t *itemsAvailable = buffer->itemsAvailable;
	int *index = buffer->index;
	int *finishFileCounter = buffer->finishFileCounter;
	int *InFileIndex = buffer->InFileIndex;
	int *numInputFiles = buffer->numInputFiles;	
	FILE *serviceFile = buffer->serviceFile;
	FILE *performanceFile = buffer->performanceFile;
	// pthread_mutex_t *resMutex = buffer->resMutex;
	pthread_mutex_t *reqMutex = buffer->reqMutex;
	// char *logOutputLine = (char *)malloc(MAX_LOG_LINE * sizeof(char));

	//printf("HERE REQ 1 Thread ID: %lu \n", pthread_self());
	FILE* file;


	int numFilesRead = 0;
	
	pthread_mutex_lock(reqMutex);
	while(*InFileIndex < *numInputFiles)
	{
		
		//printf("HERE REQ 2 Thread ID: %lu \n", pthread_self());
		file = buffer->fileDesc[*InFileIndex];
		//printf("HERE REQ 3 \n");
		printf("FILE Name: %s %d \n", buffer->fileNames[*InFileIndex], *InFileIndex);

		// = NULL;
		size_t *len = malloc(sizeof(size_t));
		*len = MAX_NAME_LENGTH * sizeof(char);
		char *line = (char *)malloc(*len);
		numFilesRead++;
		//ssize_t numReadBytes;

		*InFileIndex+=1;
		pthread_mutex_unlock(reqMutex);
		while (file != NULL && (getline(&line, len, file)) != -1) {
			if(line[strlen(line)-1] == '\n')
				line[strlen(line) - 1] = '\0';			

			printf("HERE REQ 5 Thread ID: %lu \n", pthread_self());
			sem_wait(spaceAvailable);	//semaphore wait
			pthread_mutex_lock(mutex);	//lock mutex
			printf("Write Line: %s to buffer\n", line);
			buffer->bufferArray[*index] = (char *)malloc(MAX_NAME_LENGTH * sizeof(char));;
			strcpy(buffer->bufferArray[*index], line);			
			*index +=1;

			
			//line = NULL;			
			//*len = 0;
			pthread_mutex_unlock(mutex);	//unlock mutex
			sem_post(itemsAvailable);	//semaphore release
		}		
		free(len);
		free(line);
		pthread_mutex_lock(reqMutex);
		*finishFileCounter+=1;
		fclose(file);
			
	}

	fprintf(serviceFile, "Thread %lu serviced %d files\n", pthread_self(), numFilesRead);
	fprintf(performanceFile, "Thread %lu serviced %d files\n", pthread_self(), numFilesRead);
	pthread_mutex_unlock(reqMutex);

	/*	
	fputs("Thread ", serviceFile);

	char stringThreadID[20];
	sprintf(stringThreadID, "%lu", pthread_self());
	fputs(stringThreadID , serviceFile);
	fputs(" services ", serviceFile);

	char stringFinishFile[20];
	sprintf(stringFinishFile, "%d", numFilesRead);
	fputs(stringFinishFile, serviceFile);
	fputs(" files \n", serviceFile);
	*/
	//printf("HERE REQ 4 Thread ID: %lu \n", pthread_self());
	

	pthread_exit(0);
}

void* resolver(void *args) {
	
	//pthread_wait(1); wait for 1 second for testing

	struct args *buffer = (struct args*) args;
	pthread_mutex_t *mutex = buffer->my_mutex;
	sem_t *spaceAvailable = buffer->spaceAvailable;
	sem_t *itemsAvailable = buffer->itemsAvailable;
	int *index = buffer->index;
	int *finishFileCounter = buffer->finishFileCounter;
	int *numInputFiles = buffer->numInputFiles;
	//int *InFileIndex = buffer->InFileIndex;
	FILE *resultsFile = buffer->resultsFile;
	pthread_mutex_t *resMutex = buffer->resMutex;
	// pthread_mutex_t *reqMutex = buffer->reqMutex;	

	
	// *index is the buffer's index
	
	pthread_mutex_lock(mutex);
	while(*index > 0 || *finishFileCounter < *numInputFiles) {
		printf("Index: %d, Finish Files: %d, Thread ID: %lu \n", *index, *finishFileCounter, pthread_self());
		pthread_mutex_unlock(mutex);
		printf("HERE I AM 1 Thread ID: %lu \n", pthread_self());
		sem_wait(itemsAvailable);
		printf("HERE I AM 1.5 Thread ID: %lu \n", pthread_self());
		pthread_mutex_lock(mutex);
		printf("Read Line %d from buffer: %s \n", *index-1, buffer->bufferArray[*index-1]);

		char* ipString = malloc(sizeof(char) * INET_ADDRSTRLEN);
		
		//printf("WHAT IM I TRYING TO CONNECT TO: %s", buffer->bufferArray[*index-1]);
		char *url = buffer->bufferArray[*index-1];
		//url[strlen(url)-1] = '\0';
		
		*index -= 1;
		pthread_mutex_unlock(mutex);
		if (dnslookup(url, ipString, INET_ADDRSTRLEN) == UTIL_SUCCESS ) {
			//printf("success\n");
			pthread_mutex_lock(resMutex);
			fprintf(resultsFile, "%s,%s\n", url, ipString);
			pthread_mutex_unlock(resMutex);
		}
		else {
			pthread_mutex_lock(resMutex);
			fprintf(resultsFile, "%s,\n", url);
			pthread_mutex_unlock(resMutex);
		}
		//printf("IPSTRING: %s\n", ipString);

		free(ipString);
		free(url);
		sem_post(spaceAvailable);
		//pthread_mutex_unlock(buffer->fileMutexes[*InFileIndex]);

		pthread_mutex_lock(mutex);
	}
	pthread_mutex_unlock(mutex);
	//printf("HERE I AM 2 Thread ID: %lu \n", pthread_self());
	pthread_exit(0);
}


int main(int argc, char *argv[]) {

	// 1. Declare struct in main
	// 2. Allocate buffer in main
	// 3. Pass pointer to buffer in struct that you pass to req (producer) / res (consumer)
	//	- declare pointer in struct to buffer memory
	struct timeval current_time;
	gettimeofday(&current_time, NULL);
    	long int startSec = current_time.tv_sec;
	long int startUSec = current_time.tv_usec;

	pthread_t *req[MAX_REQUESTER_THREADS], *res[MAX_RESOLVER_THREADS];
	pthread_mutex_t mutex;
	pthread_mutex_t reqMutex;
	pthread_mutex_t resMutex;
	sem_t spaceAvailable;
	sem_t itemsAvailable;
	int index= 0;
	int InFileIndex = 0;
	int finishFileCounter = 0;
	int numInputFiles = 0;
	FILE* performanceFile = NULL;
	performanceFile = fopen("performance.txt", "w");
	struct args buffer;
	int numReqThreads = atoi(argv[1]);
	int numResThreads = atoi(argv[2]);
	fprintf(performanceFile, "Number of requester threads is %d \n", numReqThreads);
	fprintf(performanceFile, "Number of resolver threads is %d \n", numResThreads);

	// Initialize the mutex and condition variables
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&reqMutex, NULL);
	pthread_mutex_init(&resMutex, NULL);
	sem_init(&spaceAvailable, 0, ARRAY_SIZE); 
	sem_init(&itemsAvailable, 0, 0);
	
	buffer.my_mutex = &mutex;
	buffer.spaceAvailable = &spaceAvailable;
	buffer.itemsAvailable = &itemsAvailable;
	buffer.index = &index;
	buffer.finishFileCounter = &finishFileCounter;
	buffer.InFileIndex = &InFileIndex;
	buffer.numInputFiles = &numInputFiles;
	buffer.resMutex = &resMutex;
	buffer.reqMutex = &reqMutex;
	buffer.performanceFile = performanceFile;
	buffer.resultsFile = fopen(argv[4], "w");
	buffer.serviceFile = fopen(argv[3], "w");



	// --- save file names, open to file descriptor array
	FILE* fd;
	//printf("SIZE OF ARGC: %d \n", argc);
	for(int i = 5; i < argc; i++)
	{
		// --- save file names
		buffer.fileNames[i-5] = argv[i];
		
		// --- save file descriptors
		/*
		char direct[100] = "./input/";
		char *name = argv[i];
		strcat(direct, name);
		*/
		fd = fopen(argv[i], "r");
		buffer.fileDesc[i-5] = fd;

		// --- save file mutexes
		pthread_mutex_t fileMutex;
		buffer.fileMutexes[i-5] = &fileMutex;


		*buffer.numInputFiles += 1;
		//printf("FILE PATH: %s %d \n", argv[i], i-5);
	}

	
	
	//int* numReqThreads;
	//int* numResThreads;

	//numReqThreads = (int*)malloc(SIZE * sizeof(int));
	//numResThreads = (int*)malloc(SIZE * sizeof(int));
	
	//int argLen = argc;
	printf("ARGC: %d \n", argc); 
	
	if(argc < 6) {
		printf("ARGC; %d \n", argc); 
		printf("FAIL!!! \n");
		return -1;
	}




	//char *p;

	
	
	
	// creates number of Requester threads
	if(numReqThreads <= MAX_REQUESTER_THREADS) {
		printf("TEST 1");
		for(int i = 0; i < numReqThreads; i++){
			printf("TEST 2");
			req[i] = (pthread_t *) malloc(sizeof(pthread_t));
			if(pthread_create(req[i], NULL, requester, (void *)&buffer)) {
				perror("error in creating requester");
				exit(1);	
			}
			//printf("Created %d requester thread\n", i+1);
		}
	}
	printf("ARG 1 %s \n", argv[1]);	
	// creates number of Resolver threads
	if(numResThreads <= MAX_RESOLVER_THREADS) {
		for(int j = 0; j < numResThreads; j++) {
			res[j] = (pthread_t *) malloc(sizeof(pthread_t));
			if(pthread_create(res[j], NULL, resolver, (void *)&buffer)) {
				perror("error in creating resolver");
				exit(1);
			}
			//printf("Created %d resolver thread\n", j+1);
		}
	}
	
	for(int i = 0; i < numReqThreads; i++)
	{
		pthread_join(*req[i], 0);
		free(req[i]);
	}

	for(int j = 0; j < numResThreads; j++)
	{
		pthread_join(*res[j], 0);
		free(res[j]);
	}
  	

	fclose(buffer.resultsFile);
	fclose(buffer.serviceFile);

	// Cleanup -- would happen automatically at end of program
	// Should be 'destroy' or 'close'?
	pthread_mutex_destroy(&mutex);	/* Free up mutex */
	pthread_mutex_destroy(&reqMutex);
	pthread_mutex_destroy(&resMutex);
	sem_close(&spaceAvailable);
	sem_close(&itemsAvailable);


	
	gettimeofday(&current_time, NULL);
    	long int endSec = current_time.tv_sec;
	long int endUSec = current_time.tv_usec;
	long int totalMillSec = endUSec-startUSec;
	if(endUSec-startUSec < 0) {
		totalMillSec*=-1;
	}

	printf("Total time is: %ld.%ld \n", endSec-startSec, totalMillSec);

	

	

	
	
	fprintf(performanceFile, "./multi-lookup: total time is %ld.%ld \n", endSec-startSec, totalMillSec);
	
	fclose(performanceFile);

	return 0;


}
