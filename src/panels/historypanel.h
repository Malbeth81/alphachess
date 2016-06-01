/*
* HistoryPanel.h
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
#ifndef HISTORYPANEL_H_
#define HISTORYPANEL_H_

#include "../chess/chessgame.h"
#include "../theme.h"
#include <custombutton.h>
#include <cstrutils.h>
#include <winutils.h>
#include <windows.h>

#define WM_HISTORYBUTTONCLICKED WM_USER+502

class HistoryPanel
{
public:
  HistoryPanel(HINSTANCE hInstance, HWND hParent, RECT* R);
  ~HistoryPanel();

  void EnableFirstButton(bool Value);
  void EnableLastButton(bool Value);
  void EnableNextButton(bool Value);
  void EnablePreviousButton(bool Value);
  HWND GetHandle();
  void Invalidate();
  void SetChessSet(ChessSet* NewSet);
  void SetGame(ChessGame* NewGame);
  void ShowIcons(bool Value);
private:
  static ATOM ClassAtom;
  static ATOM MoveListClassAtom;

  HWND Handle;
  HWND MoveList;
  HWND FirstButton;
  HWND LastButton;
  HWND NextButton;
  HWND PreviousButton;

  ChessSet* Set;
  ChessGame* Game;

  bool IconsVisible;
  FontInfo Font;
  int LineHeight;
  int Height;
  int TopRow;
  int VisibleRows;
  int Width;

  /* GUI functions */
  int DrawChessPiece(HDC DC, int X, int Y, ChessPieceType Piece);
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
  static LRESULT __stdcall MoveListWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif
