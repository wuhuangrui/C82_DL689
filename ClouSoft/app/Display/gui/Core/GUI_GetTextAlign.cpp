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
File        : GUI_GetTextAlign.c
Purpose     : Implementation of optional routines
---------------------------END-OF-HEADER------------------------------
*/
#include "stdafx.h"


#include "GUI_Protected.h"


int GUI_GetTextAlign(void) {
  int r;
  GUI_LOCK();
  r = GUI_Context.TextAlign;
  GUI_UNLOCK();
  return r;
}



