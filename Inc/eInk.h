#ifndef EINK_H
#define EINK_H

#if 0
/* Default is GDEH0154D27. */
#define GDEH0154D27
#else
#define DEPG0154BNS75AF0
#endif

#ifdef GDEH0154D27
#include "gdeh0154d27.h"
#endif

#ifdef DEPG0154BNS75AF0
#include "DEPG0154BNS75AF0.h"
#endif

#endif
/* End of this file. */
