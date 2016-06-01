/*
* ChessBoard.cpp
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
#include "chessboard.h"
#include <algorithm>
#include <limits.h>
#include <time.h>

using namespace std;

unsigned int ChessBoard::HashValues[12][8][8][2] = {};
bool ChessBoard::HashValuesInitialised = false;
const int ChessBoard::IsolatedPawnsPenalty[8] = {4,6,8,10,10,8,6,4};

// Public function -------------------------------------------------------------

ChessBoard::ChessBoard()
{
  /* Initialise board */
  for (int x=0; x < 8; x++)
    for (int y=0; y < 8; y++)
      Cases[x][y] = NULL;

  /* Initialise statistics */
  BlackKingCastled = false;
  BlackKingPos.x = -1;
  BlackKingPos.y = -1;
  BlackHasQueen = false;
  BlackBishopCount = 0;
  BlackPawnCount = 0;
  BlackPieceCount = 0;

  WhiteKingCastled = false;
  WhiteKingPos.x = -1;
  WhiteKingPos.y = -1;
  WhiteHasQueen = false;
  WhiteBishopCount = 0;
  WhitePawnCount = 0;
  WhitePieceCount = 0;

  /* Initialise hashing values */
  if (!HashValuesInitialised)
  {
    srand(time(NULL));
    unsigned int Value1 = 0;
    unsigned int Value2 = 0;
    for (short i = 0; i < 12; i ++)
      for (short x = 0; x < 8; x ++)
        for (short y = 0; y < 8; y ++)
        {
          Value1 += 4095 + rand();
          HashValues[i][x][y][0] = Value1;
          Value2 += 4095 + rand();
          HashValues[i][x][y][1] = Value2;
        }
    HashValuesInitialised = true;
  }

  /* Initialise hash keys */
  HashKey1 = 0;
  HashKey2 = 0;
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
    /* Add the piece to the board */
    if (Cases[Pos.x][Pos.y] != NULL)
      delete Cases[Pos.x][Pos.y];
    Cases[Pos.x][Pos.y] = Piece;

    /* Update statistics */
    if (Piece->Type == King)
    {
      if (Piece->Color == Black)
      {
        BlackKingCastled = false;
        BlackKingPos = Pos;
      }
      else
      {
        WhiteKingCastled = false;
        WhiteKingPos = Pos;
      }
    }
    else if (Piece->Type == Queen)
    {
      if (Piece->Color == Black)
        BlackHasQueen = true;
      else
        WhiteHasQueen = true;
    }
    else if (Piece->Type == Bishop)
    {
      if (Piece->Color == Black)
        BlackBishopCount ++;
      else
        WhiteBishopCount ++;
    }
    else if (Piece->Type == Pawn)
    {
      if (Piece->Color == Black)
        BlackPawnCount ++;
      else
        WhitePawnCount ++;
    }
    if (Piece->Color == Black)
      BlackPieceCount ++;
    else
      WhitePieceCount ++;
    /* Update hash keys */
    HashKey1 ^= HashValues[Piece->Type-Pawn + (Piece->Color == White ? 6 : 0)][Pos.x][Pos.y][0];
    HashKey2 ^= HashValues[Piece->Type-Pawn + (Piece->Color == White ? 6 : 0)][Pos.x][Pos.y][1];
    return true;
  }
  return false;
}

