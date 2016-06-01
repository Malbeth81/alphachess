/*
* AlphaChess.cpp
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
#include "alphachess.h"

#define ChessTimerID 1
#define MainServerAddress "alphachess.dlinkddns.com"

ATOM AlphaChess::ClassAtom = 0;
string AlphaChess::ClassName = "AlphaChess";

string AlphaChess::ApplicationPath = GetApplicationPath();
string AlphaChess::ConfigFileName = AlphaChess::ApplicationPath + "\\alphachess.ini";
string AlphaChess::FontsDirectory = AlphaChess::ApplicationPath + "\\Fonts\\";
string AlphaChess::GamesDirectory = AlphaChess::ApplicationPath + "\\Games\\";
string AlphaChess::ThemesDirectory = AlphaChess::ApplicationPath + "\\Themes\\";

// PUBLIC FUNCTIONS ------------------------------------------------------------

AlphaChess::AlphaChess(HINSTANCE hInstance, HWND hParent)
{
  /* Initialise class members */
  Handle = NULL;
  Cursor = LoadCursor(NULL, IDC_ARROW);
  //Animation1 = (HICON)LoadImage(hInstance, "ID_ANIMATION", IMAGE_ICON, 0, 0, LR_SHARED);
  GameMenu = NULL;
  HelpMenu = NULL;
  MainMenu = NULL;
  ViewMenu = NULL;

  Evaluator = NULL;
  Game = NULL;
  NetworkClient = NULL;

  hChessBoardPanel = NULL;
  hWhitePlayerPanel = NULL;
  hBlackPlayerPanel = NULL;
  hCapturePanel = NULL;
  hHistoryPanel = NULL;
  hRoomPanel = NULL;
  hChatPanel = NULL;

  BlackPlayerIsComputer = false;
  WhitePlayerIsComputer = false;
  NetworkGame = false;
  NetworkPlayerName = "";

  Minimized = false;
  Maximized = false;
  InnerHeight = 300;
  InnerWidth = 300;
  Left = 0;
  Top = 0;
  Height = 300;
  Width = 300;

  float scaleFactor = GetDPIScaleFactor();
  BottomPanelHeight = (int)(120*scaleFactor);
  LeftPanelWidth = (int)(156*scaleFactor);
  MoveListMinimumHeight = (int)(50*scaleFactor);
  PlayerPanelHeight = (int)(40*scaleFactor);
  CaptureListAutoHide = false;
  NetworkPanelsVisible = false;

  AutomaticUpdate = false;
  AlwaysOnTop = false;
  AlwaysVisible = false;
  AutomaticallySwitchView = false;
  AlwaysPromoteToQueen = false;
  PlayersInfoVisible = false;
  CaptureListVisible = false;
  MoveListVisible = false;
  BoardCoordinatesVisible = false;
  BoardLastMoveVisible = false;
  BoardInvalidMovesVisible = false;
  CurrentChessSet = "";
  CurrentTheme = "";
  ShowMoveListIcons = true;

  LastTickCount = 0;

  /* Load the chess sets, settings and themes */
  LoadSettings();

  Accelerators = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_MAINACCELERATORS));
  if (ClassAtom == 0)
  {
    /* Register the window's class */
    WNDCLASSEX WndClass;
    WndClass.cbSize = sizeof(WNDCLASSEX);
    WndClass.lpszClassName = ClassName.c_str();
    WndClass.hInstance = hInstance;
    WndClass.lpfnWndProc = WindowProc;
    WndClass.style = 0;
    WndClass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
    WndClass.hIcon = (HICON)LoadImage(hInstance, "ID_MAINICON", IMAGE_ICON, 32, 32, LR_CREATEDIBSECTION);
    WndClass.hIconSm = (HICON)LoadImage(hInstance, "ID_MAINICON", IMAGE_ICON, 16, 16, LR_CREATEDIBSECTION);
    WndClass.hCursor = Cursor;
    WndClass.lpszMenuName = NULL;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    ClassAtom = RegisterClassEx(&WndClass);
  }
  /* Create the window */
  if (ClassAtom != 0)
  {
    /* Create the menu */
    GameMenu = CreatePopupMenu();
    HMENU NewGameMenu = CreatePopupMenu();
    AppendMenu(NewGameMenu,IDS_MAINMENU_GAME_NEW_LOCAL);
    AppendMenu(NewGameMenu,IDS_MAINMENU_GAME_NEW_NETWORK);
    AppendMenu(GameMenu,IDS_MAINMENU_GAME_NEW, NewGameMenu);
    AppendMenu(GameMenu,IDS_MAINMENU_GAME_LOADGAME);
    AppendMenu(GameMenu,IDS_MAINMENU_GAME_SAVEGAME);
    AppendSeparator(GameMenu);
    AppendMenu(GameMenu,IDS_MAINMENU_GAME_PAUSE);
    AppendSeparator(GameMenu);
    AppendMenu(GameMenu,IDS_MAINMENU_GAME_TAKEBACKMOVE);
    AppendMenu(GameMenu,IDS_MAINMENU_GAME_OFFERDRAW);
    AppendMenu(GameMenu,IDS_MAINMENU_GAME_RESIGN);
    AppendSeparator(GameMenu);
    AppendMenu(GameMenu,IDS_MAINMENU_GAME_PREFERENCES);
    AppendSeparator(GameMenu);
    AppendMenu(GameMenu,IDS_MAINMENU_GAME_EXIT);

    ViewMenu = CreatePopupMenu();
    AppendMenu(ViewMenu,IDS_MAINMENU_VIEW_PLAYERS);
    AppendMenu(ViewMenu,IDS_MAINMENU_VIEW_MOVES);
    AppendMenu(ViewMenu,IDS_MAINMENU_VIEW_CAPTURES);
    AppendSeparator(ViewMenu);
    AppendMenu(ViewMenu,IDS_MAINMENU_VIEW_SWITCH);

    HelpMenu = CreatePopupMenu();
    AppendMenu(HelpMenu,IDS_MAINMENU_HELP_HELP);
    AppendMenu(HelpMenu,IDS_MAINMENU_HELP_UPDATE);
    AppendSeparator(HelpMenu);
    AppendMenu(HelpMenu,IDS_MAINMENU_HELP_ABOUT);

    MainMenu = CreateMenu();
    AppendMenu(MainMenu,IDS_MAINMENU_GAME,GameMenu);
    AppendMenu(MainMenu,IDS_MAINMENU_VIEW,ViewMenu);
    AppendMenu(MainMenu,IDS_MAINMENU_HELP,HelpMenu);

    Handle = CreateWindowEx(WS_EX_CONTROLPARENT | WS_EX_WINDOWEDGE,ClassName.c_str(),"AlphaChess 4",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,Left,Top,Width,Height,hParent,MainMenu,hInstance,this);
    if (Handle != NULL)
    {
      /* Create a new the game engine */
      Game = new ChessGame();
      Game->Reset();
      Game->AddObserver(this);

      /* Create a new network client */
      NetworkClient = new GameClient(Handle);

      /* Intialise menu items */
      EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_TAKEBACKMOVE,MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_PAUSE,MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_OFFERDRAW,MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_RESIGN,MF_BYCOMMAND|MF_GRAYED);

      RECT R;

      /* Create the chess board panel */
      CalculateChessBoardPosition(&R);
      hChessBoardPanel = new ChessBoardPanel(hInstance, Handle, &R);
      hChessBoardPanel->SetGame(Game);
      hChessBoardPanel->SetStandardCursor(LoadCursor(hInstance, "ID_CURSORHAND"));
      hChessBoardPanel->SetSelectionCursor(LoadCursor(hInstance, "ID_CURSORGRAB"));

      /* Create the white player panel */
      CalculateWhitePlayerPosition(&R);
      hWhitePlayerPanel = new PlayerPanel(hInstance, Handle, &R, White);
      hWhitePlayerPanel->SetGame(Game);

      /* Create the black player panel */
      CalculateBlackPlayerPosition(&R);
      hBlackPlayerPanel = new PlayerPanel(hInstance, Handle, &R, Black);
      hBlackPlayerPanel->SetGame(Game);

      /* Create the captured pieces panel */
      CalculateCaptureListPosition(&R);
      hCapturePanel = new CapturePanel(hInstance, Handle, &R);
      hCapturePanel->SetGame(Game);

      /* Create the move list panel */
      CalculateMoveListPosition(&R);
      hHistoryPanel = new HistoryPanel(hInstance, Handle, &R);
      hHistoryPanel->SetGame(Game);

      /* Create the room panel */
      CalculateRoomPosition(&R);
      hRoomPanel = new RoomPanel(hInstance, Handle, &R);
      hRoomPanel->SetGameClient(NetworkClient);

      /* Create the chat panel */
      CalculateChatPosition(&R);
      hChatPanel = new ChatPanel(hInstance, Handle, &R);

      ApplyPreferences();

      ShowWindow(Handle, SW_SHOW);
    }
  }
}

AlphaChess::~AlphaChess()
{
  /* Destroy the window */
  if (hChessBoardPanel != NULL)
    delete hChessBoardPanel;
  if (hWhitePlayerPanel != NULL)
    delete hWhitePlayerPanel;
  if (hBlackPlayerPanel != NULL)
    delete hBlackPlayerPanel;
  if (hCapturePanel != NULL)
    delete hCapturePanel;
  if (hHistoryPanel != NULL)
    delete hHistoryPanel;
  if (NetworkClient != NULL)
    delete NetworkClient;
  if (Handle != NULL)
    DestroyWindow(Handle);

  /* Save settings */
  SaveSettings();

  /* Close connection */
  NetworkClient->Disconnect();

  /* Unregister the font currently used */
  ChessSet* Set = FindChessSet(CurrentChessSet);
  if (Set != NULL)
    RemoveFontResourceEx(Set->FileName.c_str(),FR_PRIVATE,0);
}

