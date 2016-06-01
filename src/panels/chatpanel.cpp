/*
* Chatpanel.cpp
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
#include "chatpanel.h"

const char ClassName[] = "ChatPanel";
const char MessagesClassName[] = "ChatPanelMessages";

extern char DefaultFontName[];

ATOM ChatPanel::ClassAtom = 0;
ATOM ChatPanel::MessagesClassAtom = 0;
WNDPROC ChatPanel::OldInputFieldProc = 0;

#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

// PUBLIC FUNCTIONS ------------------------------------------------------------

ChatPanel::ChatPanel(HINSTANCE hInstance, HWND hParent, RECT* R)
{
  Handle = NULL;
  Messages = NULL;
  Input = NULL;
  Button = NULL;

  Height = R->bottom-R->top;
  LineHeight = 16;
  MessageHeight = R->bottom-R->top-26;
  MessageWidth = R->right-R->left-2;
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
  if (MessagesClassAtom == 0)
  {
    /* Register the child window's class */
    WNDCLASSEX WndClass;
    WndClass.cbSize = sizeof(WNDCLASSEX);
    WndClass.lpszClassName = MessagesClassName;
    WndClass.hInstance = hInstance;
    WndClass.lpfnWndProc = MessagesWindowProc;
    WndClass.style = 0;
    WndClass.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
    WndClass.hIcon = 0;
    WndClass.hIconSm = 0;
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.lpszMenuName = NULL;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    MessagesClassAtom = RegisterClassEx(&WndClass);
  }
  /* Create the window */
  if (ClassAtom != 0 && MessagesClassAtom != 0)
  {
    Handle = CreateWindowEx(0,ClassName,NULL,WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        R->left,R->top,Width,Height,hParent,NULL,hInstance,this);
    if (Handle != NULL)
    {
      /* Create the child windows */
      Messages = CreateWindowEx(WS_EX_CLIENTEDGE,MessagesClassName,NULL,WS_CHILD|WS_CLIPSIBLINGS|WS_VISIBLE|WS_VSCROLL,
          0,0,0,0,Handle,NULL,hInstance,NULL);
      Input = CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT",NULL,WS_CHILD|WS_CLIPSIBLINGS|WS_TABSTOP|WS_VISIBLE|ES_AUTOHSCROLL,
          0,0,0,0,Handle,NULL,hInstance,NULL);
      OldInputFieldProc = (WNDPROC)SetWindowLong(Input,GWL_WNDPROC,(LONG)InputFieldProc);
      Button = CreateWindowEx(0,"BUTTON","Send",WS_CHILD|WS_CLIPSIBLINGS|WS_TABSTOP|WS_VISIBLE|BS_DEFPUSHBUTTON,
          0,0,0,0,Handle,NULL,hInstance,NULL);

      /* Initialize child window placement */
      UpdateSize(Width, Height);

      /* Change the window's appearance */
      HFONT Font = EasyCreateFont(NULL,DefaultFontName,9,0);
      PostMessage(Input,WM_SETFONT,(WPARAM)Font,TRUE);
      Font = EasyCreateFont(NULL,DefaultFontName,8,fsBold);
      PostMessage(Button,WM_SETFONT,(WPARAM)Font,TRUE);

      UpdateLineSize();
    }
  }
}

ChatPanel::~ChatPanel()
{
  /* Destroy the window */
  if (Handle != NULL)
    DestroyWindow(Handle);
  if (Messages != NULL)
    DestroyWindow(Messages);
  if (Button != NULL)
    DestroyWindow(Button);
  if (Input != NULL)
    DestroyWindow(Input);

  /* Cleanup */
  while (Lines.Size() > 0)
    delete Lines.Remove();
}

void ChatPanel::AddLine(string Text)
{
  string* Line = new string(Text);
  Lines.Add(Line);
  Invalidate();
  VScroll(SB_BOTTOM,0);
}

HWND ChatPanel::GetHandle()
{
  return Handle;
}

void ChatPanel::Invalidate()
{
  if (Messages != NULL)
    InvalidateRect(Messages, NULL, FALSE);
}

// PRIVATE GUI FUNCTIONS -------------------------------------------------------

void ChatPanel::SetVScrollBar(int Pos, int Range)
{
  if (Messages != NULL)
  {
    SCROLLINFO ScrollInfo;
    ScrollInfo.cbSize = sizeof(SCROLLINFO);
    ScrollInfo.fMask = SIF_RANGE | SIF_POS | SIF_DISABLENOSCROLL;
    ScrollInfo.nPos = Pos;
    ScrollInfo.nMin = 0;
    ScrollInfo.nMax = Range;
    SetScrollInfo(Messages, SB_VERT, &ScrollInfo, TRUE);
  }
}

