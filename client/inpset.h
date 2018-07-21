#pragma once
#include <termios.h>

static struct termios old, news;


void initTermios(bool);             // disable buffered i/o
void resetTermios();                // backup old settings
unsigned int get_term_width();