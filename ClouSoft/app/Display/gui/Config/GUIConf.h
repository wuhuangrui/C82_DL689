/*
*********************************************************************************************************
*                                                uC/GUI
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              ç»€ï¿?GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : GUIConf.h
Purpose     : Configures emWins abilities, fonts etc.
----------------------------------------------------------------------

ATTENTION

Do not modify this file. It will have no effect, since this
configuration has been used to compile the libraries.
You will need the GUI sources for changes in these settings to become
effective.
*/

#ifndef GUICONF_H
#define GUICONF_H

/*********************************************************************
*
*         Configuration of desired functionality
*/

#define GUI_OS                    (0)  /* Compile with multitasking support */
#define GUI_WINSUPPORT            (0)  /* Use window manager if true (1)  */
#define GUI_SUPPORT_MEMDEV        (0)  /* Support memory devices */
#define GUI_SUPPORT_TOUCH         (0)  /* Support a touch screen (req. win-manager) */
#define GUI_SUPPORT_UNICODE       (1)  /* Support mixed ASCII/UNICODE strings */

/*********************************************************************
*
*         Configuration of dynamic memory

Dynamic memory is used for memory devices and window manager.
If you do not use these features, there is no need for dynamic memory
and it may be switched off completely. (This section can be erased)
*/

#define GUI_ALLOC_SIZE          500000  /* Size of dynamic memory */


/*********************************************************************
*
*         Configuration of available fonts

Dynamic memory is used for memory devices and window manager.
If you do not use these features, there is no need for dynamic memory
and it may be switched off completely. (This section can be erased)
*/


#define GUI_DEFAULT_FONT    &GUI_FontHZ_SimSun_14

#endif /* GUICONF_H */
