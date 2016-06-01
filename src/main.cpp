/*
* Main.cpp
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
#define _WIN32_WINDOWS 0x0410   // Windows 98 +
#define _WIN32_IE 0x0300 // IE 3.0 controls
#include <dialogutils.h>
#include <linkedlist.h>
#include <string>
#include <winutils.h>
#include <windows.h>
#include "chessboardtheme.h"
#include "chessset.h"
#include "gameclient.h"
#include "gameinfo.h"
#include "menu.h"
#include "resource.h"
#include "chess/chessengine.h"
#include "dialogs/aboutdialog.h"
#include "dialogs/gamehistorydialog.h"
#include "dialogs/localgamedialog.h"
#include "dialogs/networkgamedialog.h"
#include "dialogs/preferencesdialog.h"
#include "dialogs/promotiondialog.h"
#include "panels/chatpanel.h"
#include "panels/capturepanel.h"
#include "panels/chessboardpanel.h"
#include "panels/movelistpanel.h"
#include "panels/roompanel.h"
#include "panels/playerpanel.h"

#define ClassName "AlphaChess4"
#define WindowTitle "AlphaChess 4"

#define LeftPanelSize 156
#define BottomPanelSize 120
#define MoveListMinHeight 50

#define ChessTimer 1

#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

HWND hWindow = NULL;
HCURSOR hAppCursor = NULL;

ChessBoardPanel* hChessBoardPanel = NULL;
PlayerPanel* hWhitePlayerPanel = NULL;
PlayerPanel* hBlackPlayerPanel = NULL;
CapturePanel* hCapturePanel = NULL;
MoveListPanel* hMoveListPanel = NULL;
RoomPanel* hRoomPanel = NULL;
ChatPanel* hChatPanel = NULL;

LinkedList<ChessSet> ChessSets;
LinkedList<GameInfo> GameHistory;

ChessEngine Chess;
GameClient* NetworkClient;

bool BlackPlayerIsComputer = false;
bool WhitePlayerIsComputer = false;
bool NetworkGame = false;
char NetworkPlayerName[60] = "";

int WindowLeft = CW_USEDEFAULT;
int WindowTop = CW_USEDEFAULT;
int WindowWidth = 800;
int WindowHeight = 600;

bool AutomaticUpdate = false;
bool AlwaysOnTop = false;
bool AlwaysVisible = false;
bool AutomaticallySwitchView = false;
bool AlwaysPromoteToQueen = false;
bool PlayersInfoVisible = false;
bool CaptureListVisible = false;
bool MoveListVisible = false;
bool NetworkPanelsVisible = false;
bool BoardCoordinatesVisible = false;
bool BoardLastMoveVisible = false;
bool BoardInvalidMovesVisible = false;
int ChessSetIndex = -1;
char CurrentThemeName[MAX_PATH];

char DefaultFontName[] = "MS Shell Dlg 2";
char ServerAddress[] = "alphachess.dlinkddns.com";

unsigned long LastTickCount = 0;

// Private functions -----------------------------------------------------------

static void CalculateBlackPlayerPosition(RECT* Rect)
{
  Rect->left = 0;
  Rect->top = 44;
  Rect->right = Rect->left+LeftPanelSize;
  Rect->bottom = Rect->top+44;
}

static void CalculateCaptureListPosition(RECT* Rect)
{
  Rect->left = 0;
  Rect->top = (MoveListVisible ? WindowHeight-(NetworkPanelsVisible ? BottomPanelSize : 0)-min(130,WindowHeight-(NetworkPanelsVisible ? BottomPanelSize : 0)-(PlayersInfoVisible ? 88 : 0)-MoveListMinHeight) : (PlayersInfoVisible ? 88 : 0));
  Rect->right = Rect->left+LeftPanelSize;
  Rect->bottom = Rect->top+min(130,WindowHeight-(NetworkPanelsVisible ? BottomPanelSize : 0)-(PlayersInfoVisible ? 88 : 0)-MoveListMinHeight);
}

static void CalculateChatPosition(RECT* Rect)
{
  Rect->left = LeftPanelSize;
  Rect->top = WindowHeight-BottomPanelSize;
  Rect->right = WindowWidth;
  Rect->bottom = Rect->top+BottomPanelSize;
}

static void CalculateChessBoardPosition(RECT* Rect)
{
  Rect->left = (PlayersInfoVisible || CaptureListVisible || MoveListVisible ? LeftPanelSize+2 : 0);
  Rect->top = 0;
  Rect->right = WindowWidth;
  Rect->bottom = WindowHeight-(NetworkPanelsVisible ? BottomPanelSize+2 : 0);
}

static void CalculateMoveListPosition(RECT* Rect)
{
  RECT R;
  CalculateCaptureListPosition(&R);
  Rect->left = 0;
  Rect->top = (PlayersInfoVisible ? 88 : 0);
  Rect->right = Rect->left+LeftPanelSize;
  Rect->bottom = WindowHeight-(NetworkPanelsVisible ? BottomPanelSize : 0)-(CaptureListVisible ? R.bottom-R.top : 0);
}

static void CalculateRoomPosition(RECT* Rect)
{
  RECT R;
  GetClientRect(hWindow,&R);
  Rect->left = 0;
  Rect->top = WindowHeight-BottomPanelSize;
  Rect->right = Rect->left+LeftPanelSize;
  Rect->bottom = Rect->top+BottomPanelSize;
}

static void CalculateWhitePlayerPosition(RECT* Rect)
{
  Rect->left = 0;
  Rect->top = 0;
  Rect->right = Rect->left+LeftPanelSize;
  Rect->bottom = Rect->top+44;
}

static void ClearAppCursor()
{
  if (hAppCursor != NULL)
  {
    ReleaseCapture();
    DestroyCursor(SetCursor(LoadCursor(NULL, IDC_ARROW)));
    hAppCursor = NULL;
  }
}

static char* GetApplicationFileName(char* Filename)
{
  char* Str = GetApplicationPath();
  char* Result = new char[strlen(Str)+strlen(Filename)+2];
  strcpy(Result, Str);
  strcat(Result, "\\");
  strcat(Result, Filename);
  delete[] Str;
  return Result;
}

static void LoadChessSets()
{
  /* Get the number of font in the file */
  char* FileName = GetApplicationFileName("chesssets.ini");
  int Count = GetPrivateProfileInt("ChessFonts", "Count", 0, FileName);
  if (Count > 0)
  {
    char* Str = new char[MAX_PATH];
    for (int i=0; i < Count; i++)
    {
      strcpy(Str,"ChessFont");
      char* Str2 = inttostr(i+1);
      strcat(Str,Str2);
      delete[] Str2;
      /* Created a new chess set */
      ChessSet* NewSet = new ChessSet;
      /* Load the chess set info from the file */
      GetPrivateProfileString(Str, "FileName", "", NewSet->FileName, MAX_PATH, FileName);
      GetPrivateProfileString(Str, "FontName", "", NewSet->FontName, MAX_PATH, FileName);
      GetPrivateProfileString(Str, "Letters", "", NewSet->Letters, MAX_PATH, FileName);
      /* Add the chess set to the list */
      ChessSets.Add(NewSet);
    }
    delete[] Str;
  }
  delete[] FileName;
}

static void LoadHistory()
{
  /* Get the number of games in the file */
  char* FileName = GetApplicationFileName("history.ini");
  int Count = GetPrivateProfileInt("GameHistory", "Count", 0, FileName);
  if (Count > 0)
  {
    char* Str = new char[MAX_PATH];
    for (int i=0; i < Count; i++)
    {
      strcpy(Str,"Game");
      char* Str2 = inttostr(i+1);
      strcat(Str,Str2);
      delete[] Str2;
      /* Created a new game info */
      GameInfo* NewGame = new GameInfo;
      /* Load the game info from the file */
      GetPrivateProfileString(Str, "Date", "", NewGame->Datetime, 32, FileName);
      GetPrivateProfileString(Str, "WhitePlayer", "", NewGame->WhitePlayerName, 60, FileName);
      GetPrivateProfileString(Str, "BlackPlayer", "", NewGame->BlackPlayerName, 60, FileName);
      GetPrivateProfileString(Str, "WhitePlayerTime", "", NewGame->WhitePlayerTime, 8, FileName);
      GetPrivateProfileString(Str, "BlackPlayerTime", "", NewGame->BlackPlayerTime, 8, FileName);
      GetPrivateProfileString(Str, "MoveCount", "", NewGame->MoveCount, 4, FileName);
      GetPrivateProfileString(Str, "Result", "", NewGame->Result, 32, FileName);
      /* Add the game info to the list */
      GameHistory.Add(NewGame);
    }
    delete[] Str;
  }
  delete[] FileName;
}

