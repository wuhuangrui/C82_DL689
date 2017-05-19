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
File        : GUICurs.C
Purpose     : Cursor routines of the graphics library
---------------------------END-OF-HEADER------------------------------
*/
#include "stdafx.h"

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Private.h"

#if GUI_SUPPORT_CURSOR

/*******************************************************************
*
*       static data
*
********************************************************************
*/

static  GUI_HMEM _hBuffer;
static  GUI_RECT _Rect;
static  char _CursorIsVis;        /* Currently visible ? */
static  char _CursorOn;
static  const GUI_CURSOR* _pCursor;
static  U8   _CursorDeActCnt;
static  int  _AllocSize;
static  int  _x, _y;              /* Position of hot spot */



/*******************************************************************
*
*       static code
*
********************************************************************
*/
static void _Undraw(void) {
  int x, y, xSize, ySize;
  LCD_PIXELINDEX* pData;
  /* Save bitmap data */
  GUI_LOCK();
  if (_hBuffer) {
    pData = (LCD_PIXELINDEX*)GUI_ALLOC_h2p(_hBuffer);
    xSize = _Rect.x1 - _Rect.x0 + 1;
    ySize = _Rect.y1 - _Rect.y0 + 1;
    for (y = 0; y < ySize; y++) {
      for (x = 0; x < xSize; x++) {
        LCD_SetPixelIndex(x + _Rect.x0, y + _Rect.y0, *(pData + x));
      }
      pData += _pCursor->pBitmap->XSize;
    }
  }
  GUI_UNLOCK();
}


static void _Draw(void) {
  int x, y, xSize, ySize;
  LCD_PIXELINDEX* pData;
  const GUI_BITMAP* pBM;
  GUI_LOCK();
  if (_hBuffer) {
    /* Save bitmap data */
    pBM = _pCursor->pBitmap;
    pData = (LCD_PIXELINDEX*)GUI_ALLOC_h2p(_hBuffer);
    xSize = _Rect.x1 - _Rect.x0 + 1;
    ySize = _Rect.y1 - _Rect.y0 + 1;
    for (y = 0; y < ySize; y++) {
      for (x = 0; x < xSize; x++) {
        *(pData + x) = LCD_GetPixelIndex(_Rect.x0 + x, _Rect.y0 + y);
      }
      pData += pBM->XSize;
    }
    /* Draw bitmap */
    GL_DrawBitmap(pBM, _Rect.x0, _Rect.y0);
  }
  GUI_UNLOCK();
}

static void  _CalcRect(void) {
  if (_pCursor) {
    _Rect.x0 = _x - _pCursor->xHot;
    _Rect.y0 = _y - _pCursor->yHot;
    _Rect.x1 = _Rect.x0 + _pCursor->pBitmap->XSize - 1;
    _Rect.y1 = _Rect.y0 + _pCursor->pBitmap->YSize - 1;
  }
}


/*******************************************************************
*
*                  static Show / Hide

Purpose:
  Show / Hide cursor.
*/
static void _Hide(void) {
  GUI_RECT r;
  if (_CursorIsVis) {
    r = GUI_Context.ClipRect;
    LCD_SetClipRectMax();
    _Undraw();
    GUI_Context.ClipRect = r;
    _CursorIsVis = 0;
  }
}

static void _Show(void) {
  GUI_RECT r;
  if (_CursorOn && (_CursorDeActCnt==0)) {
    _CursorIsVis = 1;
    r = GUI_Context.ClipRect;
    LCD_SetClipRectMax();
    _Draw();  
    GUI_Context.ClipRect = r;
  }
}

/*******************************************************************
*
*           _TempHide, _TempUnhide

Purpose:
  Hide cursor if a part of the given rectangle is located in the
  rectangle used for the cursor. This routine is called automatically
  by the window manager. This way the window manager can
  automatically make sure that the cursor is always displayed
  correctly.
Params:
  pRect   Rectangle under consideration
Return value:
  0:      No action taken
          Cursor was not visible or not affected because rectangles
          did not overlap
  1:      Cursor hidden -> WM needs to restore cursor after
          drawing operation
*/


static char _TempHide(const GUI_RECT* pRect) {
  if (!_CursorIsVis) {
    return 0;             /* Cursor not visible -> nothing to do */
  }
  if ((pRect == NULL) | GUI_RectsIntersect(pRect, &_Rect)) {
    _Hide();              /* Cursor needs to be hidden */
    return 1;
  }
  return 0;               /* Cursor not affected -> nothing to do */
}

static void _TempUnhide(void) {
  _Show();
}

/*******************************************************************
*
*       Public code
*
********************************************************************
*/

/*******************************************************************
*
*       GUI_CURSOR_Activate
*       GUI_CURSOR_Deactivate

Purpose:
  Allows activation or deactivation of cursor. Can be used to make
  cursor flash.
*/
void GUI_CURSOR_Activate(void) {
  GUI_LOCK();
  if ((--_CursorDeActCnt) ==0) {
    _Show();
  }
  GUI_UNLOCK();
}

void GUI_CURSOR_Deactivate(void) {
  GUI_LOCK();
  if (_CursorDeActCnt++ ==0)
    _Hide();
  GUI_UNLOCK();
}


/*******************************************************************
*
*       GUI_CURSOR_Select
*/

void GUI_CURSOR_Select(const GUI_CURSOR* pCursor) {
  int AllocSize;
  const GUI_BITMAP* pBM;
  GUI_LOCK();
  if (pCursor != _pCursor) {
    pBM = pCursor->pBitmap;
    _Hide();
    AllocSize = pBM->XSize * pBM->YSize * sizeof(LCD_PIXELINDEX);
    if (AllocSize != _AllocSize) {
      GUI_ALLOC_FreePtr(&_hBuffer);
    }
    _hBuffer = GUI_ALLOC_Alloc(AllocSize);
    _CursorOn = 1;
    _pCursor = pCursor;
    _CalcRect();
    _Show();
  }
  GUI_UNLOCK();
}




/*******************************************************************
*
*                  GUI_CURSOR_Clear
*
********************************************************************

Purpose:
  Clears cursor.
*/

void GUI_CURSOR_Hide(void) {
  GUI_LOCK();
  _Hide();
  _CursorOn = 0;
  /* Set function pointer which window manager can use */
  GUI_CURSOR_pfTempHide   = NULL;
  GUI_CURSOR_pfTempUnhide = NULL;
  GUI_UNLOCK();
}

/*******************************************************************
*
*                  GUI_CURSOR_Show
*
********************************************************************

Purpose:
  Shows cursor.
*/

void GUI_CURSOR_Show(void) {
  GUI_LOCK();
  _Hide();
  _CursorOn = 1;
  /* Set function pointer which window manager can use */
  GUI_CURSOR_pfTempHide   = _TempHide;
  GUI_CURSOR_pfTempUnhide = _TempUnhide;
  if (!_pCursor) {
    GUI_CURSOR_Select(GUI_DEFAULT_CURSOR);
  } else {
    _Show();
  }
  GUI_UNLOCK();
}

/*******************************************************************
*
*                  GUI_CURSOR_SetPosition

Purpose:
  Sets position of cursor.
*/

void GUI_CURSOR_SetPosition(int x, int y) {
  GUI_LOCK();
  if ((_x != x) | (_y != y)) {
    _Hide();
    _x = x;
    _y = y;
    _CalcRect();
    _Show();
  }
  GUI_UNLOCK();
}

#else

void GUICurs_C(void) {} /* avoid empty object files */

#endif   /* GUI_SUPPORT_CURSOR */








