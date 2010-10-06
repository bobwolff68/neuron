/*
 * choices.h
 *
 *  Created on: Sep 20, 2010
 *      Author: rwolff
 */
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <ncurses.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <term.h>

// For fifos, and redirecting...
#include <stdlib.h>
#include <unistd.h>

#include "NeuronBasics/ThreadMultiple.h"
#include <assert.h>

#include <string>
#include <map>
#include <deque>

#ifndef CHOICES_H_
#define CHOICES_H_

typedef map<string, int> itempair;
typedef deque<string> logs;
#define MAX_LOGSIZE 1000

class choices
{
public:
  choices();
  virtual  ~choices();
  int getchar();
protected:
  struct termios old_tio, new_tio;
  int oldf;
};

class updates : public ThreadMultiple
{
public:
  updates();
  virtual  ~updates();
  void DrawMainWin(void);
  void clear() { erase(); };
  int processChar(int c);
  void putStringAt(int x, int y, const char* str) { mvaddstr(y, x, str); refresh(); };
  void putStringAtCenter(int y, const char* str) { int cx=maxx/2 - (strlen(str)/2); mvaddstr(y, cx, str); refresh(); };
  void Finalize(void);

  void Bold(bool on) { if (on) attron(A_BOLD); else attroff(A_BOLD); };
  void Standout(bool on) { if (on) attron(A_STANDOUT); else attroff(A_STANDOUT); };
  void Blink(bool on) { if (on) attron(A_BLINK); else attroff(A_BLINK); };
  void Reverse(bool on) { if (on) attron(A_REVERSE); else attroff(A_REVERSE); };

  int getCurMaxX(void) { RefreshMaxXY(); return maxx; };
  int getCurMaxY(void) { RefreshMaxXY(); return maxy; };

  void LogLine(const char* pstr);
  void LogUp(void) { wscrl(logscr, 1); wrefresh(logscr); };
  void LogDown(void) { wscrl(logscr, -1); wrefresh(logscr);};
  void LogRedraw(void);

  void RefreshAll(void);
  void ResizeLogWin(void);

  void AddItem(const char* name);
  int getItemValue(const char* name);
  void setItemValue(const char* name, int value);
  void printItems(void);

protected:
  void CreateLogWin(void);
  void RefreshMaxXY(void) { getmaxyx(stdscr,maxy,maxx); };

  int redirectStdout(void);
  void setupStdoutReader(void);
  int StdoutReader(void);
  int RealtimeItems(void);
  int workerBeeSplitter(int tn);

  int maxx, maxy;
  WINDOW* logscr;
  WINDOW* logborderscr;
  itempair items;
  logs loghistory;
  string activeItem;
  long incDecAmount;

  char tempfifoname[TMP_MAX];
  FILE* fp_stdout_capture;
  FILE* fp_read;
  char tmplongline[200];

};

#endif /* CHOICES_H_ */
