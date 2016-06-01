/*
* ChessEngine.cpp
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
#include "chessengine.h"

// Public functions ------------------------------------------------------------

ChessEngine::ChessEngine()
{
  ActivePlayerChangedProc = NULL;
  GameStateChangedProc = NULL;
  PieceMovedProc = NULL;
  PlayerInfoChangedProc = NULL;

  CaptureCount = 0;
}

ChessEngine::~ChessEngine()
{
}

void ChessEngine::DrawGame()
{
  EndGame(EndedInDraw);
}

ChessAiMove* ChessEngine::FindBestMove(const int Depth, const int Window)
{
  return Ai.Execute(&Game,Depth,Window);
}

ChessPieceColor ChessEngine::GetActivePlayer()
{
  return Game.ActivePlayer;
}

GameState ChessEngine::GetGameState()
{
  return Game.State;
}

const ChessPiece* ChessEngine::GetPiece(const Position Pos)
{
  return Game.Board.GetPiece(Pos);
}

const ChessPlayer* ChessEngine::GetPlayer(const ChessPieceColor Player)
{
  if (Player == White)
    return &(Game.WhitePlayer);
  else
    return &(Game.BlackPlayer);
}

const ChessMove* ChessEngine::GetFirstMove()
{
  return Game.Moves.GetFirst();
}

const ChessMove* ChessEngine::GetNextMove()
{
  return Game.Moves.GetNext();
}

const ChessMove* ChessEngine::GetMove(unsigned int Index)
{
  return Game.Moves.Get(Index);
}

unsigned int ChessEngine::GetMoveCount()
{
  return Game.Moves.Size();
}

bool ChessEngine::IsMoveValid(const Position From, const Position To)
{
  return Game.Board.IsMoveValid(From, To);
}

bool ChessEngine::LoadFromFile(const char* FileName)
{
  bool Result = Game.LoadFromFile(FileName);
  CaptureCount = 0; // TODO Count number of captures
  return Result;
}

void ChessEngine::LoadFromImage(const ChessGameImage* Image)
{
  if (Image != NULL)
  {
    Game.LoadFromImage(Image);
    CaptureCount = 0; // TODO Count number of captures
    int Result = 0;
    if (Game.Board.IsKingCheck(Game.ActivePlayer))
      Result = Result | Checked;
    if (Game.Board.IsKingMate(Game.ActivePlayer))
      Result = Result | Mated;
    /* Execute call back methods */
    if (PieceMovedProc != NULL)
      (*PieceMovedProc)(Result);
    if (ActivePlayerChangedProc != NULL)
      (*ActivePlayerChangedProc)(Game.ActivePlayer);
    /* End the game */
    if (CaptureCount == 30)
      DrawGame();
    else if (Result & Mated)
    {
      if (Result & Checked)
        EndGame(EndedInCheckMate);
      else
        EndGame(EndedInStaleMate);
    }
  }
}

int ChessEngine::MovePiece(const Position From, const Position To, const bool Validate)
{
  const ChessPiece* Piece = GetPiece(From);
  if (Piece != NULL && (!Validate || (Game.State == Started && Piece->Color == Game.ActivePlayer && Game.Board.IsMoveValid(From, To))))
  {
    int Result = 0;
    /* Move the piece */
    bool EnPassant = false;
    ChessPiece* CapturedPiece = Game.Board.MovePiece(From, To, EnPassant);
    /* Calculate the move result */
    if (Piece->Type == Pawn && To.y == (Piece->Color == White ? 7 : 0))
      Result = Result | Promoted;
    if (Game.Board.IsKingCheck(!Piece->Color))
      Result = Result | Checked;
    if (Game.Board.IsKingMate(!Piece->Color))
      Result = Result | Mated;
    /* Add the move to history */
    ChessMove* Move = new ChessMove;
    Move->MovedPiece = Piece->Type;
    Move->PromotedTo = Piece->Type;
    Move->CapturedPiece = Blank;
    if (CapturedPiece != NULL)
    {
      Move->CapturedPiece = CapturedPiece->Type;
      if (EnPassant)
        Result = Result | CapturedEnPassant;
      else
        Result = Result | Captured;
      CaptureCount++;
      delete CapturedPiece;
    }
    Move->From = From;
    Move->To = To;
    Move->Result = Result;
    Game.Moves.Add(Move);
    /* Execute call back method */
    if (PieceMovedProc != NULL)
      (*PieceMovedProc)(Result);
    /* End the turn */
    if (CaptureCount == 30)
      DrawGame();
    else if (Result & Mated)
    {
      if (Result & Checked)
        EndGame(EndedInCheckMate);
      else
        EndGame(EndedInStaleMate);
    }
    else
      NextPlayer();
    return Result;
  }
  else
    return Invalid;
}

