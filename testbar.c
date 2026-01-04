#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "minibar.h"

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

#define	N_BAR	(4)
#define N_JOBS	(10)

int
main(int argc, char *argv[]) {
	int i, allocated = 0, completed = 0;
	minibar_t *bar[N_BAR];

#ifdef _WIN32
	srand(time(0));
#else
	srand(time(0) ^ getpid());
#endif

	if(minibar_open(stderr, N_BAR) < 0) {
		fprintf(stderr, "init failed.\n");
		return -1;
	}

	minibar_println("minibar: width = %d", minibar_getwidth());

	for(i = 0; i < N_BAR; i++) {
		if((bar[i] = minibar_get(gen_title(i))) == NULL) {
			fprintf(stderr, "bar[%d] alloc failed.\n", i);
			return -1;
		}
	}
	allocated = i;

	while(completed < N_JOBS) {
		for(i = 0; i < N_BAR; i++) {
			int idx = rand() % N_BAR;

			if(bar[idx] == NULL) continue;

			bar[idx]->progress += 0.01 * (300 + rand() % 600);

			if(bar[idx]->progress < 100.0) break;

			minibar_complete(bar[idx]);
			bar[idx] = NULL;
			completed++;

			if(allocated < N_JOBS) {
				if((bar[idx] = minibar_get(gen_title(allocated))) == NULL) {
					fprintf(stderr, "minibar_get failed.\n");
				}
				allocated++;
			}
		}

		minibar_refresh();

#ifdef _WIN32
		Sleep(100);
#else
		usleep(100000);  // 100 ms
#endif
	}

	minibar_close();

	return 0;
}
