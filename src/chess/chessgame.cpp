/*
* ChessGame.cpp
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
#include "chessgame.h"

#define MAKELONG(lo,hi) ((unsigned long)(((unsigned short)(lo))|(((unsigned long)((unsigned short)(hi)))<<16)))
#define MAKEWORD(lo,hi) ((unsigned short)(((unsigned char)(lo))|(((unsigned short)((unsigned char)(hi)))<<8)))
#define MAKEBYTE(lo,hi) ((unsigned char)(((unsigned char)(lo))|(((unsigned char)((unsigned char)(hi)))<<4)))
#define LOWORD(l) ((unsigned short)((unsigned long)(l)))
#define HIWORD(l) ((unsigned short)(((unsigned long)(l)>>16)&0xFFFF))
#define LOBYTE(w) ((unsigned char)(w))
#define HIBYTE(w) ((unsigned char)(((unsigned short)(w)>>8)&0xFF))
#define LOBITS(b) ((unsigned char)(((unsigned char)(b))&0x0F))
#define HIBITS(b) ((unsigned char)(((unsigned char)(b)>>4)&0x0F))

// Public functions ------------------------------------------------------------

ChessGame::ChessGame()
{
  WhitePlayer.Time = 0;
  BlackPlayer.Time = 0;

  ActivePlayer = White;
  Mode = FreeTime;
  State = Undefined;
  TimePerMove = 0;
}

ChessGame::~ChessGame()
{
  Board.Clear();
  while (Moves.Size() > 0)
    delete Moves.Remove();
}

unsigned int ChessGame::CountBackMoves(Position Pos)
{
  unsigned int Result = 0;
  ChessMove* Move = Moves.GetLast();
  while (Move != NULL)
  {
    if (Move->To == Pos)
    {
      Pos = Move->From;
      Result++;
    }
    Move = Moves.GetPrevious();
  }
  return Result;
}

bool ChessGame::LoadFromFile(const char* FileName)
{
  // TODO Load game information from PGN file.
  return false;
}

void ChessGame::LoadFromImage(const ChessGameImage* Image)
{
  if (Image != NULL)
  {
    /* Clear the current game data */
    Board.Clear();
    while (Moves.Size() > 0)
      delete Moves.Remove();
    /* Load information about the game from the image */
    Mode = Image->Mode;
    State = Image->State;
    ActivePlayer = Image->ActivePlayer;
    TimePerMove = Image->TimePerMove;
    WhitePlayer.Time = Image->WhitePlayerTime;
    BlackPlayer.Time = Image->BlackPlayerTime;
    /* Load information about the chessboard from the image */
    for (short i=0; i < 32; i++)
    {
      unsigned short HiWord = HIWORD(Image->Pieces[i]);
      unsigned char HiByte = HIBYTE(HiWord);
      unsigned char LoByte = LOBYTE(HiWord);
      ChessPiece* Piece = new ChessPiece((ChessPieceType)LOBITS(LoByte), (ChessPieceColor)HIBITS(LoByte), LOWORD(Image->Pieces[i]));
      if (HiByte != 255)
        Board.AddPiece(Position(HIBITS(HiByte), LOBITS(HiByte)), Piece);
    }
  }
}

bool ChessGame::SaveToFile(const char* FileName)
{
  bool Result = false;
  ofstream File(FileName, ios::out);
  if (File.is_open())
  {
    int Round = 0;
    ChessMove* Move = Moves.GetFirst();
    while (Move != NULL)
    {
      Round ++;
      /* Round number */
      char* Str = inttostr(Round);
      File.write(Str, strlen(Str));
      delete[] Str;
      /* Separator */
      File.write(". ", 2);
      /* White move */
      Str = Move->ToAlgebraicNotation();
      File.write(Str, strlen(Str));
      delete[] Str;
      /* Separator */
      File.write(" ", 1);
      /* Next move */
      Move = Moves.GetNext();
      if (Move != NULL)
      {
        /* Black move */
        Str = Move->ToAlgebraicNotation();
        File.write(Str, strlen(Str));
        delete[] Str;
        /* Separator */
        File.write(" ", 1);
        /* Next move */
        Move = Moves.GetNext();
      }
    }
    /* Result */
    if (State == EndedInForfeit || State == EndedInCheckMate || State == EndedInTimeout)
    {
      if (ActivePlayer == White)
        File.write("1-0", 3);
      else
        File.write("0-1", 3);
    }
    else if (State == EndedInDraw || State == EndedInStaleMate)
      File.write("1/2-1/2", 7);
    File.write("\n", 1);
    File.close();
    Result = true;
  }
  return Result;
}