void ChessBoard::Copy(const ChessBoard* Board)
{
  if (Board != NULL)
  {
    /* Copy chess board */
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

    /* Copy statistics */
    BlackKingCastled = Board->WhiteKingCastled;
    BlackKingPos.x = Board->BlackKingPos.x;
    BlackKingPos.y = Board->BlackKingPos.y;
    BlackHasQueen = Board->BlackHasQueen;
    BlackBishopCount = Board->BlackBishopCount;
    BlackPawnCount = Board->BlackPawnCount;
    BlackPieceCount = Board->BlackPieceCount;

    WhiteKingCastled = Board->WhiteKingCastled;
    WhiteKingPos.x = Board->WhiteKingPos.x;
    WhiteKingPos.y = Board->WhiteKingPos.y;
    WhiteHasQueen = Board->WhiteHasQueen;
    WhiteBishopCount = Board->WhiteBishopCount;
    WhitePawnCount = Board->WhitePawnCount;
    WhitePieceCount = Board->WhitePieceCount;

    Position* Ptr = Board->MovedPieces.GetFirst();
    while (Ptr != NULL)
    {
      Position* Pos = new Position;
      Pos->x = Ptr->x;
      Pos->y = Ptr->y;
      MovedPieces.Add(Pos);
      Ptr = Board->MovedPieces.GetNext();
    }

    /* Copy hash keys */
    HashKey1 = Board->HashKey1;
    HashKey2 = Board->HashKey2;
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

  /* Reset statistics */
  BlackKingCastled = false;
  BlackKingPos.x = -1;
  BlackKingPos.y = -1;
  BlackHasQueen = false;
  BlackBishopCount = 0;
  BlackPawnCount = 0;
  BlackPieceCount = 0;

  WhiteKingCastled = false;
  WhiteKingPos.x = -1;
  WhiteKingPos.y = -1;
  WhiteHasQueen = false;
  WhiteBishopCount = 0;
  WhitePawnCount = 0;
  WhitePieceCount = 0;

  while (MovedPieces.Size() > 0)
    delete MovedPieces.Remove();

  /* Reset hash keys */
  HashKey1 = 0;
  HashKey2 = 0;
}

int ChessBoard::Evaluate(const ChessPieceColor Player)
{
  int Result = 0;
  /* Material value */
  Position Pos;
  for (Pos.x = 0; Pos.x < 8; Pos.x++)
    for (Pos.y = 0; Pos.y < 8; Pos.y++)
      if (Cases[Pos.x][Pos.y] != NULL)
        Result += Evaluate(Pos, Player);
  /* Value a of bishop pair (from 10 to 90 based on pawns on the board) */
  if (BlackBishopCount == 2)
    Result += ((18-BlackPawnCount-WhitePawnCount)/10)*(Player == Black ? 50 : -50);
  if (WhiteBishopCount == 2)
    Result += ((18-BlackPawnCount-WhitePawnCount)/10)*(Player == White ? 50 : -50);
  /* Value of castling */
  //if (BlackKingCastled)
  //  Result += (Player == Black ? 150 : -150);
  //if (WhiteKingCastled)
  //  Result += (Player == White ? 150 : -150);
  /* Value of being able to castle */
  if ((IsCastlingAvailable(Black, false) || IsCastlingAvailable(Black, true)) && !BlackKingCastled)
    Result += (Player == Black ? 50 : -50);
  if ((IsCastlingAvailable(White, false) || IsCastlingAvailable(White, true)) && !WhiteKingCastled)
    Result += (Player == White ? 50 : -50);
  /* Value of a checkmate */
  if (IsKingCheck(Black) && IsKingMate(Black))
    Result += (Player == Black ? -1000000 : 1000000);
  if (IsKingCheck(White) && IsKingMate(White))
    Result += (Player == White ? -1000000 : 1000000);
  return Result;
}

int ChessBoard::Evaluate(const Position Pos, const ChessPieceColor Player)
{
  if (IsPositionValid(Pos) && Cases[Pos.x][Pos.y] != NULL)
  {
    int Result = 0;
    ChessPiece* Piece = Cases[Pos.x][Pos.y];
    /* Piece value */
    Result += PieceValue(Piece->Color, Piece->Type);
    /* Pawn bonus/penalty specific to it's position */
    if (Piece->Type == Pawn)
      Result += max((Piece->Color == Black ? 4-Pos.y : Pos.y-3),0)*10 + (IsPawnPassed(Pos) ? 20 : 0) - (IsPawnIsolated(Pos) ? IsolatedPawnsPenalty[(Piece->Color == White ? Pos.y : 7-Pos.y)] : 0) - (IsPawnDoubled(Pos) ? 10 : 0) - (IsPawnBlocked(Pos) ? 20 : 0);
    /* Piece mobility */
    unsigned short Mobility = 0;
    switch (Piece->Type)
    {
      case Pawn:
      {
        Position To = Position(Pos.x-1,(Piece->Color == White ? Pos.y+1 : Pos.y-1));
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x,(Piece->Color == White ? Pos.y+1 : Pos.y-1));
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x+1,(Piece->Color == White ? Pos.y+1 : Pos.y-1));
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x,(Piece->Color == White ? Pos.y+2 : Pos.y-2));
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        break;
      }
      case Rook:
      {
        Position To = Position(Pos.x-1, Pos.y);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x+1, Pos.y);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x, Pos.y-1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x, Pos.y+1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        break;
      }
      case Knight:
      {
        Position To = Position(Pos.x-2, Pos.y-1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x-2, Pos.y+1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x+2, Pos.y-1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x+2, Pos.y+1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x-1, Pos.y-2);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x+1, Pos.y-2);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x-1, Pos.y+2);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x+1, Pos.y+2);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        break;
      }
      case Bishop:
      {
        Position To = Position(Pos.x-1, Pos.y-1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x-1, Pos.y+1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x+1, Pos.y-1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x+1, Pos.y+1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        break;
      }
      case Queen:
      {
        Position To = Position(Pos.x-1, Pos.y);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x+1, Pos.y);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x, Pos.y-1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x, Pos.y+1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x-1, Pos.y-1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x-1, Pos.y+1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x+1, Pos.y-1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x+1, Pos.y+1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        break;
      }
      case King:
      {
        Position To = Position(Pos.x-1, Pos.y);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x+1, Pos.y);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x, Pos.y-1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x, Pos.y+1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x-1, Pos.y-1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x-1, Pos.y+1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x+1, Pos.y-1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x+1, Pos.y+1);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x+2, Pos.y);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        To = Position(Pos.x-2, Pos.y);
        Mobility += (IsPieceMovementValid(Pos, To) && IsMovePossible(Pos, To));
        break;
      }
      default:
        break;
    }
    Result += Mobility*10;
    return (Piece->Color == Player ? Result : -Result);
  }
  return 0;
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