HWND AlphaChess::GetHandle()
{
  return Handle;
}

int AlphaChess::ProcessAccelerators(MSG* Message)
{
  if (GetActiveWindow() == Handle)
    return TranslateAccelerator(Handle, Accelerators, Message);
  return 0;
}

// PRIVATE FUNCTIONS -----------------------------------------------------------

void AlphaChess::ApplyPreferences()
{
  SetWindowPos(Handle,(AlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST),0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

  ShowPlayerPanels(PlayersInfoVisible);
  ShowCapturePanel(CaptureListVisible);
  ShowHistoryPanel(MoveListVisible);

  hChessBoardPanel->SetShowCoordinates(BoardCoordinatesVisible);
  hChessBoardPanel->SetShowLastMove(BoardLastMoveVisible);
  hChessBoardPanel->SetShowInvalidMoves(BoardInvalidMovesVisible);
  hHistoryPanel->ShowIcons(ShowMoveListIcons);

  LoadChessSet(FindChessSet(CurrentChessSet));
  LoadTheme(CurrentTheme);
}

void AlphaChess::CalculateBlackPlayerPosition(RECT* Rect)
{
  Rect->left = 0;
  Rect->top = PlayerPanelHeight;
  Rect->right = Rect->left+LeftPanelWidth;
  Rect->bottom = Rect->top+PlayerPanelHeight;
}

void AlphaChess::CalculateCaptureListPosition(RECT* Rect)
{
  int Tmp = (LeftPanelWidth/6)*5;
  Rect->left = 0;
  Rect->top = InnerHeight-(NetworkPanelsVisible ? BottomPanelHeight : 0)-Tmp;
  Rect->right = Rect->left+LeftPanelWidth;
  Rect->bottom = Rect->top+Tmp;
}

void AlphaChess::CalculateChatPosition(RECT* Rect)
{
  Rect->left = LeftPanelWidth;
  Rect->top = InnerHeight-BottomPanelHeight;
  Rect->right = InnerWidth;
  Rect->bottom = Rect->top+BottomPanelHeight;
}

void AlphaChess::CalculateChessBoardPosition(RECT* Rect)
{
  Rect->left = (PlayersInfoVisible || (CaptureListVisible && !CaptureListAutoHide) || MoveListVisible ? LeftPanelWidth+2 : 0);
  Rect->top = 0;
  Rect->right = InnerWidth;
  Rect->bottom = InnerHeight-(NetworkPanelsVisible ? BottomPanelHeight+2 : 0);
}

void AlphaChess::CalculateMoveListPosition(RECT* Rect)
{
  RECT R;
  CalculateCaptureListPosition(&R);
  Rect->left = 0;
  Rect->top = (PlayersInfoVisible ? PlayerPanelHeight*2 : 0);
  Rect->right = Rect->left+LeftPanelWidth;
  Rect->bottom = InnerHeight-(NetworkPanelsVisible ? BottomPanelHeight : 0)-(CaptureListVisible && !CaptureListAutoHide ? R.bottom-R.top : 0);
}

void AlphaChess::CalculateRoomPosition(RECT* Rect)
{
  RECT R;
  GetClientRect(Handle,&R);
  Rect->left = 0;
  Rect->top = InnerHeight-BottomPanelHeight;
  Rect->right = Rect->left+LeftPanelWidth;
  Rect->bottom = Rect->top+BottomPanelHeight;
}

void AlphaChess::CalculateWhitePlayerPosition(RECT* Rect)
{
  Rect->left = 0;
  Rect->top = 0;
  Rect->right = Rect->left+LeftPanelWidth;
  Rect->bottom = Rect->top+PlayerPanelHeight;
}

void AlphaChess::ChangeCursor(HCURSOR NewCursor)
{
  if (Cursor != NewCursor)
  {
    Cursor = NewCursor;
    if (Cursor == NULL)
    {
      ReleaseCapture();
      Cursor = LoadCursor(NULL, IDC_ARROW);
    }
    else
      SetCapture(Handle);
    DestroyCursor(SetCursor(Cursor));
  }
}

ChessSet* AlphaChess::FindChessSet(string Name)
{
  ChessSet* Ptr = ChessSets.GetFirst();
  while (Ptr != NULL)
  {
    if (Name.compare(Ptr->FontName) == 0)
      return Ptr;
    Ptr = ChessSets.GetNext();
  }
  return NULL;
}

void AlphaChess::LoadChessSet(ChessSet* Set)
{
  if (Set != NULL)
  {
    /* Get the path to the fonts directory */
    if (!DirectoryExists(FontsDirectory.c_str()))
      CreateDirectory(FontsDirectory.c_str(), NULL);

    /* Unregister the last font */
    ChessSet* OldSet = FindChessSet(CurrentChessSet);
    if (OldSet != NULL && OldSet != Set)
    {
      string FileName = FontsDirectory + OldSet->FileName;
      RemoveFontResourceEx(FileName.c_str(),FR_PRIVATE,0);
    }

    /* Switch font */
    CurrentChessSet = Set->FontName;

    /* Register the new font */
    string FileName = FontsDirectory + Set->FileName;
    AddFontResourceEx(FileName.c_str(),FR_PRIVATE,0);

    /* Update the interface */
    hChessBoardPanel->SetChessSet(Set);
    hWhitePlayerPanel->SetChessSet(Set);
    hBlackPlayerPanel->SetChessSet(Set);
    hHistoryPanel->SetChessSet(Set);
    hCapturePanel->SetChessSet(Set);
  }
}

void AlphaChess::LoadSettings()
{
  char Section[16];

  /* Load the settings from file */
  strcpy(Section, "Window");
  Left = GetPrivateProfileInt(Section,"Left",20,ConfigFileName.c_str());
  Top = GetPrivateProfileInt(Section,"Top",20,ConfigFileName.c_str());
  Width = GetPrivateProfileInt(Section,"Width",634,ConfigFileName.c_str());
  Height = GetPrivateProfileInt(Section,"Height",514,ConfigFileName.c_str());
  strcpy(Section, "Player");
  NetworkPlayerName = GetPrivateProfileString(Section,"Name","",ConfigFileName);
  strcpy(Section, "Options");
  AutomaticUpdate = GetPrivateProfileBool(Section,"AutomaticUpdate",false,ConfigFileName.c_str());
  AlwaysOnTop = GetPrivateProfileBool(Section,"AlwaysOnTop",false,ConfigFileName.c_str());
  AlwaysVisible = GetPrivateProfileBool(Section,"AlwaysVisible",false,ConfigFileName.c_str());
  AutomaticallySwitchView = GetPrivateProfileBool(Section,"AutomaticallySwitchView",true,ConfigFileName.c_str());
  AlwaysPromoteToQueen = GetPrivateProfileBool(Section,"AlwaysPromoteToQueen",false,ConfigFileName.c_str());
  CurrentChessSet = GetPrivateProfileString(Section,"FontName","Chess Alpha",ConfigFileName);
  BoardCoordinatesVisible = GetPrivateProfileBool(Section,"ShowCoordinates",true,ConfigFileName.c_str());
  BoardLastMoveVisible = GetPrivateProfileBool(Section,"ShowLastMove",true,ConfigFileName.c_str());
  BoardInvalidMovesVisible = GetPrivateProfileBool(Section,"ShowInvalidMoves",true,ConfigFileName.c_str());
  PlayersInfoVisible = GetPrivateProfileBool(Section,"PlayersInfoVisible",true,ConfigFileName.c_str());
  CaptureListVisible = GetPrivateProfileBool(Section,"CaptureListVisible",true,ConfigFileName.c_str());
  MoveListVisible = GetPrivateProfileBool(Section,"MoveListVisible",true,ConfigFileName.c_str());
  ShowMoveListIcons = GetPrivateProfileBool(Section,"ShowMoveListIcons",true,ConfigFileName.c_str());
  CurrentTheme = GetPrivateProfileString(Section,"ColorTheme","Deep Blue Solid",ConfigFileName);

  /* Load the chess fonts from file */
  int Count = GetPrivateProfileInt("Fonts", "Count", 0, ConfigFileName.c_str());
  if (Count > 0)
  {
    for (int i=0; i < Count; i++)
    {
      strcpy(Section,"Font");
      char* Str = inttostr(i+1);
      strcat(Section,Str);
      delete[] Str;
      /* Load the chess set info from the file */
      ChessSet* NewSet = new ChessSet;
      NewSet->FileName = GetPrivateProfileString(Section, "FileName", "", ConfigFileName);
      NewSet->FontName = GetPrivateProfileString(Section, "FontName", "", ConfigFileName);
      NewSet->Letters = GetPrivateProfileString(Section, "Letters", "", ConfigFileName);
      ChessSets.Add(NewSet);
    }
  }
}

void AlphaChess::LoadTheme(string ThemeName)
{
  /* Get the path to the thenes directory */
  if (!DirectoryExists(ThemesDirectory.c_str()))
    CreateDirectory(ThemesDirectory.c_str(), NULL);

  /* Load the theme */
  ChessBoardTheme* Theme = new ChessBoardTheme();
  Theme->LoadFromFile(ThemesDirectory + ThemeName + ".ini");

  /* Apply the theme to the chess board */
  delete hChessBoardPanel->GetTheme();
  hChessBoardPanel->SetTheme(Theme);
}

void AlphaChess::Notify(const int Event, const void* Param)
{
  static ChessGameState LastGameState = Undefined;

  switch (Event)
  {
    case PlayerUpdated:
    {
      hBlackPlayerPanel->Invalidate();
      hWhitePlayerPanel->Invalidate();
      break;
    }
    case StateChanged:
    {
      if (Game->GetState() == Undefined)
      {
        /* Reset panels */
        hBlackPlayerPanel->SetReady(false);
        hWhitePlayerPanel->SetReady(false);
        hCapturePanel->Invalidate();
        hChessBoardPanel->Invalidate();
        hHistoryPanel->Invalidate();
      }
      else if (Game->GetState() == Started)
      {
        if (LastGameState == Paused)
        {
          /* Add a message in the chat panel */
          if (NetworkGame)
            hChatPanel->AddLine("The game has started.");
        }
        else if (NetworkGame)
        {
          /* Add a message in the chat panel */
          hChatPanel->AddLine("The game has resumed.");

          /* Disable menu items */
          if (NetworkClient->GetLocalPlayer() == NetworkClient->GetBlackPlayer())
            hBlackPlayerPanel->EnableLeaveButton(false);
          else if (NetworkClient->GetLocalPlayer() == NetworkClient->GetWhitePlayer())
            hWhitePlayerPanel->EnableLeaveButton(false);
        }

        /* Reset player status */
        hBlackPlayerPanel->SetReady(false);
        hWhitePlayerPanel->SetReady(false);

        /* Enable menu items */
        EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_PAUSE,MF_BYCOMMAND|MF_ENABLED);
        EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_TAKEBACKMOVE,MF_BYCOMMAND|MF_ENABLED);
        EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_OFFERDRAW,MF_BYCOMMAND|MF_ENABLED);
        EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_RESIGN,MF_BYCOMMAND|MF_ENABLED);

        /* Uncheck menu items */
        CheckMenuItem(GameMenu,IDS_MAINMENU_GAME_PAUSE,MF_BYCOMMAND|MF_UNCHECKED);

        /* Unlock the chess board */
        hChessBoardPanel->SetPaused(false);
      }
      else if (Game->GetState() == Paused)
      {
        if (NetworkGame)
        {
          /* Add a message in the chat panel */
          hChatPanel->AddLine("The game is paused.");

          /* Enable menu items */
          if (NetworkClient->GetLocalPlayer() == NetworkClient->GetBlackPlayer())
            hBlackPlayerPanel->EnableLeaveButton(true);
          else if (NetworkClient->GetLocalPlayer() == NetworkClient->GetWhitePlayer())
            hWhitePlayerPanel->EnableLeaveButton(true);
        }

        /* Disable menu items */
        EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_PAUSE,MF_BYCOMMAND|MF_ENABLED);
        EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_TAKEBACKMOVE,MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_OFFERDRAW,MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_RESIGN,MF_BYCOMMAND|MF_GRAYED);

        /* Check menu items */
        CheckMenuItem(GameMenu,IDS_MAINMENU_GAME_PAUSE,MF_BYCOMMAND|MF_CHECKED);

        /* Lock the chess board */
        hChessBoardPanel->SetPaused(true);
      }
      else if (Game->GetState() == EndedInCheckMate || Game->GetState() == EndedInStaleMate || Game->GetState() == EndedInDraw || Game->GetState() == EndedInForfeit || Game->GetState() == EndedInTimeout)
      {
        if (NetworkGame)
        {
          /* Add a message in the chat panel */
          if (Game->GetState() == EndedInDraw || Game->GetState() == EndedInStaleMate)
            hChatPanel->AddLine("The game has ended in a draw.");
          else if (Game->GetState() == EndedInForfeit)
            hChatPanel->AddLine(Game->GetPlayer(!Game->GetActivePlayer())->Name + " has resigned.");
          else
            hChatPanel->AddLine(Game->GetPlayer(Game->GetActivePlayer())->Name + " has won.");

          /* Notify the server */
          NetworkClient->GameHasEnded();

          /* Enable menu items */
          if (NetworkClient->GetLocalPlayer() == NetworkClient->GetBlackPlayer())
          {
            hBlackPlayerPanel->EnableLeaveButton(true);
            hBlackPlayerPanel->EnableReadyButton(true);
          }
          else if (NetworkClient->GetLocalPlayer() == NetworkClient->GetWhitePlayer())
          {
            hWhitePlayerPanel->EnableLeaveButton(true);
            hWhitePlayerPanel->EnableReadyButton(true);
          }
        }

        /* Lock the chess board */
        hChessBoardPanel->SetLocked(true);

        /* Disable menu items */
        EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_PAUSE,MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_TAKEBACKMOVE,MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_OFFERDRAW,MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_RESIGN,MF_BYCOMMAND|MF_GRAYED);

        /* Uncheck menu items */
        CheckMenuItem(GameMenu,IDS_MAINMENU_GAME_PAUSE,MF_BYCOMMAND|MF_UNCHECKED);

        if (LastGameState == Started && MessageBox(Handle,"Do you want to save this game?","Game ended",MB_YESNO|MB_ICONQUESTION) == IDYES)
          SaveGame();
      }
      else if (Game->GetState() == Postponed)
      {
        /* Lock the chess board */
        hChessBoardPanel->SetLocked(true);

        /* Disable menu items */
        EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_TAKEBACKMOVE,MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_PAUSE,MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_OFFERDRAW,MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(GameMenu,IDS_MAINMENU_GAME_RESIGN,MF_BYCOMMAND|MF_GRAYED);

        /* Uncheck menu items */
        CheckMenuItem(GameMenu,IDS_MAINMENU_GAME_PAUSE,MF_BYCOMMAND|MF_UNCHECKED);
      }

      LastGameState = Game->GetState();
      break;
    }
    case BoardUpdated:
    {
      /* Refresh the panels */
      hBlackPlayerPanel->Invalidate();
      hWhitePlayerPanel->Invalidate();
      hChessBoardPanel->Invalidate();
      hCapturePanel->Invalidate();
      hHistoryPanel->Invalidate();
      PostMessage(hHistoryPanel->GetHandle(),WM_VSCROLL,MAKEWPARAM(SB_BOTTOM,0),0);

      /* Lock the chess board */
      hChessBoardPanel->SetLocked(!Game->IsLastMoveDisplayed());

      /* Add a message in the chat panel */
      if (NetworkGame)
      {
        if (Game->IsPlayerChecked() && Game->IsPlayerMated())
          hChatPanel->AddLine(Game->GetPlayer(Game->GetActivePlayer())->Name + " is checkmate.");
        else if (Game->IsPlayerMated())
          hChatPanel->AddLine(Game->GetPlayer(Game->GetActivePlayer())->Name + " is stalemate.");
        else if (Game->IsPlayerChecked())
          hChatPanel->AddLine(Game->GetPlayer(Game->GetActivePlayer())->Name + " is in check.");
      }
      break;
    }
    case PiecePromoted:
    {
      /* Promote the chess piece */
      if (!NetworkGame || (Game->GetActivePlayer() == White && NetworkClient->GetLocalPlayer() == NetworkClient->GetWhitePlayer()) || (Game->GetActivePlayer() == Black && NetworkClient->GetLocalPlayer() == NetworkClient->GetBlackPlayer()))
      {
        ChessPieceType Type = Queen;
        //if (!AlwaysPromoteToQueen)
        //  Type = (ChessPieceType)ShowPromotionDialog(Handle, FindChessSet(CurrentChessSet));
        if (NetworkGame)
          NetworkClient->PromoteTo(Type);
        else
          Game->PromotePawnTo(Type);
      }
      break;
    }
    case TurnEnded:
    {
      /* Update the chess board information layer */
      hChessBoardPanel->ClearPossibleMoves();
      if (Evaluator != NULL)
        delete Evaluator;
      Evaluator = Game->GetEvaluator();
      Evaluator->AddObserver(this);

      if (NetworkGame)
      {
        /* Lock the chess board panel if it is not current player's turn */
        if (NetworkClient->GetLocalPlayer() != NULL && NetworkClient->GetBlackPlayer() != NULL && NetworkClient->GetWhitePlayer() != NULL)
        {
          if ((Game->GetActivePlayer() == Black && NetworkClient->GetLocalPlayer() == NetworkClient->GetBlackPlayer()) || (Game->GetActivePlayer() == White && NetworkClient->GetLocalPlayer() == NetworkClient->GetWhitePlayer()))
          {
            hChessBoardPanel->SetLocked(false);

            /* Start the timer for the current player */
            LastTickCount = GetTickCount();
            SetTimer(Handle, ChessTimerID, 200, NULL);
          }
          else
            hChessBoardPanel->SetLocked(true);
        }
      }
      else if ((WhitePlayerIsComputer && Game->GetActivePlayer() == White) || (BlackPlayerIsComputer && Game->GetActivePlayer() == Black))
      {
        /* Lock the chess board */
        hChessBoardPanel->SetLocked(true);

        /* Find a move and make it */
        //ChessAiMove* Move = Ai.Execute(Game,Depth,Window)(6, 100);
        //if (Move != NULL)
        {
          //Game->MakeMove(Move->From,Move->To);
          //delete Move;
        }
      }
      else
      {
        /* Unlock the chess board */
        hChessBoardPanel->SetLocked(false);

        /* Start the timer for the current player */
        LastTickCount = GetTickCount();
        SetTimer(Handle, ChessTimerID, 200, NULL);
      }

      /* Update the players information */
      hWhitePlayerPanel->Invalidate();
      hBlackPlayerPanel->Invalidate();

      /* Switch the view */
      if (AutomaticallySwitchView && !NetworkGame && !WhitePlayerIsComputer && !BlackPlayerIsComputer)
        RotateView(Game->GetActivePlayer() == Black);
      break;
    }
    case EvaluationCompleted:
    {
      LinkedList<PossibleChessMove>* Moves = (LinkedList<PossibleChessMove>*)Param;
      PossibleChessMove* Move = Moves->GetFirst();
      while (Move != NULL)
      {
        PossibleChessMove* NewMove = new PossibleChessMove;
        *NewMove = *Move;
        hChessBoardPanel->DisplayPossibleMove(NewMove);
        Move = Moves->GetNext();
      }
      break;
    }
  }
}

