/*
 * choices.cpp
 *
 *  Created on: Sep 20, 2010
 *      Author: rwolff
 */

#include "choices.h"
#include "assert.h"
#include <errno.h>
#include <unistd.h>
#include <time.h>

//#define STDOUT_REDIR
#define TOPHEIGHTREQUIRED 12

choices::choices()
{
  // Setup input to non-buffered IO - could use ncurses easier, but then 'choices' relies on setup of ncurses.
//  unsigned char c;

  /* get the terminal settings for stdin */
  tcgetattr(STDIN_FILENO,&old_tio);

  /* we want to keep the old setting to restore them a the end */
  new_tio=old_tio;

  /* disable canonical mode (buffered i/o) and local echo */
  new_tio.c_lflag &=(~ICANON & ~ECHO);

  /* set the new settings immediately */
  tcsetattr(STDIN_FILENO,TCSANOW,&new_tio);

  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

//  do {
//           c=getchar();
//           printf("%d ",c);
//  } while(c!='q');
}

choices::~choices()
{
  /* restore the former settings */
  fcntl(STDIN_FILENO, F_SETFL, oldf);
  tcsetattr(STDIN_FILENO,TCSANOW,&old_tio);
}

int choices::getchar()
{
  int c;
  c = ::getchar();

  return c==-1 ? 0 : c;
};

updates* pup=0;

void do_resize( int sig_number )
{
  if (sig_number != SIGWINCH)
    printf("Hmm. Non-sigwinch?");
  else
    {
      // Now signal the updates class of the need to redraw.
      if (pup)
        pup->CursesSignal(RESIZE_NEEDED);
    }
}

#define THREADS_REQD 3

updates::updates() : ThreadMultiple(THREADS_REQD)
{
  int err;
  incDecAmount = 100000;
  activeItem = "";
  tempfifoname[0]=0;

  signals = 0;
  bCursesReady = false;
  saved_stdout = dup(1);

  // Sleep required - else had problems with ncurses drawing garbage - possibly was an old threading
  // problem, but sleeping for a second is no big shakes so we do it...
  setupStdoutReader();

  sleep(1);
  startAllThreads();

  while (!bCursesReady)
    sleep(1);

}

updates::~updates()
{
  if (fp_stdout_capture)
  {
    fprintf(fp_stdout_capture, "Closing stdout redirection at this time.\n");
    fclose(fp_stdout_capture);
  }

  stopAllThreads();

  if (strlen(tempfifoname))
    unlink(tempfifoname);
}

int updates::processChar(void)
{
  itempair::iterator it = items.begin();
  int cur;
  int c;
  WINDOW *pdialog;
  char tmpstr[10];

  c = getch();

  if (c<=0)
    return 0;

  // Now internally processed keys can be handled and then return '0'. Otherwise, return what came in.
  switch(c)
  {
  case 'p':
    fprintf(stdout, "Another **F**printf message.\n");
    printf("Another SIMPLE printf message WITH fflush().\n");
    fflush(stdout);
//    fflush(stdout);
    break;

  case 'P':
//    if (!fp_stdout_capture)
//      printf("Regular printf to say OUCH - cannot redirect stdout in curses.\n");
    fprintf(stdout, "Another **F**printf message WITH fflush.\n");
    fflush(stdout);
    if (!fp_stdout_capture)
    {
      cerr << "CAPTURE of STDOUT didn't work apparently. NULL on our copy." << endl;
      exit(-1);
    }
    else
    {
      fprintf(fp_stdout_capture, "SPECIAL - fp_stdout_capture - **F**printf message WITH fflush.\n");
      fflush(fp_stdout_capture);
    }
    break;

  case '+':
    setItemValue(activeItem.c_str(), getItemValue(activeItem.c_str())+incDecAmount);
    printItems();
    break;

  case '-':
    cur = getItemValue(activeItem.c_str());
    if (cur - incDecAmount > 0)
    {
      setItemValue(activeItem.c_str(), cur-incDecAmount);
      printItems();
    }
    break;

  case '<':
    if (incDecAmount >= 10)
      incDecAmount /= 10;
    break;

  case '>':
    if (incDecAmount < 1000000)
      incDecAmount *= 10;
    break;

  case KEY_LEFT:
  case 'h':
    it = items.find(activeItem);
    if (it != items.end())
    {
      --it;
      // When moving left beyond the beginning, you get 'end' next which is beyond the end by one. Adjust.
      if (it==items.end())
        --it;
      activeItem = it->first;
      printItems();
    }
    break;

  case KEY_RIGHT:
  case 'l':
    it = items.find(activeItem);
    if (it != items.end())
    {
      it++;
      if (it==items.end())
        it = items.begin();
      activeItem = it->first;
      printItems();
    }
    break;

  case 12:      // ^L for refresh
    CursesSignal(RESIZE_NEEDED);
    break;

  case '/':
    // TODO
    // Put up popup dialog to ask for a value.
    pdialog = newwin(7, 34, 5, maxx/2 - 34/2);
    nodelay(pdialog, false);
    box(pdialog, 0, 0);
    Standout(true);
    mvwaddstr(pdialog, 1, 2, "Please enter incr/decr amount.");
    Bold(true);
    mvwaddstr(pdialog, 3, 6, "New Value:");
    Bold(false);
    mvwaddstr(pdialog, 5, 2, "Press <enter> when done.");
    wrefresh(pdialog);
    Standout(false);
    echo();
//    nocbreak();
    cur = mvwgetnstr(pdialog, 3, 16, tmpstr, 7);
//    cbreak();
    noecho();
    if (atoi(tmpstr))
      incDecAmount = atoi(tmpstr);
    wclear(pdialog);
    wrefresh(pdialog);
    delwin(pdialog);
    CursesSignal(RESIZE_NEEDED);
    break;

    // If no other handler picks it up...then let the process above handle it.
  default:
    return c;
  }

  // It was handled in the switch as an actual case with a 'break' after it. So effectively drop the character now.
  return 0;
}

