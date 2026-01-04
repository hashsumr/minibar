#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "minibar.h"

void runner(FILE *dev, int w, minibar_spin_t spinner, minibar_bar_t barplot) {
	unsigned int n = 0;
	double v = 0.0;
	do {
		v += 0.1 * (5 + rand()%10);
		if(v > 100.0) v = 100.0;
		fprintf(dev, "\r\x1b[2K");
		n = spinner(dev, n);
		fprintf(dev, " minibar [");
		barplot(dev, w, v);
		fprintf(dev, "] %5.1f%%", v);
		fflush(dev);
#ifdef _WIN32
		Sleep(100);
#else
		usleep(100000);  // 100 ms
#endif
	} while(v < 100.0);
	fprintf(dev, "\n");
}

int
main() {
	int w = 30;   // bar width
	FILE *dev = stderr;
#ifdef _WIN32
	srand(time(0));
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#else
	srand(time(0) ^ getpid());
#endif
	setvbuf(dev, NULL, _IOFBF, BUFSIZ);
	runner(dev, w, minibar_spinA, minibar_barA);
	runner(dev, w, minibar_spinU, minibar_barU);

	return 0;
}