void AlphaChess::RotateView(bool Value)
{
  /* Invert the chess board view */
  hChessBoardPanel->SetInvertView(Value);
  /* Check menu item */
  CheckMenuItem(ViewMenu, IDS_MAINMENU_VIEW_SWITCH, MF_BYCOMMAND | (Value ? MF_CHECKED : MF_UNCHECKED));
}

bool AlphaChess::SaveGame()
{
  string Filename;

  /* Get the path to the games directory */
  if (!DirectoryExists(GamesDirectory.c_str()))
    CreateDirectory(GamesDirectory.c_str(), NULL);

  /* Save the game in history */
  if (Game->GetStartTime() != NULL)
  {
    char* DateTime = FormatDateTime(Game->GetStartTime(), 0, "yyyy-MM-dd", 0, "HH_mm");
    Filename = GamesDirectory + DateTime + ".pgn";
    delete[] DateTime;
  }

  /* Show dialog */
  if (SaveFileAsDialog(Handle, Filename, GamesDirectory.c_str(), "pgn", "Portable Game Notation (*.pgn)\0*.pgn\0", 0))
    return Game->SaveToFile(Filename.c_str());
  return false;
}

void AlphaChess::SaveSettings()
{
  /* Save the settings to the file */
  char Section[16];
  strcpy(Section, "Window");
  WritePrivateProfileInteger(Section,"Left",Left,ConfigFileName.c_str());
  WritePrivateProfileInteger(Section,"Top",Top,ConfigFileName.c_str());
  WritePrivateProfileInteger(Section,"Width",Width,ConfigFileName.c_str());
  WritePrivateProfileInteger(Section,"Height",Height-(NetworkPanelsVisible ? BottomPanelHeight : 0),ConfigFileName.c_str());
  strcpy(Section, "Player");
  WritePrivateProfileString(Section,"Name",NetworkPlayerName.c_str(),ConfigFileName.c_str());
  strcpy(Section, "Options");
  WritePrivateProfileBool(Section,"AutomaticUpdate",AutomaticUpdate,ConfigFileName.c_str());
  WritePrivateProfileBool(Section,"AlwaysOnTop",AlwaysOnTop,ConfigFileName.c_str());
  WritePrivateProfileBool(Section,"AlwaysVisible",AlwaysVisible,ConfigFileName.c_str());
  WritePrivateProfileBool(Section,"AutomaticallySwitchView",AutomaticallySwitchView,ConfigFileName.c_str());
  WritePrivateProfileBool(Section,"AlwaysPromoteToQueen",AlwaysPromoteToQueen,ConfigFileName.c_str());
  WritePrivateProfileString(Section,"FontName",CurrentChessSet.c_str(),ConfigFileName.c_str());
  WritePrivateProfileBool(Section,"ShowCoordinates",BoardCoordinatesVisible,ConfigFileName.c_str());
  WritePrivateProfileBool(Section,"ShowLastMove",BoardLastMoveVisible,ConfigFileName.c_str());
  WritePrivateProfileBool(Section,"ShowInvalidMoves",BoardInvalidMovesVisible,ConfigFileName.c_str());
  WritePrivateProfileBool(Section,"PlayersInfoVisible",PlayersInfoVisible,ConfigFileName.c_str());
  WritePrivateProfileBool(Section,"CaptureListVisible",CaptureListVisible,ConfigFileName.c_str());
  WritePrivateProfileBool(Section,"MoveListVisible",MoveListVisible,ConfigFileName.c_str());
  WritePrivateProfileBool(Section,"ShowMoveListIcons",ShowMoveListIcons,ConfigFileName.c_str());
  WritePrivateProfileString(Section,"ColorTheme",CurrentTheme.c_str(),ConfigFileName.c_str());
}

