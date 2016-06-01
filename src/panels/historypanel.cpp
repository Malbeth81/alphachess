/*
* HistoryPanel.cpp
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
#include "historypanel.h"

const char ClassName[] = "HistoryPanel";
const char MoveListClassName[] = "HistoryPanelMoveList";

extern char DefaultFontName[];

ATOM HistoryPanel::ClassAtom = 0;
ATOM HistoryPanel::MoveListClassAtom = 0;

#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

// PUBLIC FUNCTIONS ------------------------------------------------------------

HistoryPanel::HistoryPanel(HINSTANCE hInstance, HWND hParent, RECT* R)
{
  Handle = NULL;
  MoveList = NULL;
  FirstButton = NULL;
  LastButton = NULL;
  NextButton = NULL;
  PreviousButton = NULL;

  Game = NULL;
  Set = NULL;

  IconsVisible = true;
  strcpy(Font.Name, DefaultFontName);
  Font.Size = 9;
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
    WndClass.hInstance = hInstance;
    WndClass.lpfnWndProc = PanelWindowProc;
    WndClass.style = 0;
    WndClass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
    WndClass.hIcon = 0;
    WndClass.hIconSm = 0;
    WndClass.hCursor = LoadCursor(NULL,IDC_ARROW);
    WndClass.lpszMenuName = NULL;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    ClassAtom = RegisterClassEx(&WndClass);
  }
  if (MoveListClassAtom == 0)
  {
    /* Register the window's class */
    WNDCLASSEX WndClass;
    WndClass.cbSize = sizeof(WNDCLASSEX);
    WndClass.lpszClassName = MoveListClassName;
    WndClass.hInstance = hInstance;
    WndClass.lpfnWndProc = MoveListWindowProc;
    WndClass.style = 0;
    WndClass.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
    WndClass.hIcon = 0;
    WndClass.hIconSm = 0;
    WndClass.hCursor = LoadCursor(NULL,IDC_ARROW);
    WndClass.lpszMenuName = NULL;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    MoveListClassAtom = RegisterClassEx(&WndClass);
  }
  /* Create the window */
  if (ClassAtom != 0 && MoveListClassAtom != 0)
  {
    Handle = CreateWindowEx(0,ClassName,NULL,WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
        R->left,R->left,Width,Height,hParent,NULL,hInstance,this);
    if (Handle != NULL)
    {
      /* Create the child windows */
      MoveList = CreateWindowEx(WS_EX_CLIENTEDGE,MoveListClassName,NULL,WS_CHILD|WS_CLIPSIBLINGS|WS_VSCROLL|WS_VISIBLE,
          0,0,0,0,Handle,NULL,hInstance,NULL);
      FirstButton = CreateWindowEx(0,"BUTTON",NULL,WS_CHILD|WS_CLIPSIBLINGS|WS_TABSTOP|WS_VISIBLE|BS_OWNERDRAW,
          0,0,0,0,Handle,NULL,hInstance,NULL);
      LastButton = CreateWindowEx(0,"BUTTON",NULL,WS_CHILD|WS_CLIPSIBLINGS|WS_TABSTOP|WS_VISIBLE|BS_OWNERDRAW,
          0,0,0,0,Handle,NULL,hInstance,NULL);
      NextButton = CreateWindowEx(0,"BUTTON",NULL,WS_CHILD|WS_CLIPSIBLINGS|WS_TABSTOP|WS_VISIBLE|BS_OWNERDRAW,
          0,0,0,0,Handle,NULL,hInstance,NULL);
      PreviousButton = CreateWindowEx(0,"BUTTON",NULL,WS_CHILD|WS_CLIPSIBLINGS|WS_TABSTOP|WS_VISIBLE|BS_OWNERDRAW,
          0,0,0,0,Handle,NULL,hInstance,NULL);

      /* Initialize child window placement */
      ApplyThemeToCustomButton(Handle);
      UpdateSize(Width, Height);
      UpdateLineSize();
    }
  }
}

HistoryPanel::~HistoryPanel()
{
  /* Destroy the window */
  if (Handle != NULL)
    DestroyWindow(Handle);
  if (MoveList != NULL)
    DestroyWindow(MoveList);
  if (FirstButton != NULL)
    DestroyWindow(FirstButton);
  if (LastButton != NULL)
    DestroyWindow(LastButton);
  if (NextButton != NULL)
    DestroyWindow(NextButton);
  if (PreviousButton != NULL)
    DestroyWindow(PreviousButton);
}