int ChessBoard::PieceValue(const ChessPieceColor Color, const ChessPieceType Type)
{
  /* Material value (based on Larry Kaufman) */
  switch (Type)
  {
    case Pawn:
      /* Value of a pawn */
      return 100;
    case Knight:
      /* Value of a knight (adjusted based on number of pieces and with bonus per extra pawns) */
      return 325 - (32-BlackPieceCount-WhitePieceCount)*2 + ((Color == Black ? BlackPawnCount : WhitePawnCount)-5)*5;
    case Bishop:
      /* Value of a bishop (adjusted based on number of pieces on the board) */
      return 325 - (32-BlackPieceCount-WhitePieceCount)*2;
    case Rook:
      /* Value of a rook (with bonus if no queen and penalty per extra pawns)*/
      return 500 + ((Color == Black && !BlackHasQueen) || (Color == White && !WhiteHasQueen) ? 100 : 0) - ((Color == Black ? BlackPawnCount : WhitePawnCount)-5)*10;
    case Queen:
      /* Value of a queen */
      return 975;
    default:
      return 0;
  }
}

__int64 ChessBoard::Hash()
{
  return HashKey1 | (HashKey2 << 32);
}

bool ChessBoard::IsCastlingAvailable(const ChessPieceColor Color, const bool Small)
{
  Position Pos = (Color == White ? WhiteKingPos : BlackKingPos);
  ChessPiece* King = Cases[Pos.x][Pos.y];
  if (King != NULL && King->TimesMoved == 0)
  {
    ChessPiece* Piece = Cases[(Small ? 7 : 0)][Pos.y];
    return (Piece != NULL && Piece->Type == Rook && Piece->Color == Color && Piece->TimesMoved == 0);
  }
  return false;
}