void AlphaChess::ShowCapturePanel(bool Value)
{
  if (Value != CaptureListVisible)
  {
    CaptureListVisible = Value;
    if (PlayersInfoVisible || MoveListVisible)
      UpdateSize(InnerWidth, InnerHeight);
    else
    {
      /* Calculate new window size */
      RECT R;
      GetClientRect(Handle,&R);
      if (Value && !CaptureListAutoHide)
        R.right += LeftPanelWidth;
      else
        R.right -= LeftPanelWidth;

      /* Resize the window */
      AdjustWindowRectEx(&R,GetWindowLong(Handle,GWL_STYLE),TRUE,GetWindowLong(Handle,GWL_EXSTYLE));
      SetWindowPos(Handle,NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER|SWP_NOMOVE);
    }
  }
  /* Show the panel */
  ShowWindow(hCapturePanel->GetHandle(),(CaptureListVisible & !CaptureListAutoHide ? SW_SHOW : SW_HIDE));

  /* Check menu item */
  CheckMenuItem(ViewMenu, IDS_MAINMENU_VIEW_CAPTURES, MF_BYCOMMAND | (CaptureListVisible ? MF_CHECKED : MF_UNCHECKED));
}

void AlphaChess::ShowHistoryPanel(bool Value)
{
  if (Value != MoveListVisible)
  {
    MoveListVisible = Value;
    if (PlayersInfoVisible || (CaptureListVisible && !CaptureListAutoHide))
      UpdateSize(InnerWidth, InnerHeight);
    else
    {
      /* Calculate new window size */
      RECT R;
      GetClientRect(Handle,&R);
      if (Value)
        R.right += LeftPanelWidth;
      else
        R.right -= LeftPanelWidth;

      /* Resize the window */
      AdjustWindowRectEx(&R,GetWindowLong(Handle,GWL_STYLE),TRUE,GetWindowLong(Handle,GWL_EXSTYLE));
      SetWindowPos(Handle,NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER|SWP_NOMOVE);
    }
  }
  /* Show the panel */
  ShowWindow(hHistoryPanel->GetHandle(),(MoveListVisible ? SW_SHOW : SW_HIDE));

  /* Check menu item */
  CheckMenuItem(ViewMenu, IDS_MAINMENU_VIEW_MOVES, MF_BYCOMMAND | (MoveListVisible ? MF_CHECKED : MF_UNCHECKED));
}

void AlphaChess::ShowNetworkPanels(bool Value)
{
  if (Value != NetworkPanelsVisible)
  {
    NetworkPanelsVisible = Value;

    /* Calculate new window size */
    RECT R;
    GetClientRect(Handle,&R);
    if (Value)
      R.bottom += BottomPanelHeight;
    else
      R.bottom -= BottomPanelHeight;

    /* Resize the window */
    AdjustWindowRectEx(&R,GetWindowLong(Handle,GWL_STYLE),TRUE,GetWindowLong(Handle,GWL_EXSTYLE));
    SetWindowPos(Handle,NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER|SWP_NOMOVE);
  }

  /* Show the panel */
  ShowWindow(hRoomPanel->GetHandle(),(NetworkPanelsVisible ? SW_SHOW : SW_HIDE));
  ShowWindow(hChatPanel->GetHandle(),(NetworkPanelsVisible ? SW_SHOW : SW_HIDE));
}

void AlphaChess::ShowPlayerPanels(bool Value)
{
  if (Value != PlayersInfoVisible)
  {
    PlayersInfoVisible = Value;
    if ((CaptureListVisible && !CaptureListAutoHide) || MoveListVisible)
      UpdateSize(InnerWidth, InnerHeight);
    else
    {
      /* Calculate new window size */
      RECT R;
      GetClientRect(Handle,&R);
      if (Value)
        R.right += LeftPanelWidth;
      else
        R.right -= LeftPanelWidth;

      /* Resize the window */
      AdjustWindowRectEx(&R,GetWindowLong(Handle,GWL_STYLE),TRUE,GetWindowLong(Handle,GWL_EXSTYLE));
      SetWindowPos(Handle,NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER|SWP_NOMOVE);
    }
  }

  /* Show the panel */
  ShowWindow(hWhitePlayerPanel->GetHandle(),(PlayersInfoVisible ? SW_SHOW : SW_HIDE));
  ShowWindow(hBlackPlayerPanel->GetHandle(),(PlayersInfoVisible ? SW_SHOW : SW_HIDE));

  /* Check menu item */
  CheckMenuItem(ViewMenu, IDS_MAINMENU_VIEW_PLAYERS, MF_BYCOMMAND | (PlayersInfoVisible ? MF_CHECKED : MF_UNCHECKED));
}

