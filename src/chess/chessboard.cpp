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

static const int IsolatedPawnsPenalty[8] = {4,6,8,10,10,8,6,4};

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
  WhiteKingCastled = false;
  BlackKingCastled = false;
}

ChessBoard::~ChessBoard()
{
  for (int i=0; i < 8; i++)
    for (int j=0; j < 8; j++)
      if (Cases[i][j] != NULL)
        delete Cases[i][j];
  while (MovedPieces.Size() > 0)
    delete MovedPieces.Remove();
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

void ChessBoard::Copy(const ChessBoard* Board)
{
  if (Board != NULL)
  {
    for (int i=0; i < 8; i++)
      for (int j=0; j < 8; j++)
      {
        if (Cases[i][j] != NULL)
          delete Cases[i][j];
        if (Board->Cases[i][j] != NULL)
          Cases[i][j] = new ChessPiece(Board->Cases[i][j]->Type,Board->Cases[i][j]->Color,Board->Cases[i][j]->TimesMoved);
        else
          Cases[i][j]= NULL;
      }
    WhiteKingPos.x = Board->WhiteKingPos.x;
    WhiteKingPos.y = Board->WhiteKingPos.y;
    BlackKingPos.x = Board->BlackKingPos.x;
    BlackKingPos.y = Board->BlackKingPos.y;
    WhiteKingCastled = Board->WhiteKingCastled;
    BlackKingCastled = Board->WhiteKingCastled;
    Position* Ptr = Board->MovedPieces.GetFirst();
    while (Ptr != NULL)
    {
      Position* Pos = new Position;
      Pos->x = Ptr->x;
      Pos->y = Ptr->y;
      MovedPieces.Add(Pos);
      Ptr = Board->MovedPieces.GetNext();
    }
  }
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
  WhiteKingCastled = false;
  BlackKingCastled = false;
  while (MovedPieces.Size() > 0)
    delete MovedPieces.Remove();
}

Position ChessBoard::FindStartingPos(const Position From, const Position To, const ChessPieceType Type, const ChessPieceColor Color)
{
  Position P;
  if (IsPositionValid(To))
  {
    for (P.x=0; P.x < 8; P.x++)
      if (From.x < 0 || From.x > 7 || From.x == P.x)
        for (P.y=0; P.y < 8; P.y++)
          if (From.y < 0 || From.y > 7 || From.y == P.y)
          {
            ChessPiece* Piece = GetPiece(P);
            if (Piece != NULL && Piece->Type == Type && Piece->Color == Color && IsPieceMovementValid(P,To) && IsMovePossible(P,To))
              return P;
          }
  }
  P.x = -1;
  P.y = -1;
  return P;
}

ChessPiece* ChessBoard::GetPiece(const Position Pos)
{
  if (IsPositionValid(Pos))
    return Cases[Pos.x][Pos.y];
  return NULL;
}

LinkedList<PossibleChessMove>* ChessBoard::GetPossibleMoves(const ChessPieceColor Color)
{
  LinkedList<PossibleChessMove>* Result = new LinkedList<PossibleChessMove>;
  Position From;
  Position To;
  for (From.x = 0; From.x < 8; From.x++)
    for (From.y = 0; From.y < 8; From.y++)
    {
      if (Cases[From.x][From.y] != NULL && Cases[From.x][From.y]->Color == Color)
        for (To.x = 0; To.x < 8; To.x++)
          for (To.y = 0; To.y < 8; To.y++)
            if ((From.x != To.x || From.y != To.y) && IsMoveValid(From, To))
            {
              PossibleChessMove* Move = new PossibleChessMove;
              Move->From = From;
              Move->To = To;
              Move->Value = 0;
              Result->Add(Move);
            }
    }
  return Result;
}

int ChessBoard::PieceMobility(const Position Pos)
{
  unsigned short Result = 0;
  if (IsPositionValid(Pos))
  {
    ChessPiece* Piece = Cases[Pos.x][Pos.y];
    if (Piece != NULL)
    {
      switch (Piece->Type)
      {
        case Pawn:
        {
          Position To = Position(Pos.x-1,(Piece->Color == White ? Pos.y+1 : Pos.y-1));
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x,(Piece->Color == White ? Pos.y+1 : Pos.y-1));
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x+1,(Piece->Color == White ? Pos.y+1 : Pos.y-1));
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x,(Piece->Color == White ? Pos.y+2 : Pos.y-2));
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          break;
        }
        case Rook:
        {
          Position To = Position(Pos.x-1, Pos.y);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x+1, Pos.y);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x, Pos.y-1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x, Pos.y+1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          break;
        }
        case Knight:
        {
          Position To = Position(Pos.x-2, Pos.y-1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x-2, Pos.y+1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x+2, Pos.y-1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x+2, Pos.y+1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x-1, Pos.y-2);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x+1, Pos.y-2);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x-1, Pos.y+2);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x+1, Pos.y+2);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          break;
        }
        case Bishop:
        {
          Position To = Position(Pos.x-1, Pos.y-1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x-1, Pos.y+1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x+1, Pos.y-1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x+1, Pos.y+1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          break;
        }
        case Queen:
        {
          Position To = Position(Pos.x-1, Pos.y);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x+1, Pos.y);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x, Pos.y-1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x, Pos.y+1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x-1, Pos.y-1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x-1, Pos.y+1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x+1, Pos.y-1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x+1, Pos.y+1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          break;
        }
        case King:
        {
          Position To = Position(Pos.x-1, Pos.y);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x+1, Pos.y);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x, Pos.y-1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x, Pos.y+1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x-1, Pos.y-1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x-1, Pos.y+1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x+1, Pos.y-1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x+1, Pos.y+1);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x+2, Pos.y);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          To = Position(Pos.x-2, Pos.y);
          Result += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
          break;
        }
        default:
          break;
      }
    }
  }
  return Result;
}

