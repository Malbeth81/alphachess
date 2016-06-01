/*
* ChessBoard.h
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
#ifndef CHESSBOARD_H_
#define CHESSBOARD_H_

enum ChessPieceType {Blank, Pawn, Knight, Bishop, Rook, Queen, King};

enum ChessPieceColor {White, Black};

inline ChessPieceColor operator!(ChessPieceColor Color)
{
  if (Color == White)
    return ChessPieceColor(Black);
  else
    return ChessPieceColor(White);
}

class Position
{
public:
  int x;
  int y;

  Position(const int x1 = 0, const int y1 = 0)
  {
    x = x1;
    y = y1;
  }

  char* ToAlgebraicNotation()
  {
    static const char* HorizontalValues = "abcdefgh";
    static const char* VerticalValues = "12345678";
    char* Value = new char[3];
    Value[0] = HorizontalValues[x];
    Value[1] = VerticalValues[y];
    Value[2] = '\0';
    return Value;
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

class ChessBoard
{
public:
  ChessBoard();
  ~ChessBoard();

  bool AddPiece(const Position Pos, ChessPiece* Piece);
  void Assign(ChessBoard* Board);
  void Clear();
  ChessPiece* GetPiece(const Position Pos);
  bool IsAttacked(const Position Pos, const ChessPieceColor Color);
  bool IsAttacking(const Position Pos);
  bool IsCastlingAvailable(const ChessPieceColor Color, const bool Small);
  bool IsCastlingPossible(const ChessPieceColor Color, const bool Small);
  bool IsDefended(const Position Pos, const ChessPieceColor Color);
  bool IsDefending(const Position Pos);
  bool IsKingCheck(const ChessPieceColor Color);
  bool IsKingMate(const ChessPieceColor Color);
  bool IsMovePossible(const Position From, const Position To);
  bool IsMoveValid(const Position From, const Position To);
  bool IsPawnBlocked(const Position Pos);
  bool IsPawnDoubled(const Position Pos);
  bool IsPawnIsolated(const Position Pos);
  bool IsPawnPassed(const Position Pos);
  bool IsPieceMovementValid(const ChessPiece* Piece, const Position From, const Position To);
  bool IsPieceStuck(const Position Pos);
  bool IsPositionValid(const Position Pos);
  ChessPiece* MovePiece(const Position From, const Position To, bool& EnPassant);
  void MoveBackPiece(const Position From, const Position To, ChessPiece* CapturedPiece, bool EnPassant = false);
  ChessPiece* RemovePiece(const Position Pos);
private:
  ChessPiece* Cases[8][8];
  Position WhiteKingPos;
  Position BlackKingPos;
};

#endif
