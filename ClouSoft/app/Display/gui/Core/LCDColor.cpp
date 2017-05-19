/*
*********************************************************************************************************
*                                                uC/GUI
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              �C/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : LCDColor.C
Purpose     : Color conversion routines for LCD-drivers
---------------------------END-OF-HEADER------------------------------
*/

#define LCDCOLOR_C
#include "stdafx.h"

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI.H"
#include "LCD_Private.h"      /* inter modul definitions & Config */
#include "Guidebug.h"


/*
        *********************************************************
        *                                                       *
        *       Config defaults                                 *
        *                                                       *
        *********************************************************
*/
#ifndef  LCD_SIZEOF_COLORCACHE
  #define LCD_SIZEOF_COLORCACHE 0
#endif


/*
        *********************************************************
        *                                                       *
        *       Caching (optional)                              *
        *                                                       *
        *********************************************************
*/

#if  LCD_SIZEOF_COLORCACHE
  static  const LCD_LOGPALETTE * pLogPalCache;
#endif



  /*
          *********************************************************
          *                                                       *
          *       Build color conversion table                    *
          *                                                       *
          *********************************************************
  */
LCD_PIXELINDEX LCD__aConvTable[LCD_MAX_LOG_COLORS];

LCD_PIXELINDEX* LCD_GetpPalConvTableUncached(const LCD_LOGPALETTE*  pLogPal) {
  if (pLogPal == NULL) {
    return NULL;
  }
  if (!pLogPal->pPalEntries) {
    return NULL;
  }
  /* Check if sufficient space is available */
  if (pLogPal->NumEntries > LCD_MAX_LOG_COLORS) {
    return NULL;
  }
  /* Build conversion table */
  {
    int i;
    int NumEntries = pLogPal->NumEntries;
    const LCD_COLOR* pPalEntry = &pLogPal->pPalEntries[0];
    for (i=0; i< NumEntries; i++) {
      LCD__aConvTable[i] = LCD_Color2Index(*(pPalEntry+i));
    }
  }
  return &LCD__aConvTable[0];
}

LCD_PIXELINDEX* LCD_GetpPalConvTable(const LCD_LOGPALETTE*  pLogPal) {
/* Check cache */
  #if  LCD_SIZEOF_COLORCACHE
    if (pLogPalCache == pLogPal) {
      return &LCD__aConvTable[0];
    }
    pLogPalCache = pLogPal;
  #endif
  return LCD_GetpPalConvTableUncached(pLogPal);
}



/*
        *********************************************************
        *                                                       *
        *                   LCD_InitLUT                         *
        *                                                       *
        *********************************************************
*/

void LCD_InitLUT(void) {
  #if (LCD_BITSPERPIXEL <= 8)
    {
      int i;
      for (i=0; i < LCD_NUM_COLORS; i++) {
        LCD_COLOR color = LCD_Index2Color((U8)i);
        #if LCD_REVERSE_LUT
          color ^= 0xffffff;    /* Invert R,G,B components */
        #endif
        LCD_L0_SetLUTEntry((U8)i, color);
      }
    }
  #endif
  #if (LCD_NUM_DISPLAYS > 1)
    #if (LCD_BITSPERPIXEL_1 <= 8)
    {
      int i;
      int DisplayOld = GUI_SelLCD(1);
      for (i=0; i < LCD_NUM_COLORS_1; i++) {
        LCD_COLOR color = LCD_Index2Color((U8)i);
        #if LCD_REVERSE_LUT
          color ^= 0xffffff;    /* Invert R,G,B components */
        #endif
        LCD_L0_1_SetLUTEntry((U8)i, color);
      }
      GUI_SelLCD(DisplayOld);
    }
    #endif
  #endif
}