void ChessBoard::Hash(HashTableKey Key)
{
  for (int j=0; j < 8; j++)
    for (int i=0; i < 8; i++)
    {
      Key[j] = (Key[j] << 4);
      if (Cases[i][j] != NULL)
        Key[j] |= Cases[i][j]->Hash();
    }
}

bool ChessBoard::IsCastlingAvailable(const ChessPieceColor Color, const bool Small)
{
  Position Pos = (Color == White ? WhiteKingPos : BlackKingPos);
  if (!(Color == White ? WhiteKingCastled : BlackKingCastled) && Cases[Pos.x][Pos.y] != NULL && Cases[Pos.x][Pos.y]->TimesMoved == 0)
  {
    Pos.x = (Small ? 7 : 0);
    if (Cases[Pos.x][Pos.y] != NULL && Cases[Pos.x][Pos.y]->Type == Rook && Cases[Pos.x][Pos.y]->Color == Color && Cases[Pos.x][Pos.y]->TimesMoved == 0)
      return true;
  }
  return false;
}

bool ChessBoard::IsCastlingPossible(const ChessPieceColor Color, const bool Small)
{
  if (!IsKingCheck(Color) && IsCastlingAvailable(Color, Small))
  {
    Position Pos = (Color == White ? WhiteKingPos : BlackKingPos);
    if (Small && Cases[Pos.x+1][Pos.y] == NULL && Cases[Pos.x+2][Pos.y] == NULL && !IsPieceAttacked(Position(Pos.x+1, Pos.y), Color) && !IsPieceAttacked(Position(Pos.x+2, Pos.y), Color))
      return true;
    else if (!Small && Cases[Pos.x-1][Pos.y] == NULL && Cases[Pos.x-2][Pos.y] == NULL && !IsPieceAttacked(Position(Pos.x-1, Pos.y), Color) && !IsPieceAttacked(Position(Pos.x-2, Pos.y), Color))
      return true;
  }
  return false;
}

bool ChessBoard::IsKingCheck(const ChessPieceColor Color)
{
  return IsPieceAttacked((Color == White ? WhiteKingPos : BlackKingPos), Color);
}

bool ChessBoard::IsKingMate(const ChessPieceColor Color)
{
  Position Pos = (Color == White ? WhiteKingPos : BlackKingPos);
  /* Can the king move */
  if (IsPieceMovable(Pos))
    return false;
  /* Can any other piece make a valid move */
  for (int i=0; i < 8; i++)
    for (int j=0; j < 8; j++)
    {
      Position Pos(i,j);
      if (Cases[i][j] != NULL && Cases[i][j]->Color == Color && IsPieceMovable(Pos))
        return false;
    }
  return true;
}

