/*
* ChessGame.h - Class representing a game of chess.
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
#ifndef CHESSGAME_H_
#define CHESSGAME_H_

#include "../system.h"
#include "chessboard.h"
#include "chessevaluator.h"
#include <cstrutils.h>
#include <fstream>
#include <math.h>
#include <observer.h>
#include <string>
#include <winutils.h>

using namespace std;

enum ChessGameMode {FreeTime, MoveTime};

enum ChessGameState {Undefined, Started, Paused, EndedInDraw, EndedInForfeit, EndedInStaleMate, EndedInCheckMate, EndedInTimeout, Postponed};

enum ChessGameEvent {BoardUpdated, PiecePromoted, PlayerUpdated, StateChanged, TurnEnded};

class ChessMove
{
public:
  Position From;
  Position To;
  ChessPieceType Piece;
  ChessPieceType PromotedTo;
  ChessPiece* CapturedPiece;
  bool EnPassant;
  bool Check;
  bool Mate;

  ChessMove()
  {
  }

  static ChessMove ParseAlgebraicString(string Str, const ChessPieceColor ActivePlayer)
  {
    ChessMove Move;
    Move.From.x = -1;
    Move.From.y = -1;
    Move.To.x = -1;
    Move.To.y = -1;
    Move.Piece = Blank;
    Move.PromotedTo = Blank;
    Move.CapturedPiece = NULL;
    Move.EnPassant = false;
    Move.Check = false;
    Move.Mate = false;
    if (Str.compare("O-O-O") == 0)
    {
      Move.Piece = King;
      Move.From.x = 4;
      if (ActivePlayer == White)
        Move.From.y = 0;
      else
        Move.From.y = 7;
      Move.To.x = 2;
      Move.To.y = Move.From.y;
    }
    else if (Str.compare("O-O") == 0)
    {
      Move.Piece = King;
      Move.From.x = 4;
      if (ActivePlayer == White)
        Move.From.y = 0;
      else
        Move.From.y = 7;
      Move.To.x = 6;
      Move.To.y = Move.From.y;
    }
    else
    {
      /* Extract piece type */
      Move.Piece = ChessPiece::GetPieceType(Str.c_str());
      if (Move.Piece != Pawn)
        Str.erase(0,1);
      /* Remove status indicators */
      unsigned int Pos = Str.find_first_of(":x#+");
      while (Pos != string::npos)
      {
        if (Str[Pos] == ':')
          Move.EnPassant = true;
        else if (Str[Pos] == '+')
          Move.Check = true;
        else if (Str[Pos] == '#')
        {
          Move.Check = true;
          Move.Mate = true;
        }
        Str.erase(Pos,1);
        Pos = Str.find_first_of(":x#+");
      }
      /* Extract promotion */
      Pos = Str.find_first_of("=");
      if (Pos != string::npos && Str[Pos] == '=' && Str.length() > Pos+1)
      {
        Move.PromotedTo = ChessPiece::GetPieceType(Str.substr(Pos+1).c_str());
        Str.erase(Pos,1);
      }
      /* Extract move */
      if (Str.length() >= 4)
      {
        Move.From = Position::ParseString(Str.c_str());
        Move.To = Position::ParseString(Str.substr(2,2).c_str());
      }
      else if (Str.length() == 3)
      {
        Move.From = Position::ParseString(Str.substr(0,1).c_str());
        Move.To = Position::ParseString(Str.substr(1,2).c_str());
      }
      else
        Move.To = Position::ParseString(Str.c_str());
    }
    return Move;
  }

  char* ToAlgebraicNotation() const
  {
    char* Text = new char[16];
    if (Piece == King && abs(To.x-From.x) == 2)
    {
      if (To.x == 2)
        strcpy(Text, "O-O-O");
      else if (To.x == 6)
        strcpy(Text, "O-O");
    }
    else
    {
      char* Str = ChessPiece::GetNotation(Piece);
      strcpy(Text, Str);
      delete[] Str;
      Str = From.ToString();
      strcat(Text, Str);
      delete[] Str;
      if (CapturedPiece != NULL)
      {
        if (EnPassant)
          strcat(Text, ":");
        else
          strcat(Text, "x");
      }
      Str = To.ToString();
      strcat(Text, Str);
      delete[] Str;
      if (PromotedTo != Blank)
      {
        strcat(Text, "=");
        Str = ChessPiece::GetNotation(PromotedTo);
        strcat(Text, Str);
        delete[] Str;
      }
      if (Check && Mate)
        strcat(Text, "#");
      else if (Check)
        strcat(Text, "+");
    }
    return Text;
  }
};

struct ChessPlayer
{
  string Name;
  unsigned int MoveTime;
  unsigned int TotalTime;
};

struct ChessGameImage
{
  long Pieces[32];
  ChessGameMode Mode;
  ChessGameState State;
  ChessPieceColor ActivePlayer;
  unsigned int TimePerMove;
  unsigned long WhitePlayerMoveTime;
  unsigned long WhitePlayerTotalTime;
  unsigned long BlackPlayerMoveTime;
  unsigned long BlackPlayerTotalTime;
  // TODO Add move list and timestamp
};

class ChessGame : public Observable
{
public:
  ChessGame();
  ~ChessGame();

  void AddObserver(Observer* Object);
  bool CanBePromoted(const Position Pos);
  void Clear();
  unsigned int CountBackMoves(const Position Pos);
  void DrawGame();
  void EndTurn();
  void EvaluateMoves();
  ChessPieceColor GetActivePlayer();
  int GetBoardValue(ChessPieceColor Player);
  int GetCaptureCount();
  int GetDisplayedMove();
  const LinkedList<ChessMove>* GetMoves();
  ChessGameMode GetMode();
  const ChessPiece* GetPiece(const Position Pos);
  const ChessPlayer* GetPlayer(const ChessPieceColor Player);
  SYSTEMTIME* GetStartTime();
  ChessGameState GetState();
  unsigned int GetTimePerMove();
  void GotoFirstMove();
  void GotoLastMove();
  void GotoNextMove();
  void GotoPreviousMove();
  bool IsLastMoveDisplayed();
  bool IsMoveValid(const Position From, const Position To);
  bool IsPlayerChecked();
  bool IsPlayerMated();
  bool LoadFromFile(const string FileName);
  bool LoadFromImage(const ChessGameImage* Image);
  bool MakeMove(const Position From, const Position To, const bool Validate = true);
  void PauseGame();
  void PostponeGame();
  bool PromotePawnTo(const ChessPieceType Type);
  void Reset();
  void ResignGame();
  void ResumeGame();
  bool SaveToFile(const string FileName);
  bool SaveToImage(ChessGameImage* Image);
  bool SetMode(const ChessGameMode NewMode, const unsigned int Time);
  void SetPlayerName(const ChessPieceColor Player, const string Name);
  bool SetPlayerTime(const ChessPieceColor Player, const unsigned long Time);
  void StartGame();
  void TakeBackMove();
  bool UpdateTime(const unsigned long Time);

private :
  ChessBoard* DisplayBoard;
  ChessBoard* GameBoard;
  LinkedList<ChessMove>* Moves;
  ChessEvaluator* Evaluator;

  ChessPlayer WhitePlayer;
  ChessPlayer BlackPlayer;

  ChessPieceColor ActivePlayer;
  unsigned int CaptureCount;
  int CurrentMove;
  ChessGameMode Mode;
  ChessGameState State;
  unsigned int TimePerMove;
  SYSTEMTIME* Timestamp;
};

#endif
