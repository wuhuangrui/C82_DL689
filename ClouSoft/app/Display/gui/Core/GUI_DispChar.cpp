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
File        : GUI_DispChar.c
Purpose     : Implementation of optional routines
---------------------------END-OF-HEADER------------------------------
*/
#include "stdafx.h"


#include "GUI_Protected.h"



#if (GUI_WINSUPPORT)
/* Note on usage of this routine:
   If at all possible, try to avoid using this, since
   every call will invoke the window manager. If possible,
   use the string routines.
*/
static void CL_DispChar(U16 c) {
  GUI_RECT r;
  WM_ADDORG(GUI_Context.DispPosX, GUI_Context.DispPosY);
  r.x1 = (r.x0 = GUI_Context.DispPosX) + GUI_GetCharDistX(c)-1;
  r.y1 = (r.y0 = GUI_Context.DispPosY) + GUI_GetFontSizeY()-1;
  WM_ITERATE_START(&r) {
    GL_DispChar(c);
    if (c != '\n')
      GUI_Context.DispPosX = r.x1 + 1;
  } WM_ITERATE_END();
  WM_SUBORG(GUI_Context.DispPosX,GUI_Context.DispPosY);
}
#endif

void GUI_DispChar(U16 c) {
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    CL_DispChar(c);
  #else
    GL_DispChar(c);
  #endif
  GUI_UNLOCK();
}



/***********************************************************
*
*             GL/CL DispCharAt
*
************************************************************

*/

void GUI_DispCharAt(U16 c, I16P x, I16P y) {
  GUI_LOCK();
  GUI_Context.DispPosX =x;
  GUI_Context.DispPosY =y;
  #if (GUI_WINSUPPORT)
    CL_DispChar(c);
  #else
    GL_DispChar(c);
  #endif
  GUI_UNLOCK();
}





