/*
* ChessEvaluator.h - Class that evaluates a chess move.
*
* Copyright (C) 2011 Marc-André Lamothe.
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
//#include <tr1/unordered_map>
#include <map>
#include <observer.h>
#include <thread.h>

//using namespace std::tr1;

enum EvaluatorEvent { EvaluationCompleted = 10 };

//class PasstroughHash {
//public:
//  unsigned int operator() (const unsigned int &a) const
//  {
//    return a;
//  };
//};

struct HashMapEntry
{
  int Value;
  bool Flag;
};

//typedef unordered_map<__int64, int, PasstroughHash> HashMap;
typedef map<__int64, HashMapEntry> HashMap;

class ChessEvaluator : public Observable, public Thread
{
public:
  void Evaluate(ChessBoard* GameBoard, ChessPieceColor Player);

private:
  ChessBoard Board;
  ChessPieceColor Color;
  HashMap HashTable;

  int EvaluateMove(PossibleChessMove* Move, int Depth, int Alpha, int Beta);
  void EvaluateMoves(LinkedList<PossibleChessMove>* Moves, int Depth);
  int GetChessBoardValue();
  unsigned int Run();
};

#endif
