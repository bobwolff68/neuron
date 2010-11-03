//!
//! \file skipfiles.h
//! \brief Use <b>\#define SKIP_<CODENAME></b> to signal whether to compile-in or -out certain
//!        modules on various platforms.
//!
//! Usage of \#define SKIP_* in this file and the associated modules.
//!
//!    As an example, we imagine a module of the Common library as threads. And suppose
//!    we know that threads wont compile on WIN32 nor on PSOS.\n
//!    -# In the WIN32 area, <b>\#define SKIP_THREADS</b> and in the PSOS section <b>\#define SKIP_THREADS.</b>\n
//!    -# In the threads module, one must <b>\#include "skipfiles.h"</b> and then <b>\#ifndef SKIP_THREADS</b>
//!       followed by the active code, followed by <b>\#endif</b>\n
//!    -# For modules who are valid on all platforms, they may wish to not include skipfiles.h at all.\n
//!    This will help to ensure uniformity of what is and what is not portable on any given platform.
//!
//! \author Bob Wolff (rwolff)
//! \date June 15th, 2010
//!

#ifndef SKIPFILES_H_
#define SKIPFILES_H_

#ifdef WIN32
#endif

#ifdef LINUX
#endif

#ifdef PSOS
#endif

#endif /* SKIPFILES_H_ */