void ChessGame::SaveToImage(ChessGameImage* Image)
{
  if (Image != NULL)
  {
    /* Save information about the game */
    Image->Mode = Mode;
    Image->State = State;
    Image->ActivePlayer = ActivePlayer;
    Image->TimePerMove = TimePerMove;
    Image->WhitePlayerTime = WhitePlayer.Time;
    Image->BlackPlayerTime = BlackPlayer.Time;
    /* Save information about the chess board */
    int Index = 0;
    Position Pos;
    for (Pos.x=0; Pos.x < 8; Pos.x++)
      for (Pos.y=0; Pos.y < 8; Pos.y++)
        if (Board.GetPiece(Pos) != NULL)
        {
          Image->Pieces[Index] = MAKELONG(Board.GetPiece(Pos)->TimesMoved,MAKEWORD(MAKEBYTE(Board.GetPiece(Pos)->Type,Board.GetPiece(Pos)->Color),MAKEBYTE(Pos.y,Pos.x)));
          Index ++;
        }
  }
}

void ChessGame::Reset()
{
  Board.Clear();
  while (Moves.Size() > 0)
    delete Moves.Remove();
  Position Pos;
  Pos.y = 7;
  Pos.x = 0;
  Board.AddPiece(Pos, new ChessPiece(Rook, Black));
  Pos.x = 1;
  Board.AddPiece(Pos, new ChessPiece(Knight, Black));
  Pos.x = 2;
  Board.AddPiece(Pos, new ChessPiece(Bishop, Black));
  Pos.x = 3;
  Board.AddPiece(Pos, new ChessPiece(Queen, Black));
  Pos.x = 4;
  Board.AddPiece(Pos, new ChessPiece(King, Black));
  Pos.x = 5;
  Board.AddPiece(Pos, new ChessPiece(Bishop, Black));
  Pos.x = 6;
  Board.AddPiece(Pos, new ChessPiece(Knight, Black));
  Pos.x = 7;
  Board.AddPiece(Pos, new ChessPiece(Rook, Black));
  Pos.y = 6;
  for (Pos.x=0; Pos.x < 8; Pos.x++)
    Board.AddPiece(Pos, new ChessPiece(Pawn, Black));
  Pos.y = 0;
  Pos.x = 0;
  Board.AddPiece(Pos, new ChessPiece(Rook, White));
  Pos.x = 1;
  Board.AddPiece(Pos, new ChessPiece(Knight, White));
  Pos.x = 2;
  Board.AddPiece(Pos, new ChessPiece(Bishop, White));
  Pos.x = 3;
  Board.AddPiece(Pos, new ChessPiece(Queen, White));
  Pos.x = 4;
  Board.AddPiece(Pos, new ChessPiece(King, White));
  Pos.x = 5;
  Board.AddPiece(Pos, new ChessPiece(Bishop, White));
  Pos.x = 6;
  Board.AddPiece(Pos, new ChessPiece(Knight, White));
  Pos.x = 7;
  Board.AddPiece(Pos, new ChessPiece(Rook, White));
  Pos.y = 1;
  for (Pos.x=0; Pos.x < 8; Pos.x++)
    Board.AddPiece(Pos, new ChessPiece(Pawn, White));
  ActivePlayer = White;
  State = Created;
}