void updates::AddItem(const char* name)
{
  items[name]=-1;
  if (items.size()==1)
    activeItem = name;
};

int updates::getItemValue(const char* name)
{
  return items[name];
};

void updates::setItemValue(const char* name, int value)
{
  items[name] = value;
  if (items.size()==1)
    activeItem = name;
};

//#define USE_STDERR
#ifdef USE_STDERR
#define STD stderr
#else
#define STD stdout
#endif
int updates::redirectStdout(void)
{
#ifdef STDOUT_REDIR
  // Now open fifo for WRITING first (open of reader blocks until writer starts).
  // This requires doing freopen() on stdout.

  // Reassign stdout.
    if (!STD)
      exit(-10);

    fp_stdout_capture = freopen(tempfifoname, "w+", STD);
    if (!fp_stdout_capture)
      exit(-5);

    assert(fp_stdout_capture);

    if (!fp_stdout_capture || !stdout)
    {
      cerr << "Error re-assiging stdout to fifo: '" << tempfifoname << "'" << endl;
      return -1;
    }
#endif
    return 0;
}

void updates::setupStdoutReader(void)
{
#ifdef STDOUT_REDIR
  char *ptmp;
  int err;

  // Create a temporary fifo.
  ptmp = tempnam(NULL, "fifo");
  strcpy(tempfifoname, ptmp);

  if (strlen(tempfifoname))
  {
      // Create fifo
      err = mkfifo(tempfifoname, S_IRUSR| S_IWUSR);
      if (err)
      {
        cerr << "Error opening fifo: '" << tempfifoname << "'" << endl;
        exit(-1);
      }
  }
  else
  {
    cerr << "Temp name does not exist. tempnam() failed." << endl;
    exit(-1);
  }
#endif
}

int updates::StdoutReader(void)
{
#ifdef STDOUT_REDIR
  int fillindex;
  int ch;

  fflush(stdout);
  // Now open the reader.
  fp_read = fopen(tempfifoname, "r");
  if (!fp_read)
  {
    cerr << "Error opening the fifo: '" << tempfifoname << "' for reading." << endl;
    exit(-1);
  }

  LogLine("post-fopen(fifo) - thread - ready to read from the fifo/pipe.");
//  fflush(stdout);

  fillindex = 0;
  ch = 0;

  while (!bCursesReady)
    sleep(1);




  return 0;



  // Now read until EOF.
//  while (fp_read && !isStopRequested[0] && !feof(fp_read) && fgets(tmplongline, 199, fp_read))
  while (fp_read && !isStopRequested[0] && !feof(fp_read) && (ch=fgetc(fp_read))!=EOF)
  {

    if (ch <= 0)
      cerr << "ch was zero or less. Value=" << ch << endl;
    // We need to add the character to the tmplongline.
    tmplongline[fillindex++] = ch;

    if (ch=='\n' || fillindex==199)
    {
      tmplongline[fillindex]=0;         // Terminate null.
      LogLine(tmplongline);
//      cerr << tmplongline << endl;
      fillindex = 0;                    // Start anew.
    }

  }

  fclose(fp_read);
#endif
  return 0;
}

