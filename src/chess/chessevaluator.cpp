/*
* ChessEvaluator.cpp - Class that evaluates a chess move.
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
#include "chessevaluator.h"

class ChessEvaluatorThread : public Observable, private Thread
{
public:
  ChessEvaluatorThread(ChessBoard* GameBoard, LinkedList<PossibleChessMove>* PossibleMoves, LinkedList<HashTableEntry>* HashBoards)
  {
    /* Initialization */
    Board = new ChessBoard();
    Board->Copy(GameBoard);
    Moves = PossibleMoves;
    HashTable = new LinkedList<HashTableEntry>;
    Resume();
  }

  ~ChessEvaluatorThread()
  {
    /* Disable thread */
    Active = false;
    /* Wait for thread to close */
    Sleep(100);
    /* Clean up */
    delete Board;
    delete Moves;
  }

private:
  ChessBoard* Board;
  LinkedList<PossibleChessMove>* Moves;
  LinkedList<HashTableEntry>* HashTable;

  int EvaluateMove(PossibleChessMove* Move, int Depth)
  {
    int Result = 0;
    if (Active && Move != NULL)
    {
      ChessPiece* Piece = Board->GetPiece(Move->From);
      if (Active && Piece != NULL)
      {
        /* Evaluate the move */
        bool EnPassant = false;
        ChessPiece* Capture = Board->MovePiece(Move->From,Move->To,EnPassant);
        /* Look for a pre-calculated value in the HashTable */
        //HashTableKey BoardKey;
        //Board->Hash(BoardKey);
        //HashTableEntry* Entry = HashTable->GetFirst();
        //while (Entry != NULL && CompareHashKeys(Entry->Key, BoardKey))
        //  Entry = HashTable->GetNext();
        //if (Entry != NULL && Entry->Depth >= Depth)
        //  Result = (Piece->Color == White ? Entry->Value : -Entry->Value);
        //else
        {
          /* Find all the possible counter moves */
          if (Active && Depth == 0)
            Result = (Piece->Color == White ? Board->Value() : -Board->Value());
          else if (Active)
          {
            LinkedList<PossibleChessMove>* CounterMoves = Board->GetPossibleMoves(!Piece->Color);
            if (Active && CounterMoves != NULL && CounterMoves->Size() > 0)
            {
              /* Evaluate all the counter moves */
              PossibleChessMove* BestMove = NULL;
              PossibleChessMove* CounterMove = CounterMoves->GetFirst();
              while (Active && CounterMove != NULL)
              {
                CounterMove->Value = EvaluateMove(CounterMove, Depth-1);
                /* Find the best counter move */
                if (BestMove == NULL || CounterMove->Value > BestMove->Value)
                  BestMove = CounterMove;
                CounterMove = CounterMoves->GetNext();
              }
              if (BestMove != NULL)
                Result = -BestMove->Value;
            }
            else if (Active)
              Result = (Piece->Color == White ? Board->Value() : -Board->Value());
            /* Clean up */
            if (CounterMoves != NULL)
              while (CounterMoves->Size() > 0)
                delete CounterMoves->Remove();
            delete CounterMoves;
          }
          /*CriticalSectionBegin();
          if (Entry == NULL)
          {
            Entry = new HashTableEntry;
            for (int i=0; i < 8; i++)
              Entry->Key[i] = BoardKey[i];
            Entry->Depth = Depth;
            Entry->Value = (Piece->Color == White ? Result : -Result);
            HashTable->Add(Entry);
          }
          else
          {
            Entry->Depth = Depth;
            Entry->Value = (Piece->Color == White ? Result : -Result);
          }
          CriticalSectionEnd();*/
        }
        Board->MoveBackPiece(Move->From,Move->To,Capture,EnPassant);
      }
    }
    return Result;
  }

  int Run()
  {
    SetPriority(THREAD_PRIORITY_LOWEST);

    if (Active && Moves != NULL)
    {
      /* Evaluate each move */
      PossibleChessMove* Move = Moves->GetFirst();
      while (Active && Move != NULL)
      {
        /* Evaluate the move (always use odd number for depth so that it ends with opponent's move) */
        Move->Value = EvaluateMove(Move, 3);
        Move = Moves->GetNext();
      }
    }

    /* Notify observers */
    if (Active)
      NotifyObservers(EvaluationCompleted, this);
    return 0;
  }
};

// Public functions ------------------------------------------------------------

ChessEvaluator::ChessEvaluator(ChessBoard* Board, ChessPieceColor Color)
{
  /* Obtain a list of all the possible moves */
  Moves = Board->GetPossibleMoves(Color);
  if (Moves->Size() > 0)
  {
    /* Create up to 8 thread that will each evaluate a part of the move list */
    for (unsigned short i = 0; i < 8; i++)
    {
      /* Create a list containing a section of the moves to evaluate */
      LinkedList<PossibleChessMove>* SubMoves = new LinkedList<PossibleChessMove>;
      unsigned int Count = (int)(Moves->Size()/8)+1;
      for (unsigned int j = i*Count; j < (i+1)*Count; j++)
        SubMoves->Add(Moves->Get(j));
      /* Create a new evaluation thread */
      if (SubMoves->Size() > 0)
      {
        ChessEvaluatorThread* Thread = new ChessEvaluatorThread(Board, SubMoves, &HashTable);
        Thread->AddObserver(this);
        Threads.Add(Thread);
      }
    }
  }
}

ChessEvaluator::~ChessEvaluator()
{
  /* Clean up (must delete threads first) */
  while (Threads.Size() > 0)
    delete Threads.Remove();
  while (Moves->Size() > 0)
    delete Moves->Remove();
  while (HashTable.Size() > 0)
    delete HashTable.Remove();
  delete Moves;
}

// Private functions -----------------------------------------------------------

void ChessEvaluator::Notify(const int Event, const void* Param)
{
  switch (Event)
  {
    case EvaluationCompleted:
    {
      ChessEvaluatorThread* Thread = (ChessEvaluatorThread*)Param;

      /* Close the thread */
      Threads.Remove(Thread);
      delete Thread;

      /* Notify observers when there is no more active threads */
      if (Threads.Size() == 0)
        NotifyObservers(EvaluationCompleted, Moves);
      break;
    }
  }
}
