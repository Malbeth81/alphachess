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

//#define MAKELONG(lo,hi) ((unsigned long)(((unsigned short)(lo))|(((unsigned long)((unsigned short)(hi)))<<16)))
//#define MAKEWORD(lo,hi) ((unsigned short)(((unsigned char)(lo))|(((unsigned short)((unsigned char)(hi)))<<8)))
#define MAKEBYTE(lo,hi) ((unsigned char)(((unsigned char)(lo))|(((unsigned char)((unsigned char)(hi)))<<4)))
//#define LOWORD(l) ((unsigned short)((unsigned long)(l)))
//#define HIWORD(l) ((unsigned short)(((unsigned long)(l)>>16)&0xFFFF))
//#define LOBYTE(w) ((unsigned char)(w))
//#define HIBYTE(w) ((unsigned char)(((unsigned short)(w)>>8)&0xFF))
#define LOBITS(b) ((unsigned char)(((unsigned char)(b))&0x0F))
#define HIBITS(b) ((unsigned char)(((unsigned char)(b)>>4)&0x0F))

// Public functions ------------------------------------------------------------

ChessGame::ChessGame()
{
  DisplayBoard = new ChessBoard();
  GameBoard = new ChessBoard();
  Moves = new LinkedList<ChessMove>();

  WhitePlayer.MoveTime = 0;
  WhitePlayer.TotalTime = 0;
  BlackPlayer.MoveTime = 0;
  BlackPlayer.TotalTime = 0;

  ActivePlayer = White;
  CaptureCount = 0;
  CurrentMove = -1;
  Mode = FreeTime;
  State = Undefined;
  TimePerMove = 0;
  Timestamp = NULL;
}

ChessGame::~ChessGame()
{
  Clear();
  /* Clean up */
  delete DisplayBoard;
  delete GameBoard;
  delete Moves;
}

bool ChessGame::CanBePromoted(const Position Pos)
{
  const ChessPiece* Piece = GameBoard->GetPiece(Pos);
  if (Piece != NULL && Piece->Type == Pawn && Pos.y == (Piece->Color == White ? 7 : 0))
    return true;
  return false;
}

void ChessGame::Clear()
{
  ActivePlayer = White;
  CaptureCount = 0;
  CurrentMove = -1;
  Mode = FreeTime;
  State = Undefined;
  TimePerMove = 0;
  WhitePlayer.Name = "";
  WhitePlayer.MoveTime = 0;
  WhitePlayer.TotalTime = 0;
  BlackPlayer.Name = "";
  BlackPlayer.MoveTime = 0;
  BlackPlayer.TotalTime = 0;

  GameBoard->Clear();
  DisplayBoard->Copy(GameBoard);
  while (Moves->Size() > 0)
  {
    ChessMove* Move = Moves->Remove();
    delete Move->CapturedPiece;
    delete Move;
  }

  NotifyObservers(BoardUpdated);
  NotifyObservers(PlayerUpdated);
  NotifyObservers(StateChanged);
  NotifyObservers(TurnEnded);
}

unsigned int ChessGame::CountBackMoves(const Position Pos)
{
  Position CurPos = Pos;
  unsigned int Result = 0;
  ChessMove* Move = Moves->GetLast();
  while (Move != NULL)
  {
    if (Move->To == CurPos)
    {
      CurPos = Move->From;
      Result++;
    }
    Move = Moves->GetPrevious();
  }
  return Result;
}

void ChessGame::DrawGame()
{
  State = EndedInDraw;
  NotifyObservers(StateChanged);
}

void ChessGame::EndTurn()
{
  /* Switch active player */
  ActivePlayer = !ActivePlayer;
  if (Mode == MoveTime)
  {
    if (ActivePlayer == White)
      WhitePlayer.MoveTime = TimePerMove*60000;
    else
      BlackPlayer.MoveTime = TimePerMove*60000;
  }
  NotifyObservers(TurnEnded);
}

ChessPieceColor ChessGame::GetActivePlayer()
{
  return ActivePlayer;
}

int ChessGame::GetBoardValue()
{
  return GameBoard->Value();
}

int ChessGame::GetCaptureCount()
{
  return CaptureCount;
}

const ChessMove* ChessGame::GetDisplayedMove()
{
  return Moves->Get(CurrentMove);
}

ChessEvaluator* ChessGame::GetEvaluator()
{
  return new ChessEvaluator(GameBoard, ActivePlayer);
}

