/*
* ChessBoard.cpp
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
#include "chessboard.h"

#ifndef NULL
  #define NULL 0
#endif
#ifndef abs
  #define abs(a) ((a) < 0 ? -(a) : (a))
#endif
#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

// Public function -------------------------------------------------------------

ChessBoard::ChessBoard()
{
  for (int i=0; i < 8; i++)
    for (int j=0; j < 8; j++)
      Cases[i][j] = NULL;
  WhiteKingPos.x = -1;
  WhiteKingPos.y = -1;
  BlackKingPos.x = -1;
  BlackKingPos.y = -1;
}

ChessBoard::~ChessBoard()
{
  for (int i=0; i < 8; i++)
    for (int j=0; j < 8; j++)
      if (Cases[i][j] != NULL)
        delete Cases[i][j];
}

bool ChessBoard::AddPiece(const Position Pos, ChessPiece* Piece)
{
  if (Piece != NULL && IsPositionValid(Pos))
  {
    /* Save the king's position */
    if (Piece->Type == King)
    {
      if (Piece->Color == White)
        WhiteKingPos = Pos;
      else
        BlackKingPos = Pos;
    }
    /* Add the piece to the board */
    if (Cases[Pos.x][Pos.y] != NULL)
      delete Cases[Pos.x][Pos.y];
    Cases[Pos.x][Pos.y] = Piece;
    return true;
  }
  return false;
}

void ChessBoard::Assign(ChessBoard* Board)
{
  for (int i=0; i < 8; i++)
    for (int j=0; j < 8; j++)
      if (Board->Cases[i][j] != NULL)
        Cases[i][j] = new ChessPiece(Board->Cases[i][j]->Type,Board->Cases[i][j]->Color,Board->Cases[i][j]->TimesMoved);
      else if (Cases[i][j] != NULL)
      {
        delete Cases[i][j];
        Cases[i][j]= NULL;
      }
  WhiteKingPos = Board->WhiteKingPos;
  BlackKingPos = Board->BlackKingPos;
}

void ChessBoard::Clear()
{
  /* Clear the game board */
  for (int i=0; i < 8; i++)
    for (int j=0; j < 8; j++)
      if (Cases[i][j] != NULL)
      {
        delete Cases[i][j];
        Cases[i][j] = NULL;
      }
  WhiteKingPos.x = -1;
  WhiteKingPos.y = -1;
  BlackKingPos.x = -1;
  BlackKingPos.y = -1;
}

ChessPiece* ChessBoard::GetPiece(const Position Pos)
{
  if (IsPositionValid(Pos))
    return Cases[Pos.x][Pos.y];
  return NULL;
}

bool ChessBoard::IsAttacked(const Position Pos, const ChessPieceColor Color)
{
  if (IsPositionValid(Pos))
  {
    Position P;
    for (P.x=0; P.x < 8; P.x++)
      for (P.y=0; P.y < 8; P.y++)
      {
        ChessPiece* Piece = GetPiece(P);
        if (Piece != NULL && Piece->Color != Color && IsPieceMovementValid(Piece,P,Pos)
            && IsMovePossible(P,Pos))
          return true;
      }
  }
  return false;
}

bool ChessBoard::IsAttacking(const Position Pos)
{
  if (IsPositionValid(Pos))
    if (Cases[Pos.x][Pos.y] != NULL)
      for (int i=0; i < 8; i++)
        for (int j=0; j < 8; j++)
          if (Cases[i][j] != NULL && Cases[i][j]->Color != Cases[Pos.x][Pos.y]->Color
              && IsPieceMovementValid(Cases[Pos.x][Pos.y], Pos, Position(i, j))
              && IsMovePossible(Pos,Position(i, j)))
            return true;
  return false;
}

bool ChessBoard::IsCastlingAvailable(const ChessPieceColor Color, const bool Small)
{
  Position Pos = (Color == White ? WhiteKingPos : BlackKingPos);
  if (Cases[Pos.x][Pos.y] != NULL && Cases[Pos.x][Pos.y]->TimesMoved == 0)
  {
    Pos.x = (Small ? 7 : 0);
    if (Cases[Pos.x][Pos.y] != NULL && Cases[Pos.x][Pos.y]->Type == Rook
        && Cases[Pos.x][Pos.y]->Color == Color && Cases[Pos.x][Pos.y]->TimesMoved == 0)
      return true;
  }
  return false;
}