int updates::RealtimeItems(void)
{
  char datestr[40];
  char a_p_m;
  time_t rawtime;
  time_t start_time;
  struct tm *ptimeinfo;
  long runningtime;
  char runningstr[40];

  FILE* fp;
  char loadavgline[50];
  char loadavg[30];
  double floadavg;

  //
  // Initialize Curses as we will do all work on curses from this thread.
  //
  initscr();

  noecho();
  nonl();
  intrflush(stdscr,FALSE);
  keypad(stdscr,TRUE);
  nodelay(stdscr, true);

  signal(SIGWINCH, do_resize);
  clear();

  DrawMainWin();

  // Order is important here. Create the log window before starting the threads as the stdout redirector
  // requires access to logscr.
  // Lastly - after starting the threads, then it's ok to redirect stdout as the matching other side of the fifo is ready.
  CreateLogWin();

  ResizeLogWin();

//  LogLine("pre-redirect.");
  fp_stdout_capture = NULL;
// Not using the stdout redirection at this time due to troubles with ncurses 'stealing' it.
#ifdef STDOUT_REDIR
  // Must first re-open STDOUT_FILENO since curses has closed it.
  // Then setup for redirection.
  stdout = fdopen(STDOUT_FILENO, "a+");
  if (!stdout)
  {
    cerr << "Error re-opening stdout." << endl;
    exit(-1);
  }
#endif

  RefreshMaxXY();

  bCursesReady = true;

  time(&start_time);

  fp = fopen("/proc/loadavg", "r");
  if (!fp)
    strcpy(loadavg, "N/A");

  while (1)
    {
      // Cope with any pending signals first thing.
      if (signals)
      {
          if (signals & REDRAW_LOG)
          {
            signals &= ~REDRAW_LOG;     // Reset signal. Then process. Allows for another to come in.
            LogRedraw();
          }

          if (signals & RESIZE_NEEDED)
          {
            int newx, newy;
            struct winsize window_size;

              signals &= ~RESIZE_NEEDED;     // Reset signal. Then process. Allows for another to come in.

              if (ioctl (fileno (stdout), TIOCGWINSZ, &window_size) == 0)
              {
                  newx = (int) window_size.ws_col;
                  newy = (int) window_size.ws_row;
              }

              assert(newx!=-1 && newy!=-1);
              resizeterm(newy, newx);

              redrawwin(stdscr);
              doupdate();
              ResizeLogWin();
          }

          if (signals & REDRAW_ITEMS)
          {
            signals &= ~REDRAW_ITEMS;     // Reset signal. Then process. Allows for another to come in.
            printItems();
          }
      }

      // Get load average information.
      if (fp)
      {
        fgets(loadavgline, 49, fp);
        floadavg = atof(loadavgline);
        sprintf(loadavg, "System-Load: %4.2lf ", floadavg);
        rewind(fp);
        fflush(fp);
      }

      // Get local time and update 'runtime'
      time(&rawtime);
      runningtime = (long) difftime(rawtime, start_time);
      ptimeinfo = localtime(&rawtime);

      if (ptimeinfo->tm_hour >= 12)
      {
        a_p_m = 'p';
        ptimeinfo->tm_hour = ptimeinfo->tm_hour - 12;
      }
      else
        a_p_m = 'a';

//    ... sprintf hh/mm/ss and difftime hhh/mm/ss or xhrs ymins zsecs
      sprintf(datestr, "Current Time: %02d:%02d:%02d %cm ",ptimeinfo->tm_hour, ptimeinfo->tm_min, ptimeinfo->tm_sec, a_p_m);
      if (runningtime > 60*60*24)
        strcpy(runningstr, "More than 1 day.         ");
      else
      {
        if (runningtime/(60*60)>0)
          sprintf(runningstr, "Running: %ld hrs,%ld mins,%ld secs. ",runningtime/(60*60), (runningtime/60)%60, runningtime%60);
        else if (runningtime/60 > 0)
          sprintf(runningstr, "Running: %ld mins,%ld seconds. ",runningtime/60, runningtime%60);
        else
          sprintf(runningstr, "Running: %ld seconds. ",runningtime%60);
      }

      // Now do our work on the display here.
      putStringAt(2, 1, datestr);
      putStringAt(2, 2, runningstr);
      putStringAt(2, 3, loadavg);

      // Now show the increment/decrement amount.

      sprintf(loadavgline, "+/- = Incr/Decr by:%7ld", incDecAmount);
      Standout(true);
      putStringAt(maxx-28, 1, loadavgline);
      Standout(false);
      putStringAt(maxx-27, 2, "Change Inc/Dec via '<>/'");

      if (isStopRequested[1])
        break;

      usleep(250000);
    }

  fclose(fp);

  // Undo all curses items.
  Finalize();

  return 0;
}