void ChessEngine::NewGame()
{
  Game.Reset();
  CaptureCount = 0;
  if (GameStateChangedProc != NULL)
    (*GameStateChangedProc)(Undefined, Game.State);
}

void ChessEngine::PauseGame()
{
  if (Game.State == Started)
  {
    Game.State = Paused;
    if (GameStateChangedProc != NULL)
      (*GameStateChangedProc)(Started, Game.State);
  }
}

void ChessEngine::PostponeGame()
{
  EndGame(Postponed);
}

void ChessEngine::PromotePawnTo(const ChessPieceType Type)
{
  if (Type != Blank && Game.Moves.Size() > 0)
  {
    Position LastMoved = Game.Moves.Get(Game.Moves.Size()-1)->To;
    ChessPiece* PromotedPiece = Game.Board.GetPiece(LastMoved);
    if (PromotedPiece != NULL && PromotedPiece->Type == Pawn && LastMoved.y == (PromotedPiece->Color == White ? 7 : 0))
    {
      int Result = 0;
      /* Change the piece type */
      PromotedPiece->Type = Type;
      /* Calculate the move result */
      if (Game.Board.IsKingCheck(PromotedPiece->Color))
        Result = Result | Checked;        // The opponent is Checked
      if (Game.Board.IsKingMate(PromotedPiece->Color))
        Result = Result | Mated;          // The opponent is Mated
      /* Execute call back method */
      if (PieceMovedProc != NULL)
        (*PieceMovedProc)(Result);
    }
  }
}

void ChessEngine::ResumeGame()
{
  if (Game.State == Paused)
  {
    Game.State = Started;
    if (GameStateChangedProc != NULL)
      (*GameStateChangedProc)(Paused, Game.State);
  }
}

void ChessEngine::ResignGame()
{
  EndGame(EndedInForfeit);
}

bool ChessEngine::SaveToFile(const char* FileName)
{
  return Game.SaveToFile(FileName);
}

void ChessEngine::SaveToImage(ChessGameImage* Image)
{
  Game.SaveToImage(Image);
}

void ChessEngine::SetGameMode(const GameMode NewMode, const int Time)
{
  if (Game.State == Created)
  {
    Game.Mode = NewMode;
    if (Game.Mode == MoveTime)
    {
      Game.TimePerMove = Time;
      Game.WhitePlayer.Time = Time;
      Game.BlackPlayer.Time = 0;
    }
    else
    {
      Game.TimePerMove = 0;
      Game.WhitePlayer.Time = Time;
      Game.BlackPlayer.Time = Time;
    }
  }
}

void ChessEngine::SetPlayerName(const ChessPieceColor Player, const char* Name)
{
  if (Player == Black)
    Game.BlackPlayer.Name.assign(Name);
  else
    Game.WhitePlayer.Name.assign(Name);
  if (PlayerInfoChangedProc != NULL)
    (*PlayerInfoChangedProc)(Player);
}

void ChessEngine::SetPlayerTime(const ChessPieceColor Player, const unsigned long Time)
{
  if (Player == Black)
    Game.BlackPlayer.Time = Time;
  else
    Game.WhitePlayer.Time = Time;
  if (PlayerInfoChangedProc != NULL)
    (*PlayerInfoChangedProc)(Player);
}

