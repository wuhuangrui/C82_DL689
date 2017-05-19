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
File        : GUI_SetPhysColor.c
Purpose     : Implementation of GUI_SetPhysColor
---------------------------END-OF-HEADER------------------------------
*/

#include "stdafx.h"

#include "GUI_Protected.h"
#include "LCD_ConfDefaults.h"


/*********************************************************************
*
*      Public code
*
**********************************************************************
*/

void GUI_SetPhysColor(U8 Pos, LCD_COLOR Color) {
  #if LCD_PHYSCOLORS_IN_RAM
    GUI_LOCK();
    LCD_PhysColors[Pos] = Color;
    LCD_SetLUTEntry(Pos, Color);
    GUI_UNLOCK();
  #else
    GUI_USE_PARA(Pos);
    GUI_USE_PARA(Color);
  #endif
}





