/*
* MoveListPanel.cpp
*
* Copyright (C) 2007-2010 Marc-André Lamothe.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Library General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
#include "movelistpanel.h"

const char ClassName[] = "MoveListPanel";

extern ChessEngine Chess;

extern char DefaultFontName[];

ATOM MoveListPanel::ClassAtom = 0;

#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

// PUBLIC FUNCTIONS ------------------------------------------------------------

MoveListPanel::MoveListPanel(HWND hParent, RECT* R)
{
  HINSTANCE Instance = (hParent != NULL ? (HINSTANCE)GetWindowLong(hParent, GWL_HINSTANCE) : GetModuleHandle(NULL));

  Handle = NULL;

  strcpy(Font.Name, DefaultFontName);
  Font.Size = 8;
  Font.Style = 0;
  LineHeight = 16;
  Height = R->bottom-R->top;
  TopRow = 0;
  VisibleRows = 0;
  Width = R->right-R->left;

  if (ClassAtom == 0)
  {
    /* Register the window's class */
    WNDCLASSEX WndClass;
    WndClass.cbSize = sizeof(WNDCLASSEX);
    WndClass.lpszClassName = ClassName;
    WndClass.hInstance = Instance;
    WndClass.lpfnWndProc = PanelWindowProc;
    WndClass.style = 0;
    WndClass.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
    WndClass.hIcon = 0;
    WndClass.hIconSm = 0;
    WndClass.hCursor = LoadCursor(NULL,IDC_ARROW);
    WndClass.lpszMenuName = 0;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    ClassAtom = RegisterClassEx(&WndClass);
  }
  /* Create the window */
  if (ClassAtom != 0)
    CreateWindowEx(WS_EX_CLIENTEDGE,ClassName,NULL,WS_CHILD | WS_CLIPSIBLINGS | WS_VSCROLL,
    R->left,R->top,R->right-R->left,R->bottom-R->top,hParent,NULL,Instance,this);
}

MoveListPanel::~MoveListPanel()
{
  /* Destroy the window */
  if (Handle != NULL)
    DestroyWindow(Handle);
}

HWND MoveListPanel::GetHandle()
{
  return Handle;
}

void MoveListPanel::Invalidate()
{
  if (Handle != NULL)
    InvalidateRect(Handle, NULL, FALSE);
}

// PRIVATE GUI FUNCTIONS -------------------------------------------------------

void MoveListPanel::DrawMoves(HDC DC, unsigned int First, unsigned int Last)
{
  if (First*2 < Chess.GetMoveCount())
  {
    int X = 4;
    int Y = 2;
    SetBkMode(DC,TRANSPARENT);
    SetTextColor(DC, GetSysColor(COLOR_WINDOWTEXT));
    HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC, &Font));
    /* Draw the move */
    for (unsigned int i=First*2; i < min((Last+1)*2, Chess.GetMoveCount()); i++)
    {
      /* Draw the move number */
      if (i%2 == 0)
      {
        char* Str = inttostr((i/2)+1);
        strcat(Str, ".");
        TextOut(DC,X,Y+(i/2-TopRow)*LineHeight,Str,strlen(Str));
        delete[] Str;
      }
      /* Draw the move */
      char* Text = MoveToString(Chess.GetMove(i));
      TextOut(DC,X+30+(i%2)*((Width-30)/2),Y+(i/2-TopRow)*LineHeight,Text,strlen(Text));
      delete[] Text;
    }
    /* Clean up */
    DeleteObject(SelectObject(DC,OldFont));
    SetBkMode(DC,OPAQUE);
  }
}

int MoveListPanel::LineCount()
{
  return Chess.GetMoveCount()/2+Chess.GetMoveCount()%2;
}

char* MoveListPanel::MoveToString(const ChessMove* Move)
{
  if (Move != NULL)
  {
    char* Text = new char[10];
    char* Str = PosToString(Move->From);
    strcpy(Text,Str);
    delete[] Str;
    strcat(Text,"-");
    Str = PosToString(Move->To);
    strcat(Text,Str);
    delete[] Str;
    return Text;
  }
  return NULL;
}

char* MoveListPanel::PosToString(Position Pos)
{
  const char Indexes[17] = "abcdefgh12345678";
  char* Text = new char[3];
  Text[0] = Indexes[Pos.x];
  Text[1] = Indexes[8+Pos.y];
  Text[2] = '\0';
  return Text;
}

// PRIVATE EVENT FUNCTIONS -----------------------------------------------------

void MoveListPanel::MouseWheel(int Keys, int Delta, int x, int y)
{
  UINT NumLines = 3;
  SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &NumLines, 0);
  if (Keys & MK_CONTROL)
    VScroll(Delta < 0 ? SB_PAGEDOWN : SB_PAGEUP, 1);
  else
    VScroll(Delta < 0 ? SB_LINEDOWN : SB_LINEUP, NumLines);
}

