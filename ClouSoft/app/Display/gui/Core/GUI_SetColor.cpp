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
 File        : GUI_SetColor.c
 Purpose     : Implementation of optional routines
 ---------------------------END-OF-HEADER------------------------------
 */
#include "stdafx.h"

#include "GUI_Protected.h"

/*
 *********************************************************
 *                                                       *
 *       GUI_SetColor                                    *
 *       GUI_SetBkColor                                  *
 *                                                       *
 *********************************************************

 Purpose:  Assign color/index to foreground/ background
 */

void GUI_SetBkColor(GUI_COLOR color)
{
	GUI_LOCK();
	{
		LCD_SetBkColor(color);
	}GUI_UNLOCK();
}

void GUI_SetColor(GUI_COLOR color)
{
	GUI_LOCK();
	{
		LCD_SetColor(color);
	}GUI_UNLOCK();
}