static void LoadSettings()
{
  /* Load the settings from file */
  char* FileName = GetApplicationFileName("alphachess.ini");
  char Section[16];
  strcpy(Section, "Window");
  WindowLeft = GetPrivateProfileInt(Section,"Left",20,FileName);
  WindowTop = GetPrivateProfileInt(Section,"Top",20,FileName);
  WindowWidth = GetPrivateProfileInt(Section,"Width",634,FileName);
  WindowHeight = GetPrivateProfileInt(Section,"Height",514,FileName);
  strcpy(Section, "Player");
  GetPrivateProfileString(Section,"Name","",NetworkPlayerName,60,FileName);
  strcpy(Section, "Options");
  AutomaticUpdate = GetPrivateProfileBool(Section,"AutomaticUpdate",false,FileName);
  AlwaysOnTop = GetPrivateProfileBool(Section,"AlwaysOnTop",false,FileName);
  AlwaysVisible = GetPrivateProfileBool(Section,"AlwaysVisible",false,FileName);
  AutomaticallySwitchView = GetPrivateProfileBool(Section,"AutomaticallySwitchView",true,FileName);
  AlwaysPromoteToQueen = GetPrivateProfileBool(Section,"AlwaysPromoteToQueen",false,FileName);
  ChessSetIndex = GetPrivateProfileInt(Section,"ChessSet",2,FileName);
  BoardCoordinatesVisible = GetPrivateProfileBool(Section,"ShowCoordinates",true,FileName);
  BoardLastMoveVisible = GetPrivateProfileBool(Section,"ShowLastMove",true,FileName);
  BoardInvalidMovesVisible = GetPrivateProfileBool(Section,"ShowInvalidMoves",true,FileName);
  PlayersInfoVisible = GetPrivateProfileBool(Section,"PlayersInfoVisible",true,FileName);
  CaptureListVisible = GetPrivateProfileBool(Section,"CaptureListVisible",true,FileName);
  MoveListVisible = GetPrivateProfileBool(Section,"MoveListVisible",true,FileName);
  GetPrivateProfileString(Section,"ColorTheme","Deep Blue Solid",CurrentThemeName,MAX_PATH,FileName);

  /* Clean up */
  delete[] FileName;
}

static void ResizePanels()
{
  RECT R;

  /* Resize the chess board */
  if (hChessBoardPanel != NULL && hChessBoardPanel->GetHandle() != NULL)
  {
    CalculateChessBoardPosition(&R);
    SetWindowPos(hChessBoardPanel->GetHandle(),NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER);
  }

  /* Resize the white player panel */
  if (PlayersInfoVisible && hWhitePlayerPanel != NULL && hWhitePlayerPanel->GetHandle() != NULL)
  {
    CalculateWhitePlayerPosition(&R);
    SetWindowPos(hWhitePlayerPanel->GetHandle(),NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER);
  }

  /* Resize the black player panel */
  if (PlayersInfoVisible && hBlackPlayerPanel != NULL && hBlackPlayerPanel->GetHandle() != NULL)
  {
    CalculateBlackPlayerPosition(&R);
    SetWindowPos(hBlackPlayerPanel->GetHandle(),NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER);
  }

  /* Resize the captured pieces panel */
  if (CaptureListVisible && hCapturePanel != NULL && hCapturePanel->GetHandle() != NULL)
  {
    CalculateCaptureListPosition(&R);
    SetWindowPos(hCapturePanel->GetHandle(),NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER);
  }

  /* Resize the move list panel */
  if (MoveListVisible && hMoveListPanel != NULL && hMoveListPanel->GetHandle() != NULL)
  {
    CalculateMoveListPosition(&R);
    SetWindowPos(hMoveListPanel->GetHandle(),NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER);
  }

  /* Resize the room panel */
  if (NetworkPanelsVisible && hRoomPanel != NULL && hRoomPanel->GetHandle() != NULL)
  {
    CalculateRoomPosition(&R);
    SetWindowPos(hRoomPanel->GetHandle(),NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER);
  }

  /* Resize the chat panel */
  if (NetworkPanelsVisible && hChatPanel != NULL && hChatPanel->GetHandle() != NULL)
  {
    CalculateChatPosition(&R);
    SetWindowPos(hChatPanel->GetHandle(),NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER);
  }
}

static void SaveHistory()
{
  /* Write the number of games in the file */
  char* FileName = GetApplicationFileName("history.ini");
  WritePrivateProfileInteger("GameHistory", "Count", GameHistory.Size(), FileName);
  if (GameHistory.Size() > 0)
  {
    char* Str = new char[MAX_PATH];
    for (int i=0; i < GameHistory.Size(); i++)
    {
      strcpy(Str,"Game");
      char* Str2 = inttostr(i+1);
      strcat(Str,Str2);
      delete[] Str2;
      GameInfo* Ptr = GameHistory.Get(i);
      /* Save the game info in the file */
      WritePrivateProfileString(Str, "Date", Ptr->Datetime, FileName);
      WritePrivateProfileString(Str, "WhitePlayer", Ptr->WhitePlayerName, FileName);
      WritePrivateProfileString(Str, "BlackPlayer", Ptr->BlackPlayerName, FileName);
      WritePrivateProfileString(Str, "WhitePlayerTime", Ptr->WhitePlayerTime, FileName);
      WritePrivateProfileString(Str, "BlackPlayerTime", Ptr->BlackPlayerTime, FileName);
      WritePrivateProfileString(Str, "MoveCount", Ptr->MoveCount, FileName);
      WritePrivateProfileString(Str, "Result", Ptr->Result, FileName);
    }
    delete[] Str;
  }
  delete[] FileName;
}

static void SaveSettings()
{
  /* Save the settings to the file */
  char* FileName = GetApplicationFileName("alphachess.ini");
  char Section[16];
  WINDOWPLACEMENT Placement;
  Placement.length = sizeof(WINDOWPLACEMENT);
  if (GetWindowPlacement(hWindow,&Placement) != 0 && Placement.showCmd == SW_SHOWNORMAL)
  {
    RECT R;
    GetWindowRect(hWindow,&R);
    strcpy(Section, "Window");
    WritePrivateProfileInteger(Section,"Left",R.left,FileName);
    WritePrivateProfileInteger(Section,"Top",R.top,FileName);
    WritePrivateProfileInteger(Section,"Width",R.right-R.left,FileName);
    WritePrivateProfileInteger(Section,"Height",R.bottom-R.top-(NetworkPanelsVisible ? BottomPanelSize : 0),FileName);
  }
  strcpy(Section, "Player");
  WritePrivateProfileString(Section,"Name",NetworkPlayerName,FileName);
  strcpy(Section, "Options");
  WritePrivateProfileBool(Section,"AutomaticUpdate",AutomaticUpdate,FileName);
  WritePrivateProfileBool(Section,"AlwaysOnTop",AlwaysOnTop,FileName);
  WritePrivateProfileBool(Section,"AlwaysVisible",AlwaysVisible,FileName);
  WritePrivateProfileBool(Section,"AutomaticallySwitchView",AutomaticallySwitchView,FileName);
  WritePrivateProfileBool(Section,"AlwaysPromoteToQueen",AlwaysPromoteToQueen,FileName);
  WritePrivateProfileInteger(Section,"ChessSet",ChessSetIndex,FileName);
  WritePrivateProfileBool(Section,"ShowCoordinates",hChessBoardPanel->GetShowCoordinates(),FileName);
  WritePrivateProfileBool(Section,"ShowLastMove",hChessBoardPanel->GetShowLastMove(),FileName);
  WritePrivateProfileBool(Section,"ShowInvalidMoves",hChessBoardPanel->GetShowInvalidMoves(),FileName);
  WritePrivateProfileBool(Section,"PlayersInfoVisible",PlayersInfoVisible,FileName);
  WritePrivateProfileBool(Section,"CaptureListVisible",CaptureListVisible,FileName);
  WritePrivateProfileBool(Section,"MoveListVisible",MoveListVisible,FileName);
  WritePrivateProfileString(Section,"ColorTheme",hChessBoardPanel->GetTheme()->Name.c_str(),FileName);

  /* Clean up */
  delete[] FileName;
}

static void SetAppCursor(HCURSOR Cursor)
{
  if (Cursor != NULL)
  {
    DestroyCursor(SetCursor(Cursor));
    SetCapture(hWindow);
    hAppCursor = Cursor;
  }
}

static void ShowNetworkPanels(bool Value)
{
  /* Resize the window */
  if (Value != NetworkPanelsVisible)
  {
    RECT R;
    GetClientRect(hWindow,&R);
    if (NetworkPanelsVisible)
      R.bottom -= BottomPanelSize;
    if (Value)
      R.bottom += BottomPanelSize;
    AdjustWindowRectEx(&R,GetWindowLong(hWindow,GWL_STYLE),TRUE,GetWindowLong(hWindow,GWL_EXSTYLE));
    SetWindowPos(hWindow,NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER|SWP_NOMOVE);
  }

  /* Show the panel */
  NetworkPanelsVisible = Value;
  ShowWindow(hRoomPanel->GetHandle(),(NetworkPanelsVisible ? SW_SHOW : SW_HIDE));
  ShowWindow(hChatPanel->GetHandle(),(NetworkPanelsVisible ? SW_SHOW : SW_HIDE));
  ResizePanels();
}