void ChatPanel::VScroll(int ScrollCode, int Pos)
{
  if (Messages != NULL)
  {
    unsigned int NewPos = TopRow;
    switch (ScrollCode)
    {
      case SB_LINEUP: NewPos -= Pos; break;
      case SB_LINEDOWN: NewPos += Pos; break;
      case SB_PAGEUP: NewPos -= VisibleRows*Pos; break;
      case SB_PAGEDOWN: NewPos += VisibleRows*Pos; break;
      case SB_TOP: NewPos = 1; break;
      case SB_BOTTOM: NewPos = (Lines.Size() > VisibleRows ? Lines.Size()-VisibleRows : 0); break;
      case SB_THUMBPOSITION:
      case SB_THUMBTRACK: NewPos = Pos; break;
    }
    NewPos = max(0, min(NewPos, (Lines.Size() > VisibleRows ? Lines.Size()-VisibleRows : 0)));
    ScrollWindow(Messages, 0, (TopRow-NewPos)*LineHeight, NULL, NULL);
    TopRow = NewPos;
    SetVScrollBar(TopRow, (Lines.Size() > VisibleRows ? Lines.Size()-VisibleRows : 0));
  }
}

// PRIVATE EVENT FUNCTIONS ----------------------------------------------------

void ChatPanel::Paint()
{
  if (Messages != NULL)
  {
    PAINTSTRUCT PS;
    HDC DC = BeginPaint(Messages, &PS);
    if (DC != NULL)
    {
      /* Draw background */
      HBRUSH OldBrush = (HBRUSH)SelectObject(DC,CreateSolidBrush(GetSysColor(COLOR_WINDOW)));
      HPEN OldPen = (HPEN)SelectObject(DC,CreatePen(PS_SOLID,1,GetSysColor(COLOR_WINDOW)));
      Rectangle(DC,PS.rcPaint.left,PS.rcPaint.top,PS.rcPaint.right,PS.rcPaint.bottom);
      DeleteObject(SelectObject(DC,OldPen));
      DeleteObject(SelectObject(DC,OldBrush));
      /* Calculates lines that needs to be painted */
      unsigned int Top = TopRow+max(0,(int)PS.rcPaint.top-2)/LineHeight;
      unsigned int Bottom = TopRow+max(0,(int)PS.rcPaint.bottom-2)/LineHeight;
      /* Paints lines */
      HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(NULL,DefaultFontName,9,0));
      if (Top < Lines.Size())
      {
        int X = 2;
        int Y = 2;
        SetBkMode(DC, TRANSPARENT);
        SetTextColor(DC, GetSysColor(COLOR_WINDOWTEXT));
        /* Draws the lines */
        for (unsigned int i=Top; i < min(Bottom+1, Lines.Size()); i++)
        {
          TextOut(DC, X, Y+(i-TopRow)*LineHeight, Lines.Get(i)->c_str(), Lines.Get(i)->length());
          // TODO Wrap lines
        }
        /* Clean up */
        SetBkMode(DC,OPAQUE);
      }
      DeleteObject(SelectObject(DC,OldFont));
      EndPaint(Messages, &PS);
    }
  }
}

void ChatPanel::UpdateLineSize()
{
  if (Messages != NULL)
  {
    SIZE Size;
    HDC DC = GetWindowDC(Messages);
    HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(NULL,DefaultFontName,9,0));
    /* Calculate the editor's cell size */
    GetTextExtentPoint32(DC, "Mj", 2, &Size);
    LineHeight = max(1, Size.cy);
    /* Clean up */
    DeleteObject(SelectObject(DC,OldFont));
    ReleaseDC(Messages, DC);
    /* Update the message panel */
    UpdateMessageSize(MessageWidth, MessageHeight);
  }
}

void ChatPanel::UpdateMessageSize(int NewWidth, int NewHeight)
{
  /* Stores the size of the window */
  MessageWidth = NewWidth;
  MessageHeight = NewHeight;
  /* Calculate the board size */
  VisibleRows = (LineHeight > 0 ? MessageHeight/LineHeight : 0);
  SetVScrollBar(TopRow, (Lines.Size() > VisibleRows ? Lines.Size()-VisibleRows : 0));
}

void ChatPanel::UpdateSize(int NewWidth, int NewHeight)
{
  /* Stores the size of the window */
  Width = NewWidth;
  Height = NewHeight;
  /* Resize the child windows */
  float scaleFactor = GetDPIScaleFactor();
  if (Messages != NULL)
    SetWindowPos(Messages,NULL,2,2,Width-2,Height-4-(int)(22*scaleFactor),SWP_NOZORDER);
  if (Input != NULL)
    SetWindowPos(Input,NULL,2,Height-(int)(22*scaleFactor),Width-4-(int)(60*scaleFactor),(int)(20*scaleFactor),SWP_NOZORDER);
  if (Button != NULL)
    SetWindowPos(Button,NULL,Width-(int)(60*scaleFactor),Height-(int)(22*scaleFactor),(int)(60*scaleFactor),(int)(22*scaleFactor),SWP_NOZORDER);
}

