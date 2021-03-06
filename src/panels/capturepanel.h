/*
* CaptureList.h - A Windows control to display captured pieces.
*
* Copyright (C) 2007-2011 Marc-André Lamothe.
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
#ifndef CAPTUREPANEL_H_
#define CAPTUREPANEL_H_

#include "../chess/chessgame.h"
#include "../theme.h"
#include "../system.h"
#include <winutils.h>

class CapturePanel
{
public:
  CapturePanel(HINSTANCE hInstance, HWND hParent, RECT* R);
  ~CapturePanel();

  HWND GetHandle();
  void Invalidate();
  void SetChessSet(ChessSet* NewSet);
  void SetGame(ChessGame* NewGame);
private:
  static ATOM ClassAtom;

  HWND Handle;
  ChessGame* Game;
  ChessSet* Set;

  int Height;
  int Width;

  /* GUI functions */
  void DrawChessPieces(HDC DC);

  /* Event functions */
  void Paint();
  void UpdateSize(int NewWidth, int NewHeight);

  /* WinAPI functions */
  static LRESULT __stdcall WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
};

#endif