void AlphaChess::UpdateInterface(WPARAM wParam, LPARAM lParam)
{
  // Messages sent from the network interface
  switch (LOWORD(wParam))
  {
    case ConnectionSucceeded:
    {
      /* Initialize dialog values */
      NetworkGameDialogValues* Values = new NetworkGameDialogValues;
      Values->PlayerName = NetworkPlayerName;
      ChangeCursor(NULL);

      /* Show dialog */
      if (ShowNetworkGameDialog((HINSTANCE)GetWindowLong(Handle, GWL_HINSTANCE), Handle, Values))
      {
        /* Save dialog values */
        NetworkPlayerName = Values->PlayerName;

        if (Values->RoomId > 0)
        {
          /* Join the room */
          NetworkClient->SetPlayerName(NetworkPlayerName);
          if (NetworkClient->JoinRoom(Values->RoomId))
          {
            hRoomPanel->SetRoomName(Values->RoomName);
            SetCursor(LoadCursor(NULL,IDC_WAIT));
          }
        }
        else
        {
          /* Create a new room */
          NetworkClient->SetPlayerName(NetworkPlayerName);
          if (NetworkClient->CreateRoom(Values->RoomName))
          {
            hRoomPanel->SetRoomName(Values->RoomName);
            SetCursor(LoadCursor(NULL,IDC_WAIT));
          }
        }
      }
      else
        NetworkClient->Disconnect();
      delete Values;
      break;
    }
    case ConnectionFailed:
    {
      ChangeCursor(NULL);
      MessageBox(Handle,"Connection attempt to the game server failed!\nMake sure that you have an active Internet connection and try again.","Play a new online game",MB_OK|MB_ICONERROR);
      break;
    }
    case Disconnected:
    {
      NetworkGame = false;

      /* Cancels the game if it is in progress */
      if (Game->GetState() == Started || Game->GetState() == Paused)
        Game->PostponeGame();

      /* Hide the buttons and panels */
      ShowNetworkPanels(NetworkGame);
      hWhitePlayerPanel->EnableJoinButton(false);
      hWhitePlayerPanel->EnableLeaveButton(false);
      hWhitePlayerPanel->EnableReadyButton(false);
      hBlackPlayerPanel->EnableJoinButton(false);
      hBlackPlayerPanel->EnableLeaveButton(false);
      hBlackPlayerPanel->EnableReadyButton(false);
      break;
    }
    case GameDataReceived:
    {
      Game->LoadFromImage((ChessGameImage*)lParam);
      break;
    }
    case GameDataRequested:
    {
      /* Create a game image */
      ChessGameImage* Data = new ChessGameImage;
      Game->SaveToImage(Data);
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
      hChatPanel->AddLine(Message->PlayerName + ": " + Message->Message);
      /* Clean up */
      delete Message;
      break;
    }
    case MoveReceived:
    {
      /* A player made a move */
      Game->MakeMove(Position(LOBYTE(LOWORD(lParam)),HIBYTE(LOWORD(lParam))),Position(LOBYTE(HIWORD(lParam)),HIBYTE(HIWORD(lParam))));
      break;
    }
    case NotificationReceived:
    {
      switch (HIWORD(wParam))
      {
        case JoinedRoom:
        {
          /* Close the dialog if we joined a room */
          NetworkGame = true;

          /* Reset the game */
          Game->Reset();
          hChessBoardPanel->SetLocked(true);

          /* Display the bottom panels */
          ShowNetworkPanels(NetworkGame);
          hRoomPanel->Invalidate();

          /* Display buttons */
          hWhitePlayerPanel->EnableJoinButton(true);
          hWhitePlayerPanel->EnableLeaveButton(false);
          hWhitePlayerPanel->EnableReadyButton(false);
          hBlackPlayerPanel->EnableJoinButton(true);
          hBlackPlayerPanel->EnableLeaveButton(false);
          hBlackPlayerPanel->EnableReadyButton(false);
          hRoomPanel->EnableLeaveRoomButton(true);
          hRoomPanel->EnableKickPlayerButton(false);

          /* Add a message in the chat panel */
          hChatPanel->AddLine("You have joined the room: " + hRoomPanel->GetRoomName());
          break;
        }
        case LeftRoom:
        {
          /* Hide buttons */
          Game->Reset();
          hWhitePlayerPanel->EnableJoinButton(false);
          hWhitePlayerPanel->EnableLeaveButton(false);
          hWhitePlayerPanel->EnableReadyButton(false);
          hBlackPlayerPanel->EnableJoinButton(false);
          hBlackPlayerPanel->EnableLeaveButton(false);
          hBlackPlayerPanel->EnableReadyButton(false);
          hRoomPanel->EnableLeaveRoomButton(false);
          hRoomPanel->EnableKickPlayerButton(false);

          /* Add a message in the chat panel */
          hChatPanel->AddLine("You have left the room: " + hRoomPanel->GetRoomName());

          /* Update the room list */
          hRoomPanel->SetRoomName("");
          hRoomPanel->Invalidate();

          /* Go back to room selection */
          UpdateInterface(MAKEWPARAM(ConnectionSucceeded,0), 0);
          break;
        }
        case GameStarted:
        {
          /* Hide buttons */
          hWhitePlayerPanel->EnableJoinButton(false);
          hWhitePlayerPanel->EnableLeaveButton(false);
          hWhitePlayerPanel->EnableReadyButton(false);
          hBlackPlayerPanel->EnableJoinButton(false);
          hBlackPlayerPanel->EnableLeaveButton(false);
          hBlackPlayerPanel->EnableReadyButton(false);

          /* Start the game */
          Game->Reset();
          Game->SetMode(FreeTime,0);
          Game->StartGame();
          break;
        }
        case GameDrawed:
        {
          /* Draw the game */
          Game->DrawGame();
          break;
        }
        case GamePaused:
        {
          /* Pause the game */
          Game->PauseGame();
          break;
        }
        case GameResumed:
        {
          /* Resume the game */
          Game->ResumeGame();
          break;
        }
        case Resigned:
        {
          /* Resign the game */
          Game->ResignGame();
          break;
        }
        case TookbackMove:
        {
          /* Take back the last move */
          Game->TakeBackMove();

          /* Add a message in the chat panel */
          hChatPanel->AddLine(Game->GetPlayer(!Game->GetActivePlayer())->Name + " has took back his last move.");
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
        hChatPanel->AddLine(Player->Name + " has joined the room.");
      }
      break;
    }
    case PlayerLeft:
    {
      char* PlayerName = (char*)lParam;

      /* Change the game state */
      if (Game->GetState() == Started && (NetworkClient->GetWhitePlayer() == NULL || NetworkClient->GetBlackPlayer() == NULL))
        Game->PauseGame();

      /* Clear player name */
      if (NetworkClient->GetBlackPlayer() == NULL)
      {
        Game->SetPlayerName(Black, "");
        hBlackPlayerPanel->EnableJoinButton(true);
        hBlackPlayerPanel->EnableLeaveButton(false);
        hBlackPlayerPanel->EnableReadyButton(false);
      }
      if (NetworkClient->GetWhitePlayer() == NULL)
      {
        Game->SetPlayerName(White, "");
        hWhitePlayerPanel->EnableJoinButton(true);
        hBlackPlayerPanel->EnableLeaveButton(false);
        hWhitePlayerPanel->EnableReadyButton(false);
      }

      /* Update the room information */
      hRoomPanel->Invalidate();

      /* Add a message in the chat panel */
      hChatPanel->AddLine(string(PlayerName) + " has left the room.");
      delete[] PlayerName;
      break;
    }
    case PlayerNameChanged:
    {
      PlayerInfo* Player = (PlayerInfo*)lParam;

      /* Update the player name */
      if (Player == NetworkClient->GetBlackPlayer())
        Game->SetPlayerName(Black, Player->Name);
      else if (Player == NetworkClient->GetWhitePlayer())
        Game->SetPlayerName(White, Player->Name);

      /* Update the player list */
      hRoomPanel->Invalidate();
      break;
    }
    case PlayerStateChanged:
    {
      hBlackPlayerPanel->Invalidate();
      hWhitePlayerPanel->Invalidate();

      /* Show the player's ready button */
      if (NetworkClient->GetWhitePlayer() != NULL && NetworkClient->GetBlackPlayer() != NULL)
      {
        hBlackPlayerPanel->EnableReadyButton(NetworkClient->GetLocalPlayer() == NetworkClient->GetBlackPlayer() && !NetworkClient->GetBlackPlayer()->Ready);
        hWhitePlayerPanel->EnableReadyButton(NetworkClient->GetLocalPlayer() == NetworkClient->GetWhitePlayer() && !NetworkClient->GetWhitePlayer()->Ready);
      }
      break;
    }
    case PlayerTimeChanged:
    {
      PlayerInfo* Player = (PlayerInfo*)lParam;
      if (Player == NetworkClient->GetBlackPlayer())
        Game->SetPlayerTime(Black,Player->Time);
      else if (Player == NetworkClient->GetWhitePlayer())
        Game->SetPlayerTime(White,Player->Time);
      break;
    }
    case PlayerTypeChanged:
    {
      PlayerType OldType = (PlayerType)HIWORD(wParam);
      PlayerInfo* Player = (PlayerInfo*)lParam;

      /* Clear player name */
      if (OldType == BlackPlayerType)
      {
        Game->SetPlayerName(Black,"");
        hBlackPlayerPanel->EnableJoinButton(true);
        hBlackPlayerPanel->EnableLeaveButton(false);
        hBlackPlayerPanel->EnableReadyButton(false);
      }
      else if (OldType == WhitePlayerType)
      {
        Game->SetPlayerName(White,"");
        hWhitePlayerPanel->EnableJoinButton(true);
        hWhitePlayerPanel->EnableLeaveButton(false);
        hWhitePlayerPanel->EnableReadyButton(false);
      }

      hBlackPlayerPanel->Invalidate();
      hWhitePlayerPanel->Invalidate();

      /* Update the player name */
      if (Player == NetworkClient->GetBlackPlayer())
      {
        Game->SetPlayerName(Black,Player->Name);
        hBlackPlayerPanel->EnableJoinButton(false);
        hBlackPlayerPanel->EnableLeaveButton(Player == NetworkClient->GetLocalPlayer());
      }
      else if (Player == NetworkClient->GetWhitePlayer())
      {
        Game->SetPlayerName(White,Player->Name);
        hWhitePlayerPanel->EnableJoinButton(false);
        hWhitePlayerPanel->EnableLeaveButton(Player == NetworkClient->GetLocalPlayer());
      }

      /* Update the player list */
      hRoomPanel->Invalidate();
      break;
    }
    case PromotionReceived:
    {
      /* Promote the last moved piece */
      Game->PromotePawnTo((ChessPieceType)lParam);
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
            hChatPanel->AddLine("Your opponent is offering you to draw this game.");
            int Result = MessageBox(Handle, "Your opponent wants to draw this game. Do you accept?", "Draw game offer", MB_YESNO|MB_ICONQUESTION);
            NetworkClient->AcceptDrawRequest(Result == IDYES);
            break;
          }
          case TakebackRequest:
          {
            /* Add a message in the chat panel */
            hChatPanel->AddLine("Your opponent has requested to take back his last move.");
            int Result = MessageBox(Handle, "Your opponent wants to take back his last move. Do you accept?", "Takeback last move request", MB_YESNO|MB_ICONQUESTION);
            NetworkClient->AcceptTakebackRequest(Result == IDYES);
            break;
          }
        }
      break;
    }
    case RoomInfoReceived:
    {
      const RoomInfo* Room = (RoomInfo*)lParam;
      HWND Dialog = GetNetworkDialogHandle();
      if (Dialog != NULL)
      {
        /* The server sent information on a room */
        NetworkGameRoomInfo* RoomInfo = new NetworkGameRoomInfo;
        RoomInfo->RoomId = Room->RoomId;
        RoomInfo->Name = Room->Name;
        RoomInfo->Locked = Room->Locked;
        RoomInfo->PlayerCount = Room->PlayerCount;
        SendMessage(Dialog, WM_ADDROOMTOLIST, (WPARAM)RoomInfo, 0);
        /* Clean up */
        delete RoomInfo;
      }
      /* Clean up */
      delete Room;
      break;
    }
  }
}