// PRIVATE WINAPI FUNCTIONS ----------------------------------------------------

LRESULT __stdcall ChatPanel::PanelWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch(Msg)
  {
    case WM_CREATE:
    {
      CREATESTRUCT* Params = (CREATESTRUCT*)lParam;
      ChatPanel* Panel = (ChatPanel*)(Params->lpCreateParams);
      SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)Panel);
      return 0;
    }
    case WM_COMMAND:
    {
      ChatPanel* Panel = (ChatPanel*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Panel != NULL && Panel->Input != NULL)
      {
        if ((HWND)lParam == Panel->Button)
        {
          char* Str = GetWindowText(Panel->Input);
          SendMessage(GetParent(hWnd), WM_SENDMESSAGEBUTTONCLICKED, (WPARAM)Str, 0);
          delete[] Str;
        }
      }
      return 0;
    }
    case WM_KEYUP:
    {
      ChatPanel* Panel = (ChatPanel*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Panel != NULL && Panel->Input != NULL)
      {
        if (wParam == VK_RETURN)
        {
          char* Str = GetWindowText(Panel->Input);
          SendMessage(GetParent(hWnd), WM_SENDMESSAGEBUTTONCLICKED, (WPARAM)Str, 0);
          delete[] Str;
        }
      }
      break;
    }
    case WM_SIZE:
    {
      ChatPanel* Panel = (ChatPanel*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Panel != NULL)
      {
        if (LOWORD(lParam) != Panel->Width || HIWORD(lParam) != Panel->Height)
            Panel->UpdateSize((short)LOWORD(lParam), (short)HIWORD(lParam));
      }
      return 0;
    }
  }
  return DefWindowProc(hWnd,Msg,wParam,lParam);
}

LRESULT __stdcall ChatPanel::MessagesWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch(Msg)
  {
    case WM_ERASEBKGND:
    {
      return 1;
    }
    case WM_FONTCHANGE:
    {
      ChatPanel* Panel = (ChatPanel*)GetWindowLong(GetParent(hWnd), GWL_USERDATA);
      if (Panel != NULL)
      {
        Panel->UpdateLineSize();
      }
      return 0;
    }
    case WM_MOUSEWHEEL:
    {
      ChatPanel* Panel = (ChatPanel*)GetWindowLong(GetParent(hWnd), GWL_USERDATA);
      if (Panel != NULL)
      {
        UINT NumLines = 3;
        SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &NumLines, 0);
        if (LOWORD(wParam) & MK_CONTROL)
          Panel->VScroll((short)HIWORD(wParam) < 0 ? SB_PAGEDOWN : SB_PAGEUP, 1);
        else
          Panel->VScroll((short)HIWORD(wParam) < 0 ? SB_LINEDOWN : SB_LINEUP, NumLines);
      }
      return 0;
    }
    case WM_PAINT:
    {
      ChatPanel* Panel = (ChatPanel*)GetWindowLong(GetParent(hWnd), GWL_USERDATA);
      if (Panel != NULL && GetUpdateRect(hWnd, NULL, 0) != 0)
      {
        Panel->Paint();
      }
      return 0;
    }
    case WM_SIZE:
    {
      ChatPanel* Panel = (ChatPanel*)GetWindowLong(GetParent(hWnd), GWL_USERDATA);
      if (Panel != NULL)
      {
        if (LOWORD(lParam) != Panel->MessageWidth || HIWORD(lParam) != Panel->MessageHeight)
            Panel->UpdateMessageSize((short)LOWORD(lParam), (short)HIWORD(lParam));
      }
      return 0;
    }
    case WM_VSCROLL:
    {
      ChatPanel* Panel = (ChatPanel*)GetWindowLong(GetParent(hWnd), GWL_USERDATA);
      if (Panel != NULL)
      {
        Panel->VScroll((short)LOWORD(wParam), (short)HIWORD(wParam));
      }
      return 0;
    }
  }
  return DefWindowProc(hWnd,Msg,wParam,lParam);
}

LRESULT __stdcall ChatPanel::InputFieldProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
    case WM_KEYUP:
    {
      if (wParam == VK_RETURN)
      {
        PostMessage(GetParent(hWnd),Msg,wParam,lParam);
        return 0;
      }
      break;
    }
  }
  return CallWindowProc(OldInputFieldProc, hWnd, Msg, wParam, lParam);
}
