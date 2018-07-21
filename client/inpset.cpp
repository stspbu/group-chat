#include "inpset.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>

void initTermios(bool echo) {
  tcgetattr(0, &old); /* grab old terminal i/o settings */
  news = old; /* make new settings same as old settings */
  news.c_lflag &= ~ICANON; /* disable buffered i/o */
  if (echo) {
      news.c_lflag |= ECHO; /* set echo mode */
  } else {
      news.c_lflag &= ~ECHO; /* set no echo mode */
  }
  tcsetattr(0, TCSANOW, &news); /* use these new terminal i/o settings now */
}

void resetTermios() {
  tcsetattr(0, TCSANOW, &old);
}

unsigned int get_term_width() {
    struct winsize ws;
    if (ioctl( STDIN_FILENO , TIOCGWINSZ, &ws ) != 0 &&
        ioctl( STDOUT_FILENO, TIOCGWINSZ, &ws ) != 0 &&
        ioctl( STDERR_FILENO, TIOCGWINSZ, &ws ) != 0 ) {
        
        fprintf(stderr, "ioctl() failed (%d): %s\n", errno, strerror( errno ));
        return 0;
    }
    return ws.ws_col;
}