ChessGameMode ChessGame::GetMode()
{
  return Mode;
}

const LinkedList<ChessMove>* ChessGame::GetMoves()
{
  return Moves;
}

const ChessPiece* ChessGame::GetPiece(const Position Pos)
{
  return DisplayBoard->GetPiece(Pos);
}

const ChessPlayer* ChessGame::GetPlayer(const ChessPieceColor Player)
{
  if (Player == White)
    return &(WhitePlayer);
  else
    return &(BlackPlayer);
}

SYSTEMTIME* ChessGame::GetStartTime()
{
  return Timestamp;
}

ChessGameState ChessGame::GetState()
{
  return State;
}

unsigned int ChessGame::GetTimePerMove()
{
  return TimePerMove;
}

void ChessGame::GotoFirstMove()
{
  if (Moves->Size() > 0)
  {
    while (CurrentMove >= 0)
    {
      /* Undo the previous move */
      ChessMove* Move = Moves->Get(CurrentMove);
      if (Move != NULL)
      {
        ChessPiece* Piece = DisplayBoard->GetPiece(Move->To);
        if (Piece != NULL)
        {
          DisplayBoard->MoveBackPiece(Move->From,Move->To,Move->CapturedPiece,Move->EnPassant);
          if (Move->CapturedPiece != NULL)
            CaptureCount--;
          if (Move->PromotedTo = Blank)
            Piece->Type = Pawn;
          CurrentMove--;
        }
      }
    }
    NotifyObservers(BoardUpdated);
  }
}

void ChessGame::GotoLastMove()
{
  if (Moves->Size() > 0)
  {
    while (CurrentMove < (int)Moves->Size()-1)
    {
      /* Undo the previous move */
      CurrentMove++;
      ChessMove* Move = Moves->Get(CurrentMove);
      if (Move != NULL)
      {
        bool EnPassant = false;
        DisplayBoard->MovePiece(Move->From, Move->To, EnPassant);
      }
    }
    NotifyObservers(BoardUpdated);
  }
}

void ChessGame::GotoNextMove()
{
  if (Moves->Size() > 0 && CurrentMove < (int)Moves->Size()-1)
  {
    /* Undo the previous move */
    CurrentMove++;
    ChessMove* Move = Moves->Get(CurrentMove);
    if (Move != NULL)
    {
      bool EnPassant = false;
      DisplayBoard->MovePiece(Move->From, Move->To, EnPassant);
      NotifyObservers(BoardUpdated);
    }
  }
}

void ChessGame::GotoPreviousMove()
{
  if (Moves->Size() > 0 && CurrentMove >= 0)
  {
    /* Undo the previous move */
    ChessMove* Move = Moves->Get(CurrentMove);
    if (Move != NULL)
    {
      ChessPiece* Piece = DisplayBoard->GetPiece(Move->To);
      if (Piece != NULL)
      {
        DisplayBoard->MoveBackPiece(Move->From,Move->To,Move->CapturedPiece,Move->EnPassant);
        if (Move->CapturedPiece != NULL)
          CaptureCount--;
        if (Move->PromotedTo = Blank)
          Piece->Type = Pawn;
        CurrentMove--;
        NotifyObservers(BoardUpdated);
      }
    }
  }
}

bool ChessGame::IsLastMoveDisplayed()
{
  return CurrentMove == (int)Moves->Size()-1;
}

bool ChessGame::IsMoveValid(const Position From, const Position To)
{
  return GameBoard->IsMoveValid(From, To);
}

bool ChessGame::IsPlayerChecked()
{
  return DisplayBoard->IsKingCheck(ActivePlayer);
}

bool ChessGame::IsPlayerMated()
{
  return DisplayBoard->IsKingMate(ActivePlayer);
}