static void UpdateDisplay(WPARAM wParam, LPARAM lParam)
{
  // Messages sent from the network interface
  switch (LOWORD(wParam))
  {
    case Disconnected:
    {
      NetworkGame = false;

      /* Close the dialog if we didn't join a game yet */
      CloseNetworkGameDialog();

      /* Cancels the game if it is in progress */
      if (Chess.GetGameState() == Started || Chess.GetGameState() == Paused)
        Chess.PostponeGame();

      /* Hide the buttons and panels */
      ShowNetworkPanels(NetworkGame);
      hWhitePlayerPanel->ShowJoinButton(false);
      hWhitePlayerPanel->ShowReadyButton(false);
      hBlackPlayerPanel->ShowJoinButton(false);
      hBlackPlayerPanel->ShowReadyButton(false);
      break;
    }
    case ConnectionSucceeded:
    {
      ClearAppCursor();
      ShowNetworkGameDialog(hWindow);
      break;
    }
    case ConnectionFailed:
    {
      ClearAppCursor();
      MessageBox(hWindow,"Connection attempt to the game server failed!\nMake sure that you have an active Internet connection and try again.","Connection failed!",MB_OK|MB_ICONERROR);
      break;
    }
    case GameDataReceived:
    {
      Chess.LoadFromImage((ChessGameImage*)lParam);
      break;
    }
    case GameDataRequested:
    {
      /* Create a game image */
      ChessGameImage* Data = new ChessGameImage;
      Chess.SaveToImage(Data);
      /* Send the game image to the server */
      NetworkClient->SendGameData(Data,sizeof(ChessGameImage));
      /* Clean up */
      delete Data;
      break;
    }
    case HostChanged:
    {
      /* Update the menu */
      if (NetworkClient->GetLocalPlayer() == NetworkClient->GetHost())
        hRoomPanel->EnableKickPlayerButton(true);
      else
        hRoomPanel->EnableKickPlayerButton(false);
      break;
    }
    case MessageReceived:
    {
      /* A player sent a message */
      MessageInfo* Message = (MessageInfo*)lParam;
      /* Add the message to the chat panel */
      char* Str = new char[strlen(Message->Message)+61];
      strcpy(Str,Message->PlayerName);
      strcat(Str,": ");
      strcat(Str,Message->Message);
      hChatPanel->AddLine(Str);
      delete[] Str;
      /* Clean up */
      delete[] Message->Message;
      delete Message;
      break;
    }
    case MoveReceived:
    {
      /* A player made a move */
      int FromX = LOBYTE(LOWORD(lParam));
      int FromY = HIBYTE(LOWORD(lParam));
      int ToX = LOBYTE(HIWORD(lParam));
      int ToY = HIBYTE(HIWORD(lParam));
      if (FromX >= 0 && FromX < 8 && FromY >= 0 && FromY < 8 && ToX >= 0 && ToX < 8 && ToY >= 0 && ToY < 8)
      {
        Chess.MovePiece(Position(FromX,FromY),Position(ToX,ToY));
      }
      break;
    }
    case NotificationReceived:
    {
      switch (HIWORD(wParam))
      {
        case JoinedRoom:
        {
          /* Close the dialog if we joined a room */
          CloseNetworkGameDialog();
          NetworkGame = true;

          /* Reset the game */
          Chess.NewGame();
          hChessBoardPanel->SetLocked(true);

          /* Display the bottom panels */
          ShowNetworkPanels(NetworkGame);
          hRoomPanel->Invalidate();

          /* Display buttons */
          hWhitePlayerPanel->ShowJoinButton(true);
          hWhitePlayerPanel->ShowReadyButton(false);
          hBlackPlayerPanel->ShowJoinButton(true);
          hBlackPlayerPanel->ShowReadyButton(false);
          hRoomPanel->EnableLeaveRoomButton(true);
          hRoomPanel->EnableKickPlayerButton(false);

          /* Add a message in the chat panel */
          const char* RoomName = hRoomPanel->GetRoomName();
          char* Str = new char[255];
          strcpy(Str,"You have joined the room: ");
          strcat(Str,RoomName);
          hChatPanel->AddLine(Str);
          delete[] Str;
          break;
        }
        case LeftRoom:
        {
          /* Erase the room information */
          hRoomPanel->Invalidate();

          /* Hide buttons */
          Chess.SetPlayerName(Black,"");
          Chess.SetPlayerName(White,"");
          Chess.NewGame();
          hWhitePlayerPanel->ShowJoinButton(false);
          hWhitePlayerPanel->ShowReadyButton(false);
          hBlackPlayerPanel->ShowJoinButton(false);
          hBlackPlayerPanel->ShowReadyButton(false);
          hRoomPanel->EnableLeaveRoomButton(false);
          hRoomPanel->EnableKickPlayerButton(false);

          /* Add a message in the chat panel */
          const char* RoomName = hRoomPanel->GetRoomName();
          char* Str = new char[255];
          strcpy(Str,"You have left the room: ");
          strcat(Str,RoomName);
          hChatPanel->AddLine(Str);
          delete[] Str;

          /* Update the room list */
          hRoomPanel->SetRoomName(NULL);

          /* Go back to room selection */
          ShowNetworkGameDialog(hWindow);
          break;
        }
        case GameStarted:
        {
          /* Hide buttons */
          hWhitePlayerPanel->ShowJoinButton(false);
          hWhitePlayerPanel->ShowReadyButton(false);
          hBlackPlayerPanel->ShowJoinButton(false);
          hBlackPlayerPanel->ShowReadyButton(false);

          /* Start the game */
          if (Chess.GetGameState() == Paused)
            Chess.ResumeGame();
          else if (Chess.GetGameState() == Created)
            Chess.StartGame();
          else if (Chess.GetGameState() != Started)
          {
            Chess.NewGame();
            Chess.SetGameMode(FreeTime,0);
            Chess.StartGame();
          }
          break;
        }
        case GameDrawed:
        {
          /* Draw the game */
          Chess.DrawGame();
          break;
        }
        case GamePaused:
        {
          /* Pause the game */
          Chess.PauseGame();
          break;
        }
        case GameResumed:
        {
          /* Resume the game */
          Chess.ResumeGame();
          break;
        }
        case Resigned:
        {
          /* Resign the game */
          Chess.ResignGame();
          break;
        }
        case TookbackMove:
        {
          /* Take back the last move */
          Chess.TakeBackMove();

          /* Add a message in the chat panel */
          char* Str = new char[255];
          // TODO set array size = string size
          strcpy(Str,Chess.GetPlayer(!Chess.GetActivePlayer())->Name.c_str());
          strcat(Str," has took back his last move.");
          hChatPanel->AddLine(Str);
          delete[] Str;
          break;
        }
      }
      break;
    }
    case PlayerJoined:
    {
      PlayerInfo* Player = (PlayerInfo*)lParam;
      if (Player != NULL)
      {
        /* Update the player list */
        hRoomPanel->Invalidate();

        /* Add a message in the chat panel */
        char* Str = new char[255];
        strcpy(Str,Player->Name);
        strcat(Str," has joined the room.");
        hChatPanel->AddLine(Str);
        delete[] Str;
      }
      break;
    }
    case PlayerLeft:
    {
      char* PlayerName = (char*)lParam;

      /* Change the game state */
      if (Chess.GetGameState() == Started && (NetworkClient->GetWhitePlayer() == NULL || NetworkClient->GetBlackPlayer() == NULL))
        Chess.PauseGame();

      /* Clear player name */
      if (NetworkClient->GetBlackPlayer() == NULL)
      {
        Chess.SetPlayerName(Black, "");
        hBlackPlayerPanel->ShowJoinButton(true);
        hBlackPlayerPanel->ShowReadyButton(false);
      }
      if (NetworkClient->GetWhitePlayer() == NULL)
      {
        Chess.SetPlayerName(White, "");
        hWhitePlayerPanel->ShowJoinButton(true);
        hWhitePlayerPanel->ShowReadyButton(false);
      }

      /* Update the room information */
      hRoomPanel->Invalidate();

      /* Add a message in the chat panel */
      char* Str = new char[255];
      strcpy(Str,PlayerName);
      strcat(Str," has left the room.");
      hChatPanel->AddLine(Str);
      delete[] Str;
      delete[] PlayerName;
      break;
    }
    case PlayerNameChanged:
    {
      PlayerInfo* Player = (PlayerInfo*)lParam;

      /* Update the player name */
      if (Player == NetworkClient->GetBlackPlayer())
        Chess.SetPlayerName(Black, Player->Name);
      else if (Player == NetworkClient->GetWhitePlayer())
        Chess.SetPlayerName(White, Player->Name);

      /* Update the player list */
      hRoomPanel->Invalidate();
      break;
    }
    case PlayerStateChanged:
    {
      PlayerInfo* Player = (PlayerInfo*)lParam;

      /* Update the player state */
      if (Player->Ready)
      {
        if (Player == NetworkClient->GetBlackPlayer())
        {
          hBlackPlayerPanel->SetState(Ready);
          hBlackPlayerPanel->ShowReadyButton(false);
        }
        else if (Player == NetworkClient->GetWhitePlayer())
        {
          hWhitePlayerPanel->SetState(Ready);
          hWhitePlayerPanel->ShowReadyButton(false);
        }
      }
      else
      {
        /* Change the player's state */
        if (Player == NetworkClient->GetBlackPlayer())
          hBlackPlayerPanel->SetState(0);
        else if (Player == NetworkClient->GetWhitePlayer())
          hWhitePlayerPanel->SetState(0);
        /* Show the player's ready button */
        if (NetworkClient->GetWhitePlayer() != NULL && NetworkClient->GetBlackPlayer() != NULL)
        {
          hBlackPlayerPanel->ShowReadyButton(NetworkClient->GetLocalPlayer() == NetworkClient->GetBlackPlayer());
          hWhitePlayerPanel->ShowReadyButton(NetworkClient->GetLocalPlayer() == NetworkClient->GetWhitePlayer());
        }
      }
      break;
    }
    case PlayerTimeChanged:
    {
      PlayerInfo* Player = (PlayerInfo*)lParam;
      if (Player == NetworkClient->GetBlackPlayer())
        Chess.SetPlayerTime(Black,Player->Time);
      else if (Player == NetworkClient->GetWhitePlayer())
        Chess.SetPlayerTime(White,Player->Time);
      break;
    }
    case PlayerTypeChanged:
    {
      PlayerType OldType = (PlayerType)HIWORD(wParam);
      PlayerInfo* Player = (PlayerInfo*)lParam;

      /* Clear player name */
      if (OldType == BlackPlayerType)
      {
        Chess.SetPlayerName(Black,"");
        hBlackPlayerPanel->SetState(0);
        hBlackPlayerPanel->ShowJoinButton(true);
        hBlackPlayerPanel->ShowReadyButton(false);
      }
      else if (OldType == WhitePlayerType)
      {
        Chess.SetPlayerName(White,"");
        hWhitePlayerPanel->SetState(0);
        hWhitePlayerPanel->ShowJoinButton(true);
        hWhitePlayerPanel->ShowReadyButton(false);
      }

      /* Update the player name */
      if (Player == NetworkClient->GetBlackPlayer())
      {
        Chess.SetPlayerName(Black,Player->Name);
        hBlackPlayerPanel->ShowLeaveButton(Player == NetworkClient->GetLocalPlayer());
      }
      else if (Player == NetworkClient->GetWhitePlayer())
      {
        Chess.SetPlayerName(White,Player->Name);
        hWhitePlayerPanel->ShowLeaveButton(Player == NetworkClient->GetLocalPlayer());
      }

      /* Update the player list */
      hRoomPanel->Invalidate();
      break;
    }
    case PromotionReceived:
    {
      /* Promote the last moved piece */
      Chess.PromotePawnTo((ChessPieceType)lParam);
      break;
    }
    case RequestReceived:
    {
      if (NetworkClient->GetLocalPlayer() == NetworkClient->GetBlackPlayer() || NetworkClient->GetLocalPlayer() == NetworkClient->GetWhitePlayer())
        switch (HIWORD(wParam))
        {
          case DrawRequest:
          {
            /* Add a message in the chat panel */
            char* Str = new char[250];
            strcpy(Str,"Your opponent is offering you to draw this game.");
            hChatPanel->AddLine(Str);
            delete[] Str;
            int Result = MessageBox(hWindow, "Your opponent wants to draw this game. Do you accept?", "Accept request to draw the game?", MB_YESNO|MB_ICONQUESTION);
            NetworkClient->AcceptDrawRequest(Result == IDYES);
            break;
          }
          case TakebackRequest:
          {
            /* Add a message in the chat panel */
            char* Str = new char[250];
            strcpy(Str,"Your opponent has requested to take back his last move.");
            hChatPanel->AddLine(Str);
            delete[] Str;
            int Result = MessageBox(hWindow, "Your opponent wants to take back his last move. Do you accept?", "Accept request to take back last move?", MB_YESNO|MB_ICONQUESTION);
            NetworkClient->AcceptTakebackRequest(Result == IDYES);
            break;
          }
        }
      break;
    }
    case RoomInfoReceived:
    {
      /* The server sent information on a room */
      const RoomInfo* Room = (RoomInfo*)lParam;
      AddRoomToList(Room);
      /* Clean up */
      delete Room;
      break;
    }
  }
}

