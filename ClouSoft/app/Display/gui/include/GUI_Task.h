#ifndef GUI_TASK_H
#define GUI_TASK_H

/*
      *************************************************************
      *                                                           *
      *               Configuration check                         *
      *                                                           *
      *************************************************************
*/

/*
      *************************************************************
      *                                                           *
      *               Locking macros                              *
      *                                                           *
      *************************************************************
  For performance reasons, the windows manager user the same locking mechanisms
  as the GUI layer. The advantage is that wiht a single call to GUI_LOCK both
  the graphic level and the WM level are covered.
*/
//#if defined(__cplusplus)
//extern "C" {     /* Make sure we have C-declarations in C++ programs */
//#endif

extern void GUI_Lock(void);
extern void GUI_Unlock(void);
extern void GUITASK_Init(void);

//#if defined(__cplusplus)
//}
//#endif
#endif
