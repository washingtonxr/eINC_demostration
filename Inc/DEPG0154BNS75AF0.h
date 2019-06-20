#ifndef DEPG0154BNS75AF0_H
#define DEPG0154BNS75AF0_H
#include "common.h"

#define DKE154

#define ALLSCREEN_GRAGHBYTES 2888
#define PART_GRAGHBYTES	1024

//#define MAX_LINE_BYTES 16// =128/8
#define MAX_CHARLINE_BYTES 12//
//#define MAX_COLUMN_BYTES  250

#define LCDChar_MAX_LINE_NUM    7      //8*16  128/16=8
#define LCDChar_MAX_LINE_LENGTH    37 //8*16 296/8=37

#define MONOMSB_MODE 1
#define MONOLSB_MODE 2 
#define RED_MODE     3

#define PART_LINE_BYTES    8//  
#define PART_COLUMN_BYTES  128//

#define MONO_MODE     1
#define RED_MODE_21      2

#ifdef DKE154
#define MAX_LINE_BYTES      19
#define MAX_COLUMN_BYTES    152
#define ALLSCREEN_BYTES     2888
#endif

#ifdef DKE213_1
#define MAX_LINE_BYTES      16
#define MAX_COLUMN_BYTES    250 
#define ALLSCREEN_BYTES     4000
#endif

#ifdef DKE213_2
#define MAX_LINE_BYTES      13
#define MAX_COLUMN_BYTES    212  
#define ALLSCREEN_BYTES     2756
#endif

#ifdef DKE290
#define MAX_LINE_BYTES      16
#define MAX_COLUMN_BYTES    296
#define ALLSCREEN_BYTES     4736
#endif

#ifdef DKE420
#define MAX_LINE_BYTES    50
#define MAX_COLUMN_BYTES  300
#define ALLSCREEN_BYTES   15000
#endif


int EPD_Task(void);

#endif
/* End of this file. */