// Public functions ------------------------------------------------------------

void CloseApplication()
{
  DestroyWindow(hWindow);
}

void DrawGame()
{
  if (NetworkGame)
  {
    if ((Chess.GetActivePlayer() == White && NetworkClient->GetLocalPlayer() == NetworkClient->GetWhitePlayer()) || (Chess.GetActivePlayer() == Black && NetworkClient->GetLocalPlayer() == NetworkClient->GetBlackPlayer()))
      NetworkClient->RequestDrawGame();
    else
      MessageBox(hWindow,"You can only claim a draw during your turn!","Cannot claim a draw at this time!",MB_OK|MB_ICONEXCLAMATION);
  }
  else
    Chess.DrawGame();
}

LinkedList<string>* GetThemes()
{
  LinkedList<string>* List = new LinkedList<string>;
  /* Get the themes in the folder */
  WIN32_FIND_DATA FindData;
  char* Path = GetApplicationFileName("Themes\\*.ini");
  HANDLE Handle = FindFirstFile(Path, &FindData);
  if (Handle != INVALID_HANDLE_VALUE)
  {
    do if (FindData.cFileName[0] != '.' && !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
      char* Str = substr(FindData.cFileName, 0, strrpos(FindData.cFileName,"."));
      if (Str != NULL)
        List->Add(new string(Str));
      delete[] Str;
    }
    while (FindNextFile(Handle, &FindData));
    FindClose(Handle);
  }
  delete[] Path;
  return List;
}

void LoadChessSet(int Index)
{
  if (Index >= 0 && Index < ChessSets.Size())
  {
    /* Unregister the last font */
    ChessSet* CurrentChessSet = ChessSets.Get(ChessSetIndex);
    if (CurrentChessSet != NULL)
    {
      char* Str = new char[MAX_PATH];
      strcpy(Str,"ChessSets\\");
      strcat(Str,CurrentChessSet->FileName);
      RemoveFontResource(Str);
      delete[] Str;
    }

    /* Register the new font */
    ChessSetIndex = Index;
    CurrentChessSet = ChessSets.Get(ChessSetIndex);
    char* Str = new char[MAX_PATH];
    strcpy(Str,"ChessSets\\");
    strcat(Str,CurrentChessSet->FileName);
    AddFontResource(Str);
    delete[] Str;

    /* Update the interface */
    hChessBoardPanel->SetChessSet(CurrentChessSet);
    hWhitePlayerPanel->SetChessSet(CurrentChessSet);
    hBlackPlayerPanel->SetChessSet(CurrentChessSet);
    hCapturePanel->SetChessSet(CurrentChessSet);
  }
}

void LoadGame()
{
}

void LoadTheme(char* ThemeName)
{
  /* Load the theme */
  char* Str = new char[strlen(ThemeName)+12];
  strcpy(Str, "Themes\\");
  strcat(Str, ThemeName);
  strcat(Str, ".ini");
  char* FileName = GetApplicationFileName(Str);
  delete[] Str;
  ChessBoardTheme* Theme = new ChessBoardTheme();
  Theme->LoadFromFile(FileName);
  delete[] FileName;

  /* Apply the theme to the chess board */
  delete hChessBoardPanel->GetTheme();
  hChessBoardPanel->SetTheme(Theme);
}

void PauseGame(bool Value)
{
  if (!NetworkGame)
  {
    if (Value)
      Chess.PauseGame();
    else
      Chess.ResumeGame();
  }
  else if ((Chess.GetActivePlayer() == White && NetworkClient->GetLocalPlayer() == NetworkClient->GetWhitePlayer()) || (Chess.GetActivePlayer() == Black && NetworkClient->GetLocalPlayer() == NetworkClient->GetBlackPlayer()))
  {
    if (Value)
      NetworkClient->PauseGame();
    else
      NetworkClient->ResumeGame();
  }
  else
    MessageBox(hWindow,"You can only pause the game during your turn!","Cannot pause the game at this time!",MB_OK|MB_ICONEXCLAMATION);
}

void PlayLocalGame()
{
  /* Show the local game dialog */
  ShowLocalGameDialog(hWindow);
}

void PlayNetworkGame()
{
  /* Show the network game dialog if connected, otherwise connect */
  if (NetworkClient->IsConnected())
    ShowNetworkGameDialog(hWindow);
  else
  {
    NetworkClient->Connect(ServerAddress);
    SetAppCursor(LoadCursor(NULL,IDC_WAIT));
  }
}

void ResignGame()
{
  if (NetworkGame)
  {
    if ((Chess.GetActivePlayer() == White && NetworkClient->GetLocalPlayer() == NetworkClient->GetWhitePlayer()) || (Chess.GetActivePlayer() == Black && NetworkClient->GetLocalPlayer() == NetworkClient->GetBlackPlayer()))
      NetworkClient->Resign();
    else
      MessageBox(hWindow,"You can only resign during your turn!","Cannot resign at this time!",MB_OK|MB_ICONEXCLAMATION);
  }
  else
    Chess.ResignGame();
}

