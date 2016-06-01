/*
* ChessGame.h - Class representing a game of chess.
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
#ifndef CHESSGAME_H_
#define CHESSGAME_H_

#include <cstrutils.h>
#include <fstream>
#include <linkedlist.h>
#include <string>
#include "chessboard.h"

using namespace std;

enum {Invalid=1, Captured=2, CapturedEnPassant=4, Promoted=8, Checked=16, Mated=32};

enum GameMode {GameTime, MoveTime, FreeTime};

enum GameState {Undefined, Created, Started, Paused, EndedInDraw, EndedInForfeit, EndedInStaleMate, EndedInCheckMate, EndedInTimeout, Postponed};

class ChessMove
{
public:
  ChessPieceType CapturedPiece;
  ChessPieceType MovedPiece;
  ChessPieceType PromotedTo;
  Position From;
  Position To;
  int Result;

  char* ToAlgebraicNotation()
  {
    char* Value = new char[16];
    if (MovedPiece == King && abs(To.x-From.x) == 2)
    {
      if (To.x == 2)
        strcpy(Value, "O-O-O");
      else if (To.x == 6)
        strcpy(Value, "O-O");
    }
    else
    {
      char* Str = ChessPiece::GetNotation(MovedPiece);
      strcpy(Value, Str);
      delete[] Str;
      Str = From.ToAlgebraicNotation();
      strcat(Value, Str);
      delete[] Str;
      if (Result & CapturedEnPassant)
        strcat(Value, ":");
      else if (Result & Captured)
        strcat(Value, "x");
      Str = To.ToAlgebraicNotation();
      strcat(Value, Str);
      delete[] Str;
      if (Result & Promoted)
      {
        strcat(Value, "=");
        Str = ChessPiece::GetNotation(PromotedTo);
        strcpy(Value, Str);
        delete[] Str;
      }
      if (Result & Checked && Result & Mated)
        strcat(Value, "#");
      else if (Result & Checked)
        strcat(Value, "+");
    }
    return Value;
  }
};

struct ChessPlayer
{
  string Name;
  unsigned int Time;
};

struct ChessGameImage
{
  long Pieces[32];
  GameMode Mode;
  GameState State;
  ChessPieceColor ActivePlayer;
  unsigned int TimePerMove;
  unsigned long WhitePlayerTime;
  unsigned long BlackPlayerTime;
  // TODO Add move list
};

typedef LinkedList<ChessMove> MoveList;

class ChessGame
{
public:
  ChessBoard Board;
  MoveList Moves;
  ChessPlayer WhitePlayer;
  ChessPlayer BlackPlayer;

  ChessPieceColor ActivePlayer;
  GameMode Mode;
  GameState State;
  unsigned int TimePerMove;

  ChessGame();
  ~ChessGame();

  unsigned int CountBackMoves(Position Pos);
  bool LoadFromFile(const char* FileName);
  void LoadFromImage(const ChessGameImage* Image);
  bool SaveToFile(const char* FileName);
  void SaveToImage(ChessGameImage* Image);

  void Reset();
};

#endif
