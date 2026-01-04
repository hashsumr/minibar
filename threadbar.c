#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif
#include "minibar.h"

#ifdef _WIN32
#include "pthread_win32.c"
#endif

/* my secret job format */
typedef struct job_s {
	char title[48];
	double progress;
}	job_t;

/* global state */
static job_t *jobs;
static int nextjob = 0;
static int totaljob = 0;

static pthread_mutex_t mutex;
static pthread_barrier_t barrier;

char *
gen_title(int n) {
	static char cand[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz" "0123456789";
	static char output[32];
	int i;
	snprintf(output, sizeof(output), "%02d", n);
	for(i = 2; i < sizeof(output)-1; i++) {
		output[i] = cand[rand() % (sizeof(cand)-1)];
	}
	output[sizeof(output)-1] = '\0';
	return output;
}

static int running = 0;
void *
visualizer(void *__) {
	while(running) {
		minibar_refresh();
#ifdef _WIN32
		Sleep(1000);
#else
		sleep(1);
#endif
	}
	return NULL;
}

void *
worker(void *__) {
	job_t *job;
	minibar_t *bar;

	while(1) {
		/* get a job */
		pthread_mutex_lock(&mutex);
		if(nextjob >= totaljob) {
			pthread_mutex_unlock(&mutex);
			break;
		}
		job = &jobs[nextjob++];
		pthread_mutex_unlock(&mutex);
		/* get a bar state */
		if((bar = minibar_get(job->title)) == NULL) {
			fprintf(stderr, "FATAL: should not happen, because workers == bars\n");
			break;
		}
		/* run the job */
		while(job->progress < 100.0) {
			job->progress += 0.01 * (300 + rand() % 600);
			minibar_setvalue(bar, job->progress);
#ifdef _WIN32
			Sleep(100);
#else
			usleep(100000);  // 100 ms
#endif
		}
		job->progress = 100.0;
		/* update bar state */
		minibar_complete(bar);
	}
	pthread_barrier_wait(&barrier);
	return NULL;
}

#define N_WORKER_DEFAULT	(4)
#define N_JOB_DEFAULT	(10)

int
main(int argc, char *argv[]) {
	int i;
	int N_WORKERS = N_WORKER_DEFAULT;
	int N_JOBS = N_JOB_DEFAULT;
	pthread_t tid;

#ifdef _WIN32
	srand(time(0));
#else
	srand(time(0) ^ getpid());
#endif

	if(argc > 1) N_WORKERS = strtol(argv[1], NULL, 0);
	if(argc > 2) N_JOBS = strtol(argv[2], NULL, 0);
	if(N_WORKERS < 1) N_WORKERS = N_WORKER_DEFAULT;
	if(N_JOBS < 1) N_JOBS = N_JOB_DEFAULT;

	pthread_mutex_init(&mutex, NULL);

	/* create jobs */
	if((jobs = (job_t *) malloc(sizeof(job_t) * N_JOBS)) == NULL) {
		return -1;
	}
	for(i = 0; i < N_JOBS; i++) {
#ifdef _WIN32
		strncpy_s(jobs[i].title, 48, gen_title(i), 48);
#else
		strncpy(jobs[i].title, gen_title(i), 48);
#endif
		jobs[i].progress = 0.0;
	}
	nextjob = 0;
	totaljob = N_JOBS;

	/* setup a barrier - wait for all workers */
	if(pthread_barrier_init(&barrier, NULL, N_WORKERS+1) != 0) {
		fprintf(stderr, "pthread_barrier_init failed.\n");
		return -1;
	}

	/* init minibar */
	if(minibar_open(stderr, N_WORKERS) < 0) {
		fprintf(stderr, "init failed.\n");
		return -1;
	}
	minibar_println("minibar: width = %d", minibar_getwidth());

	/* create the visualizer */
	running = 1;
	if(pthread_create(&tid, NULL, visualizer, NULL) != 0) {
		fprintf(stderr, "pthread_create (visualizer) failed.\n");
		abort();
	}

	/* create workers */
	for(i = 0; i < N_WORKERS; i++) {
		if(pthread_create(&tid, NULL, worker, NULL) != 0) {
			fprintf(stderr, "pthread_create (worker) failed.\n");
			abort();
		}
	}

	/* wait for all workers */
	pthread_barrier_wait(&barrier);
	running = 0;

	minibar_refresh();
	minibar_close();

	return 0;
}