int updates::workerBeeSplitter(int tn)
{
   if (tn >= THREADS_REQD) return -1; // Error - more than the orignal desired # of threads. Should never happen.
   switch(tn)
   {
     case 0:
       return StdoutReader();
       break;
     case 1:
       return RealtimeItems();
       break;
     case 2:
       return redirectStdout();
       break;
     default:        // Should never happen. Return error.
       assert(false);
       return -1;
       break;
   }

   return 0;
}

void updates::DrawMainWin(void)
{
  Standout(true);
  putStringAtCenter(0, "                  ");
  putStringAtCenter(1, "  XVD Technology  ");
  putStringAtCenter(2, "                  ");
  Standout(false);
  Bold(true);
  putStringAtCenter(3, "DDS Test Jig");
  putStringAtCenter(4, "Version 0.60");
  Bold(false);

  Standout(true);
  putStringAtCenter(6, "----- Live Statistics -----");
  Standout(false);

  if (items.size()==0)
  {
    move(7,0);
    clrtoeol();
    move(8,0);
    clrtoeol();
    putStringAtCenter(8, "~~~ No Data ~~~");
  }

  refresh();
}

void updates::printItems(void)
{
  string headers="";
  string values="";
  string spaces = "                                                        ";
  string stars  = "********************************************************";
  string columnspace = "";
  int maxline = maxx - 2;
  int linelength = 0;

  if (items.size()==0)
    return;

  // How long is the sum of the headers?
  for(itempair::const_iterator it=items.begin(); it != items.end(); ++it)
    linelength += it->first.length();
  // Now -- spread the headers out nicely by figuring out how many spaces on either side are possible.
  //
  // #NAME1$$$NAME2$$$NAME3$$$NAME4#
  // One hardwired space at both ends (by virtue of maxline)
  // Use n-1 divisions ... maxline / (n-1) is the # of spaces to put in between each.
  if ((maxline - linelength) < 2)
    columnspace = "";
  else  if (items.size() <= 2)          // Splitting by nothing or by one column.
    columnspace = spaces.substr(0, (maxline-linelength)/2);
  else
    columnspace = spaces.substr(0,(maxline-linelength)/(items.size()-1));

  itempair::const_iterator end = items.end();
  for(itempair::const_iterator it=items.begin(); it != end; ++it)
  {
    string curheader;
    string curvalue;
    string temp;
    char scratch[20];

    curheader = it->first;

    sprintf(scratch, "%d",it->second);
    temp = scratch;
//    temp = itoa(it->second);
    // Fill with spaces to start.
    curvalue = spaces.substr(0,curheader.length());
    // Now substitute the value in the middle of the string.
    // If header is smaller than value in length, treat specially.
    if (curheader.length() < temp.length())
      curvalue = stars.substr(0, curheader.length());
    else
      curvalue.replace( (curheader.length()/2) - (temp.length()/2), temp.length(), temp);

    // Now add column space and place the value in the center.
    // Do not add column space for last entry in the list however.
    // To accomplish this, after for-loop simply lop off the last column-space items.
    curheader += columnspace;
    curvalue  += columnspace;

    headers += curheader;
    values  += curvalue;
  }

  headers = headers.substr(0, headers.length()-columnspace.length());
  values  = values.substr(0, values.length()-columnspace.length());

  // Send out the data.
  move(7,0);
  clrtoeol();
  move(8,0);
  clrtoeol();
  putStringAtCenter(7, headers.c_str());
  putStringAtCenter(8, values.c_str());

  // Now highlight the selected heading.
  // First, find the position of the active header in the final headers string.
  // Then use the length of that header to know the "x/y zone" for highlight and highlight both the
  // header line and the values line in the same zone.
  if (activeItem != "")
  {
    int start_x;
    int len;

    // From centering the final headers line -- we should be able to calculate the first column of that line.
    // Remember a left-side buffer of one space was used in calculating maxline so adjust by one.
    start_x = 1 + headers.find(activeItem) + (maxline/2 - headers.length()/2);

    len = activeItem.length();
    // Now calculate the starting offset based upon headers being centered on screen.

    // Highlight.
    mvchgat(7, start_x, len, A_STANDOUT, 1, NULL);
    mvchgat(8, start_x, len, A_STANDOUT, 1, NULL);
  }
}

