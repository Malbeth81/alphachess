/*
* ChessBoard.h
*
* Copyright (C) 2007-2011 Marc-André Lamothe.
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
#ifndef CHESSBOARD_H_
#define CHESSBOARD_H_

#include <cstrutils.h>
#include <linkedlist.h>

enum ChessPieceType {Blank, Pawn, Knight, Bishop, Rook, Queen, King};

enum ChessPieceColor {White, Black};

inline ChessPieceColor operator!(ChessPieceColor Color)
{
  if (Color == White)
    return Black;
  else
    return White;
}

class Position
{
public:
  short x;
  short y;

  Position(const int x1 = 0, const int y1 = 0)
  {
    x = x1;
    y = y1;
  }

  Position(const Position &Pos)
  {
    x = Pos.x;
    y = Pos.y;
  }

  static Position ParseString(const char* Str)
  {
    static const char* HorizontalValues = "abcdefgh";
    static const char* VerticalValues = "12345678";
    Position P;
    P.x = -1;
    P.y = -1;
    if (strlen(Str) > 0)
    {
      if (strlen(Str) >= 2)
      {
        P.x = stripos(HorizontalValues, Str[0]);
        P.y = stripos(VerticalValues, Str[1]);
      }
      else if (strcontains(Str, HorizontalValues) == 0)
        P.x = stripos(HorizontalValues, Str[0]);
      else
        P.y = stripos(VerticalValues, Str[0]);
    }
    return P;
  }

  char* ToString() const
  {
    static const char* HorizontalValues = "abcdefgh";
    static const char* VerticalValues = "12345678";
    char* Text = new char[3];
    Text[0] = HorizontalValues[x];
    Text[1] = VerticalValues[y];
    Text[2] = '\0';
    return Text;
  }

  bool operator==(Position P)
  {
    return (x == P.x && y == P.y);
  }

  bool operator!=(Position P)
  {
    return !(*this == P);
  }
};

class ChessPiece
{
public:
  ChessPieceType Type;
  ChessPieceColor Color;
  unsigned short TimesMoved;

  ChessPiece(ChessPieceType PieceType, ChessPieceColor PieceColor, unsigned short Times = 0)
  {
    Type = PieceType;
    Color = PieceColor;
    TimesMoved = Times;
  }

  static ChessPieceType GetPieceType(const char* Notation)
  {
    if (Notation != NULL)
    {
      switch (Notation[0])
      {
        case 'N': return Knight;
        case 'B': return Bishop;
        case 'R': return Rook;
        case 'Q': return Queen;
        case 'K': return King;
      }
    }
    return Pawn;
  }

  char* GetNotation()
  {
    return GetNotation(Type);
  }

  static char* GetNotation(ChessPieceType Type)
  {
    char* Str = new char[2];
    switch (Type)
    {
      case Knight: Str[0] = 'N'; break;
      case Bishop: Str[0] = 'B'; break;
      case Rook: Str[0] = 'R'; break;
      case Queen: Str[0] = 'Q'; break;
      case King: Str[0] = 'K'; break;
      default: Str[0] = '\0'; break;
    }
    Str[1] = '\0';
    return Str;
  }
};

struct PossibleChessMove
{
  Position From;
  Position To;
  int Value;
};

inline PossibleChessMove* FindBestPossibleMove(LinkedList<PossibleChessMove>* Moves, int x = -1, int y = -1)
{
  PossibleChessMove* BestMove = NULL;
  PossibleChessMove* Ptr = Moves->GetFirst();
  while (Ptr != NULL)
  {
    if ((x == -1 || y == -1 || (Ptr->To.x == x && Ptr->To.y == y)) && (BestMove == NULL || Ptr->Value > BestMove->Value))
      BestMove = Ptr;
    Ptr = Moves->GetNext();
  }
  return BestMove;
}

inline int __stdcall PossibleMoveComparator(PossibleChessMove* A, PossibleChessMove* B)
{
  if (B->Value > A->Value)
    return 1;
  else if (B->Value < A->Value)
    return -1;
  return 0;
}

class ChessBoard
{
public:
  ChessBoard();
  ~ChessBoard();

  bool AddPiece(const Position Pos, ChessPiece* Piece);
  void Copy(const ChessBoard* Board);
  void Clear();
  int Evaluate(const ChessPieceColor Player);
  int Evaluate(const Position Pos, const ChessPieceColor Player);
  Position FindStartingPos(const Position From, const Position To, const ChessPieceType Type, const ChessPieceColor Color);
  ChessPiece* GetPiece(const Position Pos);
  LinkedList<PossibleChessMove>* GetPossibleMoves(const ChessPieceColor Color);
  int PieceValue(const ChessPieceColor Color, const ChessPieceType Type);
  __int64 Hash();
  bool IsCastlingAvailable(const ChessPieceColor Color, const bool Small);
  bool IsCastlingPossible(const ChessPieceColor Color, const bool Small);
  bool IsKingCheck(const ChessPieceColor Color);
  bool IsKingMate(const ChessPieceColor Color);
  bool IsMovePossible(const Position From, const Position To);
  bool IsMoveValid(const Position From, const Position To);
  bool IsPawnBlocked(const Position Pos);
  bool IsPawnDoubled(const Position Pos);
  bool IsPawnIsolated(const Position Pos);
  bool IsPawnPassed(const Position Pos);
  bool IsPieceAttacked(const Position Pos, const ChessPieceColor Color);
  bool IsPieceAttacking(const Position Pos);
  bool IsPieceDefended(const Position Pos, const ChessPieceColor Color);
  bool IsPieceDefending(const Position Pos);
  bool IsPieceMovable(const Position Pos);
  bool IsPieceMovementValid(const Position From, const Position To);
  bool IsPositionValid(const Position Pos);
  ChessPiece* MovePiece(const Position From, const Position To, bool& EnPassant);
  void MoveBackPiece(const Position From, const Position To, ChessPiece* CapturedPiece, bool EnPassant = false);
  ChessPiece* RemovePiece(const Position Pos);
private:
  ChessPiece* Cases[8][8];

  bool BlackKingCastled;
  Position BlackKingPos;
  bool BlackHasQueen;
  int BlackBishopCount;
  int BlackPawnCount;
  int BlackPieceCount;

  bool WhiteKingCastled;
  Position WhiteKingPos;
  bool WhiteHasQueen;
  int WhiteBishopCount;
  int WhitePawnCount;
  int WhitePieceCount;

  LinkedList<Position> MovedPieces;

  static unsigned int HashValues[12][8][8][2];
  static bool HashValuesInitialised;
  static const int IsolatedPawnsPenalty[8];

  __int64 HashKey1;
  __int64 HashKey2;
};

#endif