void HistoryPanel::EnableFirstButton(bool Value)
{
  if (FirstButton != NULL)
    EnableWindow(FirstButton,Value);
}

void HistoryPanel::EnableLastButton(bool Value)
{
  if (LastButton != NULL)
    EnableWindow(LastButton,Value);
}

void HistoryPanel::EnableNextButton(bool Value)
{
  if (NextButton != NULL)
    EnableWindow(NextButton,Value);
}

void HistoryPanel::EnablePreviousButton(bool Value)
{
  if (PreviousButton != NULL)
    EnableWindow(PreviousButton,Value);
}

HWND HistoryPanel::GetHandle()
{
  return Handle;
}

void HistoryPanel::Invalidate()
{
  if (MoveList != NULL)
  {
    InvalidateRect(MoveList, NULL, FALSE);
    UpdateScrollbars();
  }
}

void HistoryPanel::SetChessSet(ChessSet* NewSet)
{
  Set = NewSet;
  Invalidate();
}

void HistoryPanel::SetGame(ChessGame* NewGame)
{
  Game = NewGame;
  Invalidate();
}

void HistoryPanel::ShowIcons(bool Value)
{
  IconsVisible = Value;
  Invalidate();
}

// PRIVATE GUI FUNCTIONS -------------------------------------------------------

int HistoryPanel::DrawChessPiece(HDC DC, int X, int Y, ChessPieceType Piece)
{
  int Result = 0;
  if (Set != NULL)
  {
    /* Set the style */
    HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,Set->FontName.c_str(),PixelsToPoints(DC, LineHeight-4),0));
    /* Draw the piece */
    int Index;
    switch (Piece)
    {
      case King: Index = 0; break;
      case Queen: Index = 1; break;
      case Rook: Index = 2; break;
      case Bishop: Index = 3; break;
      case Knight: Index = 4; break;
      default: Index = 5; break;
    }
    TextOut(DC,X,Y,Set->Letters.substr(Index+6, 1).c_str(),1);
    /* Calculate width */
    SIZE Size;
    GetTextExtentPoint32(DC,Set->Letters.substr(Index+6, 1).c_str(),1,&Size);
    Result = max(1, Size.cx);
    /* Clean up */
    DeleteObject(SelectObject(DC,OldFont));
  }
  return Result;
}

void HistoryPanel::DrawMoves(HDC DC, unsigned int First, unsigned int Last)
{
  if (Game != NULL)
  {
    const LinkedList<ChessMove>* Moves = Game->GetMoves();
    if (First*2 < Moves->Size())
    {
      int X = 4;
      int Y = 2;
      HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC, &Font));
      /* Draw the move */
      for (unsigned int i=First*2; i < min((Last+1)*2, Moves->Size()); i++)
      {
        SetBkMode(DC,TRANSPARENT);
        SetTextColor(DC, GetSysColor(COLOR_WINDOWTEXT));
        /* Draw the move number */
        if (i%2 == 0)
        {
          HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,Font.Name,7,0));
          char* Str = inttostr((i/2)+1);
          strcat(Str, ".");
          TextOut(DC,X,Y+2+(i/2-TopRow)*LineHeight,Str,strlen(Str));
          delete[] Str;
          DeleteObject(SelectObject(DC,OldFont));
        }
        /* Highlight the move */
        if (Moves->Get(i) == Game->GetDisplayedMove())
        {
          HBRUSH OldBrush = (HBRUSH)SelectObject(DC,CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT)));
          HPEN OldPen = (HPEN)SelectObject(DC,CreatePen(PS_SOLID,1,GetSysColor(COLOR_HIGHLIGHT)));
          Rectangle(DC,X+(26-1)+(i%2)*((Width-50)/2),Y+(i/2-TopRow)*LineHeight,X+21+(i%2+1)*((Width-50)/2),Y+(i/2-TopRow+1)*LineHeight);
          DeleteObject(SelectObject(DC,OldPen));
          DeleteObject(SelectObject(DC,OldBrush));
          SetTextColor(DC,GetSysColor(COLOR_HIGHLIGHTTEXT));
        }
        /* Draw the move */
        int PosX = X+26+(i%2)*((Width-50)/2);
        int PosY = Y+(i/2-TopRow)*LineHeight;
        const ChessMove* Move = Moves->Get(i);
        char* Text = Move->ToAlgebraicNotation();
        if (IconsVisible)
        {
          /* Draw the chess piece */
          ChessPieceType Type = ChessPiece::GetPieceType(Text);
          PosX = PosX + DrawChessPiece(DC,PosX,PosY,Type);
          if (Type != Pawn)
          {
            char* Tmp = substr(Text,1);
            delete[] Text;
            Text = Tmp;
          }
        }
        /* Draw the chess move */
        TextOut(DC,PosX,PosY,Text,strlen(Text));
        delete[] Text;
      }
      /* Clean up */
      DeleteObject(SelectObject(DC,OldFont));
      SetBkMode(DC,OPAQUE);
    }
  }
}

