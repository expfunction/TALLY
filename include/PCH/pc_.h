/* Copyright (c) 2026 Burak Yazar */

/* CVGA Precompiled header file */
#ifndef PC_H
#define PC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <limits.h>

#ifndef PLATFORM_WIN64
/* DOS/DJGPP-specific headers */
#include <conio.h>
#include <dos.h>
#include <dir.h>
#include <pc.h>
#include <keys.h>
#include <dpmi.h>
#include <go32.h>
#include <unistd.h>
#include <sys/movedata.h>
#include <sys/farptr.h>
#include <sys/nearptr.h>
#else
/* Win32: platform compatibility layer */
#include "CORE/PLATFRM.H"
#endif

#endif /* PC_H */