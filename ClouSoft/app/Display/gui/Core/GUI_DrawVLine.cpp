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
File        : GUI_DrawVLine.C
Purpose     : Implementation of GUI_DrawVLine
---------------------------END-OF-HEADER------------------------------
*/
#include "stdafx.h"

#include "GUI_Protected.h"

/*********************************************************************
*
*             GUI_DrawVLine
*
**********************************************************************
*/


void GUI_DrawVLine(int x0, int y0, int y1) {
  #if (GUI_WINSUPPORT)
    GUI_RECT r;
  #endif
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORG(x0,y0);
    WM_ADDORGY(y1);
    r.x1 = r.x0 = x0;
    r.y0 = y0;
    r.y1 = y1;
    WM_ITERATE_START(&r); {
  #endif
  LCD_DrawVLine(x0, y0, y1);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
}


