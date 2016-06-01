/*
* PlayerPanel.h
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
#ifndef PLAYERPANEL_H_
#define PLAYERPANEL_H_

#define WINVER 0x0500
#include "../chess/chessgame.h"
#include "../resource.h"
#include "../theme.h"
#include <custombutton.h>
#include <winutils.h>
#include <windows.h>

class PlayerPanel
{
public:
  PlayerPanel(HINSTANCE hInstance, HWND hParent, RECT* R, ChessPieceColor NewColor);
  ~PlayerPanel();
  void EnableJoinButton(const bool Value);
  void EnableLeaveButton(const bool Value);
  void EnableReadyButton(const bool Value);
  HWND GetHandle();
  void Invalidate();
  void SetChessSet(ChessSet* NewSet);
  void SetGame(ChessGame* NewGame);
  void SetReady(bool Value);
private:
  static ATOM ClassAtom;
  static WNDPROC OldPlayerButtonProc;

  HWND Handle;
  HWND PlayerButton;
  HMENU PlayerMenu;

  ChessGame* Game;
  ChessPieceColor Color;
  ChessSet* Set;
  bool IsReady;
  int Height;
  int Width;

  /* GUI functions */
  void DrawChessPiece(HDC DC, int X, int Y);
  void DrawPlayerInformation(HDC DC, int X, int Y, int Width, int Height);
  HMENU GetMenu();
  /* Event functions */
  void Paint();
  void UpdateSize(int NewWidth, int NewHeight);
  /* WinAPI functions */
  static LRESULT __stdcall PanelWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  static LRESULT __stdcall PlayerButtonProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
};

#endif