bool ChessBoard::IsCastlingPossible(const ChessPieceColor Color, const bool Small)
{
  if (!IsKingCheck(Color) && IsCastlingAvailable(Color, Small))
  {
    Position Pos = (Color == White ? WhiteKingPos : BlackKingPos);
    if (Small && Cases[Pos.x+1][Pos.y] == NULL && Cases[Pos.x+2][Pos.y] == NULL
        && !IsAttacked(Position(Pos.x+1, Pos.y), Color) && !IsAttacked(Position(Pos.x+2, Pos.y), Color))
      return true;
    else if (!Small && Cases[Pos.x-1][Pos.y] == NULL && Cases[Pos.x-2][Pos.y] == NULL
        && !IsAttacked(Position(Pos.x-1, Pos.y), Color) && !IsAttacked(Position(Pos.x-2, Pos.y), Color))
      return true;
  }
  return false;
}

bool ChessBoard::IsDefended(const Position Pos, const ChessPieceColor Color)
{
  if (IsPositionValid(Pos))
    for (int i = 0; i < 8; i++)
      for (int j = 0; i < 8; i++)
        if (Cases[i][j] != NULL && Cases[i][j]->Color == Color
            && IsPieceMovementValid(Cases[i][j], Position(i,j), Pos))
        {
          ChessPiece* Piece = RemovePiece(Pos);
          if (IsMovePossible(Position(i,j), Pos))
          {
            AddPiece(Pos, Piece);
            return true;
          }
          AddPiece(Pos, Piece);
        }
  return false;
}

bool ChessBoard::IsDefending(const Position Pos)
{
  if (IsPositionValid(Pos))
    if (Cases[Pos.x][Pos.y] != NULL)
      for (int i = 0; i < 8; i++)
        for (int j = 0; i < 8; i++)
          if (Cases[i][j] != NULL && Cases[i][j]->Color == (Cases[Pos.x][Pos.y]->Color
              && IsPieceMovementValid(Cases[Pos.x][Pos.y], Pos, Position(i,j))))
          {
            ChessPiece* Piece = RemovePiece(Position(i, j));
            if (IsMovePossible(Pos, Position(i,j)))
            {
              AddPiece(Position(i, j), Piece);
              return true;
            }
            AddPiece(Position(i, j), Piece);
          }
  return false;
}

bool ChessBoard::IsKingCheck(const ChessPieceColor Color)
{
  return IsAttacked((Color == White ? WhiteKingPos : BlackKingPos), Color);
}

bool ChessBoard::IsKingMate(const ChessPieceColor Color)
{
  Position Pos = (Color == White ? WhiteKingPos : BlackKingPos);
  /* Can the king move */
  if (!IsPieceStuck(Pos))
    return false;
  /* Can any other piece make a valid move */
  for (int i=0; i < 8; i++)
    for (int j=0; j < 8; j++)
    {
      Position Pos(i,j);
      if (Cases[i][j] != NULL && Cases[i][j]->Color == Color && !IsPieceStuck(Pos))
        return false;
    }
  return true;
}

bool ChessBoard::IsMovePossible(const Position From, const Position To)
{
  if (IsPositionValid(From) && IsPositionValid(To))
  {
    int cx = To.x-From.x;
    int cy = To.y-From.y;
    /* Check if there is a friendly piece on the destination square */
    if (Cases[To.x][To.y] != NULL && Cases[From.x][From.y]->Color == Cases[To.x][To.y]->Color)
      return false;
    else if (cx == 0 || cy == 0 || abs(cx) == abs(cy))
    {
      /* Checks if the path to the destination square is clear */
      int dx = (cx != 0 ? cx/abs(cx) : 0);
      int dy = (cy != 0 ? cy/abs(cy) : 0);
      Position Pos = From;
      Pos.x = Pos.x+dx;
      Pos.y = Pos.y+dy;
      while (Pos.x != To.x || Pos.y != To.y)
      {
        if (Cases[Pos.x][Pos.y] != NULL)
          return false;
        Pos.x = Pos.x+dx;
        Pos.y = Pos.y+dy;
      }
    }
    return true;
  }
  return false;
}

