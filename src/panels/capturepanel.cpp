/*
* CaptureList.cpp - A Windows control to display captured pieces.
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
#include "capturepanel.h"

extern ChessEngine Chess;

const char ClassName[] = "CapturePanel";

ATOM CapturePanel::ClassAtom = 0;

// PUBLIC FUNCTIONS ------------------------------------------------------------

CapturePanel::CapturePanel(HWND hParent, RECT* R)
{
  HINSTANCE Instance = (hParent != NULL ? (HINSTANCE)GetWindowLong(hParent, GWL_HINSTANCE) : GetModuleHandle(NULL));

  Handle = NULL;

  Height = R->bottom-R->top;
  Width = R->right-R->left;

  Set = NULL;

  if (ClassAtom == 0)
  {
    WNDCLASSEX WndClass;
    WndClass.cbSize = sizeof(WNDCLASSEX);
    WndClass.lpszClassName = ClassName;
    WndClass.hInstance = Instance;
    WndClass.lpfnWndProc = WindowProc;
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
  if (ClassAtom != 0)
    CreateWindowEx(0,ClassName,NULL,WS_CHILD|WS_CLIPSIBLINGS,R->left,R->top,Width,Height,hParent,NULL,Instance,this);
}

CapturePanel::~CapturePanel()
{
  if (Handle != NULL)
    DestroyWindow(Handle);
}

HWND CapturePanel::GetHandle()
{
  return Handle;
}

void CapturePanel::Invalidate()
{
  if (Handle != NULL)
    InvalidateRect(Handle, NULL, FALSE);
}

void CapturePanel::SetChessSet(ChessSet* NewSet)
{
  Set = NewSet;
  Invalidate();
}

// PRIVATE GUI FUNCTIONS -------------------------------------------------------

void CapturePanel::DrawChessPieces(HDC DC)
{
  if (Set != NULL)
  {
    int X = 2;
    int Y = 2;
    /* Set the style */
    SetBkMode(DC,TRANSPARENT);
    int FontSize = max((int)(Height*0.15),10);
    HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,Set->FontName,FontSize,0));
    SIZE Size;
    GetTextExtentPoint32(DC,&Set->Letters[0],1,&Size);
    int SquareSize = max(Size.cx, Size.cy);
    int W = X;
    int Z = Y;
    ChessPieceColor Color = White;
    const ChessMove* Move = Chess.GetFirstMove();
    while (Move != NULL)
    {
      /* Obtain the piece to draw */
      if (Move->CapturedPiece != Blank)
      {
        /* Obtain the letter to use */
        int Index;
        switch (Move->CapturedPiece)
        {
          case King: Index = 0; break;
          case Queen: Index = 1; break;
          case Rook: Index = 2; break;
          case Bishop: Index = 3; break;
          case Knight: Index = 4; break;
          default: Index = 5; break;
        }
        /* Draw the piece */
        if (Color == White)
          TextOut(DC,W,Z,&Set->Letters[Index],1);
        else
          TextOut(DC,W,Z,&Set->Letters[Index+6],1);
        W += SquareSize;
        if (W + SquareSize >= Width)
        {
          W = X;
          Z = Z+SquareSize;
        }
      }
      Color = !Color;
      Move = Chess.GetNextMove();
    }
    /* Clean up */
    SetBkMode(DC,OPAQUE);
    DeleteObject(SelectObject(DC,OldFont));
  }
}

// PRIVATE EVENT FUNCTIONS -----------------------------------------------------

void CapturePanel::Paint()
{
  if (Handle != NULL && GetUpdateRect(Handle, NULL, 0) != 0)
  {
    PAINTSTRUCT PS;
    HDC DC = BeginPaint(Handle, &PS);
    if (DC != NULL)
    {
      /* Create a buffer DC to draw on */
      HDC Buffer = CreateCompatibleDC(DC);
      HBITMAP Bitmap = CreateCompatibleBitmap(DC,Width,Height);
      SelectObject(Buffer, Bitmap);
      /* Erase the window */
      HBRUSH OldBrush = (HBRUSH)SelectObject(Buffer,CreateSolidBrush(GetSysColor(COLOR_BTNFACE)));
      HPEN OldPen = (HPEN)SelectObject(Buffer,CreatePen(PS_SOLID,1,GetSysColor(COLOR_BTNFACE)));
      Rectangle(Buffer,0,0,Width,Height);
      DeleteObject(SelectObject(Buffer,OldPen));
      DeleteObject(SelectObject(Buffer,OldBrush));
      /* Draw the edge */
      RECT R = {0, 0, Width, Height};
      DrawEdge(Buffer, &R, BDR_SUNKENOUTER, BF_RECT);
      /* Draw the chess pieces */
      DrawChessPieces(Buffer);
      /* Draw the buffer into the destination DC */
      BitBlt(DC,0,0,Width,Height,Buffer,0,0,SRCCOPY);
      /* Cleanup */
      DeleteDC(Buffer);
      DeleteObject(Bitmap);
    }
    EndPaint(Handle, &PS);
  }
}

void CapturePanel::UpdateSize(int NewWidth, int NewHeight)
{
  Width = NewWidth;
  Height = NewHeight;
}

// WINAPI FUNCTIONS ------------------------------------------------------------

LRESULT __stdcall CapturePanel::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch(Msg)
  {
    case WM_CREATE:
    {
      CREATESTRUCT* Params = (CREATESTRUCT*)lParam;
      CapturePanel* Panel = (CapturePanel*)Params->lpCreateParams;
      SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)Panel);

      if (Panel != NULL)
        Panel->Handle = hWnd;

      return 0;
    }
    case WM_ERASEBKGND:
    {
      return 1;
    }
    case WM_PAINT:
    {
      CapturePanel* Panel = (CapturePanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL)
        Panel->Paint();
      return 0;
    }
    case WM_SIZE:
    {
      CapturePanel* Panel = (CapturePanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL && (short)LOWORD(lParam) != Panel->Width || (short)HIWORD(lParam) != Panel->Height)
        Panel->UpdateSize((short)LOWORD(lParam), (short)HIWORD(lParam));
      return 0;
    }
  }
  return DefWindowProc(hWnd,Msg,wParam,lParam);
}
