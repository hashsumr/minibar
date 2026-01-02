#ifndef __MINIBAR_H__
#define __MINIBAR_H__

#include <stdio.h>

#define MINIBAR_TITLE_MAX	128

typedef struct minibar_s {
	double progress;	/* 0.0-100.0 */
	char title[MINIBAR_TITLE_MAX];
	/* danger zone */
	unsigned int nplots;
	struct minibar_s *prev, *next;
}	minibar_t;

int minibar_open(FILE *dev, int maxrow);
void minibar_close();
void minibar_flush();
int minibar_getwidth();
void minibar_println(const char *fmt, ...);

minibar_t * minibar_get(const char *title);
void minibar_setvalue(minibar_t *bar, double progress);
void minibar_complete(minibar_t *bar);

typedef unsigned int (*minibar_spin_t)(FILE *dev, unsigned int n);
typedef void         (*minibar_bar_t)(FILE *dev, int width, double percent);

unsigned int minibar_spinA(FILE *dev, unsigned int n);
unsigned int minibar_spinU(FILE *dev, unsigned int n);
void minibar_barA(FILE *dev, int width, double percent);
void minibar_barU(FILE *dev, int width, double percent);

void minibar_plot1(minibar_t *bar);
void minibar_refresh();

#endif /* __MINIBAR_H__ */