bool ChessBoard::IsMovePossible(const Position From, const Position To)
{
  if (IsPositionValid(From) && IsPositionValid(To) && Cases[From.x][From.y] != NULL)
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
  if (IsPositionValid(From) && IsPositionValid(To) && Cases[From.x][From.y] != NULL)
  {
    ChessPiece* Piece = Cases[From.x][From.y];
    if (Piece != NULL && IsPieceMovementValid(From, To) && IsMovePossible(From, To))
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
  if (IsPositionValid(Pos) && Cases[Pos.x][Pos.y] != NULL && Cases[Pos.x][Pos.y]->Type == Pawn)
  {
    for (int i = max(0, Pos.x-1); i <= min(7, Pos.x+1); i++)
      if (IsMovePossible(Pos, Position(i, (Cases[Pos.x][Pos.y]->Color == White ? min(7,Pos.y+1) : max(0,Pos.y-1)))))
        return false;
  }
  return true;
}

bool ChessBoard::IsPawnDoubled(const Position Pos)
{
  /* A pawn is doubled if there is another pawn of the same color in the file */
  if (IsPositionValid(Pos) && Cases[Pos.x][Pos.y] != NULL && Cases[Pos.x][Pos.y]->Type == Pawn)
  {
    for (int i = 0; i <= 7; i++)
      if (i != Pos.y && Cases[Pos.x][i] != NULL && Cases[Pos.x][i]->Type == Pawn && Cases[Pos.x][i]->Color == Cases[Pos.x][Pos.y]->Color)
        return true;
  }
  return false;
}

bool ChessBoard::IsPawnIsolated(const Position Pos)
{
  /* A pawn is isolated if there is no pawn of the same color in the adjacent columns */
  if (IsPositionValid(Pos) && Cases[Pos.x][Pos.y] != NULL && Cases[Pos.x][Pos.y]->Type == Pawn)
  {
    for (int i = max(0, Pos.x-1); i <= min(7, Pos.x+1); i+=2)    // Skips i == x
      for (int j = 0; j <= 7; j++)
        if (Cases[i][j] != NULL && Cases[i][j]->Type == Pawn && Cases[i][j]->Color == Cases[Pos.x][Pos.y]->Color)
          return false;
  }
  return true;
}

bool ChessBoard::IsPawnPassed(const Position Pos)
{
  /* A pawn is passed if there is no enemy pawn on his way to the end of the board */
  if (IsPositionValid(Pos) && Cases[Pos.x][Pos.y] != NULL && Cases[Pos.x][Pos.y]->Type == Pawn)
  {
    for (int i = max(0, Pos.x-1); i <= min(7, Pos.x+1); i++)
      if (Cases[Pos.x][Pos.y]->Color == White)
      {
        for (int j = min(7, Pos.y+1); j <= 7; j++)
          if (Cases[i][j] != NULL && Cases[i][j]->Type == Pawn && Cases[i][j]->Color != Cases[Pos.x][Pos.y]->Color)
            return false;
      }
      else
      {
        for (int j = max(0, Pos.y-1); j >= 0; j--)
          if (Cases[i][j] != NULL && Cases[i][j]->Type == Pawn && Cases[i][j]->Color != Cases[Pos.x][Pos.y]->Color)
            return false;
      }
  }
  return true;
}

bool ChessBoard::IsPieceAttacked(const Position Pos, const ChessPieceColor Color)
{
  if (IsPositionValid(Pos))
  {
    Position P;
    for (P.x=0; P.x < 8; P.x++)
      for (P.y=0; P.y < 8; P.y++)
      {
        ChessPiece* Piece = GetPiece(P);
        if (Piece != NULL && Piece->Color != Color && IsPieceMovementValid(P,Pos) && IsMovePossible(P,Pos))
          return true;
      }
  }
  return false;
}

bool ChessBoard::IsPieceAttacking(const Position Pos)
{
  if (IsPositionValid(Pos) && Cases[Pos.x][Pos.y] != NULL)
  {
    for (int i=0; i < 8; i++)
      for (int j=0; j < 8; j++)
        if (Cases[i][j] != NULL && Cases[i][j]->Color != Cases[Pos.x][Pos.y]->Color && IsPieceMovementValid(Pos, Position(i, j)) && IsMovePossible(Pos,Position(i, j)))
          return true;
  }
  return false;
}

bool ChessBoard::IsPieceDefended(const Position Pos, const ChessPieceColor Color)
{
  if (IsPositionValid(Pos))
    for (int i = 0; i < 8; i++)
      for (int j = 0; i < 8; i++)
        if (Cases[i][j] != NULL && Cases[i][j]->Color == Color && IsPieceMovementValid(Position(i,j), Pos))
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

bool ChessBoard::IsPieceDefending(const Position Pos)
{
  if (IsPositionValid(Pos) && Cases[Pos.x][Pos.y] != NULL)
  {
    Position P;
    for (P.x = 0; P.x < 8; P.x++)
      for (P.y = 0; P.y < 8; P.y++)
        if (Cases[P.x][P.y] != NULL && Cases[P.x][P.y]->Color == (Cases[Pos.x][Pos.y]->Color && IsPieceMovementValid(Pos, P)))
        {
          ChessPiece* Piece = RemovePiece(P);
          if (IsMovePossible(Pos, P))
          {
            AddPiece(P, Piece);
            return true;
          }
          AddPiece(P, Piece);
        }
  }
  return false;
}

bool ChessBoard::IsPieceMovable(const Position Pos)
{
  if (IsPositionValid(Pos) && Cases[Pos.x][Pos.y] != NULL)
  {
    Position P;
    for (P.x = 0; P.x < 8; P.x++)
      for (P.y = 0; P.y < 8; P.y++)
        if ((P.x != Pos.x || P.y != Pos.y) && IsMoveValid(Pos,P))
          return true;
  }
  return false;
}

bool ChessBoard::IsPieceMovementValid(const Position From, const Position To)
{
  if (IsPositionValid(From) && IsPositionValid(To))
  {
    ChessPiece* Piece = Cases[From.x][From.y];
    if (Piece != NULL)
    {
      int cx = To.x-From.x;
      int cy = To.y-From.y;
      switch (Piece->Type)
      {
        case Pawn:
        {
          /* Pawns can move one square forward without capturing */
          if (cx == 0 && cy == (Piece->Color == White ? 1 : -1) && Cases[To.x][To.y] == NULL)
            return true;
          /* Pawns can move two square forward from their starting position without capturing */
          if (cx == 0 && cy == (Piece->Color == White ? 2 : -2) && Cases[To.x][To.y] == NULL && From.y == (Piece->Color == White ? 1 : 6))
            return true;
          /* Pawns can move one square forward in diagonal while capturing */
          if (abs(cx) == 1 && cy == (Piece->Color == White ? 1 : -1) && Cases[To.x][To.y] != NULL && Cases[To.x][To.y]->Color != Piece->Color)
            return true;
          /* Pawns can move one square forward in diagonal while capturing en passant */
          if (abs(cx) == 1 && cy == (Piece->Color == White ? 1 : -1) && Cases[To.x][To.y] == NULL && From.y == (Piece->Color == White ? 4 : 3))
          {
            Position* Pos = MovedPieces.GetLast();
            if (Pos != NULL && Pos->x == To.x && Pos->y == From.y && Cases[Pos->x][Pos->y] != NULL && Cases[Pos->x][Pos->y]->Type == Pawn && Cases[Pos->x][Pos->y]->Color != Piece->Color && Cases[Pos->x][Pos->y]->TimesMoved == 1)
              return true;
          }
          break;
        }
        case Rook:
        {
          /* Rooks can move horizontally or vertically */
          if (cx == 0 || cy == 0)
            return true;
          break;
        }
        case Knight:
        {
          /* Knights can move 1 square in a direction and 2 in another */
          if ((abs(cx) == 1 && abs(cy) == 2) || (abs(cx) == 2 && abs(cy) == 1))
            return true;
          break;
        }
        case Bishop:
        {
          /* Bishops can move diagonally */
          if (abs(cx) == abs(cy))
            return true;
          break;
        }
        case Queen:
        {
          /* Queens can move horizontally, vertically and diagonally */
          if (cx == 0 || cy == 0 || abs(cx) == abs(cy))
            return true;
          break;
        }
        case King:
        {
          /* Kings can move one square horizontally, vertically and diagonally */
          if ((cx == 0 && abs(cy) == 1) || (abs(cx) == 1 && cy == 0) || (abs(cx) == 1 && abs(cy) == 1))
            return true;
          /* Kings can move two squares horizontally if they castle */
          else if (cy == 0 && abs(cx) == 2 && ((Piece->Color == White && From.x == 4 && From.y == 0 && (To.x == 2 || To.x == 6)) || (Piece->Color == Black && From.x == 4 && From.y == 7 && (To.x == 2 || To.x == 6))))
            return IsCastlingPossible(Cases[From.x][From.y]->Color,(To.x == 6));
          break;
        }
        default:
          break;
      }
    }
  }
  return false;
}

bool ChessBoard::IsPositionValid(const Position Pos)
{
  return (Pos.x >= 0 && Pos.x < 8 && Pos.y >= 0 && Pos.y < 8);
}

ChessPiece* ChessBoard::MovePiece(const Position From, const Position To, bool& EnPassant)
{
  if (IsPositionValid(From) && IsPositionValid(To) && Cases[From.x][From.y] != NULL)
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
        if (Cases[To.x][To.y]->Color == White)
          WhiteKingCastled = true;
        else
          BlackKingCastled = true;
        if (To.x == 2 && Cases[To.x+1][To.y] == NULL)
        {
          Cases[To.x+1][To.y] = Cases[0][To.y];
          Cases[0][To.y] = NULL;
        }
        else if (To.x == 6 && Cases[To.x-1][To.y] == NULL)
        {
          Cases[To.x-1][To.y] = Cases[7][To.y];
          Cases[7][To.y] = NULL;
        }
      }
    }
    /* Add to the list of moved pieces */
    Position* Pos = new Position;
    Pos->x = To.x;
    Pos->y = To.y;
    MovedPieces.Add(Pos);
    return CapturedPiece;
  }
  return NULL;
}