bool ChessBoard::IsMoveValid(const Position From, const Position To)
{
  if (IsPositionValid(From) && IsPositionValid(To))
  {
    ChessPiece* Piece = Cases[From.x][From.y];
    if (Piece != NULL && IsPieceMovementValid(Piece, From, To) && IsMovePossible(From, To))
    {
      /* Move the piece to destination */
      ChessPiece* CapturedPiece = RemovePiece(To);
      AddPiece(To, RemovePiece(From));
      /* A move that makes your own king check is invalid */
      bool Result = !IsKingCheck(Piece->Color);
      /* Returns the piece to origin  */
      AddPiece(From, RemovePiece(To));
      AddPiece(To, CapturedPiece);
      return Result;
    }
  }
  return false;
}

bool ChessBoard::IsPawnBlocked(const Position Pos)
{
  /* A pawn is blocked if it can't move */
  if (IsPositionValid(Pos))
    if (Cases[Pos.x][Pos.y] != NULL && Cases[Pos.x][Pos.y]->Type == Pawn)
    {
      int Row = (Cases[Pos.x][Pos.y]->Color == White ? min(7,Pos.y+1) : max(0,Pos.y-1));
      for (int i = max(0, Pos.x-1); i <= min(7, Pos.x+1); i++)
        if (IsMovePossible(Pos, Position(i, Row)))
          return false;
    }
  return true;
}

bool ChessBoard::IsPawnDoubled(const Position Pos)
{
  /* A pawn is doubled if there is another pawn of the same color in the file */
  if (IsPositionValid(Pos))
    if (Cases[Pos.x][Pos.y] != NULL && Cases[Pos.x][Pos.y]->Type == Pawn)
      for (int i = 0; i < 8; i++)
        if (i != Pos.y && Cases[Pos.x][i] != NULL && Cases[Pos.x][i]->Type == Pawn
            && Cases[Pos.x][i]->Color == Cases[Pos.x][Pos.y]->Color)
          return true;
  return false;
}

bool ChessBoard::IsPawnIsolated(const Position Pos)
{
  /* A pawn is isolated if there is no pawn of the same color in the adjacent columns */
  if (IsPositionValid(Pos))
    if (Cases[Pos.x][Pos.y] != NULL && Cases[Pos.x][Pos.y]->Type == Pawn)
      for (int i = max(0, Pos.x-1); i <= min(7, Pos.x+1); i+=2)    // Skips i == x
        for (int j = max(0, Pos.y-1); j <= min(7, Pos.y+1); j++)
          if (Cases[i][j] != NULL && Cases[i][j]->Type == Pawn
              && Cases[i][j]->Color == Cases[Pos.x][Pos.y]->Color)
            return false;
  return true;
}

bool ChessBoard::IsPawnPassed(const Position Pos)
{
  /* A pawn is passed if there is no enemy pawn on his way to the end of the board */
  if (IsPositionValid(Pos))
    if (Cases[Pos.x][Pos.y] != NULL && Cases[Pos.x][Pos.y]->Type == Pawn)
      for (int i = max(0, Pos.x-1); i <= min(7, Pos.x+1); i++)
        if (Cases[Pos.x][Pos.y]->Color == White)
        {
          for (int j = min(7, Pos.y+1); j <= 7; j++)
            if (Cases[i][j] != NULL && Cases[i][j]->Type == Pawn
                && Cases[i][j]->Color != Cases[Pos.x][Pos.y]->Color)
              return false;
        }
        else
        {
          for (int j = max(0, Pos.y-1); j >= 0; j--)
            if (Cases[i][j] != NULL && Cases[i][j]->Type == Pawn
                && Cases[i][j]->Color != Cases[Pos.x][Pos.y]->Color)
              return false;
        }
  return true;
}