bool ChessGame::LoadFromFile(const string FileName)
{
  // Open the file
  ifstream File(FileName.c_str(), ios::in);
  if (File.is_open())
  {
    BeginUpdate();
    // Reset the game
    Reset();
    State = Started;
    // Load the file content
    string Buffer;
    bool Content = false;
    while (!File.eof() && !File.fail())
    {
      getline(File, Buffer);
      if (!File.fail())
      {
        // Skip byte-order mark
        char Bom[] = {0xEF, 0xBB, 0xBF};
        if (memcmp(Buffer.c_str(), Bom, 3) == 0)
          Buffer.erase(0,3);
        if (Buffer[0] == '[')
        {
          if (Content == true)  // If a header follows content then it is another game so we ignore it
            break;
          // Extract header information
          if (Buffer.compare(0,7,"[Black ") == 0)
            BlackPlayer.Name = Buffer.substr(8,Buffer.length()-10);
          else if (Buffer.compare(0,7,"[White ") ==0)
            WhitePlayer.Name = Buffer.substr(8,Buffer.length()-10);
          else if (Buffer.compare(0,11,"[BlackTime ") == 0)
          {
            BlackPlayer.MoveTime = atoi(Buffer.substr(12,Buffer.length()-14).c_str())*1000;
            BlackPlayer.TotalTime = BlackPlayer.MoveTime;
          }
          else if (Buffer.compare(0,11,"[WhiteTime ") == 0)
          {
            WhitePlayer.MoveTime = atoi(Buffer.substr(12,Buffer.length()-14).c_str())*1000;
            WhitePlayer.TotalTime = WhitePlayer.MoveTime;
          }
        }
        else
        {
          Content = true;
          // Extract moves
          unsigned int Start = 0;
          unsigned int Round = 0;
          while (Start != string::npos)
          {
            Start = Buffer.find_first_not_of(" ", Start);
            if (Start != string::npos)
            {
              unsigned int End = string::npos;
              // Skip the comments
              if (Buffer[Start] == '{')
              {
                End = Buffer.find_first_of("}", Start);
                if (Moves->Size()%2 != 0)
                {
                  // Skip a repeating round number
                  unsigned int Pos = Buffer.find_first_of("...", End);
                  if (Pos != string::npos)
                    End = Pos+3;
                }
              }
              else if (Buffer[Start] == ';')
                break;
              // Skip the round number
              else if (floor(Moves->Size()/2) == Round)
              {
                End = Buffer.find_first_of(".", Start);
                Round ++;
              }
              // Extract the move
              else
              {
                End = Buffer.find_first_of(" {;", Start);
                if (End == string::npos)
                  End = Buffer.length();
                ChessMove Move = ChessMove::ParseAlgebraicString(Buffer.substr(Start,End-Start), ActivePlayer);
                Move.From = GameBoard->FindStartingPos(Move.From, Move.To, Move.Piece, ActivePlayer);
                if (GameBoard->IsPositionValid(Move.From) && GameBoard->IsPositionValid(Move.To))
                {
                  MakeMove(Move.From, Move.To, false);
                  if (Move.PromotedTo != Blank)
                    PromotePawnTo(Move.PromotedTo);
                }
              }
              Start = End+1;
            }
          }
        }
      }
    }
    File.close();
    CurrentMove = Moves->Size()-1;
    DisplayBoard->Copy(GameBoard);
    if (State == Started)
      State = Paused;
    EndUpdate();
    NotifyObservers(PlayerUpdated);
    NotifyObservers(StateChanged);
    NotifyObservers(BoardUpdated);
    NotifyObservers(TurnEnded);
    return true;
  }
  return false;
}

bool ChessGame::LoadFromImage(const ChessGameImage* Image)
{
  if (Image != NULL)
  {
    Clear();
    /* Load information about the game from the image */
    Mode = Image->Mode;
    State = Image->State;
    ActivePlayer = Image->ActivePlayer;
    CaptureCount = 0; // TODO Count number of captures
    CurrentMove = Moves->Size()-1;
    TimePerMove = Image->TimePerMove;
    WhitePlayer.MoveTime = Image->WhitePlayerMoveTime;
    WhitePlayer.TotalTime = Image->WhitePlayerTotalTime;
    BlackPlayer.MoveTime = Image->BlackPlayerMoveTime;
    BlackPlayer.TotalTime = Image->BlackPlayerTotalTime;
    /* Load information about the chessboard from the image */
    for (short i=0; i < 32; i++)
    {
      unsigned short HiWord = HIWORD(Image->Pieces[i]);
      unsigned char HiByte = HIBYTE(HiWord);
      unsigned char LoByte = LOBYTE(HiWord);
      ChessPiece* Piece = new ChessPiece((ChessPieceType)LOBITS(LoByte), (ChessPieceColor)HIBITS(LoByte), LOWORD(Image->Pieces[i]));
      if (HiByte != 255)
        GameBoard->AddPiece(Position(HIBITS(HiByte), LOBITS(HiByte)), Piece);
    }
    DisplayBoard->Copy(GameBoard);
    NotifyObservers(PlayerUpdated);
    NotifyObservers(StateChanged);
    NotifyObservers(BoardUpdated);
    NotifyObservers(TurnEnded);
    return true;
  }
  return false;
}

