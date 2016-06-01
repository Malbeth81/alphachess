/*
* ChessEvaluator.cpp - Class that evaluates a chess move.
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
#include "chessevaluator.h"
#include <limits.h>

// Public functions ------------------------------------------------------------

void ChessEvaluator::Evaluate(ChessBoard* GameBoard, ChessPieceColor Player)
{
  Stop();
  Board.Copy(GameBoard);
  Color = Player;
  Resume();
}

// Private functions -----------------------------------------------------------

int ChessEvaluator::EvaluateMove(PossibleChessMove* Move, int Depth, int Alpha, int Beta)
{
  int Result = 0;
  if (IsActive() && Move != NULL)
  {
    ChessPiece* Piece = Board.GetPiece(Move->From);
    if (IsActive() && Piece != NULL)
    {
      /* Makes the move */
      bool EnPassant = false;
      ChessPiece* Capture = Board.MovePiece(Move->From,Move->To,EnPassant);
      if (IsActive())
      {
        if (Depth == 0)
          Result = GetChessBoardValue();
        else
        {
          /* Find all the possible counter moves */
          LinkedList<PossibleChessMove>* CounterMoves = Board.GetPossibleMoves(!Piece->Color);
          if (IsActive())
          {
            if (CounterMoves == NULL || CounterMoves->Size() == 0)
              Result = GetChessBoardValue();
            else
            {
              /* Shallow evaluation of all the counter moves */
              if (Depth > 1)
              {
                EvaluateMoves(CounterMoves, 0);
                CounterMoves->Sort(PossibleMoveComparator, Piece->Color == Color);
              }
              /* Deep evaluation of all the counter moves */
              PossibleChessMove* CounterMove = CounterMoves->GetFirst();
              while (IsActive() && CounterMove != NULL)
              {
                CounterMove->Value = EvaluateMove(CounterMove, Depth-1, Alpha, Beta);
                if (Piece->Color == Color)
                  Alpha = max(Alpha, CounterMove->Value);
                else
                  Beta = min(Beta, CounterMove->Value);
                if (Beta <= Alpha)
                  break;
                CounterMove = CounterMoves->GetNext();
              }
              if (Piece->Color == Color)
                Result = Alpha;
              else
                Result = Beta;
            }
          }
          /* Clean up */
          while (CounterMoves != NULL && CounterMoves->Size() > 0)
            delete CounterMoves->Remove();
          delete CounterMoves;
        }
      }
      Board.MoveBackPiece(Move->From,Move->To,Capture,EnPassant);
    }
  }
  return Result;
}

void ChessEvaluator::EvaluateMoves(LinkedList<PossibleChessMove>* Moves, int Depth)
{
  if (Moves != NULL)
  {
    PossibleChessMove* Move = Moves->GetFirst();
    while (IsActive() && Move != NULL)
    {
      Move->Value = EvaluateMove(Move, Depth, INT_MIN, INT_MAX);
      Move = Moves->GetNext();
    }
  }
}

int ChessEvaluator::GetChessBoardValue()
{
  int Result = 0;
  __int64 BoardKey = Board.Hash();
  HashMap::iterator Iterator = HashTable.find(BoardKey);
  HashMapEntry Entry = Iterator->second;
  if (Iterator != HashTable.end())
  {
    Result = (Color == White ? Entry.Value : -Entry.Value); /* Hash table values are stored as relative to the white player */
    Entry.Flag = true;
  }
  else
  {
    Result = Board.Evaluate(Color);
    Entry.Value = (Color == White ? Result : -Result); /* Hash table values are stored as relative to the white player */
    Entry.Flag = false;
  }
  HashTable[BoardKey] = Entry;
  return Result;
}

unsigned int ChessEvaluator::Run()
{
  /* Change thread priority */
  SetPriority(THREAD_PRIORITY_LOWEST);

  /* Obtain a list of all the possible moves and evaluates them */
  LinkedList<PossibleChessMove>* Moves = Board.GetPossibleMoves(Color);
  EvaluateMoves(Moves, 4);

  /* Notify observers (if not cancelled) */
  if (IsActive())
    NotifyObservers(EvaluationCompleted, Moves);

  /* Clean hash table */
  HashMap::iterator Iterator = HashTable.begin();
  while (Iterator != HashTable.end())
  {
    if (Iterator->second.Flag == false)
      HashTable.erase(Iterator++);
    else
    {
      Iterator->second.Flag = false;
      Iterator++;
    }
  }

  return 0;
}