void AlphaChess::UpdatePos(int NewLeft, int NewTop)
{
  UpdateState();

  /* Update the position of the window */
  if (!Minimized && !Maximized)
  {
    Left = NewLeft;
    Top = NewTop;
  }
}

void AlphaChess::UpdateSize(int NewWidth, int NewHeight)
{
  UpdateState();

  /* Update the inner size of the window */
  InnerWidth = NewWidth;
  InnerHeight = NewHeight;

  /* Update the size of the window */
  if (!Minimized && !Maximized)
  {
    RECT R;
    GetWindowRect(Handle, &R);
    Width = R.right-R.left;
    Height = R.bottom-R.top;
  }

  /* Resize the chess board */
  if (hChessBoardPanel != NULL && hChessBoardPanel->GetHandle() != NULL)
  {
    RECT R;
    CalculateChessBoardPosition(&R);
    SetWindowPos(hChessBoardPanel->GetHandle(),NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER);
  }

  /* Resize the white player panel */
  if (PlayersInfoVisible && hWhitePlayerPanel != NULL && hWhitePlayerPanel->GetHandle() != NULL)
  {
    RECT R;
    CalculateWhitePlayerPosition(&R);
    SetWindowPos(hWhitePlayerPanel->GetHandle(),NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER);
  }

  /* Resize the black player panel */
  if (PlayersInfoVisible && hBlackPlayerPanel != NULL && hBlackPlayerPanel->GetHandle() != NULL)
  {
    RECT R;
    CalculateBlackPlayerPosition(&R);
    SetWindowPos(hBlackPlayerPanel->GetHandle(),NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER);
  }

  /* Resize the captured pieces panel */
  if (CaptureListVisible && hCapturePanel != NULL && hCapturePanel->GetHandle() != NULL)
  {
    /* Auto-hide panel */
    RECT R;
    CaptureListAutoHide = false;
    CalculateMoveListPosition(&R);
    if (R.bottom-R.top < MoveListMinimumHeight)
    {
      CaptureListAutoHide = true;
      ShowWindow(hCapturePanel->GetHandle(), SW_HIDE);
    }
    else
      ShowWindow(hCapturePanel->GetHandle(), SW_SHOW);

    if (!CaptureListAutoHide)
    {
      CalculateCaptureListPosition(&R);
      SetWindowPos(hCapturePanel->GetHandle(),NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER);
    }
  }

  /* Resize the move list panel */
  if (MoveListVisible && hHistoryPanel != NULL && hHistoryPanel->GetHandle() != NULL)
  {
    RECT R;
    CalculateMoveListPosition(&R);
    SetWindowPos(hHistoryPanel->GetHandle(),NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER);
  }

  /* Resize the room panel */
  if (NetworkPanelsVisible && hRoomPanel != NULL && hRoomPanel->GetHandle() != NULL)
  {
    RECT R;
    CalculateRoomPosition(&R);
    SetWindowPos(hRoomPanel->GetHandle(),NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER);
  }

  /* Resize the chat panel */
  if (NetworkPanelsVisible && hChatPanel != NULL && hChatPanel->GetHandle() != NULL)
  {
    RECT R;
    CalculateChatPosition(&R);
    SetWindowPos(hChatPanel->GetHandle(),NULL,R.left,R.top,R.right-R.left,R.bottom-R.top,SWP_NOZORDER);
  }
}

void AlphaChess::UpdateState()
{
  WINDOWPLACEMENT Placement;
  if (GetWindowPlacement(Handle, &Placement))
  {
    if (Placement.showCmd == SW_SHOWMAXIMIZED)
    {
      Maximized = true;
      Minimized = false;
    }
    else if (Placement.showCmd == SW_SHOWMINIMIZED)
    {
      Maximized = false;
      Minimized = true;
    }
    else if (Placement.showCmd == SW_SHOWNORMAL)
    {
      Maximized = false;
      Minimized = false;
    }
  }
}

void AlphaChess::UpdateTime(const unsigned long TickCount)
{
  Game->UpdateTime(TickCount >= LastTickCount ? TickCount-LastTickCount : (UINT_MAX-LastTickCount)+TickCount);
  LastTickCount = TickCount;
  if (NetworkGame)
    NetworkClient->SendTime(Game->GetPlayer(Game->GetActivePlayer())->MoveTime);
}

// PRIVATE WINAPI FUNCTIONS ----------------------------------------------------

