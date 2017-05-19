/*
*********************************************************************************************************
*                                                uC/GUI
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              µC/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : GUI_CursorArrowL.C
Purpose     : Defines the arrow cursor, large inverted
---------------------------END-OF-HEADER------------------------------
*/
#include "stdafx.h"

#include <stdlib.h>
#include "GUI_Protected.h"

const GUI_BITMAP GUI_BitmapArrowLI = {
 18,                  /* XSize */
 30,                  /* YSize */
 5,                   /* BytesPerLine */
 2,                   /* BitsPerPixel */
 GUI_Pixels_ArrowL,   /* Pointer to picture data (indices) */
 &GUI_CursorPalI      /* Pointer to palette */
};

const GUI_CURSOR GUI_CursorArrowLI = {
  &GUI_BitmapArrowLI, 0, 0
};

/* *** End of file *** */