void updates::CreateLogWin(void)
{
  int logstarty, logendy, logwidth;
  int err;

  // Calculate the actual LOGGING area (not the border...use +1 / -1 for borders.
  logstarty = LINES/2 + 1;              // If odd height, starts @ same location.
  logendy = LINES-2;
  logwidth = COLS-2;

  cerr << "Creating LogWin - logwidth=" << logwidth << " COLS=" << COLS << " LINES=" << LINES << " getCurMaxY=="<<getCurMaxY() << " getCurMaxX()=="<<getCurMaxX()<<endl;

  // First create a bogus window just to get borders outside the logscr.
  logborderscr = newwin(logendy-logstarty+3, logwidth+2, logstarty-1, 0);
//  logborderscr = 0;

  // Now create the window inside the border area.
  logscr = newwin(logendy-logstarty+1, logwidth, logstarty, 1);
  // Turn scrolling on.
  scrollok(logscr, true);
  err = wsetscrreg(logscr, 0, logendy-logstarty);
  assert(!err);

  // Set for logging.
  wmove(logscr, 0, 0);
}

void updates::ResizeLogWin(void)
{
  int logstarty, logendy, logwidth;
  int err;
  static int lastHeight=-1;
  int deltaHeight;

  if (lastHeight==-1)
    lastHeight=getCurMaxY();

  // Calculate the actual LOGGING area (not the border...use +1 / -1 for borders.
  logstarty = getCurMaxY()/2 + 1;              // If odd height, starts @ same location.
  logendy = getCurMaxY()-2;
  logwidth = getCurMaxX()-2;

  erase();      // Start w a clean slate.
  DrawMainWin();
  printItems();

  // Now one mod - if minimum screen height is satisfied, then make starty there rather than half vert.
  if (logstarty > TOPHEIGHTREQUIRED)
    logstarty = TOPHEIGHTREQUIRED;

  // First create a bogus window just to get borders outside the logscr.
  logborderscr->_begy = logstarty-1;
  wresize(logborderscr, logendy-logstarty+3, logwidth+2);
//  wclear(logborderscr);
  box(logborderscr, 0 , 0);
  mvwaddstr(logborderscr, 0, 2, " Log Output: ");

  // Now create the window inside the border area.
  logscr->_begy = logstarty;
  wresize(logscr, logendy-logstarty+1, logwidth);

  // Now -- do we need to scroll up or down the current data in the log screen due to changed height?
  deltaHeight = lastHeight != (logendy-logstarty+1);
  if (deltaHeight)
    wscrl(logscr, deltaHeight);

  touchwin(stdscr);
  refresh();
  touchwin(logborderscr);
  wrefresh(logborderscr);

  // Now - redraw tail of loghistory into logscr.
  LogRedraw();
}

void updates::LogRedraw(void)
{
  int height = logscr->_maxy + 1;
  int i;
  logs::iterator it;

  // Clear log area and prep for drawing items.
  wclear(logscr);
  wmove(logscr, 0, 0);

  pthread_mutex_lock (&mutex);

  it = loghistory.end();
  // reverse iterating through the list to get to our highest entry to print out.
  for (i=0 ; i<height && it != loghistory.begin() ; i++)
    --it;

  // Ready to start moving forward now
  while(it != loghistory.end())
  {
      waddch(logscr, '\n');
      waddstr(logscr, it->c_str());
      it++;
  }

  pthread_mutex_unlock (&mutex);

  wrefresh(logscr);
}

void updates::LogLine(const char* pstr)
{
  int ax, ay;
  string to_log;

  pthread_mutex_lock (&mutex);

  // We are chopping off any trailing '\n' character to make best use of the scrolling region.
  // Therefore on each next line, we much add a '\n' to pre-scroll prior to printing.
  // However this action is taken during the actual redraw. So the log_history is clean.
  to_log += pstr;

  // If trailing '\n' on the pstr input, effectively lop off the trailing item.
  if (pstr[strlen(pstr)-1]=='\n')
    to_log = to_log.substr(0, to_log.length()-1);

  loghistory.push_back(to_log);

  if (loghistory.size()>MAX_LOGSIZE)
    loghistory.pop_front();

  pthread_mutex_unlock (&mutex);

  CursesSignal(REDRAW_LOG);
}

void updates::RefreshAll(void)
{
  refresh();
  wrefresh(logborderscr);
  wrefresh(logscr);
}

void updates::Finalize(void)
{
  nl();
  echo();
  delwin(logborderscr);
  delwin(logscr);
  refresh();
  endwin();
  // Make sure stdout is healthy for exit from curses - restoring if it were closed by curses and not re-directed by us.
#ifdef STDOUT_REDIR
  stdout = fdopen(saved_stdout,"a+");
//  stdout = fdopen(STDOUT_FILENO,"a+");
#endif
  printf("\n\n");
}
