/*
* ChessBoard.h - A Windows control to display a chess board.
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
#ifndef CHESSBOARDPANEL_H_
#define CHESSBOARDPANEL_H_

#include <math.h>
#include <winutils.h>
#include <windows.h>
#include "../chessboardtheme.h"
#include "../chessset.h"
#include "../chess/chessengine.h"

#define MinBorderSize 10
#define MinSquareSize 20
#define MinBoardSize MinBorderSize*2+MinSquareSize*8

typedef bool (__stdcall *MOVEPIECEPROC)(Position From, Position To);

class ChessBoardPanel
{
public:
  MOVEPIECEPROC MovePieceProc;

  ChessBoardPanel(HWND hParent, RECT* R);
  ~ChessBoardPanel();
  HWND GetHandle();
  void Invalidate();

  ChessSet* GetChessSet();
  bool GetInvertView();
  bool GetShowCoordinates();
  bool GetShowLastMove();
  bool GetShowInvalidMoves();
  int GetSquareSize();
  HCURSOR GetStandardCursor();
  HCURSOR GetSelectionCursor();
  const ChessBoardTheme* GetTheme();

  void SetChessSet(ChessSet* NewSet);
  void SetInvertView(bool Value);
  void SetLocked(bool Value);
  void SetPaused(bool Value);
  void SetShowCoordinates(bool Value);
  void SetShowLastMove(bool Value);
  void SetShowInvalidMoves(bool Value);
  void SetStandardCursor(HCURSOR Cursor);
  void SetSelectionCursor(HCURSOR Cursor);
  void SetTheme(ChessBoardTheme* NewTheme);
private:
  static ATOM ClassAtom;

  HWND Handle;
  ChessSet* Set;
  HCURSOR SelectionCursor;
  HCURSOR StandardCursor;
  ChessBoardTheme* Theme;

  int BorderSize;
  int Height;
  Position HighlightedSquare;
  bool InvertView;
  bool Locked;
  bool MouseMoved;
  bool Paused;
  Position SelectedSquare;
  bool ShowCoordinates;
  bool ShowLastMove;
  bool ShowInvalidMoves;
  int SquareSize;
  int Width;

  /* GUI functions */
  void DrawBoard(HDC DC);
  void DrawBoardIndexes(HDC DC, int X, int Y, bool Vertical);
  void DrawBoardSquares(HDC DC, int X, int Y);
  void DrawChessPieces(HDC DC, int X, int Y);
  void DrawChessPiece(HDC DC, int X, int Y, const ChessPiece* Piece);
  void DrawPaused(HDC DC);
  Position MakePos(int x, int y);
  /* Event functions */
  void MouseDown(int x, int y);
  void MouseUp(int x, int y);
  void MouseMove(int x, int y);
  void Paint();
  void UpdateSize(int NewWidth, int NewHeight);
  /* WinAPI functions */
  static LRESULT __stdcall PanelWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif
