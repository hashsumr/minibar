#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
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
        usleep(100000);  // 100 ms
    } while(v < 100.0);
    fprintf(dev, "\n");
}

int
main() {
    int w = 30;   // bar width
	FILE *dev = stderr;

	srand(time(0) ^ getpid());
	setvbuf(dev, NULL, _IOFBF, 0);
	runner(dev, w, minibar_spinA, minibar_barA);
	runner(dev, w, minibar_spinU, minibar_barU);

    return 0;
}