void ChessEngine::ShowPrevious()
{
  // TODO Change the game view to the last move
}

void ChessEngine::ShowNext()
{
  // TODO Change the game view to the next move
}

void ChessEngine::StartGame()
{
  if (Game.State == Created)
  {
    /* Change the game state */
    Game.State = Started;
    /* Execute call back methods */
    if (GameStateChangedProc != NULL)
      (*GameStateChangedProc)(Created, Game.State);
    if (ActivePlayerChangedProc != NULL)
      (*ActivePlayerChangedProc)(Game.ActivePlayer);
  }
}

void ChessEngine::TakeBackMove()
{
  if (Game.Moves.Size() > 0)
  {
    /* Get the last move played */
    ChessMove* LastMove = Game.Moves.Remove(Game.Moves.Size()-1);
    ChessPiece* Piece = Game.Board.GetPiece(LastMove->To);
    if (Piece != NULL)
    {
      /* Move the piece back to it's origin */
      if (LastMove->CapturedPiece != Blank)
      {
        if (LastMove->Result & CapturedEnPassant)
        {
          Game.Board.MoveBackPiece(LastMove->From,LastMove->To,NULL);
          Game.Board.AddPiece(Position(LastMove->To.x,LastMove->From.y),new ChessPiece(LastMove->CapturedPiece,Game.ActivePlayer,Game.CountBackMoves(Position(LastMove->To.x,LastMove->From.y))));
        }
        else
          Game.Board.MoveBackPiece(LastMove->From,LastMove->To,new ChessPiece(LastMove->CapturedPiece,Game.ActivePlayer,Game.CountBackMoves(LastMove->To)));
        CaptureCount--;
      }
      else
        Game.Board.MoveBackPiece(LastMove->From,LastMove->To,NULL);
      /* Restore the piece to a pawn if it was promoted */
      if (LastMove->Result & Promoted)
        Piece->Type = Pawn;
      /* Execute call back method */
      if (PieceMovedProc != NULL)
        (*PieceMovedProc)((Game.Moves.GetLast() != NULL ? Game.Moves.GetLast()->Result : 0));
      /* Change turn */
      NextPlayer();
    }
    /* Delete move information */
    delete LastMove;
  }
}

void ChessEngine::UpdateGameTime(const unsigned long Time)
{
  if (Game.State == Started)
  {
    if (Game.Mode == FreeTime)
    {
      if (Game.ActivePlayer == White)
        Game.WhitePlayer.Time += Time;
      else
        Game.BlackPlayer.Time += Time;
    }
    else if (Game.ActivePlayer == White)
    {
      Game.WhitePlayer.Time -= Time;
      if (Game.WhitePlayer.Time <= 0)
        EndGame(EndedInTimeout);
    }
    else
    {
      Game.BlackPlayer.Time -= Time;
      if (Game.BlackPlayer.Time <= 0)
        EndGame(EndedInTimeout);
    }
    if (PlayerInfoChangedProc != NULL)
      (*PlayerInfoChangedProc)(Game.ActivePlayer);
  }
}

// Private functions -----------------------------------------------------------

void ChessEngine::NextPlayer()
{
  /* Switch active player */
  Game.ActivePlayer = !Game.ActivePlayer;
  if (Game.ActivePlayer == White)
    Game.WhitePlayer.Time += Game.TimePerMove;
  else
    Game.BlackPlayer.Time += Game.TimePerMove;
  /* Execute call back method */
  if (ActivePlayerChangedProc != NULL)
    (*ActivePlayerChangedProc)(Game.ActivePlayer);
}

void ChessEngine::EndGame(GameState EndState)
{
  if (Game.State == Started || Game.State == Paused)
  {
    /* Change game state */
    Game.State = EndState;
    /* Execute call back method */
    if (GameStateChangedProc != NULL)
      (*GameStateChangedProc)(Started, Game.State);
  }
}