LRESULT __stdcall AlphaChess::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
    case WM_APPLYPREFERENCES:
    {
      PreferencesDialogValues* Values = (PreferencesDialogValues*)wParam;

      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Window != NULL)
      {
        /* Save dialog values */
        Window->AutomaticUpdate = Values->AutomaticUpdate;
        Window->AlwaysOnTop = Values->AlwaysOnTop;
        Window->AlwaysVisible = Values->AlwaysVisible;
        Window->AutomaticallySwitchView = Values->AutomaticallySwitchView;
        Window->AlwaysPromoteToQueen = Values->AlwaysPromoteToQueen;
        Window->PlayersInfoVisible = Values->PlayersInfoVisible;
        Window->CaptureListVisible = Values->CaptureListVisible;
        Window->MoveListVisible = Values->MoveListVisible;
        Window->BoardCoordinatesVisible = Values->BoardCoordinatesVisible;
        Window->BoardLastMoveVisible = Values->BoardLastMoveVisible;
        Window->BoardInvalidMovesVisible = Values->BoardInvalidMovesVisible;
        Window->CurrentChessSet = Values->CurrentChessSet;
        Window->CurrentTheme = Values->CurrentTheme;
        Window->ShowMoveListIcons = Values->ShowMoveListIcons;

        /* Update display */
        Window->ApplyPreferences();
        Window->SaveSettings();
      }
      return 0;
    }
    case WM_CHESSPIECEMOVED:
    {
      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
      Position* From = (Position*)wParam;
      Position* To = (Position*)lParam;

      if (Window != NULL && Window->Game->IsLastMoveDisplayed())
      {
        /* Stop the timer for the current player */
        KillTimer(hWnd, ChessTimerID);
        Window->UpdateTime(GetTickCount());

        /* Make the move */
        if (!Window->NetworkGame)
          return Window->Game->MakeMove(*From,*To);
        else if (Window->Game->IsMoveValid(*From,*To))
          Window->NetworkClient->MakeMove(From->x,From->y,To->x,To->y);
      }
      return 0;
    }
    case WM_CLOSE:
    {
      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Window != NULL)
      {
        /* Close confirmation */
        if (Window->Game->GetState() == Started || Window->Game->GetState() == Paused)
        {
          Window->Game->PauseGame();
          int Result = MessageBox(hWnd,"You have a game in progress, do you want to save it before exiting AlphaChess?","Exit AlphaChess",MB_YESNOCANCEL|MB_ICONQUESTION);

          /* Save game */
          if (Result == IDYES)
            Window->SaveGame();
          else if (Result == IDCANCEL)
          {
            Window->Game->ResumeGame();
            return 0;
          }
        }
      }
      DestroyWindow(hWnd);
      return 0;
    }
    case WM_COMMAND:
    {
      /* Process the menu and accelerator messages */
      if (lParam == 0)
      {
        AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
        if (Window != NULL)
        {
          switch (LOWORD(wParam))
          {
            case IDS_MAINMENU_GAME_NEW_LOCAL:
            {
              /* Show confirmation */
              if (Window->Game->GetState() == Started || Window->Game->GetState() == Paused)
              {
                Window->Game->PauseGame();
                int Result = MessageBox(hWnd,"You have a game in progress, do you want to save it?","Play a new local game",MB_YESNOCANCEL|MB_ICONQUESTION);
                if (Result == IDYES)
                  Window->SaveGame();
                else if (Result == IDCANCEL)
                {
                  Window->Game->ResumeGame();
                  break;
                }
              }

              /* Initialize dialog values */
              LocalGameDialogValues* Values = new LocalGameDialogValues;
              Values->BlackPlayerName = Window->BlackPlayerName;
              Values->WhitePlayerName = Window->WhitePlayerName;
              Values->WhitePlayerIsComputer = Window->WhitePlayerIsComputer;
              Values->BlackPlayerIsComputer = Window->BlackPlayerIsComputer;
              Values->GameMode = (int)Window->GameMode;
              Values->TimePerMove = Window->TimePerMove;

              /* Show dialog */
              if (ShowLocalGameDialog((HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), hWnd, Values))
              {
                /* Save dialog values */
                Window->BlackPlayerName = Values->BlackPlayerName;
                Window->WhitePlayerName = Values->WhitePlayerName;
                Window->WhitePlayerIsComputer = Values->WhitePlayerIsComputer;
                Window->BlackPlayerIsComputer = Values->BlackPlayerIsComputer;
                Window->GameMode = (ChessGameMode)Values->GameMode;
                Window->TimePerMove = Values->TimePerMove;

                /* Disconnect from server */
                if (Window->NetworkClient->IsConnected())
                  Window->NetworkClient->Disconnect();

                /* Start a new game */
                Window->Game->Reset();
                Window->Game->SetPlayerName(Black, Window->BlackPlayerName);
                Window->Game->SetPlayerName(White, Window->WhitePlayerName);
                Window->Game->SetMode(Window->GameMode, Window->TimePerMove);
                Window->Game->StartGame();
              }
              delete Values;
              break;
            }
            case IDS_MAINMENU_GAME_NEW_NETWORK:
            {
              /* Show confirmation */
              if (Window->Game->GetState() == Started || Window->Game->GetState() == Paused)
              {
                Window->Game->PauseGame();
                int Result = MessageBox(hWnd,"You have a game in progress, do you want to save it?","Play a new online game",MB_YESNOCANCEL|MB_ICONQUESTION);
                if (Result == IDYES)
                  Window->SaveGame();
                else if (Result == IDCANCEL)
                {
                  Window->Game->ResumeGame();
                  break;
                }
              }

              if (Window->NetworkClient->IsConnected())
                Window->UpdateInterface(MAKEWPARAM(ConnectionSucceeded,0), 0);
              else
              {
                Window->NetworkClient->Connect(MainServerAddress);
                Window->ChangeCursor(LoadCursor(NULL,IDC_WAIT));
              }
              break;
            }
            case IDS_MAINMENU_GAME_LOADGAME:
            {
              if (Window->Game->GetState() == Started)
                Window->Game->PauseGame();
              char* Filename = ShowSavedGamesDialog((HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), hWnd, GamesDirectory);
              if (Filename != NULL)
                Window->Game->LoadFromFile(Filename);
              delete[] Filename;
              break;
            }
            case IDS_MAINMENU_GAME_SAVEGAME:
            {
              if (Window->Game->GetState() == Started)
              {
                Window->Game->PauseGame();
                Window->SaveGame();
                Window->Game->ResumeGame();
              }
              else
                Window->SaveGame();
              break;
            }
            case IDS_MAINMENU_GAME_PAUSE:
            {
              if (!Window->NetworkGame)
              {
                if (!IsMenuItemChecked(Window->GameMenu,IDS_MAINMENU_GAME_PAUSE))
                  Window->Game->PauseGame();
                else
                  Window->Game->ResumeGame();
              }
              else if ((Window->Game->GetActivePlayer() == White && Window->NetworkClient->GetLocalPlayer() == Window->NetworkClient->GetWhitePlayer()) || (Window->Game->GetActivePlayer() == Black && Window->NetworkClient->GetLocalPlayer() == Window->NetworkClient->GetBlackPlayer()))
              {
                if (!IsMenuItemChecked(Window->GameMenu,IDS_MAINMENU_GAME_PAUSE))
                  Window->NetworkClient->PauseGame();
                else
                  Window->NetworkClient->ResumeGame();
              }
              else
                MessageBox(hWnd,"You can only pause the game during your turn!","Pause game",MB_OK|MB_ICONEXCLAMATION);
              break;
            }
            case IDS_MAINMENU_GAME_TAKEBACKMOVE:
            {
              if (Window->NetworkGame)
              {
                if ((Window->Game->GetActivePlayer() == White && Window->NetworkClient->GetLocalPlayer() == Window->NetworkClient->GetBlackPlayer()) || (Window->Game->GetActivePlayer() == Black && Window->NetworkClient->GetLocalPlayer() == Window->NetworkClient->GetWhitePlayer()))
                  Window->NetworkClient->RequestTakeback();
                else
                  MessageBox(hWnd,"You cannot take back your opponent's move!","Take back last move",MB_OK|MB_ICONEXCLAMATION);
              }
              else
                Window->Game->TakeBackMove();
              break;
            }
            case IDS_MAINMENU_GAME_OFFERDRAW:
            {
              if (Window->NetworkGame)
              {
                if ((Window->Game->GetActivePlayer() == White && Window->NetworkClient->GetLocalPlayer() == Window->NetworkClient->GetWhitePlayer()) || (Window->Game->GetActivePlayer() == Black && Window->NetworkClient->GetLocalPlayer() == Window->NetworkClient->GetBlackPlayer()))
                  Window->NetworkClient->RequestDrawGame();
                else
                  MessageBox(hWnd,"You can only offer to draw the game during your turn!","Offer draw",MB_OK|MB_ICONEXCLAMATION);
              }
              else
                Window->Game->DrawGame();
              break;
            }
            case IDS_MAINMENU_GAME_RESIGN:
            {
              if (Window->NetworkGame)
              {
                if ((Window->Game->GetActivePlayer() == White && Window->NetworkClient->GetLocalPlayer() == Window->NetworkClient->GetWhitePlayer()) || (Window->Game->GetActivePlayer() == Black && Window->NetworkClient->GetLocalPlayer() == Window->NetworkClient->GetBlackPlayer()))
                  Window->NetworkClient->Resign();
                else
                  MessageBox(hWnd,"You can only resign the game during your turn!","Resign game",MB_OK|MB_ICONEXCLAMATION);
              }
              else
                Window->Game->ResignGame();
              break;
            }
            case IDS_MAINMENU_GAME_PREFERENCES:
            {
              /* Initialize dialog values */
              PreferencesDialogValues* Values = new PreferencesDialogValues;
              Values->ChessSets = &(Window->ChessSets);
              Values->ThemesDirectory = Window->ThemesDirectory;

              Values->AutomaticUpdate = Window->AutomaticUpdate;
              Values->AlwaysOnTop = Window->AlwaysOnTop;
              Values->AlwaysVisible = Window->AlwaysVisible;
              Values->AutomaticallySwitchView = Window->AutomaticallySwitchView;
              Values->AlwaysPromoteToQueen = Window->AlwaysPromoteToQueen;
              Values->PlayersInfoVisible = Window->PlayersInfoVisible;
              Values->CaptureListVisible = Window->CaptureListVisible;
              Values->MoveListVisible = Window->MoveListVisible;
              Values->BoardCoordinatesVisible = Window->BoardCoordinatesVisible;
              Values->BoardLastMoveVisible = Window->BoardLastMoveVisible;
              Values->BoardInvalidMovesVisible = Window->BoardInvalidMovesVisible;
              Values->CurrentChessSet = Window->CurrentChessSet;
              Values->CurrentTheme = Window->CurrentTheme;
              Values->ShowMoveListIcons = Window->ShowMoveListIcons;

              /* Show dialog */
              if (ShowPreferencesDialog((HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), hWnd, Values))
                SendMessage(hWnd, WM_APPLYPREFERENCES, (WPARAM)Values, 0);
              delete Values;
              break;
            }
            case IDS_MAINMENU_GAME_EXIT:
            {
              DestroyWindow(hWnd);
              break;
            }
            case IDS_MAINMENU_VIEW_PLAYERS:
            {
              Window->ShowPlayerPanels(!IsMenuItemChecked(Window->ViewMenu,IDS_MAINMENU_VIEW_PLAYERS));
              break;
            }
            case IDS_MAINMENU_VIEW_CAPTURES:
            {
              Window->ShowCapturePanel(!IsMenuItemChecked(Window->ViewMenu,IDS_MAINMENU_VIEW_CAPTURES));
              break;
            }
            case IDS_MAINMENU_VIEW_MOVES:
            {
              Window->ShowHistoryPanel(!IsMenuItemChecked(Window->ViewMenu,IDS_MAINMENU_VIEW_MOVES));
              break;
            }
            case IDS_MAINMENU_VIEW_SWITCH:
            {
              Window->RotateView(!IsMenuItemChecked(Window->ViewMenu,IDS_MAINMENU_VIEW_SWITCH));
              break;
            }
            case IDS_MAINMENU_HELP_HELP:
            {
              break;
            }
            case IDS_MAINMENU_HELP_UPDATE:
            {
              break;
            }
            case IDS_MAINMENU_HELP_ABOUT:
            {
              ShowAboutDialog((HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), hWnd);
              break;
            }
            case IDS_PLAYERMENU_JOINASWHITE:
            {
              if (Window->NetworkClient->GetWhitePlayer() == NULL)
              {
                Window->NetworkClient->SetPlayerType(WhitePlayerType);
                Window->hWhitePlayerPanel->EnableLeaveButton(true);
                Window->RotateView(false);
              }
              break;
            }
            case IDS_PLAYERMENU_JOINASBLACK:
            {
              if (Window->NetworkClient->GetBlackPlayer() == NULL)
              {
                Window->NetworkClient->SetPlayerType(BlackPlayerType);
                Window->hBlackPlayerPanel->EnableLeaveButton(true);
                Window->RotateView(true);
              }
              break;
            }
            case IDS_PLAYERMENU_LEAVE:
            {
              if (Window->NetworkClient->GetBlackPlayer() == Window->NetworkClient->GetLocalPlayer())
                Window->NetworkClient->SetPlayerType(ObserverType);
              else if (Window->NetworkClient->GetWhitePlayer() == Window->NetworkClient->GetLocalPlayer())
                Window->NetworkClient->SetPlayerType(ObserverType);
              break;
            }
            case IDS_PLAYERMENU_READY:
            {
              Window->NetworkClient->SetPlayerReady();
              if (Window->NetworkClient->GetLocalPlayer() == Window->NetworkClient->GetWhitePlayer())
                Window->hWhitePlayerPanel->SetReady(true);
              else if (Window->NetworkClient->GetLocalPlayer() == Window->NetworkClient->GetBlackPlayer())
                Window->hBlackPlayerPanel->SetReady(true);
              break;
            }
          }
        }
      }
      return 0;
    }
    case WM_CONTEXTMENU:
    {
      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Window != NULL)
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
      }
      return 0;
    }
    case WM_CREATE:
    {
      CREATESTRUCT* Params = (CREATESTRUCT*)lParam;
      AlphaChess* Window = (AlphaChess*)(Params->lpCreateParams);
      SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)Window);
      return 0;
    }
    case WM_DESTROY:
    {
      PostQuitMessage(0);
      return 0;
    }
    case WM_GETMINMAXINFO:
    {
      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Window != NULL)
      {
        MINMAXINFO* Info = (MINMAXINFO*)lParam;

        /* Update window minimum tracking size info */
        RECT R = {0,0,MinBoardSize+(Window->PlayersInfoVisible || (Window->CaptureListVisible && !Window->CaptureListAutoHide) || Window->MoveListVisible ? Window->LeftPanelWidth : 0),MinBoardSize+(Window->NetworkPanelsVisible ? Window->BottomPanelHeight : 0)};
        AdjustWindowRectEx(&R,GetWindowLong(hWnd,GWL_STYLE),TRUE,GetWindowLong(hWnd,GWL_EXSTYLE));
        Info->ptMinTrackSize.x = R.right-R.left;
        Info->ptMinTrackSize.y = R.bottom-R.top;

        /* Update the window maximised size info */
        MONITORINFO Monitor;
        Monitor.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(MonitorFromWindow(hWnd,MONITOR_DEFAULTTONEAREST), &Monitor);
        InflateRect(&(Monitor.rcWork), GetSystemMetrics(SM_CXSIZEFRAME), GetSystemMetrics(SM_CYSIZEFRAME));
        Info->ptMaxSize.x = Monitor.rcWork.bottom-Monitor.rcWork.top-GetSystemMetrics(SM_CYSIZEFRAME)*2-GetSystemMetrics(SM_CYCAPTION)-GetSystemMetrics(SM_CYMENU)-(Window->NetworkPanelsVisible ? Window->BottomPanelHeight : 0)+GetSystemMetrics(SM_CXSIZEFRAME)*2+(Window->PlayersInfoVisible || (Window->CaptureListVisible && !Window->CaptureListAutoHide) || Window->MoveListVisible ? Window->LeftPanelWidth : 0);
        Info->ptMaxSize.y = Monitor.rcWork.bottom-Monitor.rcWork.top;
      }
      return 0;
    }
    case WM_HISTORYBUTTONCLICKED:
    {
      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Window != NULL)
      {
        switch (wParam)
        {
          case 1:
          {
            Window->Game->GotoFirstMove();
            break;
          }
          case 2:
          {
            Window->Game->GotoLastMove();
            break;
          }
          case 3:
          {
            Window->Game->GotoNextMove();
            break;
          }
          case 4:
          {
            Window->Game->GotoPreviousMove();
            break;
          }
        }
      }
      return 0;
    }
    case WM_KICKPLAYERBUTTONCLICKED:
    {
      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Window != NULL)
      {
        unsigned int PlayerId = Window->hRoomPanel->GetSelectedPlayer();
        if (PlayerId > 0 && PlayerId != Window->NetworkClient->GetLocalPlayer()->PlayerId)
          if (MessageBox(hWnd,"Are you sure that you wish to kick the selected player from game?", "Kick player", MB_YESNO|MB_ICONQUESTION) == IDYES)
            Window->NetworkClient->KickPlayer(PlayerId);
      }
      return 0;
    }
    case WM_LEAVEROOMBUTTONCLICKED:
    {
      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Window != NULL)
      {
        if (MessageBox(hWnd,"Are you sure that you wish to leave the game?", "Leave game", MB_YESNO|MB_ICONQUESTION) == IDYES)
          Window->NetworkClient->LeaveRoom();
      }
      return 0;
    }
    case WM_MOUSEACTIVATE:
    {
      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Window != NULL)
      {
        Window->ChangeCursor(Window->Cursor);
      }
      return MA_ACTIVATE;
    }
    case WM_MOUSEWHEEL:
    {
      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Window != NULL)
      {
        /* Redirect to history window */
        if (Window->hHistoryPanel != NULL)
          SendMessage(Window->hHistoryPanel->GetHandle(), Msg, wParam, lParam);
      }
      return 0;
    }
    case WM_MOVE:
    {
      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Window != NULL)
      {
        RECT R;
        GetWindowRect(hWnd, &R);
        if (R.left != Window->Left || R.top != Window->Top)
            Window->UpdatePos(R.left, R.top);
      }
      return 0;
    }
    case WM_MOVING:
    {
      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Window != NULL)
      {
        RECT* R = (RECT*)lParam;
        WINDOWPLACEMENT wd;
        wd.length = sizeof(WINDOWPLACEMENT);
        if (GetWindowPlacement(hWnd, &wd) && (Window->AlwaysVisible || wd.showCmd == SW_SHOWMAXIMIZED))
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
      }
      return 1;
    }
    case WM_SENDMESSAGEBUTTONCLICKED:
    {
      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Window != NULL)
      {
        Window->NetworkClient->SendText((char*)wParam);
      }
      return 0;
    }
    case WM_SIZE:
    {
      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Window != NULL)
      {
        if (LOWORD(lParam) != Window->InnerWidth || HIWORD(lParam) != Window->InnerHeight)
            Window->UpdateSize((short)LOWORD(lParam), (short)HIWORD(lParam));
      }
      return 0;
    }
    case WM_SIZING:
    {
      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Window != NULL)
      {
        RECT* R = (RECT*)lParam;

        /* Constrain the chess board area size to 1:1 ratio */
        int FrameWidth = GetSystemMetrics(SM_CXSIZEFRAME);
        int FrameHeight = GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYSIZEFRAME)+GetSystemMetrics(SM_CYMENU);
        int Width = R->right-R->left-FrameWidth-(Window->PlayersInfoVisible || (Window->CaptureListVisible && !Window->CaptureListAutoHide) || Window->MoveListVisible ? Window->LeftPanelWidth : 0);
        int Height = R->bottom-R->top-FrameHeight-(Window->NetworkPanelsVisible ? Window->BottomPanelHeight : 0);
        if (wParam == WMSZ_BOTTOM || wParam == WMSZ_TOP)
          Width = Height;
        else if (wParam == WMSZ_LEFT || wParam == WMSZ_RIGHT)
          Height = Width;
        else if (Width != Height)
          Width = Height = min(Height, Width);
        if (wParam == WMSZ_TOPLEFT)
        {
          R->left = R->right-Width-FrameWidth-(Window->PlayersInfoVisible || (Window->CaptureListVisible && !Window->CaptureListAutoHide) || Window->MoveListVisible ? Window->LeftPanelWidth : 0);
          R->top = R->bottom-Height-FrameHeight-(Window->NetworkPanelsVisible ? Window->BottomPanelHeight : 0);
        }
        else if (wParam == WMSZ_BOTTOMLEFT)
        {
          R->left = R->right-Width-FrameWidth-(Window->PlayersInfoVisible || (Window->CaptureListVisible && !Window->CaptureListAutoHide) || Window->MoveListVisible ? Window->LeftPanelWidth : 0);
          R->bottom = R->top+Height+FrameHeight+(Window->NetworkPanelsVisible ? Window->BottomPanelHeight : 0);
        }
        else if (wParam == WMSZ_TOPRIGHT)
        {
          R->right = R->left+Width+FrameWidth+(Window->PlayersInfoVisible || (Window->CaptureListVisible && !Window->CaptureListAutoHide) || Window->MoveListVisible ? Window->LeftPanelWidth : 0);
          R->top = R->bottom-Height-FrameHeight-(Window->NetworkPanelsVisible ? Window->BottomPanelHeight : 0);
        }
        else
        {
          R->right = R->left+Width+FrameWidth+(Window->PlayersInfoVisible || (Window->CaptureListVisible && !Window->CaptureListAutoHide) || Window->MoveListVisible ? Window->LeftPanelWidth : 0);
          R->bottom = R->top+Height+FrameHeight+(Window->NetworkPanelsVisible ? Window->BottomPanelHeight : 0);
        }
      }
      return 1;
    }
    case WM_TIMER:
    {
      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Window != NULL)
      {
        if (wParam == ChessTimerID)
          Window->UpdateTime(GetTickCount());
      }
      DrawMenuBar(hWnd);
      return 0;
    }
    case WM_UPDATEROOMLIST:
    {
      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Window != NULL)
        Window->NetworkClient->UpdateRoomList();
      return 0;
    }
    case WM_UPDATEINTERFACE:
    {
      AlphaChess* Window = (AlphaChess*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Window != NULL)
      {
        Window->UpdateInterface(wParam,lParam);
      }
      return 0;
    }
  }
  return DefWindowProc(hWnd,Msg,wParam,lParam);
}