bool ChessBoard::IsCastlingPossible(const ChessPieceColor Color, const bool Small)
{
  if (!IsKingCheck(Color) && IsCastlingAvailable(Color, Small))
  {
    Position Pos = (Color == White ? WhiteKingPos : BlackKingPos);
    if (Small)
    {
      Pos.x ++;
      bool Value = (Cases[Pos.x][Pos.y] == NULL && !IsPieceAttacked(Pos, Color));
      Pos.x ++;
      Value = Value && (Cases[Pos.x][Pos.y] == NULL && !IsPieceAttacked(Pos, Color));
      return Value;
    }
    else
    {
      Pos.x --;
      bool Value = (Cases[Pos.x][Pos.y] == NULL && !IsPieceAttacked(Pos, Color));
      Pos.x --;
      Value = Value && (Cases[Pos.x][Pos.y] == NULL && !IsPieceAttacked(Pos, Color));
      Pos.x --;
      Value = Value && (Cases[Pos.x][Pos.y] == NULL && !IsPieceAttacked(Pos, Color));
      return Value;
    }
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
    for (int i = max(0, Pos.x-1); i <= min(7, Pos.x+1); i+=2) /* Skip i == x */
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
    ChessPiece* Piece = Cases[From.x][From.y];
    ChessPiece* CapturedPiece = Cases[To.x][To.y];
    Cases[To.x][To.y] = Cases[From.x][From.y];
    Cases[From.x][From.y] = NULL;
    if (CapturedPiece == NULL && Piece->Type == Pawn && abs(To.y-From.y) == abs(To.x-From.x))
    {
      CapturedPiece = RemovePiece(Position(To.x,From.y));
      EnPassant = true;
    }
    else
      EnPassant = false;
    /* Update statistics */
    Piece->TimesMoved++;
    if (Piece->Type == King)
    {
      if (Piece->Color == Black)
        BlackKingPos = To;
      else
        WhiteKingPos = To;

      /* Move the rook if the king castled */
      if (abs(To.x-From.x) == 2)
      {
        if (Piece->Color == White)
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
    if (CapturedPiece != NULL)
    {
      if (CapturedPiece->Type == Queen)
      {
        if (CapturedPiece->Color == Black)
          BlackHasQueen = false;
        else
          WhiteHasQueen = false;
      }
      else if (CapturedPiece->Type == Bishop)
      {
        if (CapturedPiece->Color == Black)
          BlackBishopCount --;
        else
          WhiteBishopCount --;
      }
      else if (CapturedPiece->Type == Pawn)
      {
        if (CapturedPiece->Color == Black)
          BlackPawnCount --;
        else
          WhitePawnCount --;
      }
      if (CapturedPiece->Color == Black)
        BlackPieceCount --;
      else
        WhitePieceCount --;
    }
    Position* Pos = new Position;
    Pos->x = To.x;
    Pos->y = To.y;
    MovedPieces.Add(Pos);
    /* Update hash keys */
    HashKey1 ^= HashValues[Piece->Type-Pawn + (Piece->Color == White ? 6 : 0)][From.x][From.y][0];
    HashKey2 ^= HashValues[Piece->Type-Pawn + (Piece->Color == White ? 6 : 0)][From.x][From.y][1];
    if (CapturedPiece != NULL)
    {
      HashKey1 ^= HashValues[CapturedPiece->Type-Pawn + (CapturedPiece->Color == White ? 6 : 0)][To.x][EnPassant ? From.y : To.y][0];
      HashKey2 ^= HashValues[CapturedPiece->Type-Pawn + (CapturedPiece->Color == White ? 6 : 0)][To.x][EnPassant ? From.y : To.y][1];
    }
    HashKey1 ^= HashValues[Piece->Type-Pawn + (Piece->Color == White ? 6 : 0)][To.x][To.y][0];
    HashKey2 ^= HashValues[Piece->Type-Pawn + (Piece->Color == White ? 6 : 0)][To.x][To.y][1];
    return CapturedPiece;
  }
  return NULL;
}

void ChessBoard::MoveBackPiece(const Position From, const Position To, ChessPiece* CapturedPiece, bool EnPassant)
{
  if (IsPositionValid(From) && IsPositionValid(To) && Cases[To.x][To.y] != NULL && Cases[From.x][From.y] == NULL)
  {
    /* Move back the piece */
    ChessPiece* Piece = Cases[To.x][To.y];
    Cases[From.x][From.y] = Cases[To.x][To.y];
    Cases[To.x][To.y] = NULL;
    if (CapturedPiece != NULL)
    {
      if (EnPassant && Piece->Type == Pawn && abs(To.y-From.y) == abs(To.x-From.x))
        Cases[To.x][From.y] = CapturedPiece;
      else
      {
        Cases[To.x][To.y] = CapturedPiece;
        EnPassant = false;
      }
    }
    /* Update statistics */
    Piece->TimesMoved--;
    if (Piece->Type == King)
    {
      if (Piece->Color == White)
        WhiteKingPos = From;
      else
        BlackKingPos = From;

      /* Move back the rook if the king castled */
      if (abs(To.x-From.x) == 2)
      {
        if (Piece->Color == White)
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
    if (CapturedPiece != NULL)
    {
      if (CapturedPiece->Type == Queen)
      {
        if (CapturedPiece->Color == Black)
          BlackHasQueen = true;
        else
          WhiteHasQueen = true;
      }
      else if (CapturedPiece->Type == Bishop)
      {
        if (CapturedPiece->Color == Black)
          BlackBishopCount ++;
        else
          WhiteBishopCount ++;
      }
      else if (CapturedPiece->Type == Pawn)
      {
        if (CapturedPiece->Color == Black)
          BlackPawnCount ++;
        else
          WhitePawnCount ++;
      }
      if (CapturedPiece->Color == Black)
        BlackPieceCount ++;
      else
        WhitePieceCount ++;
    }
    if (MovedPieces.Size() > 0)
      delete MovedPieces.Remove(MovedPieces.Size()-1);
    /* Update hash keys */
    HashKey1 ^= HashValues[Piece->Type-Pawn + (Piece->Color == White ? 6 : 0)][To.x][To.y][0];
    HashKey2 ^= HashValues[Piece->Type-Pawn + (Piece->Color == White ? 6 : 0)][To.x][To.y][1];
    if (CapturedPiece != NULL)
    {
      HashKey1 ^= HashValues[CapturedPiece->Type-Pawn + (CapturedPiece->Color == White ? 6 : 0)][To.x][EnPassant ? From.y : To.y][0];
      HashKey2 ^= HashValues[CapturedPiece->Type-Pawn + (CapturedPiece->Color == White ? 6 : 0)][To.x][EnPassant ? From.y : To.y][1];
    }
    HashKey1 ^= HashValues[Piece->Type-Pawn + (Piece->Color == White ? 6 : 0)][From.x][From.y][0];
    HashKey2 ^= HashValues[Piece->Type-Pawn + (Piece->Color == White ? 6 : 0)][From.x][From.y][1];
  }
}

ChessPiece* ChessBoard::RemovePiece(const Position Pos)
{
  if (IsPositionValid(Pos) && Cases[Pos.x][Pos.y] != NULL)
  {
    ChessPiece* Piece = Cases[Pos.x][Pos.y];
    Cases[Pos.x][Pos.y] = NULL;
    /* Update statistics */
    if (Piece->Type == King)
    {
      if (Piece->Color == Black)
      {
        BlackKingCastled = false;
        BlackKingPos.x = -1;
        BlackKingPos.y = -1;
      }
      else
      {
        WhiteKingCastled = false;
        WhiteKingPos.x = -1;
        WhiteKingPos.y = -1;
      }
    }
    else if (Piece->Type == Queen)
    {
      if (Piece->Color == Black)
        BlackHasQueen = false;
      else
        WhiteHasQueen = false;
    }
    else if (Piece->Type == Bishop)
    {
      if (Piece->Color == Black)
        BlackBishopCount --;
      else
        WhiteBishopCount --;
    }
    else if (Piece->Type == Pawn)
    {
      if (Piece->Color == Black)
        BlackPawnCount --;
      else
        WhitePawnCount --;
    }
    if (Piece->Color == Black)
      BlackPieceCount --;
    else
      WhitePieceCount --;
    /* Update hash keys */
    HashKey1 ^= HashValues[Piece->Type-Pawn + (Piece->Color == White ? 6 : 0)][Pos.x][Pos.y][0];
    HashKey2 ^= HashValues[Piece->Type-Pawn + (Piece->Color == White ? 6 : 0)][Pos.x][Pos.y][1];
    return Piece;
  }
  return NULL;
}
