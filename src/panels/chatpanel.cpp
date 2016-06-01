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
WNDPROC ChatPanel::OldInputFieldProc = 0;

#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

// PUBLIC FUNCTIONS ------------------------------------------------------------

ChatPanel::ChatPanel(HWND hParent, RECT* R)
{
  HINSTANCE Instance = (hParent != NULL ? (HINSTANCE)GetWindowLong(hParent, GWL_HINSTANCE) : GetModuleHandle(NULL));

  SendTextProc = NULL;

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
    WndClass.hInstance = Instance;
    WndClass.lpfnWndProc = PanelWindowProc;
    WndClass.style = 0;
    WndClass.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
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
    CreateWindowEx(0,ClassName,NULL,WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
    R->left,R->top,R->right-R->left,R->bottom-R->top,hParent,NULL,Instance,this);
}

ChatPanel::~ChatPanel()
{
  /* Destroy the window */
  if (Handle != NULL)
    DestroyWindow(Handle);

  /* Cleanup */
  while (Lines.Size() > 0)
    delete Lines.Remove();
}

void ChatPanel::AddLine(char* Text)
{
  Lines.Add(new string(Text));
  Invalidate();
  VScroll(SB_BOTTOM,0);
}

HWND ChatPanel::GetHandle()
{
  return Handle;
}

void ChatPanel::Invalidate()
{
  if (Handle != NULL)
    InvalidateRect(Messages, NULL, FALSE);
}

// PRIVATE GUI FUNCTIONS -------------------------------------------------------

void ChatPanel::SendString()
{
  if (Handle != NULL)
  {
    char* Str = GetWindowText(Input);
    if (strlen(Str) > 0)
    {
      SetWindowText(Input,"");
      if (SendTextProc != NULL)
        (*SendTextProc)(Str);
    }
  }
}

void ChatPanel::SetVScrollBar(int Pos, int Range)
{
  if (Handle != NULL)
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
      case SB_BOTTOM: NewPos = Lines.Size()-VisibleRows; break;
      case SB_THUMBPOSITION:
      case SB_THUMBTRACK: NewPos = Pos; break;
    }
    NewPos = max(0, min(NewPos, Lines.Size()-VisibleRows));
    ScrollWindow(Messages, 0, (TopRow-NewPos)*LineHeight, NULL, NULL);
    TopRow = NewPos;
    SetVScrollBar(TopRow, (Lines.Size() > VisibleRows ? Lines.Size()-VisibleRows : 0));
  }
}

// PRIVATE EVENT FUNCTIONS ----------------------------------------------------

void ChatPanel::UpdateSize(int NewWidth, int NewHeight)
{
  /* Stores the size of the window */
  Width = NewWidth;
  Height = NewHeight;
  /* Resize the child windows */
  SetWindowPos(Messages,NULL,2,2,Width-2,Height-26,SWP_NOZORDER);
  SetWindowPos(Input,NULL,2,Height-22,Width-64,20,SWP_NOZORDER);
  SetWindowPos(Button,NULL,Width-60,Height-22,60,22,SWP_NOZORDER);
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

// PRIVATE WINAPI FUNCTIONS ----------------------------------------------------

LRESULT __stdcall ChatPanel::PanelWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch(Msg)
  {
    case WM_CREATE:
    {
      CREATESTRUCT* Params = (CREATESTRUCT*)lParam;
      ChatPanel* Panel = (ChatPanel*)Params->lpCreateParams;
      SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)Panel);

      if (Panel != NULL)
      {
        Panel->Handle = hWnd;

        /* Register the child window's class */
        WNDCLASSEX WndClass;
        WndClass.lpszClassName = MessagesClassName;
        WndClass.lpfnWndProc = MessagesWindowProc;
        WndClass.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
        RegisterClassEx(&WndClass);

        /* Create the child windows */
        Panel->Messages = CreateWindowEx(WS_EX_CLIENTEDGE,MessagesClassName,NULL,WS_CHILD|WS_CLIPSIBLINGS|WS_VISIBLE|WS_VSCROLL,
            2,2,Panel->MessageWidth,Panel->MessageHeight,hWnd,NULL,GetModuleHandle(NULL),Panel);
        Panel->Input = CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT",NULL,WS_CHILD|WS_CLIPSIBLINGS|WS_TABSTOP|WS_VISIBLE|ES_AUTOHSCROLL,
            2,Panel->Height-22,Panel->Width-64,20,hWnd,NULL,GetModuleHandle(NULL),NULL);
        OldInputFieldProc = (WNDPROC)SetWindowLong(Panel->Input,GWL_WNDPROC,(LONG)InputFieldProc);
        Panel->Button = CreateWindowEx(0,"BUTTON","Send",WS_CHILD|WS_CLIPSIBLINGS|WS_TABSTOP|WS_VISIBLE|BS_DEFPUSHBUTTON,
            Panel->Width-60,Panel->Height-22,60,20,hWnd,NULL,GetModuleHandle(NULL),NULL);

        /* Change the window's appearance */
        HFONT Font = EasyCreateFont(NULL,DefaultFontName,8,0);
        PostMessage(Panel->Input,WM_SETFONT,(WPARAM)Font,TRUE);
        Font = EasyCreateFont(NULL,DefaultFontName,8,fsBold);
        PostMessage(Panel->Button,WM_SETFONT,(WPARAM)Font,TRUE);
      }
      return 0;
    }
    case WM_COMMAND:
    {
      ChatPanel* Panel = (ChatPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      /* Process the menu and accelerator messages */
      if (Panel != NULL)
      {
        if ((HWND)lParam == Panel->Button)
          Panel->SendString();
      }
      return 0;
    }
    case WM_DESTROY:
    {
      ChatPanel* Panel = (ChatPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      /* Destroy the child windows */
      if (Panel != NULL)
      {
        DestroyWindow(Panel->Messages);
        DestroyWindow(Panel->Button);
        DestroyWindow(Panel->Input);
      }
      return 0;
    }
    case WM_KEYUP:
    {
      ChatPanel* Panel = (ChatPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL)
      {
        if (wParam == VK_RETURN)
          Panel->SendString();
      }
      break;
    }
    case WM_SIZE:
    {
      ChatPanel* Panel = (ChatPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL && LOWORD(lParam) != Panel->Width || HIWORD(lParam) != Panel->Height)
          Panel->UpdateSize((short)LOWORD(lParam), (short)HIWORD(lParam));
      return 0;
    }
  }
  return DefWindowProc(hWnd,Msg,wParam,lParam);
}

