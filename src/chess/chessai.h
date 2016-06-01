/*
* ChessAI.h
*
* Copyright (C) 2007-2009 Marc-André Lamothe.
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
#ifndef CHESSAI_H_
#define CHESSAI_H_

#include "chessgame.h"

//struct HashCode
//{
//  unsigned int Code;
//  int Value;
//
//  inline bool operator!=(const HashCode& rhs)
//  {
//    return (Code != rhs.Code);
//  }
//};

struct ChessAiMove
{
  Position From;
  Position To;
  int Value;
  ChessAiMove* Child;
};

class ChessAi
{
public:
  ChessAi();
  ~ChessAi();

  ChessAiMove* Execute(ChessGame* Game, int SearchDepth, int SearchWindow);
private:
  ChessBoard Board;
  ChessPieceColor Color;
//  LinkedList<HashCode> HashTable;
  int SearchDepth;
  int SearchWindow;

  LinkedList<ChessAiMove>* CreateMoveList();
  void DeleteMoveList(LinkedList<ChessAiMove>* Moves);
  int EvaluateChessBoard();
  void EvaluateMoveList(LinkedList<ChessAiMove>* Moves, int Depth, int Alpha, int Beta);
  ChessAiMove* FindBestMove(LinkedList<ChessAiMove>* Moves);
  static int __stdcall MoveComparator(void* A, void* B);
};

#endif