int HistoryPanel::LineCount()
{
  if (Game != NULL)
  {
    const LinkedList<ChessMove>* Moves = Game->GetMoves();
    return Moves->Size()/2+Moves->Size()%2;
  }
  return 0;
}

// PRIVATE EVENT FUNCTIONS -----------------------------------------------------

void HistoryPanel::MouseWheel(int Keys, int Delta, int x, int y)
{
  if (MoveList != NULL)
  {
    int NewPos = TopRow;
    if (Keys & MK_CONTROL)
    {
      if (Delta > 0)
        NewPos = NewPos -= VisibleRows;
      else
        NewPos = NewPos += VisibleRows;
    }
    else
    {
      UINT ScrollSize = 3;
      SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ScrollSize, 0);
      if (Delta > 0)
        NewPos = NewPos -= ScrollSize;
      else
        NewPos = NewPos += ScrollSize;
    }
    NewPos = max(0, min(NewPos, LineCount()-VisibleRows));
    ScrollWindow(MoveList, 0, (TopRow-NewPos)*LineHeight, NULL, NULL);
    TopRow = NewPos;
    UpdateScrollbars();
  }
}

void HistoryPanel::Paint()
{
  if (MoveList != NULL && GetUpdateRect(MoveList, NULL, 0) != 0)
  {
    PAINTSTRUCT PS;
    HDC DC = BeginPaint(MoveList, &PS);
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
    EndPaint(MoveList, &PS);
  }
}

void HistoryPanel::SetVScrollBar(int Pos, int Range)
{
  if (MoveList != NULL)
  {
    SCROLLINFO ScrollInfo;
    ScrollInfo.cbSize = sizeof(SCROLLINFO);
    ScrollInfo.fMask = SIF_RANGE|SIF_POS|SIF_DISABLENOSCROLL;
    ScrollInfo.nPos = Pos;
    ScrollInfo.nMin = 0;
    ScrollInfo.nMax = Range;
    SetScrollInfo(MoveList, SB_VERT, &ScrollInfo, TRUE);
  }
}

void HistoryPanel::UpdateLineSize()
{
  if (Handle != NULL)
  {
    SIZE Size;
    HDC DC = GetWindowDC(Handle);
    /* Calculate the line size */
    HFONT OldFont = (HFONT)SelectObject(DC, EasyCreateFont(DC, &Font));
    GetTextExtentPoint32(DC, "Mj", 2, &Size);
    LineHeight = max(1, Size.cy+2);
    /* Clean up */
    DeleteObject(SelectObject(DC, OldFont));
    ReleaseDC(Handle, DC);
    /* Update the editor */
    UpdateSize(Width, Height);
  }
}

void HistoryPanel::UpdateScrollbars()
{
  if (LineCount() > VisibleRows)
    SetVScrollBar(TopRow, LineCount()-VisibleRows);
  else
    SetVScrollBar(0, 0);
}