void SaveGame()
{
  string Filename;
  if (SaveFileAsDialog(hWindow, Filename, NULL, "pgn", "Portable Game Notation (*.pgn)\0*.pgn\0", 0))
  {
    Chess.SaveToFile(Filename.c_str());
  }
  else
  {
    MessageBox(hWindow,inttostr(CommDlgExtendedError()),"",MB_OK);
    //MessageBox(hWindow,inttostr(CDERR_DIALOGFAILURE),"",MB_OK);
    //DWORD b = GetLastError();
    //LPVOID lpMsgBuf;
    //FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, b, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
    //MessageBox(NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK);
    //LocalFree(lpMsgBuf);
  }
}

void SetAlwaysOnTop(bool Value)
{
  AlwaysOnTop = Value;
  SetWindowPos(hWindow,(AlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST),0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
}

void ShowAbout()
{
  /* Show dialog */
  ShowAboutDialog(hWindow);
}

void ShowPlayersInfo(bool Value)
{
  if (Value != PlayersInfoVisible)
  {
    /* Calculate new window size */
    RECT R;
    GetClientRect(hWindow,&R);
    if (PlayersInfoVisible || CaptureListVisible || MoveListVisible)
      R.right -= LeftPanelSize;
    if (Value || CaptureListVisible || MoveListVisible)
      R.right += LeftPanelSize;

    /* Resize the window */
    PlayersInfoVisible = Value;
    AdjustWindowRectEx(&R,GetWindowLong(hWindow,GWL_STYLE),TRUE,GetWindowLong(hWindow,GWL_EXSTYLE));
    SetWindowPos(hWindow,NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER|SWP_NOMOVE);
    ResizePanels();
  }

  /* Show the panel */
  ShowWindow(hWhitePlayerPanel->GetHandle(),(PlayersInfoVisible ? SW_SHOW : SW_HIDE));
  ShowWindow(hBlackPlayerPanel->GetHandle(),(PlayersInfoVisible ? SW_SHOW : SW_HIDE));

  /* Check menu item */
  CheckMenuItem(ViewMenu(), IDS_MENU_VIEW_PLAYERS, MF_BYCOMMAND | (PlayersInfoVisible ? MF_CHECKED : MF_UNCHECKED));
}

void ShowPreferences()
{
  /* Show dialog */
  ShowPreferencesDialog(hWindow);

  /* Save the preferences */
  SaveSettings();
}

void ShowCaptureList(bool Value)
{
  if (Value != CaptureListVisible)
  {
    /* Calculate new window size */
    RECT R;
    GetClientRect(hWindow,&R);
    if (PlayersInfoVisible || CaptureListVisible || MoveListVisible)
      R.right -= LeftPanelSize;
    if (Value || PlayersInfoVisible || MoveListVisible)
      R.right += LeftPanelSize;

    /* Resize the window */
    CaptureListVisible = Value;
    AdjustWindowRectEx(&R,GetWindowLong(hWindow,GWL_STYLE),TRUE,GetWindowLong(hWindow,GWL_EXSTYLE));
    SetWindowPos(hWindow,NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER|SWP_NOMOVE);
    ResizePanels();
  }
  /* Show the panel */
  ShowWindow(hCapturePanel->GetHandle(),(CaptureListVisible ? SW_SHOW : SW_HIDE));

  /* Check menu item */
  CheckMenuItem(ViewMenu(), IDS_MENU_VIEW_CAPTURES, MF_BYCOMMAND | (CaptureListVisible ? MF_CHECKED : MF_UNCHECKED));
}

void ShowMoveList(bool Value)
{
  if (Value != MoveListVisible)
  {
    /* Calculate new window size */
    RECT R;
    GetClientRect(hWindow,&R);
    if (PlayersInfoVisible || CaptureListVisible || MoveListVisible)
      R.right -= LeftPanelSize;
    if (Value || PlayersInfoVisible || CaptureListVisible)
      R.right += LeftPanelSize;

    /* Resize the window */
    MoveListVisible = Value;
    AdjustWindowRectEx(&R,GetWindowLong(hWindow,GWL_STYLE),TRUE,GetWindowLong(hWindow,GWL_EXSTYLE));
    SetWindowPos(hWindow,NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER|SWP_NOMOVE);
    ResizePanels();
  }
  /* Show the panel */
  ShowWindow(hMoveListPanel->GetHandle(),(MoveListVisible ? SW_SHOW : SW_HIDE));

  /* Check menu item */
  CheckMenuItem(ViewMenu(), IDS_MENU_VIEW_MOVES, MF_BYCOMMAND | (MoveListVisible ? MF_CHECKED : MF_UNCHECKED));
}

void ShowGameHistory()
{
  /* Show dialog */
  ShowGameHistoryDialog(hWindow);

  /* Save the history */
  SaveHistory();
}

void SwitchView(bool Value)
{
  /* Invert the chess board view */
  hChessBoardPanel->SetInvertView(Value);
  /* Check menu item */
  CheckMenuItem(ViewMenu(), IDS_MENU_VIEW_SWITCH, MF_BYCOMMAND | (Value ? MF_CHECKED : MF_UNCHECKED));
}

void TakebackMove()
{
  if (NetworkGame)
  {
    if ((Chess.GetActivePlayer() == White && NetworkClient->GetLocalPlayer() == NetworkClient->GetBlackPlayer()) || (Chess.GetActivePlayer() == Black && NetworkClient->GetLocalPlayer() == NetworkClient->GetWhitePlayer()))
      NetworkClient->RequestTakeback();
    else
      MessageBox(hWindow,"You can only take back your move during your opponent's turn!","Cannot take back move at this time!",MB_OK|MB_ICONEXCLAMATION);
  }
  else
    Chess.TakeBackMove();
}

// Call back functions ----------------------------------------------------------
static void __stdcall UpdateTime(HWND hWnd, UINT Msg, UINT_PTR TimerID, DWORD TickCount);

// Called from the chess engine when the active player changed.
static void __stdcall ActivePlayerChanged(ChessPieceColor ActivePlayer)
{
  if (NetworkGame)
  {
    /* Lock the chess board panel if it is not current player's turn */
    if (NetworkClient->GetLocalPlayer() != NULL && NetworkClient->GetBlackPlayer() != NULL && NetworkClient->GetWhitePlayer() != NULL)
    {
      if ((NetworkClient->GetLocalPlayer() == NetworkClient->GetBlackPlayer() && ActivePlayer == Black) || (NetworkClient->GetLocalPlayer() == NetworkClient->GetWhitePlayer() && ActivePlayer == White))
      {
        hChessBoardPanel->SetLocked(false);

        /* Start the timer for the current player */
        LastTickCount = GetTickCount();
        SetTimer(hWindow, ChessTimer, 250, UpdateTime);
      }
      else
        hChessBoardPanel->SetLocked(true);
    }
  }
  else if ((WhitePlayerIsComputer && ActivePlayer == White) || (BlackPlayerIsComputer && ActivePlayer == Black))
  {
    /* Lock the chess board */
    hChessBoardPanel->SetLocked(true);

    /* Find a move and make it */
    ChessAiMove* Move = Chess.FindBestMove(6, 100);
    if (Move != NULL)
    {
      Chess.MovePiece(Move->From,Move->To);
      delete Move;
    }
  }
  else
  {
    /* Unlock the chess board */
    hChessBoardPanel->SetLocked(false);

    /* Start the timer for the current player */
    LastTickCount = GetTickCount();
    SetTimer(hWindow, ChessTimer, 250, UpdateTime);
  }

  /* Update the players information */
  if (ActivePlayer == White)
    hBlackPlayerPanel->SetState(0);
  else if (ActivePlayer == Black)
    hWhitePlayerPanel->SetState(0);
  hWhitePlayerPanel->Invalidate();
  hBlackPlayerPanel->Invalidate();

  /* Switch the view */
  if (AutomaticallySwitchView && !NetworkGame && !WhitePlayerIsComputer && !BlackPlayerIsComputer)
    SwitchView(ActivePlayer == Black);
}

// Called from the player info panels when the button has been clicked.
static void __stdcall JoinButtonClicked(ChessPieceColor Color)
{
  if (Color == Black)
  {
    if (NetworkClient->GetBlackPlayer() == NULL)
    {
      /* Join the game as Black player */
      NetworkClient->SetPlayerType(BlackPlayerType);
      hBlackPlayerPanel->ShowLeaveButton(true);
      SwitchView(true);
    }
    else if (NetworkClient->GetBlackPlayer() == NetworkClient->GetLocalPlayer())
      /* Leave the game */
      NetworkClient->SetPlayerType(ObserverType);
  }
  else
  {
    if (NetworkClient->GetWhitePlayer() == NULL)
    {
      /* Join the game as White player */
      NetworkClient->SetPlayerType(WhitePlayerType);
      hWhitePlayerPanel->ShowLeaveButton(true);
      SwitchView(false);
    }
    else if (NetworkClient->GetWhitePlayer() == NetworkClient->GetLocalPlayer())
      /* Leave the game */
      NetworkClient->SetPlayerType(ObserverType);
  }
}

// Called from the room panel when the button has been clicked.
static void __stdcall KickPlayerButtonClicked()
{
  /* Kick selected player from the game room */
  unsigned int PlayerId = hRoomPanel->GetSelectedPlayer();
  if (PlayerId > 0 && PlayerId != NetworkClient->GetLocalPlayer()->PlayerId)
    if (MessageBox(hWindow,"Are you sure that you wish to kick the selected player from game?", "Kick the selected player from game?", MB_YESNO|MB_ICONQUESTION) == IDYES)
        NetworkClient->KickPlayer(PlayerId);
}

// Called from the room panel when the button has been clicked.
static void __stdcall LeaveRoomButtonClicked()
{
  /* Exit the game room */
  if (MessageBox(hWindow,"Are you sure that you wish to leave the game?", "Leave the game?", MB_YESNO|MB_ICONQUESTION) == IDYES)
    NetworkClient->LeaveRoom();
}

// Called from the chess engine when the game state changed.
static void __stdcall GameStateChanged(GameState PreviousState, GameState CurrentState)
{
  if (CurrentState == Created)
  {
    /* Reset panels */
    hCapturePanel->Invalidate();
    hChessBoardPanel->Invalidate();
    hMoveListPanel->Invalidate();
  }
  else if (CurrentState == Started)
  {
    if (PreviousState != Paused)
    {
      /* Enable menu items */
      EnableMenuItem(GameMenu(),IDS_MENU_GAME_PAUSE,MF_BYCOMMAND|MF_ENABLED);
      EnableMenuItem(GameMenu(),IDS_MENU_GAME_TAKEBACKMOVE,MF_BYCOMMAND|MF_ENABLED);
      EnableMenuItem(GameMenu(),IDS_MENU_GAME_OFFERDRAW,MF_BYCOMMAND|MF_ENABLED);
      EnableMenuItem(GameMenu(),IDS_MENU_GAME_RESIGN,MF_BYCOMMAND|MF_ENABLED);

      /* Add a message in the chat panel */
      if (NetworkGame)
      {
        char* Str = new char[250];
        strcpy(Str,"The game has started.");
        hChatPanel->AddLine(Str);
        delete[] Str;
      }
    }
    else if (NetworkGame)
    {
      /* Reset player status */
      hBlackPlayerPanel->SetState(0);
      hWhitePlayerPanel->SetState(0);

      /* Hide buttons */
      if (NetworkClient->GetLocalPlayer() == NetworkClient->GetBlackPlayer())
        hBlackPlayerPanel->ShowLeaveButton(false);
      else if (NetworkClient->GetLocalPlayer() == NetworkClient->GetWhitePlayer())
        hWhitePlayerPanel->ShowLeaveButton(false);

      /* Add a message in the chat panel */
      char* Str = new char[250];
      strcpy(Str,"The game has resumed.");
      hChatPanel->AddLine(Str);
      delete[] Str;
    }

    /* Unlock the chess board */
    hChessBoardPanel->SetPaused(false);

    /* Uncheck menu items */
    CheckMenuItem(GameMenu(),IDS_MENU_GAME_PAUSE,MF_BYCOMMAND|MF_UNCHECKED);
  }
  else if (CurrentState == Paused)
  {
    if (NetworkGame)
    {
      /* Show buttons */
      if (NetworkClient->GetLocalPlayer() == NetworkClient->GetBlackPlayer())
        hBlackPlayerPanel->ShowLeaveButton(true);
      else if (NetworkClient->GetLocalPlayer() == NetworkClient->GetWhitePlayer())
        hWhitePlayerPanel->ShowLeaveButton(true);

      /* Add a message in the chat panel */
      char* Str = new char[250];
      strcpy(Str,"The game has paused.");
      hChatPanel->AddLine(Str);
      delete[] Str;
    }

    /* Lock the chess board */
    hChessBoardPanel->SetPaused(true);

    /* Check menu items */
    CheckMenuItem(GameMenu(),IDS_MENU_GAME_PAUSE,MF_BYCOMMAND|MF_CHECKED);
  }
  else if (CurrentState == EndedInCheckMate || CurrentState == EndedInStaleMate || CurrentState == EndedInDraw || CurrentState == EndedInForfeit || CurrentState == EndedInTimeout)
  {
    if (NetworkGame)
    {
      /* Notify the server */
      NetworkClient->GameHasEnded();
      /* Show buttons */
      if (NetworkClient->GetLocalPlayer() == NetworkClient->GetBlackPlayer())
      {
        hBlackPlayerPanel->ShowLeaveButton(true);
        hBlackPlayerPanel->ShowReadyButton(true);
      }
      else if (NetworkClient->GetLocalPlayer() == NetworkClient->GetWhitePlayer())
      {
        hWhitePlayerPanel->ShowLeaveButton(true);
        hWhitePlayerPanel->ShowReadyButton(true);
      }
      /* Add a message in the chat panel */
      if (CurrentState == EndedInDraw)
      {
        char* Str = new char[250];
        strcpy(Str,"The game has ended in a draw.");
        hChatPanel->AddLine(Str);
        delete[] Str;
      }
      else if (CurrentState == EndedInForfeit)
      {
        char* Str = new char[255];
        // TODO Initialize the array to the str size;
        strcpy(Str,Chess.GetPlayer(!Chess.GetActivePlayer())->Name.c_str());
        strcat(Str," has resigned.");
        hChatPanel->AddLine(Str);
        delete[] Str;
      }
    }

    /* Lock the chess board */
    hChessBoardPanel->SetLocked(true);

    /* Disable menu items */
    EnableMenuItem(GameMenu(),IDS_MENU_GAME_TAKEBACKMOVE,MF_BYCOMMAND|MF_GRAYED);
    EnableMenuItem(GameMenu(),IDS_MENU_GAME_PAUSE,MF_BYCOMMAND|MF_GRAYED);
    EnableMenuItem(GameMenu(),IDS_MENU_GAME_OFFERDRAW,MF_BYCOMMAND|MF_GRAYED);
    EnableMenuItem(GameMenu(),IDS_MENU_GAME_RESIGN,MF_BYCOMMAND|MF_GRAYED);

    /* Uncheck menu items */
    CheckMenuItem(GameMenu(),IDS_MENU_GAME_PAUSE,MF_BYCOMMAND|MF_UNCHECKED);

    /* Add the game to the history */
    GameInfo* Game = new GameInfo;
    SYSTEMTIME Time;
    GetSystemTime(&Time);
    char* Str = FormatDateTime(&Time, 0, "dd/MM/yyyy", TIME_NOSECONDS | TIME_NOTIMEMARKER | TIME_FORCE24HOURFORMAT, NULL);
    strcpy(Game->Datetime,Str);
    delete[] Str;
    strcpy(Game->WhitePlayerName,Chess.GetPlayer(White)->Name.c_str());
    strcpy(Game->BlackPlayerName,Chess.GetPlayer(Black)->Name.c_str());
    Str = inttostr(Chess.GetPlayer(White)->Time);
    strcpy(Game->WhitePlayerTime,Str);
    delete[] Str;
    Str = inttostr(Chess.GetPlayer(Black)->Time);
    strcpy(Game->BlackPlayerTime,Str);
    delete[] Str;
    Str = inttostr(Chess.GetMoveCount());
    strcpy(Game->MoveCount,Str);
    delete[] Str;
    if (CurrentState == EndedInCheckMate)
    {
      if (Chess.GetActivePlayer() == White)
        strcpy(Game->Result, "Checkmate for white player");
      else
        strcpy(Game->Result, "Checkmate for black player");
    }
    else if (CurrentState == EndedInStaleMate)
    {
      if (Chess.GetActivePlayer() == White)
        strcpy(Game->Result, "Stalemate for white player");
      else
        strcpy(Game->Result, "Stalemate for black player");
    }
    else if (CurrentState == EndedInDraw)
      strcpy(Game->Result,"Draw");
    else if (CurrentState == EndedInForfeit)
    {
      if (Chess.GetActivePlayer() == White)
        strcpy(Game->Result, "Black player resigned");
      else
        strcpy(Game->Result, "White player resigned");
    }
    else if (CurrentState == EndedInTimeout)
    {
      if (Chess.GetActivePlayer() == White)
        strcpy(Game->Result, "Time out for black player");
      else
        strcpy(Game->Result, "Time out for white player");
    }
    GameHistory.Add(Game);
    SaveHistory();
  }
  else if (CurrentState == Postponed)
  {
    /* Lock the chess board */
    hChessBoardPanel->SetLocked(true);

    /* Disable menu items */
    EnableMenuItem(GameMenu(),IDS_MENU_GAME_TAKEBACKMOVE,MF_BYCOMMAND|MF_GRAYED);
    EnableMenuItem(GameMenu(),IDS_MENU_GAME_PAUSE,MF_BYCOMMAND|MF_GRAYED);
    EnableMenuItem(GameMenu(),IDS_MENU_GAME_OFFERDRAW,MF_BYCOMMAND|MF_GRAYED);
    EnableMenuItem(GameMenu(),IDS_MENU_GAME_RESIGN,MF_BYCOMMAND|MF_GRAYED);

    /* Uncheck menu items */
    CheckMenuItem(GameMenu(),IDS_MENU_GAME_PAUSE,MF_BYCOMMAND|MF_UNCHECKED);
  }
}

// Called from the chess board panel when a piece has to be moved.
static bool __stdcall MovePiece(Position From, Position To)
{
  /* Stop the timer for the current player */
  KillTimer(hWindow, ChessTimer);
  UpdateTime(hWindow, WM_TIMER, ChessTimer, GetTickCount());
  /* Make the move */
  if (NetworkGame)
  {
    if (Chess.IsMoveValid(From,To))
      NetworkClient->MakeMove(From.x,From.y,To.x,To.y);
    return false;
  }
  else
  {
    Chess.MovePiece(From,To);
    return true;
  }
}

// Called from the chess engine when a piece was moved.
static void __stdcall PieceMoved(int Result)
{
  /* Refresh the panels */
  hChessBoardPanel->Invalidate();
  hCapturePanel->Invalidate();
  hMoveListPanel->Invalidate();
  PostMessage(hMoveListPanel->GetHandle(),WM_VSCROLL,MAKEWPARAM(SB_BOTTOM,0),0);
  if (Chess.GetActivePlayer() == White)
    hBlackPlayerPanel->SetState(Result);
  else
    hWhitePlayerPanel->SetState(Result);

  /* Add a message in the chat panel */
  if (NetworkGame)
  {
    if (Result & Mated && Result & Checked)
    {
      char* Str = new char[255];
      strcpy(Str,Chess.GetPlayer(!Chess.GetActivePlayer())->Name.c_str());
      strcat(Str," is checkmate.");
      hChatPanel->AddLine(Str);
      delete[] Str;
    }
    else if (Result & Mated)
    {
      char* Str = new char[255];
      strcpy(Str,Chess.GetPlayer(!Chess.GetActivePlayer())->Name.c_str());
      strcat(Str," is stalemate.");
      hChatPanel->AddLine(Str);
      delete[] Str;
    }
    else if (Result & Checked)
    {
      char* Str = new char[255];
      strcpy(Str,Chess.GetPlayer(!Chess.GetActivePlayer())->Name.c_str());
      strcat(Str," is in check.");
      hChatPanel->AddLine(Str);
      delete[] Str;
    }
  }

  /* Promote the chess piece */
  if (Result & Promoted && (!NetworkGame || (Chess.GetActivePlayer() == White && NetworkClient->GetLocalPlayer() == NetworkClient->GetWhitePlayer()) || (Chess.GetActivePlayer() == Black && NetworkClient->GetLocalPlayer() == NetworkClient->GetBlackPlayer())))
  {
    ChessPieceType Type = Queen;
    if (!AlwaysPromoteToQueen)
      Type = (ChessPieceType)ShowPromotionDialog(hWindow, ChessSets.Get(ChessSetIndex));
    if (NetworkGame)
      NetworkClient->PromoteTo(Type);
    else
      Chess.PromotePawnTo(Type);
  }
}

// Called from the chess engine when the information on a player changed.
static void __stdcall PlayerInfoChanged(ChessPieceColor Color)
{
  if (Color == White)
    hWhitePlayerPanel->Invalidate();
  else
    hBlackPlayerPanel->Invalidate();
}

// Called from the player info panels when the button has been clicked.
static void __stdcall ReadyButtonClicked(ChessPieceColor Color)
{
  NetworkClient->SetPlayerReady();
}

// Called from the chat panel when a message should be sent.
static void __stdcall SendText(char* Str)
{
  NetworkClient->SendText(Str);
}

// Called from the Windows timer.
static void __stdcall UpdateTime(HWND hWnd, UINT Msg, UINT_PTR TimerID, DWORD TickCount)
{
  if (TimerID == ChessTimer)
  {
    unsigned long Time = (TickCount >= LastTickCount ? TickCount-LastTickCount : (UINT_MAX-LastTickCount)+TickCount);
    LastTickCount = TickCount;
    if (NetworkGame)
    {
      Time = Chess.GetPlayer(Chess.GetActivePlayer())->Time + Time;
      NetworkClient->SendTime(Time);
      Chess.SetPlayerTime(Chess.GetActivePlayer(), Time);
    }
    else
      Chess.UpdateGameTime(Time);
  }
}

// WinAPI functions ------------------------------------------------------------

static LRESULT __stdcall WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
    case WM_CLOSE:
    {
      /* Close confirmation */
      int Result = IDOK;
      if (Chess.GetGameState() == Started || Chess.GetGameState() == Paused)
      {
        if (NetworkGame)
          Result = MessageBox(hWindow,"You have a game in progress, are you sure that you want to close AlphaChess?","Close?",MB_OKCANCEL|MB_ICONQUESTION);
        else
          Result = MessageBox(hWindow,"You have a game in progress, do you want to save it before closing AlphaChess?","Close?",MB_YESNOCANCEL|MB_ICONQUESTION);
      }
      /* Save game */
      if (Result == IDYES)
        SaveGame();
      /* Close application */
      if (Result != IDCANCEL)
        DestroyWindow(hWnd);
      return 0;
    }
    case WM_COMMAND:
    {
      /* Process the menu and accelerator messages */
      if (lParam == 0)
        MenuProc((short)LOWORD(wParam));
      return 0;
    }
    case WM_CONTEXTMENU:
    {
      HMENU Menu = NULL;
      //if ((HWND)wParam == hRoomPanel->GetHandle())
      //  Menu = RoomMenu();
      if (Menu != NULL)
      {
        int x = (short)LOWORD(lParam);
        int y = (short)HIWORD(lParam);
        if (x < 0 || y < 0)
        {
          RECT R;
          GetWindowRect((HWND)wParam, &R);
          x = R.left+2;
          y = R.top+2;
        }
        TrackPopupMenu(Menu, TPM_LEFTALIGN, x, y, 0, hWnd, NULL);
      }
      return 0;
    }
    case WM_CREATE:
    {
      RECT R;

      /* Initialise the game objects */
      Chess.ActivePlayerChangedProc = ActivePlayerChanged;
      Chess.GameStateChangedProc = GameStateChanged;
      Chess.PieceMovedProc = PieceMoved;
      Chess.PlayerInfoChangedProc = PlayerInfoChanged;
      NetworkClient = new GameClient(hWnd);

      /* Create the chess board panel */
      CalculateChessBoardPosition(&R);
      hChessBoardPanel = new ChessBoardPanel(hWnd, &R);
      hChessBoardPanel->MovePieceProc = MovePiece;
      hChessBoardPanel->SetStandardCursor(LoadCursor(GetModuleHandle(NULL), "IDI_CURSORHAND"));
      hChessBoardPanel->SetSelectionCursor(LoadCursor(GetModuleHandle(NULL), "IDI_CURSORGRAB"));

      /* Create the white player panel */
      CalculateWhitePlayerPosition(&R);
      hWhitePlayerPanel = new PlayerPanel(hWnd, &R, White);
      hWhitePlayerPanel->JoinButtonClickedProc = JoinButtonClicked;
      hWhitePlayerPanel->ReadyButtonClickedProc = ReadyButtonClicked;

      /* Create the black player panel */
      CalculateBlackPlayerPosition(&R);
      hBlackPlayerPanel = new PlayerPanel(hWnd, &R, Black);
      hBlackPlayerPanel->JoinButtonClickedProc = JoinButtonClicked;
      hBlackPlayerPanel->ReadyButtonClickedProc = ReadyButtonClicked;

      /* Create the captured pieces panel */
      CalculateCaptureListPosition(&R);
      hCapturePanel = new CapturePanel(hWnd, &R);

      /* Create the move list panel */
      CalculateMoveListPosition(&R);
      hMoveListPanel = new MoveListPanel(hWnd, &R);

      /* Create the room panel */
      CalculateRoomPosition(&R);
      hRoomPanel = new RoomPanel(hWnd, &R);
      hRoomPanel->KickPlayerButtonClickedProc = KickPlayerButtonClicked;
      hRoomPanel->LeaveRoomButtonClickedProc = LeaveRoomButtonClicked;

      /* Create the chat panel */
      CalculateChatPosition(&R);
      hChatPanel = new ChatPanel(hWnd, &R);
      hChatPanel->SendTextProc = SendText;

      /* Create the main menu */
      EnableMenuItem(GameMenu(),IDS_MENU_GAME_TAKEBACKMOVE,MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(GameMenu(),IDS_MENU_GAME_PAUSE,MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(GameMenu(),IDS_MENU_GAME_OFFERDRAW,MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(GameMenu(),IDS_MENU_GAME_RESIGN,MF_BYCOMMAND|MF_GRAYED);
      SetMenu(hWnd, MainMenu());

      /* Apply the settings */
      hChessBoardPanel->SetShowCoordinates(BoardCoordinatesVisible);
      hChessBoardPanel->SetShowLastMove(BoardLastMoveVisible);
      hChessBoardPanel->SetShowInvalidMoves(BoardInvalidMovesVisible);
      ShowPlayersInfo(PlayersInfoVisible);
      ShowCaptureList(CaptureListVisible);
      ShowMoveList(MoveListVisible);
      LoadChessSet(ChessSetIndex);
      LoadTheme(CurrentThemeName);

      return 0;
    }
    case WM_DESTROY:
    {
      /* Save settings */
      SaveSettings();

      /* Close connection */
      NetworkClient->Disconnect();

      /* Clean up */
      delete hChessBoardPanel;
      delete hWhitePlayerPanel;
      delete hBlackPlayerPanel;
      delete hCapturePanel;
      delete hMoveListPanel;
      delete NetworkClient;

      /* Unregister the font currently used */
      ChessSet* CurrentChessSet = ChessSets.Get(ChessSetIndex);
      if (CurrentChessSet != NULL)
        RemoveFontResource(CurrentChessSet->FileName);

      PostQuitMessage(0);
      return 0;
    }
    case WM_GETMINMAXINFO:
    {
      MINMAXINFO* Info = (MINMAXINFO*)lParam;

      /* Update window minimum tracking size info */
      RECT R = {0,0,MinBoardSize+(PlayersInfoVisible || CaptureListVisible || MoveListVisible ? LeftPanelSize : 0),MinBoardSize+(NetworkPanelsVisible ? BottomPanelSize : 0)};
      AdjustWindowRectEx(&R,GetWindowLong(hWnd,GWL_STYLE),TRUE,GetWindowLong(hWnd,GWL_EXSTYLE));
      Info->ptMinTrackSize.x = R.right-R.left;
      Info->ptMinTrackSize.y = R.bottom-R.top;

      /* Update the window maximised size info */
      MONITORINFO Monitor;
      Monitor.cbSize = sizeof(MONITORINFO);
      GetMonitorInfo(MonitorFromWindow(hWnd,MONITOR_DEFAULTTONEAREST), &Monitor);
      InflateRect(&(Monitor.rcWork), GetSystemMetrics(SM_CXSIZEFRAME), GetSystemMetrics(SM_CYSIZEFRAME));
      Info->ptMaxSize.x = Monitor.rcWork.bottom-Monitor.rcWork.top-GetSystemMetrics(SM_CYSIZEFRAME)*2-GetSystemMetrics(SM_CYCAPTION)-GetSystemMetrics(SM_CYMENU)-(NetworkPanelsVisible ? BottomPanelSize : 0)+GetSystemMetrics(SM_CXSIZEFRAME)*2+(PlayersInfoVisible || CaptureListVisible || MoveListVisible ? LeftPanelSize : 0);
      Info->ptMaxSize.y = Monitor.rcWork.bottom-Monitor.rcWork.top;
      return 0;
    }
    case WM_MOUSEACTIVATE:
    {
      SetAppCursor(hAppCursor);
      return MA_ACTIVATE;
    }
    case WM_MOVING:
    {
      RECT* R = (RECT*)lParam;
      WINDOWPLACEMENT wd;
      wd.length = sizeof(WINDOWPLACEMENT);
      if (GetWindowPlacement(hWnd, &wd) && (AlwaysVisible || wd.showCmd == SW_SHOWMAXIMIZED))
      {
        int Height = R->bottom-R->top;
        int Width = R->right-R->left;

        /* Constrain the window to the monitor's edges */
        MONITORINFO Monitor;
        Monitor.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(MonitorFromWindow(hWnd,MONITOR_DEFAULTTONEAREST), &Monitor);
        if (wd.showCmd == SW_SHOWMAXIMIZED)
          InflateRect(&(Monitor.rcWork), GetSystemMetrics(SM_CXSIZEFRAME), GetSystemMetrics(SM_CYSIZEFRAME));
        if (R->left <= Monitor.rcWork.left)
        {
          R->left = Monitor.rcWork.left;
          R->right = R->left+Width;
        }
        else if (R->right > Monitor.rcWork.right)
        {
          R->right = Monitor.rcWork.right;
          R->left = R->right-Width;
        }
        if (R->top <= Monitor.rcWork.top)
        {
          R->top = Monitor.rcWork.top;
          R->bottom = R->top+Height;
        }
        else if (R->bottom > Monitor.rcWork.bottom)
        {
          R->bottom = Monitor.rcWork.bottom;
          R->top = R->bottom-Height;
        }
      }
      return 1;
    }
    case WM_SIZE:
    {
      /* Resize the window's components */
      WindowWidth = LOWORD(lParam);
      WindowHeight = HIWORD(lParam);
      ResizePanels();
      return 0;
    }
    case WM_SIZING:
    {
      RECT* R = (RECT*)lParam;

      /* Constrain the chess board area size to 1:1 ratio */
      int FrameWidth = GetSystemMetrics(SM_CXSIZEFRAME);
      int FrameHeight = GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYSIZEFRAME)+GetSystemMetrics(SM_CYMENU);
      int Width = R->right-R->left-FrameWidth-(PlayersInfoVisible || CaptureListVisible || MoveListVisible ? LeftPanelSize : 0);
      int Height = R->bottom-R->top-FrameHeight-(NetworkPanelsVisible ? BottomPanelSize : 0);
      if (wParam == WMSZ_BOTTOM || wParam == WMSZ_TOP)
        Width = Height;
      else if (wParam == WMSZ_LEFT || wParam == WMSZ_RIGHT)
        Height = Width;
      else if (Width != Height)
        Width = Height = min(Height, Width);
      if (wParam == WMSZ_TOPLEFT)
      {
        R->left = R->right-Width-FrameWidth-(PlayersInfoVisible || CaptureListVisible || MoveListVisible ? LeftPanelSize : 0);
        R->top = R->bottom-Height-FrameHeight-(NetworkPanelsVisible ? BottomPanelSize : 0);
      }
      else if (wParam == WMSZ_BOTTOMLEFT)
      {
        R->left = R->right-Width-FrameWidth-(PlayersInfoVisible || CaptureListVisible || MoveListVisible ? LeftPanelSize : 0);
        R->bottom = R->top+Height+FrameHeight+(NetworkPanelsVisible ? BottomPanelSize : 0);
      }
      else if (wParam == WMSZ_TOPRIGHT)
      {
        R->right = R->left+Width+FrameWidth+(PlayersInfoVisible || CaptureListVisible || MoveListVisible ? LeftPanelSize : 0);
        R->top = R->bottom-Height-FrameHeight-(NetworkPanelsVisible ? BottomPanelSize : 0);
      }
      else
      {
        R->right = R->left+Width+FrameWidth+(PlayersInfoVisible || CaptureListVisible || MoveListVisible ? LeftPanelSize : 0);
        R->bottom = R->top+Height+FrameHeight+(NetworkPanelsVisible ? BottomPanelSize : 0);
      }
      return 1;
    }
    case WM_UPDATEUI:
    {
      /* Update the interface */
      UpdateDisplay(wParam,lParam);
      return 0;
    }
  }
  return DefWindowProc(hWnd,Msg,wParam,lParam);
}

int __stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int Show)
{
  MSG Message;
  WNDCLASSEX WndClass;

  /* Load the chess sets, settings and themes */
  LoadChessSets();
  LoadSettings();
  LoadHistory();

  /* Specify the window class information */
  WndClass.cbSize = sizeof(WNDCLASSEX);
  WndClass.lpszClassName = ClassName;
  WndClass.hInstance = hInst;
  WndClass.lpfnWndProc = WindowProc;
  WndClass.style = 0;
  WndClass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
  WndClass.hIcon = (HICON)LoadImage(hInst, "IDI_MAINICON", IMAGE_ICON, 32, 32, LR_CREATEDIBSECTION);
  WndClass.hIconSm = (HICON)LoadImage(hInst, "IDI_MAINICON", IMAGE_ICON, 16, 16, LR_CREATEDIBSECTION);
  WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
  WndClass.lpszMenuName = NULL;
  WndClass.cbClsExtra = 0;
  WndClass.cbWndExtra = 0;

  /* Register the new window class */
  if (RegisterClassEx(&WndClass) == 0)
    return 0;

  /* Create the window */
  hWindow = CreateWindowEx(WS_EX_CONTROLPARENT|WS_EX_WINDOWEDGE|(AlwaysOnTop ? WS_EX_TOPMOST : 0),
      ClassName,WindowTitle,WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_VISIBLE,
      WindowLeft,WindowTop,WindowWidth,WindowHeight,HWND_DESKTOP,NULL,hInst,NULL);
  if (hWindow != NULL)
  {
    /* Message loop */
    HACCEL hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_MAINACCELERATORS));
    while (GetMessage(&Message, NULL, 0, 0) != 0)
    {
      /* Detect keyboard shortcuts only in the main window */
      if (GetActiveWindow() != hWindow || TranslateAccelerator(hWindow, hAccel, &Message) == 0)
        //if (IsDialogMessage(hWindow, &Message) == 0)
        {
          TranslateMessage(&Message);
          DispatchMessage(&Message);
        }
    }
  }
  return 0;
}
