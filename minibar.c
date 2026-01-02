#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <locale.h>
#include <langinfo.h>
#include <sys/ioctl.h>

#include "minibar.h"

/* a minimal non-curses progress bar in C */

static int _dumb = 0;
static unsigned _nplot = 0;

static FILE *_outdev = NULL;
static int  _nrows = 0;
static int  _maxrow = 0;
static int  _width = -1;
static int  _rendered = 0;
static minibar_t *_avail_head = NULL;
static minibar_t *_inuse_head = NULL;
static minibar_t *_inuse_tail = NULL;
static minibar_t *_bars;

static minibar_spin_t _spinner = minibar_spinA;
static minibar_bar_t  _barplot = minibar_barA;

static void
handle_winch(int s) {
	struct winsize ws;
	if(_outdev == NULL) return;
	if(ioctl(fileno(_outdev), TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
		_width = ws.ws_col;
		minibar_refresh();
	}
}

static int
minibar_alloc(int maxrow) {
	int i;
	if(maxrow < 1)    return -1;
	_bars = (minibar_t *) malloc(sizeof(minibar_t) * maxrow);
	if(_bars == NULL) return -1;
	memset(_bars, 0, sizeof(minibar_t) * maxrow);
	for(i = 0; i < maxrow; i++) {
		_bars[i].prev = &_bars[i-1];
		_bars[i].next = &_bars[i+1];
	}
	_bars[0].prev = NULL;
	_bars[maxrow-1].next = NULL;
	_avail_head = &_bars[0];
	_inuse_head = _inuse_tail = NULL;
	_nrows = 0;
	return _maxrow = maxrow;
}

int
support_unicode() {
	const char *codeset = nl_langinfo(CODESET);
	if(codeset == NULL) return 0;
    return strcmp(codeset, "UTF-8") == 0;
}

int
support_ansi(FILE *dev) {
	const char *term = getenv("TERM");
	if(!isatty(fileno(dev))) return 0;
	if(term == NULL)         return 0;
	if(term[0] == '\0')      return 0;
	if(strcmp(term, "dumb") == 0)   return 0;
	if(strncmp(term, "vt", 2) == 0) return 0;
	if(strstr(term, "xterm")
	|| strstr(term, "ansi")
	|| strstr(term, "linux")
	|| strstr(term, "screen")
	|| strstr(term, "tmux")
	|| strstr(term, "color")) {
		return 1;
	}
	return 0;
}

int
minibar_open(FILE *dev, int maxrow) {
	if(minibar_alloc(maxrow) <= 0) {
		return -1;
	}

	_dumb = 0;
	_nplot = 0;
	_outdev = dev;

	setlocale(LC_ALL, "");
	if(support_unicode()) {
		_spinner = minibar_spinU;
		_barplot = minibar_barU;
	}

	if(support_ansi(dev)) {
		struct winsize ws;
		if(ioctl(fileno(dev), TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
			setvbuf(dev, NULL, _IOFBF, 0);
			signal(SIGWINCH, handle_winch);
			_width = ws.ws_col;
		}
	} else {
		_dumb = 1;
	}
	return 0;
}

void
minibar_close() {
	if(_outdev == NULL) return;
	if(_nrows > 0) {
		if(_dumb) {
			fprintf(_outdev, "\n\r");
		}
	}
	_nrows = _maxrow = 0;
	_avail_head = _inuse_head = _inuse_tail = NULL;
	if(_bars != NULL) free(_bars);
	_bars = NULL;
}

void
minibar_flush() {
	if(_outdev == NULL) return;
	fflush(_outdev);
}

int
minibar_getwidth() {
	return _width;
}

void
minibar_println(const char *fmt, ...) {
	va_list ap;
	if(_outdev == NULL) return;

	if(_dumb) {
		fprintf(_outdev, "\r");
	} else if(_rendered > 0) {
		fprintf(_outdev, "\r\x1b[%dA2K", _rendered); 
	}
	va_start(ap, fmt);
	vfprintf(_outdev, fmt, ap);
	va_end(ap);
	fprintf(_outdev, "\n\r");

	minibar_refresh();
}

minibar_t *
minibar_get(const char *title) {
	minibar_t *curr = NULL;
	if(_bars == NULL)       return NULL;
	if(_avail_head == NULL) return NULL;

	curr = _avail_head;

	/* remove curr from avail list */
	_avail_head = curr->next;
	curr->next = curr->prev = NULL;
	/* append curr into inuse lise */
	if(_inuse_head == NULL) {
		/* inuse list is empty */
		_inuse_head = _inuse_tail = curr;
	} else {
		/* otherwise */
		_inuse_tail->next = curr;
		curr->prev = _inuse_tail;
		_inuse_tail = curr;
	}
	_nrows++;

	curr->nplots = 0;
	curr->progress = 0.0;
	strncpy(curr->title, title, MINIBAR_TITLE_MAX);
	return curr;
}

void
minibar_setvalue(minibar_t *bar, double progress) {
	if(bar == NULL) return;

	if(progress < 0.0)   progress = 0.0;
	if(progress > 100.0) progress = 100.0;

	bar->progress = progress;
}

void
minibar_complete(minibar_t *bar) {
	if(bar == NULL) return;

	if(!_dumb && _rendered > 0) {
		fprintf(_outdev, "\r\x1b[%dA", _rendered);
	}
	_rendered = 0;
	minibar_plot1(bar);

	/** must be in the inuse list **/
	/* remove from inuse list */
	if(bar->next != NULL)
		bar->next->prev = bar->prev;
	if(bar->prev != NULL)
		bar->prev->next = bar->next;
	if(_inuse_head == bar) _inuse_head = bar->next;
	if(_inuse_tail == bar) _inuse_tail = bar->prev;

	/* add to avail list */
	bar->prev = NULL;
	bar->next = _avail_head;
	if(bar->next != NULL)
		bar->next->prev = bar;
	_avail_head = bar;

	_nrows--;
	minibar_refresh();
}

unsigned int
minibar_spinA(FILE *dev, unsigned int n) {
	static char spin[][8] = { "|", "/", "-", "\\" };
	fprintf(dev, "%s", spin[n % 4]);
	return n+1;
}

unsigned int
minibar_spinU(FILE *dev, unsigned int n) {
	/* Unicode Braille Patterns */
	static char spin[][8] = { "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏" };
	fprintf(dev, "%s", spin[n % 10]);
	return n+1;
}

void
minibar_bar_generic(FILE *dev, int width, double percent, char *ch[], int nch) {
	int i, full, remainder;
	int level = nch - 1;
	double total;

	if(percent < 0.0)   percent = 0.0;
	if(percent > 100.0) percent = 100.0;

	total = (percent / 100.0) * width * 1.0 * level;
	full = (int) (total / (1.0 * level));
	remainder = (int) round(total) % level;

	for(i = 0; i < width; i++) {
		if(i < full) {
			fprintf(dev, "%s", ch[level]);
		} else if(i == full && remainder > 0) {
			fprintf(dev, "%s", ch[remainder]);
		} else {
			fprintf(dev, "%s", ch[0]);
		}
	}
}

void
minibar_barA(FILE *dev, int width, double percent) {
	static char *ch[] = { ".", "#" };
	minibar_bar_generic(dev, width, percent, ch, 2);
}

void
minibar_barU(FILE *dev, int width, double percent) {
	/* Unicode Braille Patterns */
	//static char *ch[] = { " ", "⡀", "⡄", "⡆", "⡇", "⣇", "⣧", "⣷", "⣿" };
	static char *ch[] = { "⣀", "⣄", "⣆", "⣇", "⣧", "⣷", "⣿" };
	minibar_bar_generic(dev, width, percent, ch, 7);
}

void
minibar_plot1(minibar_t *bar) {
#define W_MINIMAL	20	/* 10 + 10 + 10 */
#define W_SUFFIX	9	/* '] OOO.O%\n' */
#define W_BARMAX	64	/* exclude [ and ] */
	/* name + [ bar ] + 8-byte for progress ' OOO.O%\n' */
	int w_name, w_bar;
	if(_dumb) {
		double progress = bar->progress;
		if(progress < 0.0)   progress = 0.0;
		if(progress > 100.0) progress = 100.0;
		fprintf(_outdev, "\r");
		bar->nplots = _spinner(_outdev, bar->nplots);
		fprintf(_outdev, " %6.2f%%", bar->progress);
		return;
	}
	if(_width < 3) {
		/* no output */
		return;
	}
	w_name = (_width - W_SUFFIX) * 2 / 3;
	w_bar = _width - W_SUFFIX - w_name - 1 /* [ */;
	if(w_bar > W_BARMAX) {
		w_bar = W_BARMAX;
		w_name = _width - W_BARMAX - W_SUFFIX - 1 /* [ */;
	}
	if(_width < W_MINIMAL) {
		fprintf(_outdev, "\r\x1b[2K");
		bar->nplots = _spinner(_outdev, bar->nplots);
		fprintf(_outdev, " %*.*s\n", -(_width-3), _width-3, bar->title);
		return;
	}
	fprintf(_outdev, "\r\x1b[2K");
	bar->nplots = _spinner(_outdev, bar->nplots);
	fprintf(_outdev, " %*.*s |", -(w_name-3), w_name-3, bar->title);
	_barplot(_outdev, w_bar, bar->progress);
	fprintf(_outdev, "|%*.1f%%\n", W_SUFFIX-3,
		bar->progress > 100.0 ? 100.0 : bar->progress);
#undef W_MINIMAL
#undef W_PERCENT
#undef W_BARMAX
}

void
minibar_refresh() {
	minibar_t *curr;
	if(_dumb) {
		fprintf(_outdev, "\r");
		if(_inuse_head != NULL) {
			minibar_plot1(_inuse_head);
			fflush(_outdev);
		}
		goto quit;
	}
	if(_width < 3) return;
	if(_rendered > 0)
		fprintf(_outdev, "\x1b[%dA\r", _rendered);
	_rendered = 0;
	for(curr = _inuse_head; curr != NULL; curr = curr->next) {
		minibar_plot1(curr);
		_rendered++;
	}
quit:
	fflush(_outdev);
}