void ChessBoard::MoveBackPiece(const Position From, const Position To, ChessPiece* CapturedPiece, const bool EnPassant)
{
  if (IsPositionValid(From) && IsPositionValid(To) && Cases[To.x][To.y] != NULL && Cases[From.x][From.y] == NULL)
  {
    /* Move back the piece */
    Cases[From.x][From.y] = Cases[To.x][To.y];
    Cases[To.x][To.y] = NULL;
    if (CapturedPiece != NULL)
    {
      if (!EnPassant)
        Cases[To.x][To.y] = CapturedPiece;
      else if (Cases[From.x][From.y]->Type == Pawn && abs(To.y-From.y) == abs(To.x-From.x))
        Cases[To.x][From.y] = CapturedPiece;
    }
    /* Update the piece's information */
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
        if (Cases[From.x][From.y]->Color == White)
          WhiteKingCastled = false;
        else
          BlackKingCastled = false;
        if (To.x == 2 && Cases[0][To.y] == NULL)
        {
          Cases[0][To.y] = Cases[To.x+1][To.y];
          Cases[To.x+1][To.y] = NULL;
        }
        else if (To.x == 6 && Cases[7][To.y] == NULL)
        {
          Cases[7][To.y] = Cases[To.x-1][To.y];
          Cases[To.x-1][To.y] = NULL;
        }
      }
    }
    /* Delete from the list of moved pieces */
    if (MovedPieces.Size() > 0)
      delete MovedPieces.Remove(MovedPieces.Size()-1);
  }
}