bool ChessGame::MakeMove(const Position From, const Position To, const bool Validate)
{
  const ChessPiece* Piece = GameBoard->GetPiece(From);
  if (Piece != NULL && (!Validate || (State == Started && Piece->Color == ActivePlayer && GameBoard->IsMoveValid(From, To))))
  {
    /* Records the move */
    ChessMove* Move = new ChessMove;
    Move->From = From;
    Move->To = To;
    Move->Piece = Piece->Type;
    Move->PromotedTo = Blank;
    Move->CapturedPiece = NULL;
    Move->EnPassant = false;
    Move->Check = false;
    Move->Mate = false;
    /* Move the piece */
    Move->CapturedPiece = GameBoard->MovePiece(From, To, Move->EnPassant);
    if (CurrentMove == (int)Moves->Size()-1)
      DisplayBoard->Copy(GameBoard);
    if (GameBoard->IsKingCheck(!Piece->Color))
      Move->Check = true;
    if (GameBoard->IsKingMate(!Piece->Color))
      Move->Mate = true;
    if (Move->CapturedPiece != NULL)
      CaptureCount++;
    /* Add the move to history */
    Moves->Add(Move);
    if (CurrentMove == (int)Moves->Size()-2)
      CurrentMove ++;
    /* Perform promotion */
    NotifyObservers(BoardUpdated);
    if (Move->Piece == Pawn && (To.y == 0 || To.y == 7))
      NotifyObservers(PiecePromoted);
    /* End the turn */
    EndTurn();
    /* End the game */
    if (CaptureCount == 30)
    {
      State = EndedInDraw;
      NotifyObservers(StateChanged);
    }
    else if (Move->Check && Move->Mate)
    {
      State = EndedInCheckMate;
      NotifyObservers(StateChanged);
    }
    else if (Move->Mate)
    {
      State = EndedInStaleMate;
      NotifyObservers(StateChanged);
    }
    return true;
  }
  return false;
}

void ChessGame::PauseGame()
{
  if (State == Started)
  {
    State = Paused;
    NotifyObservers(StateChanged);
  }
}

void ChessGame::PostponeGame()
{
  State = Postponed;
  NotifyObservers(StateChanged);
}

bool ChessGame::PromotePawnTo(const ChessPieceType Type)
{
  if (Type != Blank && Moves->Size() > 0)
  {
    ChessMove* Move = Moves->Get(Moves->Size()-1);
    if (CanBePromoted(Move->To))
    {
      ChessPiece* Piece = GameBoard->GetPiece(Move->To);
      if (Piece != NULL)
      {
        /* Change the piece type */
        Piece->Type = Type;
        /* Update the move */
        Move->PromotedTo = Type;
        if (GameBoard->IsKingCheck(Piece->Color))
          Move->Check = true;
        if (GameBoard->IsKingMate(Piece->Color))
          Move->Mate = true;
        NotifyObservers(BoardUpdated);
        return true;
      }
    }
  }
  return false;
}

