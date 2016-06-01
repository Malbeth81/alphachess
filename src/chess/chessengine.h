/*
* ChessEngine.h - Engine that allows to play a game of chess.
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
#ifndef CHESSENGINE_H_
#define CHESSENGINE_H_

#include "chessgame.h"
#include "chessai.h"

typedef void (__stdcall *ACTIVEPLAYERCHANGEDPROC)(ChessPieceColor ActivePlayer);
typedef void (__stdcall *GAMESTATECHANGEDPROC)(GameState PreviousState, GameState CurrentState);
typedef void (__stdcall *PIECEMOVEDPROC)(int MoveResult);
typedef void (__stdcall *PLAYERINFOCHANGEDPROC)(ChessPieceColor Color);

// Chess engine methods --------------------------------------------------------

class ChessEngine
{
public:
  ACTIVEPLAYERCHANGEDPROC ActivePlayerChangedProc;
  GAMESTATECHANGEDPROC GameStateChangedProc;
  PIECEMOVEDPROC PieceMovedProc;
  PLAYERINFOCHANGEDPROC PlayerInfoChangedProc;

  ChessEngine();
  ~ChessEngine();

  void DrawGame();
  ChessAiMove* FindBestMove(const int Depth, const int Window);
  ChessPieceColor GetActivePlayer();
  GameState GetGameState();
  const ChessPiece* GetPiece(const Position Pos);
  const ChessPlayer* GetPlayer(const ChessPieceColor Player);
  const ChessMove* GetFirstMove();
  const ChessMove* GetNextMove();
  const ChessMove* GetMove(unsigned int Index);
  unsigned int GetMoveCount();
  bool IsMoveValid(const Position From, const Position To);
  bool LoadFromFile(const char* FileName);
  void LoadFromImage(const ChessGameImage* Image);
  int MovePiece(const Position From, const Position To, const bool Validate = true);
  void NewGame();
  void PauseGame();
  void PostponeGame();
  void PromotePawnTo(const ChessPieceType Type);
  void ResumeGame();
  void ResignGame();
  bool SaveToFile(const char* FileName);
  void SaveToImage(ChessGameImage* Image);
  void SetGameMode(const GameMode NewMode, const int Time);
  void SetPlayerName(const ChessPieceColor Player, const char* Name);
  void SetPlayerTime(const ChessPieceColor Player, const unsigned long Time);
  void ShowPrevious();
  void ShowNext();
  void StartGame();
  void TakeBackMove();
  void UpdateGameTime(const unsigned long Time);
private:
  ChessGame Game;
  ChessAi Ai;

  unsigned int CaptureCount;

  void EndGame(const GameState EndState);
  void NextPlayer();
};

#endif