void HistoryPanel::UpdateSize(int NewWidth, int NewHeight)
{
  /* Stores the size of the window */
  Width = NewWidth;
  Height = NewHeight;
  /* Resize child windows */
  float scaleFactor = GetDPIScaleFactor();
  if (MoveList != NULL)
    SetWindowPos(MoveList,NULL,0,1,Width,Height-(int)(20*scaleFactor)-2,SWP_NOZORDER);
  if (FirstButton != NULL)
    SetWindowPos(FirstButton,NULL,0,Height-(int)(20*scaleFactor)-1,Width/4,(int)(20*scaleFactor),SWP_NOZORDER);
  if (LastButton != NULL)
    SetWindowPos(LastButton,NULL,Width-Width/4,Height-(int)(20*scaleFactor)-1,Width/4,(int)(20*scaleFactor),SWP_NOZORDER);
  if (NextButton != NULL)
    SetWindowPos(NextButton,NULL,Width/2,Height-(int)(20*scaleFactor)-1,Width/4,(int)(20*scaleFactor),SWP_NOZORDER);
  if (PreviousButton != NULL)
    SetWindowPos(PreviousButton,NULL,Width/4,Height-(int)(20*scaleFactor)-1,Width/4,(int)(20*scaleFactor),SWP_NOZORDER);
  /* Calculate the amount of visible cells */
  if (LineHeight > 0)
    VisibleRows = (Height-(int)(20*scaleFactor)-4)/LineHeight;
  else
    VisibleRows = 0;
  UpdateScrollbars();
}

void HistoryPanel::VScroll(int ScrollCode, int Pos)
{
  if (MoveList != NULL)
  {
    int NewPos = TopRow;
    switch (ScrollCode)
    {
      case SB_LINEUP: NewPos -= 1; break;
      case SB_LINEDOWN: NewPos += 1; break;
      case SB_PAGEUP: NewPos -= VisibleRows; break;
      case SB_PAGEDOWN: NewPos += VisibleRows; break;
      case SB_TOP: NewPos = 1; break;
      case SB_BOTTOM: NewPos = LineCount()-VisibleRows; break;
      case SB_THUMBPOSITION:
      case SB_THUMBTRACK: NewPos = Pos; break;
    }
    NewPos = max(0, min(NewPos, LineCount()-VisibleRows));
    ScrollWindow(MoveList, 0, (TopRow-NewPos)*LineHeight, NULL, NULL);
    TopRow = NewPos;
    UpdateScrollbars();
  }
}

// PRIVATE WINAPI FUNCTIONS ----------------------------------------------------

