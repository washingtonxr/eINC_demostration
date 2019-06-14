#ifndef COMMON_H
#define COMMON_H

#include "usart.h"
//#include "cmsis_os.h"

#define  configDebugPrintf   1

#if   (1 == configDebugPrintf)
    #define  Debug_printf_detail(fmt,args...)   printf(fmt"----[file name = %s  line num = %d]""\n",__FILE__,__LINE__,##args)
    #define  Debug_printf(fmt,args...)   printf(fmt)
    
#else
    #define Debug_printf_detail(fmt,args...)
    #define Debug_printf(fmt,args...)
#endif
//#undef __GNUC__
#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

#endif
/* End of this file. */
