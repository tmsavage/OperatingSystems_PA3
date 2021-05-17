#ifndef BUFFSTRUCT_H
#define BUFFSTRUCT_H
#define ARRAY_SIZE 20
#define MAX_INPUT_FILES 10
#define MAX_RESOLVER_THREADS 10
#define MAX_REQUESTER_THREADS 5
#define MAX_NAME_LENGTH 1025
#define MAX_LOG_LINE 1025


// Buffer
struct args {
	pthread_mutex_t *my_mutex;
	pthread_mutex_t *reqMutex;
	pthread_mutex_t *resMutex;
	sem_t *spaceAvailable;
	sem_t *itemsAvailable;
	int *index;
	int *finishFileCounter;
	char *bufferArray[ARRAY_SIZE]; /* array of char* pointers -> an array of strings!!!. Now we have a staticly allocated array*/
	char *fileNames[MAX_INPUT_FILES];
	FILE *fileDesc[MAX_INPUT_FILES];
	pthread_mutex_t *fileMutexes[MAX_INPUT_FILES];
	int *InFileIndex;
	int *numInputFiles;
	FILE *resultsFile;
	FILE *serviceFile;
	FILE *performanceFile;
	//pthread_cond_t condc, condp;
};

//-------------------------------------------------

struct InFiles {
	char *fileNames[MAX_INPUT_FILES];
	FILE *fileDescriptors[MAX_INPUT_FILES];
	pthread_mutex_t *FD_m[MAX_INPUT_FILES];
	int *InFileIndex;
	
	
};

struct OutFiles {
	char *fileNames[MAX_INPUT_FILES];
	FILE *fileDescriptors[MAX_INPUT_FILES];
};

//-------------------------------------------------


// Requester Struct
struct Requester {
	struct InFiles inputFileNames;
	struct OutFiles outputFileNames;
	struct args buffer;

};

// Resolver Struct
struct Resolver {
	struct InFiles inputFileNames;
	struct OutFiles outputFileNames;
	struct args buffer;

};

#endif