void ChessGame::Reset()
{
  Clear();

  Position Pos;
  Pos.y = 7;
  Pos.x = 0;
  GameBoard->AddPiece(Pos, new ChessPiece(Rook, Black));
  Pos.x = 1;
  GameBoard->AddPiece(Pos, new ChessPiece(Knight, Black));
  Pos.x = 2;
  GameBoard->AddPiece(Pos, new ChessPiece(Bishop, Black));
  Pos.x = 3;
  GameBoard->AddPiece(Pos, new ChessPiece(Queen, Black));
  Pos.x = 4;
  GameBoard->AddPiece(Pos, new ChessPiece(King, Black));
  Pos.x = 5;
  GameBoard->AddPiece(Pos, new ChessPiece(Bishop, Black));
  Pos.x = 6;
  GameBoard->AddPiece(Pos, new ChessPiece(Knight, Black));
  Pos.x = 7;
  GameBoard->AddPiece(Pos, new ChessPiece(Rook, Black));
  Pos.y = 6;
  for (Pos.x=0; Pos.x < 8; Pos.x++)
    GameBoard->AddPiece(Pos, new ChessPiece(Pawn, Black));
  Pos.y = 0;
  Pos.x = 0;
  GameBoard->AddPiece(Pos, new ChessPiece(Rook, White));
  Pos.x = 1;
  GameBoard->AddPiece(Pos, new ChessPiece(Knight, White));
  Pos.x = 2;
  GameBoard->AddPiece(Pos, new ChessPiece(Bishop, White));
  Pos.x = 3;
  GameBoard->AddPiece(Pos, new ChessPiece(Queen, White));
  Pos.x = 4;
  GameBoard->AddPiece(Pos, new ChessPiece(King, White));
  Pos.x = 5;
  GameBoard->AddPiece(Pos, new ChessPiece(Bishop, White));
  Pos.x = 6;
  GameBoard->AddPiece(Pos, new ChessPiece(Knight, White));
  Pos.x = 7;
  GameBoard->AddPiece(Pos, new ChessPiece(Rook, White));
  Pos.y = 1;
  for (Pos.x=0; Pos.x < 8; Pos.x++)
    GameBoard->AddPiece(Pos, new ChessPiece(Pawn, White));
  DisplayBoard->Copy(GameBoard);
  NotifyObservers(BoardUpdated);
}

void ChessGame::ResignGame()
{
  State = EndedInForfeit;
  NotifyObservers(StateChanged);
}

void ChessGame::ResumeGame()
{
  if (State == Paused)
  {
    State = Started;
    NotifyObservers(StateChanged);
  }
}