bool ChessBoard::IsPieceMovementValid(const ChessPiece* Piece, const Position From, const Position To)
{
  if (Piece != NULL && IsPositionValid(From) && IsPositionValid(To))
  {
    int cx = To.x-From.x;
    int cy = To.y-From.y;
    switch (Piece->Type)
    {
      case Pawn:
        switch (Piece->Color)
        {
          case White:
            /* Pawns can move one square forward without capturing */
            if (cx == 0 && cy == 1 && Cases[To.x][To.y] == NULL)
              return true;
            /* Pawns can move two square forward from their starting position without capturing */
            if (cx == 0 && cy == 2 && From.y == 1 && Cases[To.x][To.y] == NULL)
              return true;
            /* Pawns can move one square forward in diagonal while capturing */
            if (abs(cx) == 1 && cy == 1 && Cases[To.x][To.y] != NULL)
              return true;
            /* Pawns can move one square forward in diagonal while capturing en passant */
            if (abs(cx) == 1 && cy == 1 && From.y == 4 && Cases[To.x][From.y] != NULL && Cases[To.x][From.y]->Type == Pawn && Cases[To.x][From.y]->Color != Piece->Color && Cases[To.x][From.y]->TimesMoved == 1)
              return true;
            break;
          case Black:
            /* Pawns can move one square forward without capturing */
            if (cx == 0 && cy == -1 && Cases[To.x][To.y] == NULL)
              return true;
            /* Pawns can move two square forward from their starting position without capturing */
            if (cx == 0 && cy == -2 && From.y == 6 && Cases[To.x][To.y] == NULL)
              return true;
            /* Pawns can move one square forward in diagonal while capturing */
            if (abs(cx) == 1 && cy == -1 && Cases[To.x][To.y] != NULL)
              return true;
            /* Pawns can move one square forward in diagonal while capturing en passant */
            if (abs(cx) == 1 && cy == -1 && From.y == 3 && Cases[To.x][From.y] != NULL && Cases[To.x][From.y]->Type == Pawn && Cases[To.x][From.y]->Color != Piece->Color && Cases[To.x][From.y]->TimesMoved == 1)
              return true;
            break;
        }
        break;
      case Rook:
        /* Rooks can move horizontally or vertically */
        if (cx == 0 || cy == 0)
          return true;
        break;
      case Knight:
        /* Knights can move 1 square in a direction and 2 in another */
        if ((abs(cx) == 1 && abs(cy) == 2) || (abs(cx) == 2 && abs(cy) == 1))
          return true;
        break;
      case Bishop:
        /* Bishops can move diagonally */
        if (abs(cx) == abs(cy))
          return true;
        break;
      case Queen:
        /* Queens can move horizontally, vertically and diagonally */
        if (cx == 0 || cy == 0 || abs(cx) == abs(cy))
          return true;
        break;
      case King:
        /* Kings can move one square horizontally, vertically and diagonally */
        if ((cx == 0 && abs(cy) == 1) || (abs(cx) == 1 && cy == 0) || (abs(cx) == 1 && abs(cy) == 1))
          return true;
        /* Kings can move two squares horizontally if they castle */
        else if (cy == 0 && abs(cx) == 2
            && ((Piece->Color == White && From.x == 4 && From.y == 0 && (To.x == 2 || To.x == 6))
            || (Piece->Color == Black && From.x == 4 && From.y == 7 && (To.x == 2 || To.x == 6)))
            && IsCastlingPossible(Cases[From.x][From.y]->Color,(To.x == 6)))
          return true;
        break;
      default:
        break;
    }
  }
  return false;
}

bool ChessBoard::IsPieceStuck(const Position Pos)
{
  if (IsPositionValid(Pos))
    if (Cases[Pos.x][Pos.y] != NULL)
    {
      Position P;
      for (P.x = 0; P.x < 8; P.x++)
        for (P.y = 0; P.y < 8; P.y++)
          if ((P.x != Pos.x || P.y != Pos.y) && IsMoveValid(Pos,P))
            return false;
    }
  return true;
}

bool ChessBoard::IsPositionValid(const Position Pos)
{
  return (Pos.x >= 0 && Pos.x < 8 && Pos.y >= 0 && Pos.y < 8);
}

