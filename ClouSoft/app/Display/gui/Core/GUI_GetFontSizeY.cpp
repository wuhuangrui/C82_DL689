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
File        : GUI_GetFontSizeY.c
Purpose     : Implementation of optional routine
---------------------------END-OF-HEADER------------------------------
*/

#include "stdafx.h"

#include <stddef.h>           /* needed for definition of NULL */
#include <stdio.h>
#include <string.h>
#include "GUI_Protected.h"


/*********************************************************************
*
*        GUI_GetFontSizeY
*/
int GUI_GetFontSizeY(void) {
  int r;
  GUI_LOCK();
  r = GUI__GetFontSizeY();
  GUI_UNLOCK();
  return r;
}