bool ChessGame::SaveToFile(const string FileName)
{
  ofstream File(FileName.c_str(), ios::out);
  if (File.is_open())
  {
    /* Save headers */
    File.write("[Event \"?\"]\n", 12);
    File.write("[Site \"?\"]\n", 11);
    File.write("[Date \"", 7);
    char* DateTime = FormatDateTime(Timestamp, 0, "yyyy.MM.dd", 0, "HH:mm");
    File.write(DateTime, strlen(DateTime));
    delete[] DateTime;
    File.write("\"]\n", 3);
    File.write("[Round \"", 8);
    char* Str = inttostr((int)ceil(Moves->Size()/2.0));
    File.write(Str, strlen(Str));
    delete[] Str;
    File.write("\"]\n", 3);
    File.write("[White \"", 8);
    File.write(WhitePlayer.Name.c_str(), WhitePlayer.Name.length());
    File.write("\"]\n", 3);
    File.write("[Black \"", 8);
    File.write(BlackPlayer.Name.c_str(), BlackPlayer.Name.length());
    File.write("\"]\n", 3);
    File.write("[Result \"", 9);
    if (State == EndedInForfeit || State == EndedInCheckMate || State == EndedInTimeout)
    {
      if (ActivePlayer == White)
        File.write("1-0", 3);
      else
        File.write("0-1", 3);
    }
    else if (State == EndedInDraw || State == EndedInStaleMate)
      File.write("1/2-1/2", 7);
    else
      File.write("*", 1);
    File.write("\"]\n", 3);
    File.write("[WhiteTime \"", 12);
    Str = inttostr(WhitePlayer.TotalTime/1000);
    File.write(Str, strlen(Str));
    File.write("\"]\n", 3);
    File.write("[BlackTime \"", 12);
    Str = inttostr(BlackPlayer.TotalTime/1000);
    File.write(Str, strlen(Str));
    File.write("\"]\n", 3);
    File.write("\n", 1);

    /* Save moves */
    int Round = 0;
    ChessMove* Move = Moves->GetFirst();
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
      Move = Moves->GetNext();
      if (Move != NULL)
      {
        /* Black move */
        Str = Move->ToAlgebraicNotation();
        File.write(Str, strlen(Str));
        delete[] Str;
        /* Separator */
        File.write(" ", 1);
        /* Next move */
        Move = Moves->GetNext();
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
    return true;
  }
  return false;
}

bool ChessGame::SaveToImage(ChessGameImage* Image)
{
  if (Image != NULL)
  {
    /* Save information about the game */
    Image->Mode = Mode;
    Image->State = State;
    Image->ActivePlayer = ActivePlayer;
    Image->TimePerMove = TimePerMove;
    Image->WhitePlayerMoveTime = WhitePlayer.MoveTime;
    Image->WhitePlayerTotalTime = WhitePlayer.TotalTime;
    Image->BlackPlayerMoveTime = BlackPlayer.MoveTime;
    Image->BlackPlayerTotalTime = BlackPlayer.TotalTime;
    /* Save information about the chess board */
    int Index = 0;
    Position Pos;
    for (Pos.x=0; Pos.x < 8; Pos.x++)
      for (Pos.y=0; Pos.y < 8; Pos.y++)
        if (GameBoard->GetPiece(Pos) != NULL)
        {
          Image->Pieces[Index] = MAKELONG(GameBoard->GetPiece(Pos)->TimesMoved,MAKEWORD(MAKEBYTE(GameBoard->GetPiece(Pos)->Type,GameBoard->GetPiece(Pos)->Color),MAKEBYTE(Pos.y,Pos.x)));
          Index ++;
        }
    return true;
  }
  return false;
}

bool ChessGame::SetMode(const ChessGameMode NewMode, const unsigned int Time)
{
  if (State == Undefined)
  {
    Mode = NewMode;
    if (Mode == MoveTime)
    {
      TimePerMove = Time;
      WhitePlayer.MoveTime = TimePerMove*60000;
      WhitePlayer.TotalTime = 0;
      BlackPlayer.MoveTime = 0;
      BlackPlayer.TotalTime = 0;
    }
    else
    {
      TimePerMove = 0;
      WhitePlayer.MoveTime = 0;
      WhitePlayer.TotalTime = 0;
      BlackPlayer.MoveTime = 0;
      BlackPlayer.TotalTime = 0;
    }
    return true;
  }
  return false;
}

void ChessGame::SetPlayerName(const ChessPieceColor Player, const string Name)
{
  if (Player == Black)
    BlackPlayer.Name = Name;
  else
    WhitePlayer.Name = Name;
  NotifyObservers(PlayerUpdated);
}

bool ChessGame::SetPlayerTime(const ChessPieceColor Player, const unsigned long Time)
{
  if (Player == Black)
  {
    BlackPlayer.TotalTime += abs((int)(Time-BlackPlayer.MoveTime));
    BlackPlayer.MoveTime = Time;
  }
  else
  {
    WhitePlayer.TotalTime += abs((int)(Time-WhitePlayer.MoveTime));
    WhitePlayer.MoveTime = Time;
  }
  NotifyObservers(PlayerUpdated);
  return true;
}

void ChessGame::StartGame()
{
  if (State == Undefined)
  {
    State = Started;
    Timestamp = new SYSTEMTIME;
    GetLocalTime(Timestamp);
    NotifyObservers(StateChanged);
    NotifyObservers(TurnEnded);
  }
}

void ChessGame::TakeBackMove()
{
  if (Moves->Size() > 0)
  {
    /* Get the last move played */
    ChessMove* Move = Moves->Remove(Moves->Size()-1);
    if (CurrentMove == (int)Moves->Size())
      CurrentMove--;
    /* Undo the last move player */
    if (Move != NULL)
    {
      ChessPiece* Piece = GameBoard->GetPiece(Move->To);
      if (Piece != NULL)
      {
        /* Move the piece back to it's origin */
        GameBoard->MoveBackPiece(Move->From,Move->To,Move->CapturedPiece,Move->EnPassant);
        if (CurrentMove == (int)Moves->Size()-1)
          DisplayBoard->Copy(GameBoard);
        if (Move->CapturedPiece != NULL)
          CaptureCount--;
        /* Restore the piece to a pawn if it was promoted */
        if (Move->PromotedTo != Blank)
          Piece->Type = Pawn;
      }
    }
    /* Delete move information */
    delete Move;
    NotifyObservers(BoardUpdated);
    /* Change turn */
    EndTurn();
  }
}

bool ChessGame::UpdateTime(const unsigned long Time)
{
  if (State == Started)
  {
    if (Mode == FreeTime)
      SetPlayerTime(ActivePlayer, GetPlayer(ActivePlayer)->MoveTime + Time);
    else if (Mode == MoveTime)
    {
      const ChessPlayer* Player = GetPlayer(ActivePlayer);
      SetPlayerTime(ActivePlayer, max((int)(Player->MoveTime - Time), 0));
      if (Player->MoveTime == 0)
      {
        State = EndedInTimeout;
        NotifyObservers(StateChanged);
      }
    }
    return true;
  }
  return false;
}