ChessPiece* ChessBoard::RemovePiece(const Position Pos)
{
  if (IsPositionValid(Pos) && Cases[Pos.x][Pos.y] != NULL)
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

int ChessBoard::Value()
{
  bool BlackHasQueen = false;
  int BlackPieceCount = 0;
  int BlackBishopCount = 0;
  int BlackPawnCount = 0;
  bool WhiteHasQueen = false;
  int WhitePieceCount = 0;
  int WhiteBishopCount = 0;
  int WhitePawnCount = 0;
  for (int x = 0; x < 8; x++)
    for (int y = 0; y < 8; y++)
      if (Cases[x][y] != NULL)
      {
        if (Cases[x][y]->Color == Black)
        {
          BlackPieceCount ++;
          if (Cases[x][y]->Type == Queen)
            BlackHasQueen = true;
          else if (Cases[x][y]->Type == Pawn)
            BlackPawnCount ++;
          else if (Cases[x][y]->Type == Bishop)
            BlackBishopCount ++;
        }
        else
        {
          WhitePieceCount ++;
          if (Cases[x][y]->Type == Queen)
            WhiteHasQueen = true;
          else if (Cases[x][y]->Type == Pawn)
            WhitePawnCount ++;
          else if (Cases[x][y]->Type == Bishop)
            WhiteBishopCount ++;
        }
      }
  Position Pos;
  int Result = 0;
  for (Pos.x = 0; Pos.x < 8; Pos.x++)
    for (Pos.y = 0; Pos.y < 8; Pos.y++)
    {
      ChessPiece* Piece = Cases[Pos.x][Pos.y];
      if (Piece != NULL)
      {
        /* Material value (based on Larry Kaufman) */
        int Value = 0;
        switch (Piece->Type)
        {
          case Pawn:
          {
            /* Value of a pawn (with promotion bonus and passed bonus) */
            Value = 100;+max((Piece->Color == Black ? 4-Pos.y : Pos.y-3),0)*10+(IsPawnPassed(Pos) ? 20 : 0);
            /* Other penalties */
            Value -= (IsPawnIsolated(Pos) ? IsolatedPawnsPenalty[(Piece->Color == White ? Pos.y : 7-Pos.y)] : 0)+(IsPawnDoubled(Pos) ? 10 : 0)+(IsPawnBlocked(Pos) ? 20 : 0);
            break;
          }
          case Knight:
          {
            /* Value of a knight (adjusted based on number of pieces and with bonus per extra pawns) */
            Value = 325-(32-BlackPieceCount-WhitePieceCount)*2+((Piece->Color == Black ? BlackPawnCount : WhitePawnCount)-5)*5;
            break;
          }
          case Bishop:
          {
            /* Value of a bishop (adjusted based on number of pieces) */
            Value = 325-(32-BlackPieceCount-WhitePieceCount)*2;
            break;
          }
          case Rook:
          {
            /* Value of a rook (with bonus if no queen and penalty per extra pawns)*/
            Value = 500+((Piece->Color == Black && !BlackHasQueen) || (Piece->Color == White && !WhiteHasQueen) ? 100 : 0)-((Piece->Color == Black ? BlackPawnCount : WhitePawnCount)-5)*10;
            break;
          }
          case Queen:
          {
            /* Value of a queen */
            Value = 975;
            break;
          }
          default:
            break;
        }
        /* Value of mobility */
        Value += PieceMobility(Pos)*10;
        /* Value of attacking (with bonus if defended) */
        Value += (IsPieceAttacking(Pos) ? 10+(IsPieceDefended(Pos, Piece->Color) ? 10 : 0) : 0);
        Result += (Piece->Color == White ? Value : -Value);
      }
    }
  /* Value a of bishop pair (from 10 to 90 based on pawns on the board) */
  if (BlackBishopCount == 2)
    Result += ((18-BlackPawnCount-WhitePawnCount)/10)*-50;
  if (WhiteBishopCount == 2)
    Result += ((18-BlackPawnCount-WhitePawnCount)/10)*50;
  /* Value of castling */
  if (BlackKingCastled || IsCastlingAvailable(Black, false) || IsCastlingAvailable(Black, true))
    Result += -100;
  if (WhiteKingCastled || IsCastlingAvailable(White, false) || IsCastlingAvailable(White, true))
    Result += 100;
  /* Value of a checkmate */
  Result += (IsKingCheck(Black) && IsKingMate(Black) ? 1000000 : 0);
  Result += (IsKingCheck(White) && IsKingMate(White) ? -1000000 : 0);
  return Result;
}