ChessPiece* ChessBoard::MovePiece(Position From, Position To, bool& EnPassant)
{
  if (IsPositionValid(From) && IsPositionValid(To))
  {
    /* Move the piece */
    ChessPiece* CapturedPiece = Cases[To.x][To.y];
    Cases[To.x][To.y] = Cases[From.x][From.y];
    Cases[From.x][From.y] = NULL;
    if (CapturedPiece == NULL && Cases[To.x][To.y]->Type == Pawn && abs(To.y-From.y) == abs(To.x-From.x))
    {
      CapturedPiece = RemovePiece(Position(To.x,From.y));
      EnPassant = true;
    }
    else
      EnPassant = false;
    /* Update the piece's information */
    if (Cases[To.x][To.y] != NULL)
    {
      Cases[To.x][To.y]->TimesMoved++;
      if (Cases[To.x][To.y]->Type == King)
      {
        /* Update the position of the king */
        if (Cases[To.x][To.y]->Color == White)
          WhiteKingPos = To;
        else
          BlackKingPos = To;
        /* Move the rook if the king castled */
        if (abs(To.x-From.x) == 2)
        {
          if (To.x == 2)
          {
            if (Cases[To.x+1][To.y] != NULL)
              delete Cases[To.x+1][To.y];
            Cases[To.x+1][To.y] = Cases[0][To.y];
            Cases[0][To.y] = NULL;
          }
          else if (To.x == 6)
          {
            if (Cases[To.x-1][To.y] != NULL)
              delete Cases[To.x-1][To.y];
            Cases[To.x-1][To.y] = Cases[7][To.y];
            Cases[7][To.y] = NULL;
          }
        }
      }
    }
    return CapturedPiece;
  }
  return NULL;
}

void ChessBoard::MoveBackPiece(const Position From, const Position To, ChessPiece* CapturedPiece, bool EnPassant)
{
  if (IsPositionValid(From) && IsPositionValid(To))
  {
    /* Move back the piece */
    if (Cases[From.x][From.y] != NULL)
      delete Cases[From.x][From.y];
    Cases[From.x][From.y] = Cases[To.x][To.y];
    Cases[To.x][To.y] = NULL;
    if (CapturedPiece != NULL)
    {
      if (EnPassant && Cases[From.x][From.y]->Type == Pawn && abs(To.y-From.y) == abs(To.x-From.x))
        Cases[To.x][From.y] = CapturedPiece;
      else if (!EnPassant)
        Cases[To.x][To.y] = CapturedPiece;
    }
    /* Update the piece's information */
    if (Cases[From.x][From.y] != NULL)
    {
      Cases[From.x][From.y]->TimesMoved--;
      /* Update the position of the king */
      if (Cases[From.x][From.y]->Type == King)
      {
        if (Cases[From.x][From.y]->Color == White)
          WhiteKingPos = From;
        else
          BlackKingPos = From;
        /* Move back the rook if the king castled */
        if (abs(To.x-From.x) == 2)
        {
          if (From.x == 2)
          {
            if (Cases[0][From.y] != NULL)
              delete Cases[0][From.y];
            Cases[0][From.y] = Cases[From.x+1][From.y];
            Cases[From.x+1][From.y] = NULL;
          }
          else if (From.x == 6)
          {
            if (Cases[7][From.y] != NULL)
              delete Cases[7][From.y];
            Cases[7][From.y] = Cases[From.x-1][From.y];
            Cases[From.x-1][From.y] = NULL;
          }
        }
      }
    }
  }
}

ChessPiece* ChessBoard::RemovePiece(const Position Pos)
{
  if (IsPositionValid(Pos))
  {
    ChessPiece* Piece = Cases[Pos.x][Pos.y];
    Cases[Pos.x][Pos.y] = NULL;
    if (Piece != NULL && Piece->Type == King)
    {
      if (Piece->Color == White)
      {
        WhiteKingPos.x = -1;
        WhiteKingPos.y = -1;
      }
      else
      {
        BlackKingPos.x = -1;
        BlackKingPos.y = -1;
      }
    }
    return Piece;
  }
  return NULL;
}
