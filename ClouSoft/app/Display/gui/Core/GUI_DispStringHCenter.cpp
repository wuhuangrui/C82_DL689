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
File        : GUI_DispStringHCenter.c
Purpose     : Implementation of optional routine
---------------------------END-OF-HEADER------------------------------
*/
#include "stdafx.h"


#include <stddef.h>           /* needed for definition of NULL */
#include <stdio.h>
#include <string.h>
#include "GUI_Protected.h"


/*
	*********************************
	*                               *
	*	  Disp string                 *
  *   horizontally centered       *
	*                               *
	*********************************
*/

void GUI_DispStringHCenterAt(const char GUI_FAR *s, int x, int y) {
  int Align;
	GUI_LOCK();
  Align = GUI_SetTextAlign((GUI_Context.TextAlign&~GUI_TA_LEFT)|GUI_TA_CENTER);
  GUI_DispStringAt (s,x, y);
  GUI_SetTextAlign(Align);
  GUI_UNLOCK();
}

void GUI_DispStringHCenter(const char GUI_FAR *s) {
  int Align;
	GUI_LOCK();
  Align = GUI_SetTextAlign((GUI_Context.TextAlign&~GUI_TA_HORIZONTAL)|GUI_TA_CENTER);
  GUI_DispString (s);
  GUI_SetTextAlign(Align);
  GUI_UNLOCK();
}