LRESULT __stdcall HistoryPanel::PanelWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch(Msg)
  {
    case WM_COMMAND:
    {
      HistoryPanel* Panel = (HistoryPanel*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Panel != NULL)
      {
        if ((HWND)lParam == Panel->FirstButton)
          SendMessage(GetParent(hWnd), WM_HISTORYBUTTONCLICKED, 1, 0);
        else if ((HWND)lParam == Panel->LastButton)
          SendMessage(GetParent(hWnd), WM_HISTORYBUTTONCLICKED, 2, 0);
        else if ((HWND)lParam == Panel->NextButton)
          SendMessage(GetParent(hWnd), WM_HISTORYBUTTONCLICKED, 3, 0);
        else if ((HWND)lParam == Panel->PreviousButton)
          SendMessage(GetParent(hWnd), WM_HISTORYBUTTONCLICKED, 4, 0);
      }
      return 0;
    }
    case WM_CREATE:
    {
      CREATESTRUCT* Params = (CREATESTRUCT*)lParam;
      HistoryPanel* Panel = (HistoryPanel*)(Params->lpCreateParams);
      SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)Panel);
      return 0;
    }
    case WM_DRAWITEM:
    {
      HistoryPanel* Panel = (HistoryPanel*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Panel != NULL)
      {
        /* Draw button frame */
        DRAWITEMSTRUCT* Item = (LPDRAWITEMSTRUCT)lParam;
        DrawCustomButton(Item->hwndItem, Item);

        /* Draw button content */
        if (Panel != NULL)
        {
          float scaleFactor = GetDPIScaleFactor();
          int Height = (int)(20*scaleFactor);
          int Width = (int)Panel->Width/4;
          HBRUSH OldBrush = (HBRUSH)SelectObject(Item->hDC,CreateSolidBrush(GetSysColor(COLOR_BTNTEXT)));
          HPEN OldPen = (HPEN)SelectObject(Item->hDC,CreatePen(PS_SOLID,1,GetSysColor(COLOR_3DDKSHADOW)));
          if (Item->hwndItem == Panel->FirstButton)
          {
            POINT p[3] = {Point(Width/2+Height/8-Height/6, Height/4), Point(Width/2+Height/8-Height/6, Height-Height/4), Point(Width/2-Height/4-Height/6, Height/2)};
            Polygon(Item->hDC, p, 3);
            POINT q[3] = {Point(Width/2+Height/8+Height/6, Height/4), Point(Width/2+Height/8+Height/6, Height-Height/4), Point(Width/2-Height/4+Height/6, Height/2)};
            Polygon(Item->hDC, q, 3);
          }
          else if (Item->hwndItem == Panel->LastButton)
          {
            POINT p[3] = {Point(Width/2-Height/8-Height/6, Height/4), Point(Width/2-Height/8-Height/6, Height-Height/4), Point(Width/2+Height/4-Height/6, Height/2)};
            Polygon(Item->hDC, p, 3);
            POINT q[3] = {Point(Width/2-Height/8+Height/6, Height/4), Point(Width/2-Height/8+Height/6, Height-Height/4), Point(Width/2+Height/4+Height/6, Height/2)};
            Polygon(Item->hDC, q, 3);
          }
          else if (Item->hwndItem == Panel->NextButton)
          {
            POINT p[3] = {Point(Width/2-Height/8, Height/4), Point(Width/2-Height/8, Height-Height/4), Point(Width/2+Height/4, Height/2)};
            Polygon(Item->hDC, p, 3);
          }
          else if (Item->hwndItem == Panel->PreviousButton)
          {
            POINT p[3] = {Point(Width/2+Height/8, Height/4), Point(Width/2+Height/8, Height-Height/4), Point(Width/2-Height/4, Height/2)};
            Polygon(Item->hDC, p, 3);
          }
          DeleteObject(SelectObject(Item->hDC,OldPen));
          DeleteObject(SelectObject(Item->hDC,OldBrush));
        }
      }
      return 1;
    }
    case WM_MOUSEWHEEL:
    {
      HistoryPanel* Panel = (HistoryPanel*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Panel != NULL)
      {
        /* Redirect to move list */
        if (Panel->MoveList != NULL)
          SendMessage(Panel->MoveList, Msg, wParam, lParam);
      }
      return 0;
    }
    case WM_SIZE:
    {
      HistoryPanel* Panel = (HistoryPanel*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Panel != NULL)
      {
        if (LOWORD(lParam) != Panel->Width || HIWORD(lParam) != Panel->Height)
            Panel->UpdateSize((short)LOWORD(lParam), (short)HIWORD(lParam));
      }
      return 0;
    }
    case WM_THEMECHANGED:
    {
      ApplyThemeToCustomButton(hWnd);
      return 1;
    }
  }
  return DefWindowProc(hWnd,Msg,wParam,lParam);
}

LRESULT __stdcall HistoryPanel::MoveListWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch(Msg)
  {
    case WM_ERASEBKGND:
    {
      return 1;
    }
    case WM_FONTCHANGE:
    {
      HistoryPanel* Panel = (HistoryPanel*)GetWindowLong(GetParent(hWnd), GWL_USERDATA);
      if (Panel != NULL)
      {
        Panel->UpdateLineSize();
      }
      return 0;
    }
    case WM_MOUSEWHEEL:
    {
      HistoryPanel* Panel = (HistoryPanel*)GetWindowLong(GetParent(hWnd), GWL_USERDATA);
      if (Panel != NULL)
      {
        Panel->MouseWheel(LOWORD(wParam), (short)HIWORD(wParam), (short)LOWORD(lParam), (short)HIWORD(lParam));
      }
      return 0;
    }
    case WM_PAINT:
    {
      HistoryPanel* Panel = (HistoryPanel*)GetWindowLong(GetParent(hWnd), GWL_USERDATA);
      if (Panel != NULL)
      {
        Panel->Paint();
        Panel->UpdateScrollbars();
      }
      return 0;
    }
    case WM_VSCROLL:
    {
      HistoryPanel* Panel = (HistoryPanel*)GetWindowLong(GetParent(hWnd), GWL_USERDATA);
      if (Panel != NULL)
      {
        Panel->VScroll((short)LOWORD(wParam), (short)HIWORD(wParam));
      }
      return 0;
    }
  }
  return DefWindowProc(hWnd,Msg,wParam,lParam);
}
