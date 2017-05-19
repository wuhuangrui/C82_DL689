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
File        : GUI_SetFont.C
Purpose     : Optional routines
----------------------------------------------------------------------
*/
#include "stdafx.h"

#include "GUI_Protected.h"
#include "Guidebug.h"

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/


/***********************************************************
*
*     GUI_SetFont

*/
const GUI_FONT* GUI_SetFont(const GUI_FONT* pNewFont) {
  const GUI_FONT * pOldFont = GUI_Context.pAFont;
  GUI_LOCK();
  if (pNewFont)
    GUI_Context.pAFont = pNewFont;
  GUI_UNLOCK();
  return pOldFont;
}




