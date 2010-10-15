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

// includes for CCriticalSection class
#ifdef _WIN32
#       include <windows.h>
#else
#       include <unistd.h>
#       include <pthread.h>
#endif

#ifndef CHOICES_H_
#define CHOICES_H_

typedef map<string, int> itempair;
typedef deque<string> logs;
#define MAX_LOGSIZE 1000

/**
 * @class A wrapper-class around Critical Section functionality, WIN32 & PTHREADS.
 */
class CCriticalSection
{
public:
        /**
         * @brief CriticalSection class constructor.
         */
        explicit CCriticalSection(void)
        {
        #ifdef _WIN32
                if (0 == InitializeCriticalSectionAndSpinCount(&m_cSection, 0))
                        throw("Could not create a CriticalSection");
        #else
                if (pthread_mutex_init(&m_cSection, NULL) != 0)
                        throw("Could not create a CriticalSection");
        #endif
        }; // CriticalSection()

        /**
         * @brief CriticalSection class destructor
         */
        ~CCriticalSection(void)
        {
                WaitForFinish(); // acquire ownership
        #ifdef _WIN32
                DeleteCriticalSection(&m_cSection);
        #else
                pthread_mutex_destroy(&m_cSection);
        #endif
        }; // ~CriticalSection()

        /**
         * @fn void WaitForFinish(void)
         * @brief Waits for the critical section to unlock.
         * This function puts the waiting thread in a waiting
         * state.
         * @see TryEnter()
         * @return void
         */
        void WaitForFinish(void)
        {
                while(!TryEnter())
                {
                #ifdef _WIN32
                        Sleep(1); // put waiting thread to sleep for 1ms
                #else
                        usleep(1000); // put waiting thread to sleep for 1ms (1000us)
                #endif
                };
        }; // WaitForFinish()

        /**
         * @fn void Enter(void)
         * @brief Wait for unlock and enter the CriticalSection object.
         * @see TryEnter()
         * @return void
         */
        void Enter(void)
        {
                WaitForFinish(); // acquire ownership
        #ifdef _WIN32
                EnterCriticalSection(&m_cSection);
        #else
                pthread_mutex_lock(&m_cSection);
        #endif
        }; // Enter()

        /**
         * @fn void Leave(void)
         * @brief Leaves the critical section object.
         * This function will only work if the current thread
         * holds the current lock on the CriticalSection object
         * called by Enter()
         * @see Enter()
         * @return void
         */
        void Leave(void)
        {
        #ifdef _WIN32
                LeaveCriticalSection(&m_cSection);
        #else
                pthread_mutex_unlock(&m_cSection);
        #endif
        }; // Leave()

        /**
         * @fn bool TryEnter(void)
         * @brief Attempt to enter the CriticalSection object
         * @return bool(true) on success, bool(false) if otherwise
         */
        bool TryEnter(void)
        {
                // Attempt to acquire ownership:
        #ifdef _WIN32
                return(TRUE == TryEnterCriticalSection(&m_cSection));
        #else
                return(0 == pthread_mutex_trylock(&m_cSection));
        #endif
        }; // TryEnter()

private:
#ifdef _WIN32
        CRITICAL_SECTION m_cSection; //!&lt; internal system critical section object (windows)
#else
        pthread_mutex_t m_cSection; //!&lt; internal system critical section object (*nix)
#endif
}; // class CriticalSection

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

#define REDRAW_LOG      (int)(1 << 0)
#define RESIZE_NEEDED   (int)(1 << 1)
#define REDRAW_ITEMS    (int)(1 << 2)

class updates : public ThreadMultiple, CCriticalSection
{
public:
  updates();
  virtual  ~updates();
  void DrawMainWin(void);
  void clear() { erase(); };
  int processChar(void);
  void putStringAt(int x, int y, const char* str) { mvaddstr(y, x, str); refresh(); };
  void putStringAtCenter(int y, const char* str) { int cx=maxx/2 - (strlen(str)/2); mvaddstr(y, cx, str); refresh(); };
  void Finalize(void);

  // Curses must be run from a single thread. So we must have a signalling mechanism
  // to facilitate other threads letting curses know it needs refreshed, resized, redrawn, etc.
  // REDRAW_LOG, RESIZE_NEEDED, ITEMS_NEEDED
  int signals;
  void CursesSignal(int sig) { signals |= sig; };

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
  void RefreshMaxXY(void) { maxx = COLS; maxy = LINES; };  // getmaxyx(stdscr,maxy,maxx); };

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
  int saved_stdout;
  FILE* fp_read;
  char tmplongline[201];
  bool bCursesReady;

};

#endif /* CHOICES_H_ */