LRESULT __stdcall ChatPanel::MessagesWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch(Msg)
  {
    case WM_CREATE:
    {
      CREATESTRUCT* Params = (CREATESTRUCT*)lParam;
      ChatPanel* Panel = (ChatPanel*)Params->lpCreateParams;
      SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)Panel);
      return 0;
    }
    case WM_ERASEBKGND:
    {
      return 1;
    }
    case WM_FONTCHANGE:
    {
      ChatPanel* Panel = (ChatPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL)
      {
        SIZE Size;
        HDC DC = GetWindowDC(Panel->Messages);
        /* Calculate the editor's cell size */
        GetTextExtentPoint32(DC, "Wg", 1, &Size);
        Panel->LineHeight = max(1, Size.cy);
        /* Clean up */
        ReleaseDC(Panel->Messages, DC);
        /* Update the message panel */
        Panel->UpdateMessageSize(Panel->MessageWidth, Panel->MessageHeight);
      }
      return 0;
    }
    case WM_MOUSEWHEEL:
    {
      ChatPanel* Panel = (ChatPanel*)GetWindowLong(hWnd, GWL_USERDATA);

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
      ChatPanel* Panel = (ChatPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL && GetUpdateRect(Panel->Messages, NULL, 0) != 0)
      {
        PAINTSTRUCT PS;
        HDC DC = BeginPaint(Panel->Messages, &PS);
        if (DC != NULL)
        {
          /* Draw background */
          HBRUSH OldBrush = (HBRUSH)SelectObject(DC,CreateSolidBrush(GetSysColor(COLOR_WINDOW)));
          HPEN OldPen = (HPEN)SelectObject(DC,CreatePen(PS_SOLID,1,GetSysColor(COLOR_WINDOW)));
          Rectangle(DC,PS.rcPaint.left,PS.rcPaint.top,PS.rcPaint.right,PS.rcPaint.bottom);
          DeleteObject(SelectObject(DC,OldPen));
          DeleteObject(SelectObject(DC,OldBrush));
          /* Calculates lines that needs to be painted */
          int Top = Panel->TopRow+max(0,PS.rcPaint.top-2)/Panel->LineHeight;
          int Bottom = Panel->TopRow+max(0,PS.rcPaint.bottom-2)/Panel->LineHeight;
          /* Paints lines */
          HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(NULL,DefaultFontName,8,0));
          if (Top < Panel->Lines.Size())
          {
            int X = 2;
            int Y = 2;
            SetBkMode(DC, TRANSPARENT);
            SetTextColor(DC, GetSysColor(COLOR_WINDOWTEXT));
            /* Draws the lines */
            for (int i=Top; i < min(Bottom+1, Panel->Lines.Size()); i++)
            {
              TextOut(DC, X, Y+(i-Panel->TopRow)*Panel->LineHeight, Panel->Lines.Get(i)->c_str(), Panel->Lines.Get(i)->size());
              // TODO Wrap lines
            }
            /* Clean up */
            SetBkMode(DC,OPAQUE);
          }
          DeleteObject(SelectObject(DC,OldFont));
          EndPaint(Panel->Messages, &PS);
        }
      }
      return 0;
    }
    case WM_SIZE:
    {
      ChatPanel* Panel = (ChatPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL && LOWORD(lParam) != Panel->MessageWidth || HIWORD(lParam) != Panel->MessageHeight)
          Panel->UpdateMessageSize((short)LOWORD(lParam), (short)HIWORD(lParam));
      return 0;
    }
    case WM_VSCROLL:
    {
      ChatPanel* Panel = (ChatPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL)
        Panel->VScroll((short)LOWORD(wParam), (short)HIWORD(wParam));
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
