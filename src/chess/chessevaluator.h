/*
* ChessEvaluator.h - Class that evaluates a chess move.
*
* Copyright (C) 2010 Marc-André Lamothe.
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
#ifndef CHESSEVALUATOR_H_
#define CHESSEVALUATOR_H_

#include "chessboard.h"
#include <observer.h>
#include <thread.h>

enum EvaluatorEvent { EvaluationCompleted = 10 };

struct HashTableEntry
{
  HashTableKey Key;
  int Value;
  int Depth;
};

class ChessEvaluatorThread;
class ChessEvaluator : public Observable, public Observer
{
public:
  ChessEvaluator(ChessBoard* Board, ChessPieceColor PieceColor);
  ~ChessEvaluator();

private:
  LinkedList<PossibleChessMove>* Moves;
  LinkedList<ChessEvaluatorThread> Threads;
  LinkedList<HashTableEntry> HashTable;

  void Notify(const int Event, const void* Param);
};

#endif
