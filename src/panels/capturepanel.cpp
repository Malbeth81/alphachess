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

const char ClassName[] = "CapturePanel";

ATOM CapturePanel::ClassAtom = 0;

// PUBLIC FUNCTIONS ------------------------------------------------------------

CapturePanel::CapturePanel(HINSTANCE hInstance, HWND hParent, RECT* R)
{
  Handle = NULL;
  Game = NULL;
  Set = NULL;

  Height = R->bottom-R->top;
  Width = R->right-R->left;

  if (ClassAtom == 0)
  {
    WNDCLASSEX WndClass;
    WndClass.cbSize = sizeof(WNDCLASSEX);
    WndClass.lpszClassName = ClassName;
    WndClass.hInstance = hInstance;
    WndClass.lpfnWndProc = WindowProc;
    WndClass.style = 0;
    WndClass.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
    WndClass.hIcon = 0;
    WndClass.hIconSm = 0;
    WndClass.hCursor = LoadCursor(NULL,IDC_ARROW);
    WndClass.lpszMenuName = NULL;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    ClassAtom = RegisterClassEx(&WndClass);
  }
  if (ClassAtom != 0)
  {
    Handle = CreateWindowEx(0,ClassName,NULL,WS_CHILD|WS_CLIPSIBLINGS,
        R->left,R->top,Width,Height,hParent,NULL,hInstance,this);
  }
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

void CapturePanel::SetGame(ChessGame* NewGame)
{
  Game = NewGame;
  Invalidate();
}

// PRIVATE GUI FUNCTIONS -------------------------------------------------------

void CapturePanel::DrawChessPieces(HDC DC)
{
  if (Game != NULL && Set != NULL)
  {
    /* Set the style */
    SetBkMode(DC,TRANSPARENT);
    HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,Set->FontName.c_str(),PixelsToPoints(DC,(Width-4)/6),0));
    SIZE Size;
    GetTextExtentPoint32(DC,Set->Letters.substr(0,1).c_str(),1,&Size);
    int SquareSize = max(Size.cx, Size.cy);
    int W = 0;
    int Z = 0;
    ChessPieceColor CurrentPlayer = White;
    const LinkedList<ChessMove>* Moves = Game->GetMoves();
    const ChessMove* Move = Moves->GetFirst();
    while (Move != NULL)
    {
      /* Obtain the piece to draw */
      if (Move->CapturedPiece != NULL)
      {
        /* Obtain the letter to use */
        int Index;
        switch (Move->CapturedPiece->Type)
        {
          case King: Index = 0; break;
          case Queen: Index = 1; break;
          case Rook: Index = 2; break;
          case Bishop: Index = 3; break;
          case Knight: Index = 4; break;
          default: Index = 5; break;
        }
        /* Draw the piece */
        TextOut(DC,W,Z,Set->Letters.substr(CurrentPlayer == White ? Index+6 : Index, 1).c_str(),1);
        W += SquareSize;
        if (W + SquareSize >= Width)
        {
          W = 1;
          Z = Z+SquareSize;
        }
      }
      CurrentPlayer = !CurrentPlayer;
      /* Get next move */
      if (Move == Game->GetDisplayedMove() || Game->GetDisplayedMove() == NULL)
        Move = NULL;
      else
        Move = Moves->GetNext();
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
      /* Create a buffer to draw on */
      HDC Buffer = CreateCompatibleDC(DC);
      HBITMAP Bitmap = CreateCompatibleBitmap(DC,Width,Height);
      SelectObject(Buffer, Bitmap);
      /* Paint background */
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
      CapturePanel* Panel = (CapturePanel*)(Params->lpCreateParams);
      SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)Panel);
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
      {
        Panel->Paint();
      }
      return 0;
    }
    case WM_SIZE:
    {
      CapturePanel* Panel = (CapturePanel*)GetWindowLong(hWnd, GWL_USERDATA);
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
