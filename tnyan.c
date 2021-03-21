/* tnyan.c */
/* shamelessly lifted from https://github.com/koron/nyancat-vim */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <ncurses.h>
#include <signal.h>
#include "tnyan.h"

int acat_flag = 0;
int color_flag = 1;
int inverse_flag = 0;
int bgcolor = -1;
int delay = 50;

int yoffset, xoffset;
int frameheight, framewidth;

WINDOW *frames[FRAMES];

void quit() {
	endwin();
	exit(0);
}
 
void resize_win(int sig) {
	endwin();
	refresh();
	clear();

	yoffset = LINES / 2 - frameheight / 2;
	xoffset = COLS / 2 - framewidth / 2;
}

void init_colors() {
	if (color_flag < 1 || ! has_colors()) {
		color_flag = 0;
		return;
	}

	start_color();

	if (bgcolor == -1) {
		use_default_colors();
	}

	/* 256 colors */
	init_pair(1, 17, bgcolor);	/* , (bg) */
	init_pair(2, 255, bgcolor);	/* . */
	init_pair(3, 16, bgcolor); 	/* ' */
	init_pair(4, 222, bgcolor);	/* @ */
	init_pair(5, 210, bgcolor);	/* % */
	init_pair(6, 213, bgcolor);	/* $ */
	init_pair(7, 199, bgcolor);	/* - */
	init_pair(8, 196, bgcolor);	/* > */
	init_pair(9, 208, bgcolor);	/* & */
	init_pair(10, 226, bgcolor);	/* + */
	init_pair(11, 46, bgcolor);	/* # */
	init_pair(12, 33, bgcolor);	/* = */
	init_pair(13, 93, bgcolor);	/* ; */
	init_pair(14, 102, bgcolor);	/* * */

	bkgd(' ' | COLOR_PAIR(1));
}

void draw (char **catp, int frame, int h, int w, int y, int x) {
	int col, line, linecount, start;
	short color;
	char ch, chout;
	attr_t attr;

	start = frame * h;
	linecount = 0;
 
	for (line = start; line < start + h; line++) {
		/* print per character to set each color */
		col = 0;
		while ((ch = catp[line][col])) {
			attr = A_REVERSE;

			switch (ch) {
				case '.':
					color = 2;
					break;
				case '\'':
					color = 3;
					chout = '.';
					break;
				case '@':
					color = 4;
					break;
				case '%':
					color = 5;
					break;
				case '$':
					color = 6;
					break;
				case '-':
					color = 7;
					break;
				case '>':
					color = 8;
					break;
				case '&':
					color = 9;
					break;
				case '+':
					color = 10;
					break;
				case '#':
					color = 11;
					break;
				case '=':
					color = 12;
					break;
				case ';':
					color = 13;
					break;
				case '*':
					color = 14;
					break;
				case ' ':
				default:
					color = 0;
					attr = A_NORMAL;
					chout = ' ';
					break;
			}

			if (color_flag) {
				if (inverse_flag) {
					chout = ' ';
				} else {
					attr = A_NORMAL;

					if (ch != ' ') {
						chout = '#';
					}
				}
			} else {
				chout = ch;
				attr = A_NORMAL;
				color = 0;
			}

			mvwaddch(frames[frame], y + linecount, x + col, chout | attr | COLOR_PAIR(color));
			col++;
		}

		/* 2nd cat needs clrtoeol() to clean up right edge */
		clrtoeol();
		linecount++;
	}
}

int main (int argc, char *argv[]) {
	int opt, busy, frame, numframes;
	char **thecat;

	signal(SIGWINCH, resize_win);

	while ((opt = getopt(argc, argv, ":aBbd:lni")) != -1) {
		switch (opt) {
			case 'a':
				acat_flag = 1;
				break;
			case 'B':
				bgcolor = 16;
				break;
			case 'b':
				bgcolor = COLOR_BLACK;
				break;
			case 'd':
				delay = atoi(optarg);
				if (delay < 0 || delay > 100 || !isdigit(*optarg)) {
				    delay = 50;
				}
				break;
			case 'l':
				bgcolor = 17;
				break;
			case 'n':
				color_flag = 0;
				break;
			case 'i':
				inverse_flag = 1;
				break;
			default:
				printf("usage: %s [-aBblni] [-d 1..100] \n", argv[0]);
				exit(1);
		}
	}

	initscr();
	noecho();
	curs_set(0);
	timeout(delay);
	keypad(stdscr, TRUE);
	leaveok(stdscr, TRUE);

	init_colors();

	thecat = *cat;
	numframes = FRAMES;
	frameheight = CATHEIGHT;
	framewidth = CATWIDTH;

	if (acat_flag) {
		thecat = *acat;
		numframes = AFRAMES;
		frameheight = ACATHEIGHT;
		framewidth = ACATWIDTH;
	}

	yoffset = LINES / 2 - frameheight / 2;
	xoffset = COLS / 2 - framewidth / 2;

	/* init frames */
	for (int frame = 0; frame < numframes; frame++) {
		frames[frame] = dupwin(stdscr);
		syncok(frames[frame], TRUE);
		draw(thecat, frame, frameheight, framewidth, yoffset, xoffset);
	}

	frame = 0;
	busy = 1;

	while (busy) {
		switch (getch()) {
			case EOF:
			case KEY_RESIZE:
				break;
			default:
				busy = 0;
				break;
		}

		overwrite(frames[frame % numframes], stdscr);
		frame++;
	}

	quit();
	return 0;
}