void MoveListPanel::Paint()
{
  if (Handle != NULL && GetUpdateRect(Handle, NULL, 0) != 0)
  {
    PAINTSTRUCT PS;
    HDC DC = BeginPaint(Handle, &PS);
    if (DC != NULL)
    {
      /* Draw background */
      HBRUSH OldBrush = (HBRUSH)SelectObject(DC,CreateSolidBrush(GetSysColor(COLOR_WINDOW)));
      HPEN OldPen = (HPEN)SelectObject(DC,CreatePen(PS_SOLID,1,GetSysColor(COLOR_WINDOW)));
      Rectangle(DC,PS.rcPaint.left,PS.rcPaint.top,PS.rcPaint.right,PS.rcPaint.bottom);
      DeleteObject(SelectObject(DC,OldPen));
      DeleteObject(SelectObject(DC,OldBrush));
      /* Calculates lines that needs to be painted */
      int Top = TopRow+PS.rcPaint.top/LineHeight;
      int Bottom = TopRow+PS.rcPaint.bottom/LineHeight;
      /* Draw the moves */
      DrawMoves(DC,max(0,Top-1),Bottom);
    }
    EndPaint(Handle, &PS);
  }
}

void MoveListPanel::SetVScrollBar(int Pos, int Range)
{
  if (Handle != NULL)
  {
    SCROLLINFO ScrollInfo;
    ScrollInfo.cbSize = sizeof(SCROLLINFO);
    ScrollInfo.fMask = SIF_RANGE | SIF_POS | SIF_DISABLENOSCROLL;
    ScrollInfo.nPos = Pos;
    ScrollInfo.nMin = 0;
    ScrollInfo.nMax = Range;
    SetScrollInfo(Handle, SB_VERT, &ScrollInfo, TRUE);
  }
}

void MoveListPanel::UpdateLineSize()
{
  if (Handle != NULL)
  {
    SIZE Size;
    HDC DC = GetWindowDC(Handle);
    /* Calculate the line size */
    HFONT OldFont = (HFONT)SelectObject(DC, EasyCreateFont(DC, &Font));
    GetTextExtentPoint32(DC, "Wg", 1, &Size);
    LineHeight = max(1, Size.cy+2);
    /* Clean up */
    DeleteObject(SelectObject(DC, OldFont));
    ReleaseDC(Handle, DC);
    /* Update the editor */
    UpdateSize(Width, Height);
  }
}

void MoveListPanel::UpdateScrollbars()
{
  if (LineCount() > VisibleRows)
    SetVScrollBar(TopRow, LineCount()-VisibleRows);
  else
    SetVScrollBar(0, 0);
}

void MoveListPanel::UpdateSize(int NewWidth, int NewHeight)
{
  /* Stores the size of the window */
  Width = NewWidth;
  Height = NewHeight;
  /* Calculate the amount of visible cells */
  if (LineHeight > 0)
    VisibleRows = Height/LineHeight;
  else
    VisibleRows = 0;
  UpdateScrollbars();
}

void MoveListPanel::VScroll(int ScrollCode, int Pos)
{
  if (Handle != NULL)
  {
    int NewPos = TopRow;
    switch (ScrollCode)
    {
      case SB_LINEUP: NewPos -= Pos; break;
      case SB_LINEDOWN: NewPos += Pos; break;
      case SB_PAGEUP: NewPos -= VisibleRows*Pos; break;
      case SB_PAGEDOWN: NewPos += VisibleRows*Pos; break;
      case SB_TOP: NewPos = 1; break;
      case SB_BOTTOM: NewPos = LineCount()-VisibleRows; break;
      case SB_THUMBPOSITION:
      case SB_THUMBTRACK: NewPos = Pos; break;
    }
    NewPos = max(0, min(NewPos, LineCount()-VisibleRows));
    ScrollWindow(Handle, 0, (TopRow-NewPos)*LineHeight, NULL, NULL);
    TopRow = NewPos;
    UpdateScrollbars();
  }
}

// PRIVATE WINAPI FUNCTIONS ----------------------------------------------------

LRESULT __stdcall MoveListPanel::PanelWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch(Msg)
  {
    case WM_CREATE:
    {
      CREATESTRUCT* Params = (CREATESTRUCT*)lParam;
      MoveListPanel* Panel = (MoveListPanel*)Params->lpCreateParams;
      SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)Panel);

      if (Panel != NULL)
      {
        Panel->Handle = hWnd;

        Panel->UpdateLineSize();
      }

      return 0;
    }
    case WM_ERASEBKGND:
    {
      return 1;
    }
    case WM_FONTCHANGE:
    {
      MoveListPanel* Panel = (MoveListPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL)
        Panel->UpdateLineSize();
      return 0;
    }
    case WM_MOUSEWHEEL:
    {
      MoveListPanel* Panel = (MoveListPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL)
        Panel->MouseWheel(LOWORD(wParam), (short)HIWORD(wParam), (short)LOWORD(lParam), (short)HIWORD(lParam));
      return 0;
    }
    case WM_PAINT:
    {
      MoveListPanel* Panel = (MoveListPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL)
      {
        Panel->Paint();
        Panel->UpdateScrollbars();
      }
      return 0;
    }
    case WM_SIZE:
    {
      MoveListPanel* Panel = (MoveListPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL && LOWORD(lParam) != Panel->Width || HIWORD(lParam) != Panel->Height)
          Panel->UpdateSize((short)LOWORD(lParam), (short)HIWORD(lParam));
      return 0;
    }
    case WM_VSCROLL:
    {
      MoveListPanel* Panel = (MoveListPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL)
        Panel->VScroll((short)LOWORD(wParam), (short)HIWORD(wParam));
      return 0;
    }
  }
  return DefWindowProc(hWnd,Msg,wParam,lParam);
}
