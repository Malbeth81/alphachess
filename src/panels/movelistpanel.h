/*
* MoveListPanel.h
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
#ifndef MOVELISTPANEL_H_
#define MOVELISTPANEL_H_

#include <cstrutils.h>
#include <winutils.h>
#include <windows.h>
#include "../chess/chessengine.h"

class MoveListPanel
{
public:
  MoveListPanel(HWND hParent, RECT* R);
  ~MoveListPanel();
  HWND GetHandle();
  void Invalidate();
private:
  static ATOM ClassAtom;

  HWND Handle;

  FontInfo Font;
  int LineHeight;
  int Height;
  int TopRow;
  int VisibleRows;
  int Width;

  /* GUI functions */
  void DrawMoves(HDC DC, unsigned int First, unsigned int Last);
  int LineCount();
  char* MoveToString(const ChessMove* Move);
  char* PosToString(Position Pos);
  /* Event functions */
  void MouseWheel(int Keys, int Delta, int x, int y);
  void Paint();
  void SetVScrollBar(int Pos, int Range);
  void UpdateLineSize();
  void UpdateScrollbars();
  void UpdateSize(int NewWidth, int NewHeight);
  void VScroll(int ScrollCode, int Pos);
  /* WinAPI functions */
  static LRESULT __stdcall PanelWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif
