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

#include <winutils.h>
#include <windows.h>
#include "../chessset.h"
#include "../chess/chessengine.h"

enum {Ready=64};

typedef void (__stdcall *PLAYERPANELBUTTONCLICKEDPROC)(ChessPieceColor Color);

class PlayerPanel
{
public:
  PLAYERPANELBUTTONCLICKEDPROC JoinButtonClickedProc;
  PLAYERPANELBUTTONCLICKEDPROC ReadyButtonClickedProc;

  PlayerPanel(HWND hParent, RECT* R, ChessPieceColor NewColor);
  ~PlayerPanel();
  HWND GetHandle();
  void Invalidate();
  void SetChessSet(ChessSet* NewSet);
  void SetState(int Result);
  void ShowJoinButton(const bool Value);
  void ShowLeaveButton(const bool Value);
  void ShowReadyButton(const bool Value);
private:
  static ATOM ClassAtom;

  HWND Handle;
  HWND JoinButton;
  HWND ReadyButton;

  ChessPieceColor Color;
  ChessSet* Set;
  int State;
  int Height;
  int Width;

  /* GUI functions */
  void DrawChessPiece(HDC DC, int X, int Y);
  void DrawPlayerInformation(HDC DC, int X, int Y, int Width, int Height);
  /* Event functions */
  void Paint();
  void UpdateSize(int NewWidth, int NewHeight);
  /* WinAPI functions */
  static LRESULT __stdcall PanelWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif
